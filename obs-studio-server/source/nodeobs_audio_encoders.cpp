/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include <algorithm>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "nodeobs_audio_encoders.h"

static const std::string encoders[] = {
	"ffmpeg_aac", "ffmpeg_opus", "mf_aac", "libfdk_aac", "CoreAudio_AAC",
};
static const std::string aac_ = "aac";
static const std::string opus_ = "opus";

static const std::string &fallbackEncoder = encoders[0];

static const char *NullToEmpty(const char *str)
{
	return str ? str : "";
}

static const char *EncoderName(const char *id)
{
	return NullToEmpty(obs_encoder_get_display_name(id));
}

static std::map<int, const char *> bitrateMap;
static std::string channelSetup;

static void HandleIntProperty(obs_property_t *prop, const char *id)
{
	const int max_ = obs_property_int_max(prop);
	const int step = obs_property_int_step(prop);

	for (int i = obs_property_int_min(prop); i <= max_; i += step)
		bitrateMap[i] = id;
}

static void HandleListProperty(obs_property_t *prop, const char *id)
{
	obs_combo_format format = obs_property_list_format(prop);
	if (format != OBS_COMBO_FORMAT_INT) {
		blog(LOG_ERROR,
		     "Encoder '%s' (%s) returned bitrate "
		     "OBS_PROPERTY_LIST property of unhandled "
		     "format %d",
		     EncoderName(id), id, static_cast<int>(format));
		return;
	}

	const size_t count = obs_property_list_item_count(prop);
	for (size_t i = 0; i < count; i++) {
		if (obs_property_list_item_disabled(prop, i))
			continue;

		int bitrate = static_cast<int>(obs_property_list_item_int(prop, i));
		bitrateMap[bitrate] = id;
	}
}

static void HandleSampleRate(obs_property_t *prop, const char *id)
{
	auto ReleaseData = [](obs_data_t *data) { obs_data_release(data); };
	std::unique_ptr<obs_data_t, decltype(ReleaseData)> data{obs_encoder_defaults(id), ReleaseData};

	if (!data) {
		blog(LOG_ERROR,
		     "Failed to get defaults for encoder '%s' (%s) "
		     "while populating bitrate map",
		     EncoderName(id), id);
		return;
	}

	uint64_t sampleRate = config_get_uint(ConfigManager::getInstance().getBasic(), "Audio", "SampleRate");

	obs_data_set_int(data.get(), "samplerate", sampleRate);

	obs_property_modified(prop, data.get());
}

static void HandleEncoderProperties(const char *id)
{
	auto DestroyProperties = [](obs_properties_t *props) { obs_properties_destroy(props); };
	std::unique_ptr<obs_properties_t, decltype(DestroyProperties)> props{obs_get_encoder_properties(id), DestroyProperties};

	if (!props) {
		blog(LOG_ERROR,
		     "Failed to get properties for encoder "
		     "'%s' (%s)",
		     EncoderName(id), id);
		return;
	}

	obs_property_t *samplerate = obs_properties_get(props.get(), "samplerate");
	if (samplerate)
		HandleSampleRate(samplerate, id);

	obs_property_t *bitrate = obs_properties_get(props.get(), "bitrate");

	obs_property_type type = obs_property_get_type(bitrate);
	switch (type) {
	case OBS_PROPERTY_INT:
		return HandleIntProperty(bitrate, id);

	case OBS_PROPERTY_LIST:
		return HandleListProperty(bitrate, id);

	default:
		break;
	}

	blog(LOG_ERROR,
	     "Encoder '%s' (%s) returned bitrate property "
	     "of unhandled type %d",
	     EncoderName(id), id, static_cast<int>(type));
}

static const char *GetCodec(const char *id)
{
	return NullToEmpty(obs_get_encoder_codec(id));
}

bool IsSurround(const char *channelSetup)
{
	static const char *surroundLayouts[] = {"2.1", "4.0", "4.1", "5.1", "7.1", nullptr};

	if (!channelSetup || !*channelSetup)
		return false;

	const char **curLayout = surroundLayouts;
	for (; *curLayout; ++curLayout) {
		if (strcmp(*curLayout, channelSetup) == 0) {
			return true;
		}
	}

	return false;
}

void LimitBitrate(std::map<int, const char *> &values, int maximumBitrate)
{
	auto condition = [=](const std::pair<int, const char *> &p) { return p.first > maximumBitrate; };

	for (auto i = values.begin(); (i = std::find_if(i, values.end(), condition)) != values.end(); values.erase(i++))
		;
}

static void PopulateBitrateMap(const std::string codecType)
{
	bitrateMap.clear();

	HandleEncoderProperties(fallbackEncoder.c_str());

	const char *id = nullptr;
	for (size_t i = 0; obs_enum_encoder_types(i, &id); i++) {
		auto Compare = [=](const std::string &val) { return val.compare(NullToEmpty(id)) == 0; };

		if (find_if(begin(encoders), end(encoders), Compare) != end(encoders))
			continue;

		if (codecType != GetCodec(NullToEmpty(id)))
			continue;

		// A corrupted encoder dll will fail when requesting it's properties here by
		// calling "obs_get_encoder_properties", this try-catch block will make sure
		// that this encoder won't be used. A good solution would be checking this
		// when starting obs (checking if every encoder/module is valid) and/or
		// showing the user why the encoder why disabled (invalid version, need to
		// update, etc)
		try {
			HandleEncoderProperties(id);
		} catch (...) {
			continue;
		}
	}

	for (auto &encoder : encoders) {
		if (encoder == fallbackEncoder)
			continue;

		if (codecType != GetCodec(encoder.c_str()))
			continue;

		HandleEncoderProperties(encoder.c_str());
	}

	if (bitrateMap.empty()) {
		blog(LOG_ERROR, "Could not enumerate any AAC encoder "
				"bitrates");
		return;
	}

	// Limit the bitrate to 320 if not surround
	if (!IsSurround(channelSetup.c_str())) {
		LimitBitrate(bitrateMap, 320);
	}

#ifdef EXTENDED_DEBUG_LOG
	std::ostringstream ss;
	for (auto &entry : bitrateMap)
		ss << "\n	" << std::setw(3) << entry.first << " kbit/s: '" << EncoderName(entry.second) << "' (" << entry.second << ')';

	blog(LOG_INFO, "%s encoder bitrate mapping:%s", codecType.c_str(), ss.str().c_str());
#endif
}

const std::map<int, const char *> &GetAACEncoderBitrateMap()
{
	PopulateBitrateMap(aac_);
	return bitrateMap;
}

const std::map<int, const char *> &GetOpusEncoderBitrateMap()
{
	PopulateBitrateMap(opus_);
	return bitrateMap;
}

const char *GetAACEncoderForBitrate(int bitrate)
{
	auto &map_ = GetAACEncoderBitrateMap();
	auto res = map_.find(bitrate);
	if (res == end(map_))
		return NULL;
	return res->second;
}

const char *GetOpusEncoderForBitrate(int bitrate)
{
	auto &map_ = GetOpusEncoderBitrateMap();
	auto res = map_.find(bitrate);
	if (res == end(map_))
		return NULL;
	return res->second;
}

#define INVALID_BITRATE 10000

bool IsMultitrackAudioSupported(const char *format)
{
	if (format == nullptr || strcmp(format, "flv") == 0) {
		return false;
	}

	return true;
}

int FindClosestAvailableAACBitrate(int bitrate)
{
	auto &map_ = GetAACEncoderBitrateMap();
	int prev = 0;
	int next = INVALID_BITRATE;

	for (auto val : map_) {
		if (next > val.first) {
			if (val.first == bitrate)
				return bitrate;

			if (val.first < next && val.first > bitrate)
				next = val.first;
			if (val.first > prev && val.first < bitrate)
				prev = val.first;
		}
	}

	if (next != INVALID_BITRATE)
		return next;
	if (prev != 0)
		return prev;
	return 192;
}
