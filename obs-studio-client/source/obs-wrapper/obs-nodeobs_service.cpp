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

#include "obs-nodeobs_service.h"
#ifdef WIN32
#include <ShlObj.h>
#include <windows.h>
#include <filesystem>
#endif
#include "obs-shared.hpp"
#include "obs-utility.hpp"

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/stat.h>
#endif

obs_encoder_t* audioSimpleStreamingEncoder   = nullptr;
obs_encoder_t* audioSimpleRecordingEncoder   = nullptr;
obs_encoder_t* audioAdvancedStreamingEncoder = nullptr;
obs_encoder_t* videoStreamingEncoder         = nullptr;
obs_encoder_t* videoRecordingEncoder         = nullptr;
obs_service_t* service                       = nullptr;
obs_encoder_t* streamArchiveEncVod           = nullptr;

obs_encoder_t* aacTracks[MAX_AUDIO_MIXES];
std::string    aacEncodersID[MAX_AUDIO_MIXES];

std::string aacSimpleRecEncID;
std::string aacStreamEncID;

std::string videoEncoder;
std::string videoQuality;

std::mutex             signalMutex;
std::queue<SignalInfo> outputSignal;
std::thread            releaseWorker;

static constexpr int kSoundtrackArchiveEncoderIdx = 1;
static constexpr int kSoundtrackArchiveTrackIdx = 5;
static obs_encoder_t *streamArchiveEncST = nullptr;
static bool twitchSoundtrackEnabled = false;

obs_output_t* streamingOutput    = nullptr;
obs_output_t* recordingOutput    = nullptr;
obs_output_t* replayBufferOutput = nullptr;
obs_output_t* virtualWebcamOutput = nullptr;

bool        usingRecordingPreset = true;
bool        recordingConfigured  = false;
bool        ffmpegOutput         = false;
bool        lowCPUx264           = false;
bool        isStreaming          = false;
bool        isRecording          = false;
bool        isReplayBufferActive = false;
bool        rpUsesRec            = false;
bool        rpUsesStream         = false;

void* g_jsThread = nullptr;
signal_callback_t g_ouput_callback;

ServiceWaitData* streamingWaitData = nullptr;

OBS_service::OBS_service() {}
OBS_service::~OBS_service() {}

void OBS_service::OBS_service_resetAudioContext()
{
	if (!resetAudioContext(true))
		blog(LOG_ERROR, "Failed OBS_service_resetAudioContext.");
}

int OBS_service::OBS_service_resetVideoContext()
{
	return resetVideoContext(true);
}

void OBS_service::OBS_service_startStreaming(callbackService callJS)
{
	if (isStreamingOutputActive())
		return;

	if (!startStreaming(callJS))
		blog(LOG_ERROR, "Failed to start streaming!");
}

void OBS_service::OBS_service_startRecording(callbackService callJS)
{
	std::cout << "OBS_service_startRecording - 0" << std::endl;
	if (isRecordingOutputActive())
		return;

	std::cout << "OBS_service_startRecording - 1" << std::endl;
	if (!startRecording(callJS))
		blog(LOG_ERROR, "Failed to start recording!");
	std::cout << "OBS_service_startRecording - 2" << std::endl;
}

void OBS_service::OBS_service_startReplayBuffer(callbackService callJS)
{
	if (isReplayBufferOutputActive())
		return;

	if (!startReplayBuffer(callJS))
		blog(LOG_ERROR,  "Failed to start the replay buffer!");
}

void OBS_service::OBS_service_stopStreaming(bool forceStop, callbackService callJS)
{
	stopStreaming(forceStop, callJS);
}

void OBS_service::OBS_service_stopRecording()
{
	stopRecording();
}

void OBS_service::OBS_service_stopReplayBuffer(bool forceStop)
{
	stopReplayBuffer(forceStop);
	rpUsesRec    = false;
	rpUsesStream = false;
}

bool OBS_service::resetAudioContext(bool reload)
{
	struct obs_audio_info ai;

	if (reload)
		ConfigManager::getInstance().reloadConfig();

	ai.samples_per_sec          = config_get_uint(ConfigManager::getInstance().getBasic(), "Audio", "SampleRate");
	const char* channelSetupStr = config_get_string(ConfigManager::getInstance().getBasic(), "Audio", "ChannelSetup");

	if (strcmp(channelSetupStr, "Mono") == 0)
		ai.speakers = SPEAKERS_MONO;
	else if (strcmp(channelSetupStr, "2.1") == 0)
		ai.speakers = SPEAKERS_2POINT1;
	else if (strcmp(channelSetupStr, "4.0") == 0)
		ai.speakers = SPEAKERS_4POINT0;
	else if (strcmp(channelSetupStr, "4.1") == 0)
		ai.speakers = SPEAKERS_4POINT1;
	else if (strcmp(channelSetupStr, "5.1") == 0)
		ai.speakers = SPEAKERS_5POINT1;
	else if (strcmp(channelSetupStr, "7.1") == 0)
		ai.speakers = SPEAKERS_7POINT1;
	else
		ai.speakers = SPEAKERS_STEREO;

	return obs_reset_audio(&ai);
}

static inline enum video_format GetVideoFormatFromName(const char* name)
{
	if (name != NULL) {
		if (astrcmpi(name, "I420") == 0)
			return VIDEO_FORMAT_I420;
		else if (astrcmpi(name, "NV12") == 0)
			return VIDEO_FORMAT_NV12;
		else if (astrcmpi(name, "I444") == 0)
			return VIDEO_FORMAT_I444;
#if 0 //currently unsupported
        else if (astrcmpi(name, "YVYU") == 0)
            return VIDEO_FORMAT_YVYU;
        else if (astrcmpi(name, "YUY2") == 0)
            return VIDEO_FORMAT_YUY2;
        else if (astrcmpi(name, "UYVY") == 0)
            return VIDEO_FORMAT_UYVY;
#endif
		else
			return VIDEO_FORMAT_RGBA;
	} else {
		return VIDEO_FORMAT_I420;
	}
}

static inline enum obs_scale_type GetScaleType(config_t* config)
{
	const char* scaleTypeStr = config_get_string(config, "Video", "ScaleType");

	if (scaleTypeStr != NULL) {
		if (astrcmpi(scaleTypeStr, "bilinear") == 0)
			return OBS_SCALE_BILINEAR;
		else if (astrcmpi(scaleTypeStr, "lanczos") == 0)
			return OBS_SCALE_LANCZOS;
		else
			return OBS_SCALE_BICUBIC;
	} else {
		return OBS_SCALE_BICUBIC;
	}
}

static inline const char* GetRenderModule(config_t* config)
{
	const char* renderer = config_get_string(config, "Video", "Renderer");

	const char* DL_D3D11 = "libobs-d3d11.dll";
	const char* DL_OPENGL;

#ifdef _WIN32
	DL_OPENGL = "libobs-opengl.dll";
#else
	DL_OPENGL = "libobs-opengl.so";
#endif

	if (renderer != NULL) {
		return (astrcmpi(renderer, "Direct3D 11") == 0) ? DL_D3D11 : DL_OPENGL;
	} else {
		return DL_D3D11;
	}
}

void GetFPSInteger(config_t* basicConfig, uint32_t& num, uint32_t& den)
{
	num = (uint32_t)config_get_uint(basicConfig, "Video", "FPSInt");

	if (num <= 0)
		num = 1;

	den = 1;
}

void GetFPSFraction(config_t* basicConfig, uint32_t& num, uint32_t& den)
{
	num = (uint32_t)config_get_uint(basicConfig, "Video", "FPSNum");
	if (num <= 0)
		num = 1;

	den = (uint32_t)config_get_uint(basicConfig, "Video", "FPSDen");
	if (den <= 0)
		den = 1;

	if ((num / den) <= 0) {
		num = 1;
		den = 1;
	}
}

void GetFPSNanoseconds(config_t* basicConfig, uint32_t& num, uint32_t& den)
{
	num = 1000000000;
	den = (uint32_t)config_get_uint(basicConfig, "Video", "FPSNS");
}

void GetFPSCommon(config_t* basicConfig, uint32_t& num, uint32_t& den)
{
	const char* val = config_get_string(basicConfig, "Video", "FPSCommon");
	if (val != NULL) {
		if (strcmp(val, "10") == 0) {
			num = 10;
			den = 1;
		} else if (strcmp(val, "20") == 0) {
			num = 20;
			den = 1;
		} else if (strcmp(val, "24 NTSC") == 0) {
			num = 24000;
			den = 1001;
		} else if (strcmp(val, "25") == 0) {
			num = 25;
			den = 1;
		} else if (strcmp(val, "29.97") == 0) {
			num = 30000;
			den = 1001;
		} else if (strcmp(val, "48") == 0) {
			num = 48;
			den = 1;
		} else if (strcmp(val, "59.94") == 0) {
			num = 60000;
			den = 1001;
		} else if (strcmp(val, "60") == 0) {
			num = 60;
			den = 1;
		} else {
			num = 30;
			den = 1;
		}
	} else {
		num = 30;
		den = 1;
		config_set_uint(basicConfig, "Video", "FPSType", 0);
		config_set_string(basicConfig, "Video", "FPSCommon", "30");
		config_save_safe(basicConfig, "tmp", nullptr);
	}
}

void GetConfigFPS(config_t* basicConfig, uint32_t& num, uint32_t& den)
{
	uint64_t type = config_get_uint(basicConfig, "Video", "FPSType");
	if (type == 1) //"Integer"
		GetFPSInteger(basicConfig, num, den);
	else if (type == 2) //"Fraction"
		GetFPSFraction(basicConfig, num, den);
	else if (false) //"Nanoseconds", currently not implemented
		GetFPSNanoseconds(basicConfig, num, den);
	else
		GetFPSCommon(basicConfig, num, den);
}

/* some nice default output resolution vals */
static const double vals[] = {1.0, 1.25, (1.0 / 0.75), 1.5, (1.0 / 0.6), 1.75, 2.0, 2.25, 2.5, 2.75, 3.0};

static const size_t numVals = sizeof(vals) / sizeof(double);

int OBS_service::resetVideoContext(bool reload)
{
	obs_video_info ovi;
	std::string    gslib = g_moduleDirectory;
#ifdef _WIN32
	gslib += "/libobs-d3d11.dll";
#else
	gslib += "/libobs-opengl";
#endif
	ovi.graphics_module = gslib.c_str();

	if (reload)
		ConfigManager::getInstance().reloadConfig();

	ovi.base_width  = (uint32_t)config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCX");
	ovi.base_height = (uint32_t)config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCY");

	const char* outputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");

	if (outputMode == NULL) {
		outputMode = "Simple";
	}

	ovi.output_width  = (uint32_t)config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCX");
	ovi.output_height = (uint32_t)config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCY");

	std::vector<std::pair<uint32_t, uint32_t>> resolutions = OBS_API::availableResolutions();
	uint32_t limit_cx = 1920;
	uint32_t limit_cy = 1080;

	if (ovi.base_width == 0 || ovi.base_height == 0) {
		for (int i = 0; i < resolutions.size(); i++) {
			uint32_t nbPixels = resolutions.at(i).first * resolutions.at(i).second;
			if (int(ovi.base_width * ovi.base_height) < nbPixels &&
					nbPixels <= limit_cx * limit_cy) {
				ovi.base_width  = resolutions.at(i).first;
				ovi.base_height = resolutions.at(i).second;
			}
		}
		if (ovi.base_width == 0 || ovi.base_height == 0) {
			ovi.base_width = 1920;
			ovi.base_height = 1080;
		}
	}

	config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCX", ovi.base_width);
	config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCY", ovi.base_height);

	if (ovi.output_width == 0 || ovi.output_height == 0) {
		if (ovi.base_width > 1280 && ovi.base_height > 720) {
			int idx = 0;
			do {
				double use_val = 1.0;
				if (idx < numVals) {
					use_val = vals[idx];
				} else {
					use_val = vals[numVals - 1] + double(numVals - idx + 1) / 2.0;
				}
				ovi.output_width  = uint32_t(double(ovi.base_width) / use_val);
				ovi.output_height = uint32_t(double(ovi.base_height) / use_val);
				idx++;
			} while (ovi.output_width > 1280 && ovi.output_height > 720);
		} else {
			ovi.output_width  = ovi.base_width;
			ovi.output_height = ovi.base_height;
		}

		ovi.output_width  = 1280;
		ovi.output_height = 720;
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCX", ovi.output_width);
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCY", ovi.output_height);
	}

	GetConfigFPS(ConfigManager::getInstance().getBasic(), ovi.fps_num, ovi.fps_den);

	const char* colorFormat = config_get_string(ConfigManager::getInstance().getBasic(), "Video", "ColorFormat");
	const char* colorSpace  = config_get_string(ConfigManager::getInstance().getBasic(), "Video", "ColorSpace");
	const char* colorRange  = config_get_string(ConfigManager::getInstance().getBasic(), "Video", "ColorRange");

	ovi.output_format = GetVideoFormatFromName(colorFormat);

	ovi.adapter        = 0;
	ovi.gpu_conversion = true;

	ovi.colorspace = astrcmpi(colorSpace, "601") == 0 ? VIDEO_CS_601 : VIDEO_CS_709;
	ovi.range      = astrcmpi(colorRange, "Full") == 0 ? VIDEO_RANGE_FULL : VIDEO_RANGE_PARTIAL;

	ovi.scale_type = GetScaleType(ConfigManager::getInstance().getBasic());

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
	blog(LOG_INFO, "About to reset the video context");
	try {
		return obs_reset_video(&ovi);
	} catch (const char* error) {
		blog(LOG_ERROR, error);
		return OBS_VIDEO_FAIL;
	}
}

const char* FindAudioEncoderFromCodec(const char* type)
{
	const char* alt_enc_id = nullptr;
	size_t      i          = 0;

	while (obs_enum_encoder_types(i++, &alt_enc_id)) {
		if (alt_enc_id == nullptr)
			continue;
		const char* codec = obs_get_encoder_codec(alt_enc_id);
		if (strcmp(type, codec) == 0) {
			return alt_enc_id;
		}
	}

	return nullptr;
}

bool OBS_service::createAudioEncoder(
    obs_encoder_t** audioEncoder,
    std::string&    id,
    int             bitrate,
    const char*     name,
    size_t          idx)
{
	const char* id_ = GetAACEncoderForBitrate(bitrate);
	if (!id_) {
		id.clear();
		*audioEncoder = nullptr;
		return false;
	}

	if (id == id_)
		return true;

	id            = id_;
	*audioEncoder = obs_audio_encoder_create(id_, name, nullptr, idx, nullptr);

	if (*audioEncoder) {
		return true;
	}

	return false;
}

bool OBS_service::createVideoStreamingEncoder()
{
	const char* encoder = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "StreamEncoder");

	if (encoder == NULL || !EncoderAvailable(encoder)) {
		encoder = "obs_x264";
	}

	if (videoStreamingEncoder != NULL) {
		obs_encoder_release(videoStreamingEncoder);
	}

	videoStreamingEncoder = obs_video_encoder_create(encoder, "streaming_h264", nullptr, nullptr);
	if (videoStreamingEncoder == nullptr) {
		return false;
	}

	updateVideoStreamingEncoder(true);
	return true;
}

void OBS_service::createSimpleAudioStreamingEncoder()
{
	std::string id;

	if (audioSimpleStreamingEncoder) {
		obs_encoder_release(audioSimpleStreamingEncoder);
		audioSimpleStreamingEncoder = nullptr;
	}

	if (!createAudioEncoder(&audioSimpleStreamingEncoder, id, GetSimpleAudioBitrate(), "acc", 0)) {
		throw "Failed to create audio simple recording encoder";
	}
}

static inline bool valid_string(const char* str)
{
	while (str && *str) {
		if (*(str++) != ' ')
			return true;
	}

	return false;
}
static void replace_text(struct dstr* str, size_t pos, size_t len, const char* new_text)
{
	struct dstr front = {0};
	struct dstr back  = {0};

	dstr_left(&front, str, pos);
	dstr_right(&back, str, pos + len);
	dstr_copy_dstr(str, &front);
	dstr_cat(str, new_text);
	dstr_cat_dstr(str, &back);
	dstr_free(&front);
	dstr_free(&back);
}

static void erase_ch(struct dstr* str, size_t pos)
{
	struct dstr new_str = {0};
	dstr_left(&new_str, str, pos);
	dstr_cat(&new_str, str->array + pos + 1);
	dstr_free(str);
	*str = new_str;
}

char* os_generate_formatted_filename(const char* extension, bool space, const char* format)
{
	time_t     now = time(0);
	struct tm* cur_time;
	cur_time = localtime(&now);

	const size_t       spec_count = 23;
	static const char* spec[][2]  = {
        {"%CCYY", "%Y"}, {"%YY", "%y"}, {"%MM", "%m"}, {"%DD", "%d"}, {"%hh", "%H"},
        {"%mm", "%M"},   {"%ss", "%S"}, {"%%", "%%"},

        {"%a", ""},      {"%A", ""},    {"%b", ""},    {"%B", ""},    {"%d", ""},
        {"%H", ""},      {"%I", ""},    {"%m", ""},    {"%M", ""},    {"%p", ""},
        {"%S", ""},      {"%y", ""},    {"%Y", ""},    {"%z", ""},    {"%Z", ""},
    };

	char        convert[128] = {0};
	struct dstr sf;
	struct dstr c   = {0};
	size_t      pos = 0;

	dstr_init_copy(&sf, format);

	while (pos < sf.len) {
		for (size_t i = 0; i < spec_count && !convert[0]; i++) {
			size_t len = strlen(spec[i][0]);

			const char* cmp = sf.array + pos;

			if (astrcmp_n(cmp, spec[i][0], len) == 0) {
				if (strlen(spec[i][1]))
					strftime(convert, sizeof(convert), spec[i][1], cur_time);
				else
					strftime(convert, sizeof(convert), spec[i][0], cur_time);

				dstr_copy(&c, convert);
				if (c.len && valid_string(c.array))
					replace_text(&sf, pos, len, convert);
			}
		}

		if (convert[0]) {
			pos += strlen(convert);
			convert[0] = 0;
		} else if (!convert[0] && sf.array[pos] == '%') {
			erase_ch(&sf, pos);
		} else {
			pos++;
		}
	}

	if (!space)
		dstr_replace(&sf, " ", "_");

	dstr_cat_ch(&sf, '.');
	dstr_cat(&sf, extension);
	dstr_free(&c);

	if (sf.len > 255)
		dstr_mid(&sf, &sf, 0, 255);

	return sf.array;
}

std::string GenerateSpecifiedFilename(const char* extension, bool noSpace, const char* format)
{
	char* filename = os_generate_formatted_filename(extension, !noSpace, format);
	if (filename == nullptr) {
		throw "Invalid filename";
	}

	std::string result(filename);

	bfree(filename);

	return result;
}

static void FindBestFilename(std::string& strPath, bool noSpace)
{
	int num = 2;

	if (!os_file_exists(strPath.c_str()))
		return;

	const char* ext = strrchr(strPath.c_str(), '.');
	if (!ext)
		return;

	int extStart = int(ext - strPath.c_str());
	for (;;) {
		std::string testPath = strPath;
		std::string numStr;

		numStr = noSpace ? "_" : " (";
		numStr += std::to_string(num++);
		if (!noSpace)
			numStr += ")";

		testPath.insert(extStart, numStr);

		if (!os_file_exists(testPath.c_str())) {
			strPath = testPath;
			break;
		}
	}
}

static void remove_reserved_file_characters(std::string& s)
{
	replace(s.begin(), s.end(), '/', '_');
	replace(s.begin(), s.end(), '\\', '_');
	replace(s.begin(), s.end(), '*', '_');
	replace(s.begin(), s.end(), '?', '_');
	replace(s.begin(), s.end(), '"', '_');
	replace(s.begin(), s.end(), '|', '_');
	replace(s.begin(), s.end(), ':', '_');
	replace(s.begin(), s.end(), '>', '_');
	replace(s.begin(), s.end(), '<', '_');
}

bool OBS_service::createVideoRecordingEncoder()
{
	if (videoRecordingEncoder != NULL) {
		obs_encoder_release(videoRecordingEncoder);
	}
	videoRecordingEncoder = obs_video_encoder_create("obs_x264", "simple_h264_recording", nullptr, nullptr);
	if (videoRecordingEncoder == nullptr) {
		return false;
	}

	return true;
}

bool OBS_service::createService()
{
	const char* type        = nullptr;
	obs_data_t* data        = nullptr;
	obs_data_t* settings    = nullptr;
	obs_data_t* hotkey_data = nullptr;
	struct stat buffer;

	auto CreateNewService = [&]() {
		service = obs_service_create("rtmp_common", "default_service", nullptr, nullptr);
		if (service == nullptr) {
			return false;
		}

		data     = obs_data_create();
		settings = obs_service_get_settings(service);

		obs_data_set_string(settings, "streamType", "rtmp_common");
		obs_data_set_string(settings, "service", "Twitch");
		obs_data_set_bool(settings, "show_all", 0);
		obs_data_set_string(settings, "server", "auto");
		obs_data_set_string(settings, "key", "");

		obs_data_set_string(data, "type", obs_service_get_type(service));
		obs_data_set_obj(data, "settings", settings);
	};

    bool fileExist = (os_stat(ConfigManager::getInstance().getService().c_str(), &buffer) == 0);
	if (!fileExist) {
		CreateNewService();

	} else {
		// Verify if the service.json was corrupted
		data = obs_data_create_from_json_file_safe(ConfigManager::getInstance().getService().c_str(), "bak");
		if (data == nullptr) {
			blog(LOG_WARNING, "Failed to create data from service json, using default properties!");
			CreateNewService();
		} else {
			obs_data_set_default_string(data, "type", "rtmp_common");
			type = obs_data_get_string(data, "type");

			settings    = obs_data_get_obj(data, "settings");
			hotkey_data = obs_data_get_obj(data, "hotkeys");

			// If the type is invalid it could cause a crash since internally obs uses strcmp (nullptr = undef behavior)
			if (type == nullptr || strlen(type) == 0) {
				obs_data_release(data);
				obs_data_release(hotkey_data);
				obs_data_release(settings);

				blog(LOG_WARNING, "Failed to retrieve a valid service type from the data, using default properties!");
				CreateNewService();

			// Create the service normally since the service.json info looks valid
			} else {
				service = obs_service_create(type, "default_service", settings, hotkey_data);
				if (service == nullptr) {
					obs_data_release(data);
					obs_data_release(hotkey_data);
					obs_data_release(settings);
					blog( LOG_ERROR, "Failed to create service using service info from a file!");
					return false;
				}

				obs_data_release(hotkey_data);
			}
		}
	}

	if (!obs_data_save_json_safe(data, ConfigManager::getInstance().getService().c_str(), "tmp", "bak")) {
		blog(LOG_WARNING, "Failed to save service %s", ConfigManager::getInstance().getService().c_str());
	}

	obs_data_release(settings);
	obs_data_release(data);

	return true;
}

bool OBS_service::createStreamingOutput(void)
{
	const char* type = obs_service_get_output_type(service);
	if (!type)
		type = "rtmp_output";

	streamingOutput = obs_output_create(type, "simple_stream", nullptr, nullptr);
	if (streamingOutput == nullptr) {
		return false;
	}

	connectOutputSignals();

	return true;
}

bool OBS_service::createRecordingOutput(void)
{
	recordingOutput = obs_output_create("ffmpeg_muxer", "simple_file_output", nullptr, nullptr);
	if (recordingOutput == nullptr) {
		return false;
	}

	connectOutputSignals();

	return true;
}

void OBS_service::createReplayBufferOutput(void)
{
	replayBufferOutput = obs_output_create("replay_buffer", "ReplayBuffer", nullptr, nullptr);
	connectOutputSignals();
}

void OBS_service::setupAudioEncoder(void)
{
	for (int i = 0; i < MAX_AUDIO_MIXES; i++) {
		char name[9];
		sprintf(name, "adv_aac%d", i);

		if (!createAudioEncoder(&(aacTracks[i]), aacEncodersID[i], GetAdvancedAudioBitrate(i), name, i))
			throw "Failed to create audio encoder "
			      "(advanced output)";
		obs_encoder_set_audio(aacTracks[i], obs_get_audio());
	}
}

void OBS_service::clearAudioEncoder(void)
{
	std::cout << "clearAudioEncoder - 0" << std::endl;
	for (int i = 0; i < MAX_AUDIO_MIXES; i++) {
		std::cout << "clearAudioEncoder - 1: " << i << std::endl;
		if (aacTracks[i]) {
			std::cout << "clearAudioEncoder - 2: " << i << std::endl;
			obs_encoder_release(aacTracks[i]);
			std::cout << "clearAudioEncoder - 3: " << i << std::endl;
			aacTracks[i] = nullptr;
			std::cout << "clearAudioEncoder - 4: " << i << std::endl;
		}
		std::cout << "clearAudioEncoder - 5: " << i << std::endl;
	}
	std::cout << "clearAudioEncoder - 6" << std::endl;
}

void OBS_service::updateStreamingEncoders(bool isSimpleMode)
{
	if (!isStreaming) {
		updateAudioStreamingEncoder(isSimpleMode);
		updateVideoStreamingEncoder(isSimpleMode);
	}
}

std::condition_variable cv_streaming;
std::mutex mtx_output_stop;
bool ready = false;

bool OBS_service::startStreaming(callbackService callJS)
{
	const char* type = obs_service_get_output_type(service);
	if (!type)
		type = "rtmp_output";

	if (streamingOutput) {
		blog(LOG_INFO, "release streaming output 1");
		obs_output_release(streamingOutput);
	}

	streamingOutput = obs_output_create(type, "simple_stream", nullptr, nullptr);
	if (!streamingOutput)
		return false;

	connectOutputSignals();

	// streamingWaitData = new ServiceWaitData();
	// releaseWorker = std::thread(releaseStreamingOutput);
	// auto on_stopped = [&]() {
	// 	std::unique_lock<std::mutex> lck(mtx_output_stop);
	// 	ready = true;
	// 	std::cout << "Streaming deactivate signal received" << std::endl;
	// 	cv_streaming.notify_all();
	// 	std::cout << "notified" << std::endl;
	// };

	// using on_stopped_t = decltype(on_stopped);
	// auto pre_on_stopped = [](void* data, calldata_t*) {
	// 	on_stopped_t& on_stopped = *reinterpret_cast<on_stopped_t*>(data);
	// 	on_stopped();
	// };
	
	// auto on_stopped = [&](void* data, calldata_t*) {
	// 	std::cout << "Streaming deactivate signal received" << std::endl;
	// 	ServiceWaitData* streamingWaitData = reinterpret_cast<ServiceWaitData*>(data);
	// 	// blog(LOG_INFO, "Streaming deactivate signal received");
	// 	streamingWaitData->cv.notify_one();
	// 	std::cout << "notified" << std::endl;
	// };

	// signal_handler* sh = obs_output_get_signal_handler(streamingOutput);
	// signal_handler_connect(sh, "deactivate", pre_on_stopped, &on_stopped);

	std::string currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");
	bool        isSimpleMode      = currentOutputMode.compare("Simple") == 0;

	updateStreamingEncoders(isSimpleMode);
	updateService();
	updateStreamingOutput();

	obs_output_set_video_encoder(streamingOutput, videoStreamingEncoder);

	if (isSimpleMode)
		obs_output_set_audio_encoder(streamingOutput, audioSimpleStreamingEncoder, 0);
	else
		obs_output_set_audio_encoder(
		    streamingOutput,
		    audioAdvancedStreamingEncoder, 0);

	twitchSoundtrackEnabled = startTwitchSoundtrackAudio();
	if (!twitchSoundtrackEnabled)
		setupVodTrack(isSimpleMode);

	isStreaming = obs_output_start(streamingOutput);
	if (!isStreaming) {
		SignalInfo* signal = new SignalInfo(
			std::string("streaming"),
			std::string("stop"),
			0,
			std::string(""),
			g_jsThread
		);
		std::string outdated_driver_error = outdated_driver_error::instance()->get_error();
		if (outdated_driver_error.size() != 0) {
			signal->m_errorMessage = outdated_driver_error;
			signal->m_code = OBS_OUTPUT_OUTDATED_DRIVER;
		} else {
			const char* error = obs_output_get_last_error(streamingOutput);
			if (error) {
				signal->m_errorMessage = error;
				blog(LOG_INFO, "Last streaming error: %s", error);
			}
			signal->m_code = OBS_OUTPUT_ERROR;
		}

		callJS(signal);
	}
	return isStreaming;
}

void OBS_service::updateAudioStreamingEncoder(bool isSimpleMode)
{
	const char* codec = nullptr;

	if (streamingOutput) {
		codec = obs_output_get_supported_audio_codecs(streamingOutput);
	} else {
		const char* type = obs_service_get_output_type(service);
		if (!type)
			type = "rtmp_output";

		obs_output_t* currentOutput = obs_output_create(type, "temp_stream", nullptr, nullptr);
		if (!currentOutput)
			return;

		codec = obs_output_get_supported_audio_codecs(currentOutput);
		blog(LOG_INFO, "release streaming output 2");
		obs_output_release(currentOutput);
	}

	if (!codec) {
		return;
	}

	obs_encoder_t** enc = isSimpleMode ? &audioSimpleStreamingEncoder : &audioAdvancedStreamingEncoder;

	if (*enc && obs_encoder_active(*enc))
		return;

	if (*enc && isSimpleMode) {
		obs_encoder_release(*enc);
		*enc = nullptr;
	}

	if (strcmp(codec, "aac") == 0 && isSimpleMode) {
		createSimpleAudioStreamingEncoder();
		*enc = audioSimpleStreamingEncoder;
	} else if (strcmp(codec, "aac") == 0 && !isSimpleMode) {
		uint64_t trackIndex = config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "TrackIndex");
		updateAudioTracks();
		*enc                = aacTracks[trackIndex - 1];
	} else {
		uint64_t    trackIndex   = config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "TrackIndex");
		const char* id           = FindAudioEncoderFromCodec(codec);
		int         audioBitrate = GetAdvancedAudioBitrate(trackIndex - 1);
		obs_data_t* settings     = obs_data_create();
		obs_data_set_int(settings, "bitrate", audioBitrate);

		*enc = obs_audio_encoder_create(id, "alt_audio_enc", nullptr, isSimpleMode ? 0 : trackIndex - 1, nullptr);
		if (!(*enc))
			return;

		obs_encoder_update(*enc, settings);
		obs_data_release(settings);
	}
	obs_encoder_set_audio(*enc, obs_get_audio());

	return;
}

void OBS_service::updateAudioRecordingEncoder(bool isSimpleMode)
{
	if (isRecording && rpUsesRec)
		return;

	if (isSimpleMode) {
		if (!createAudioEncoder(&audioSimpleRecordingEncoder, aacSimpleRecEncID, 192, "simple_aac_recording", 0))
			throw "Failed to create audio simple recording encoder";

		obs_encoder_set_audio(audioSimpleRecordingEncoder, obs_get_audio());
	} else {
		updateAudioTracks();
	}
}

void OBS_service::updateVideoRecordingEncoder(bool isSimpleMode)
{
	if (isRecording && rpUsesRec)
		return;

	const char* quality = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecQuality");
	const char* encoder = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecEncoder");

	videoEncoder = encoder;
	videoQuality = quality;
	ffmpegOutput = false;

	if (isSimpleMode) {
		lowCPUx264 = false;
		if (strcmp(encoder, SIMPLE_ENCODER_X264) == 0 || strcmp(encoder, ADVANCED_ENCODER_X264) == 0) {
			LoadRecordingPreset_h264("obs_x264");
		} else if (strcmp(encoder, SIMPLE_ENCODER_X264_LOWCPU) == 0) {
			LoadRecordingPreset_h264("obs_x264");
			lowCPUx264 = true;
		} else if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0 || strcmp(encoder, ADVANCED_ENCODER_QSV) == 0) {
			LoadRecordingPreset_h264("obs_qsv11");
		} else if (strcmp(encoder, SIMPLE_ENCODER_AMD) == 0 || strcmp(encoder, ADVANCED_ENCODER_AMD) == 0) {
			LoadRecordingPreset_h264("amd_amf_h264");
		} else if (strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0 || strcmp(encoder, ADVANCED_ENCODER_NVENC) == 0) {
			LoadRecordingPreset_h264("ffmpeg_nvenc");
		} else if (strcmp(encoder, ENCODER_NEW_NVENC) == 0) {
			LoadRecordingPreset_h264("jim_nvenc");
		}
		usingRecordingPreset = true;
		updateVideoRecordingEncoderSettings();
	} else {
		const char* recordingEncoder = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecEncoder");
		if (recordingEncoder && strcmp(recordingEncoder, ENCODER_NEW_NVENC) != 0) {
			unsigned int cx = 0;
			unsigned int cy = 0;

			bool        rescale    = config_get_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "RecRescale");
			const char* rescaleRes = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecRescaleRes");

			if (rescale && rescaleRes && *rescaleRes) {
				if (sscanf(rescaleRes, "%ux%u", &cx, &cy) != 2) {
					cx = 0;
					cy = 0;
				}
				obs_encoder_set_scaled_size(videoRecordingEncoder, cx, cy);
			}
		}
	}
	obs_encoder_set_video(videoRecordingEncoder, obs_get_video());
}

bool OBS_service::updateRecordingEncoders(bool isSimpleMode)
{
	std::string simpleQuality =
	    config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecQuality");
	std::string advancedQuality = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecEncoder");
	bool        useStreamEncoder = false;

	bool simpleUsesStream = isSimpleMode && simpleQuality.compare("Stream") == 0;
	bool advancedUsesStream =
	    !isSimpleMode && (advancedQuality.compare("") == 0 || advancedQuality.compare("none") == 0);

	if (simpleUsesStream || advancedUsesStream) {
		usingRecordingPreset = false;
		ffmpegOutput         = false;

		if (isSimpleMode) {
			if (!isStreaming)
				updateAudioStreamingEncoder(isSimpleMode);

			if (!obs_get_multiple_rendering())
				useStreamEncoder = true;
			else {
				duplicate_encoder(&audioSimpleRecordingEncoder, audioSimpleStreamingEncoder, 0);
				obs_encoder_set_audio(audioSimpleRecordingEncoder, obs_get_audio());
				useStreamEncoder = false;
			}
		} else {
			updateAudioRecordingEncoder(isSimpleMode);
		}

		if (!isStreaming)
			updateVideoStreamingEncoder(isSimpleMode);

		if (!obs_get_multiple_rendering()) {
			obs_encoder_set_video(videoStreamingEncoder, obs_get_video());
			useStreamEncoder = true;
		} else {
			duplicate_encoder(&videoRecordingEncoder, videoStreamingEncoder);
			obs_encoder_set_video(videoRecordingEncoder, obs_get_video());
			useStreamEncoder = false;
		}
	} else {
		updateAudioRecordingEncoder(isSimpleMode);
		updateVideoRecordingEncoder(isSimpleMode);

		useStreamEncoder = false;
	}
	return useStreamEncoder;
}

bool OBS_service::startRecording(callbackService callJS)
{
	std::cout << "startRecording - 0" << std::endl;
	if (recordingOutput)
		obs_output_release(recordingOutput);

	std::cout << "startRecording - 1" << std::endl;
	recordingOutput = obs_output_create("ffmpeg_muxer", "simple_file_output", nullptr, nullptr);
	if (!recordingOutput)
		return false;

	std::cout << "startRecording - 2" << std::endl;
	connectOutputSignals();

	std::string currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");
	bool        isSimpleMode      = currentOutputMode.compare("Simple") == 0;
	std::string simpleQuality =
	    config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecQuality");

	std::cout << "startRecording - 3" << std::endl;
	bool useStreamEncoder = false;

	usingRecordingPreset = true;
	if (isSimpleMode && simpleQuality.compare("Lossless") == 0) {
		LoadRecordingPreset_Lossless();
		ffmpegOutput = true;
	} else {
		if (!(obs_get_multiple_rendering()
		      && obs_get_replay_buffer_rendering_mode() == OBS_RECORDING_REPLAY_BUFFER_RENDERING
		      && isReplayBufferActive)) {
			useStreamEncoder = updateRecordingEncoders(isSimpleMode);
		}
	}
	std::cout << "startRecording - 4" << std::endl;
	updateFfmpegOutput(isSimpleMode, recordingOutput);

	obs_output_set_video_encoder(recordingOutput, useStreamEncoder ? videoStreamingEncoder : videoRecordingEncoder);
	if (isSimpleMode) {
		obs_output_set_audio_encoder(recordingOutput,
			                         useStreamEncoder
			                         ? audioSimpleStreamingEncoder
			                         : audioSimpleRecordingEncoder, 0);
	} else {
		int tracks = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "RecTracks"));
		int idx    = 0;
		for (int i = 0; i < MAX_AUDIO_MIXES; i++) {
			if ((tracks & (1 << i)) != 0) {
				obs_output_set_audio_encoder(recordingOutput, aacTracks[i], idx);
				idx++;
			}
		}
	}

	std::cout << "startRecording - 5" << std::endl;
	isRecording = obs_output_start(recordingOutput);
	std::cout << "startRecording - 6" << std::endl;
	if (!isRecording) {
		SignalInfo* signal = new SignalInfo(
			std::string("recording"),
			std::string("stop"),
			0,
			std::string(""),
			g_jsThread
		);
		std::string outdated_driver_error = outdated_driver_error::instance()->get_error();
		if (outdated_driver_error.size() != 0) {
			signal->m_errorMessage = outdated_driver_error;
			signal->m_code = OBS_OUTPUT_OUTDATED_DRIVER;
		} else {
			const char* error = obs_output_get_last_error(recordingOutput);
			if (error) {
				signal->m_errorMessage = error;
				blog(LOG_INFO, "Last recording error: %s", error);
			}
			signal->m_code = OBS_OUTPUT_ERROR;
		}

		callJS(signal);
	}
	std::cout << "startRecording - 7" << std::endl;
	return isRecording;
}

void OBS_service::stopStreaming(bool forceStop, callbackService callJS)
{
	blog(LOG_INFO, "stopStreaming - 0");
	if (!obs_output_active(streamingOutput) && !obs_output_reconnecting(streamingOutput))
	{
		blog(LOG_WARNING, "stopStreaming was ignored as stream not active or reconnecting");
		return;
	}

	blog(LOG_INFO, "stopStreaming - 1");
	if (forceStop)
		obs_output_force_stop(streamingOutput);
	else
		obs_output_stop(streamingOutput);

	blog(LOG_INFO, "stopStreaming - 3");
	// waitReleaseWorker();
	releaseStreamingOutput();

	blog(LOG_INFO, "stopStreaming - 4");
	isStreaming = false;
	blog(LOG_INFO, "stopStreaming - 5");

	// if (callJS) {
	// 	SignalInfo* signal = new SignalInfo(
	// 		std::string("streaming"),
	// 		std::string("deactivate"),
	// 		0,
	// 		std::string(""),
	// 		g_jsThread
	// 	);
	// 	callJS(signal);
	// }
}

void OBS_service::stopRecording(void)
{
	obs_output_stop(recordingOutput);
	isRecording = false;
}

void OBS_service::updateReplayBufferOutput(bool isSimpleMode, bool useStreamEncoder)
{
	const char* path;
	const char* format;
	const char* mux;
	bool        noSpace;
	const char* fileNameFormat;
	bool        overwriteIfExists;
	const char* rbPrefix;
	const char* rbSuffix;
	int64_t     rbTime;
	int64_t     rbSize;

	if (isSimpleMode) {
		path    = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "FilePath");
		format  = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecFormat");
		mux     = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "MuxerCustom");
		noSpace = config_get_bool(ConfigManager::getInstance().getBasic(), "SimpleOutput", "FileNameWithoutSpace");
		fileNameFormat    = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "FilenameFormatting");
		overwriteIfExists = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "OverwriteIfExists");
		rbPrefix          = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecRBPrefix");
		rbSuffix          = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecRBSuffix");
		rbTime            = int(config_get_int(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecRBTime"));
		rbSize            = int(config_get_int(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecRBSize"));
	} else {
		path              = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecFilePath");
		format            = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecFormat");
		fileNameFormat    = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "FilenameFormatting");
		overwriteIfExists = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "OverwriteIfExists");
		noSpace  = config_get_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "RecFileNameWithoutSpace");
		rbPrefix = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecRBPrefix");
		rbSuffix = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecRBSuffix");
		rbTime   = config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "RecRBTime");
		rbSize   = config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "RecRBSize");
	}

	std::string f;
	if (rbPrefix && *rbPrefix) {
		f += rbPrefix;
		if (f.back() != ' ')
			f += " ";
	}
	f += fileNameFormat;
	if (rbSuffix && *rbSuffix) {
		if (*rbSuffix != ' ')
			f += " ";
		f += rbSuffix;
	}
	remove_reserved_file_characters(f);

	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "directory", path);
	obs_data_set_string(settings, "format", f.c_str());
	obs_data_set_string(settings, "extension", format);
	obs_data_set_bool(settings, "allow_spaces", !noSpace);
	obs_data_set_int(settings, "max_time_sec", rbTime);
	obs_data_set_int(settings, "max_size_mb", usingRecordingPreset ? rbSize : 0);

	if (!isSimpleMode) {
		bool        usesBitrate = false;
		obs_data_t* streamEncSettings =
		    obs_data_create_from_json_file_safe(ConfigManager::getInstance().getStream().c_str(), "bak");
		obs_data_t* recordEncSettings =
		    obs_data_create_from_json_file_safe(ConfigManager::getInstance().getRecord().c_str(), "bak");

		const char* rate_control =
		    obs_data_get_string(useStreamEncoder ? streamEncSettings : recordEncSettings, "rate_control");
		if (!rate_control)
			rate_control = "";
		usesBitrate = astrcmpi(rate_control, "CBR") == 0 || astrcmpi(rate_control, "VBR") == 0
		              || astrcmpi(rate_control, "ABR") == 0;
		obs_data_set_int(settings, "max_size_mb", usesBitrate ? 0 : rbSize);
	}

	obs_output_update(replayBufferOutput, settings);
	obs_data_release(settings);
}

bool OBS_service::startReplayBuffer(callbackService callJS)
{
	if (replayBufferOutput)
		obs_output_release(replayBufferOutput);

	replayBufferOutput = obs_output_create("replay_buffer", "ReplayBuffer", nullptr, nullptr);
	if (!replayBufferOutput)
		return false;

	connectOutputSignals();

	std::string     currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");
	bool            isSimpleMode      = currentOutputMode.compare("Simple") == 0;
	bool useStreamEncoder = false;

	if (obs_get_multiple_rendering()
	    && obs_get_replay_buffer_rendering_mode() == OBS_STREAMING_REPLAY_BUFFER_RENDERING) {
		updateStreamingEncoders(isSimpleMode);
		useStreamEncoder = true;
		rpUsesStream     = true;
	} else {
		useStreamEncoder = isRecording ? !usingRecordingPreset
			: updateRecordingEncoders(isSimpleMode);

		rpUsesRec = true;
	}

	updateFfmpegOutput(isSimpleMode, replayBufferOutput);
	updateReplayBufferOutput(isSimpleMode, useStreamEncoder);

	obs_output_set_video_encoder(replayBufferOutput, useStreamEncoder ? videoStreamingEncoder : videoRecordingEncoder);
	if (isSimpleMode) {
		obs_output_set_audio_encoder(replayBufferOutput,
			                         useStreamEncoder
			                         ? audioSimpleStreamingEncoder
			                         : audioSimpleRecordingEncoder,
									 0);
	} else {
		int tracks = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "RecTracks"));
		int idx    = 0;
		for (int i = 0; i < MAX_AUDIO_MIXES; i++) {
			if ((tracks & (1 << i)) != 0) {
				obs_output_set_audio_encoder(replayBufferOutput, aacTracks[i], idx);
				idx++;
			}
		}
	}

	bool result = obs_output_start(replayBufferOutput);
	if (!result) {
		SignalInfo* signal = new SignalInfo(
			std::string("replay-buffer"),
			std::string("stop"),
			0,
			std::string(""),
			g_jsThread
		);
		// obs::SignalInfo signal    = obs::SignalInfo("replay-buffer", "stop", g_jsThread);
		isReplayBufferActive = false;
		rpUsesRec            = false;
		rpUsesStream         = false;
		std::string outdated_driver_error = outdated_driver_error::instance()->get_error();
		if (outdated_driver_error.size() != 0) {
			signal->m_errorMessage = outdated_driver_error;
			signal->m_code = OBS_OUTPUT_OUTDATED_DRIVER;
		} else {
			const char* error = obs_output_get_last_error(replayBufferOutput);
			if (error) {
				signal->m_errorMessage = error;
				blog(LOG_INFO, "Last replay buffer error: %s", error);
			}
			signal->m_code = OBS_OUTPUT_ERROR;
		}

		callJS(signal);
	} else {
		isReplayBufferActive = true;
	}

	return isReplayBufferActive;
}

void OBS_service::stopReplayBuffer(bool forceStop)
{
	if (forceStop)
		obs_output_force_stop(replayBufferOutput);
	else
		obs_output_stop(replayBufferOutput);
}

obs_service_t* OBS_service::getService(void)
{
	const char* serviceType = obs_service_get_type(service);
	return service;
}

void OBS_service::setService(obs_service_t* newService)
{
	obs_service_release(service);
	service = newService;
}

void OBS_service::saveService(void)
{
	if (!service)
		return;

	obs_data_t* data     = obs_data_create();
	obs_data_t* settings = obs_service_get_settings(service);

	const char* serviceType = obs_service_get_type(service);

	if (serviceType && strlen(serviceType) > 0) {
		obs_data_set_string(data, "type", serviceType);
		obs_data_set_obj(data, "settings", settings);

		if (!obs_data_save_json_safe(data, ConfigManager::getInstance().getService().c_str(), "tmp", "bak"))
			blog(LOG_WARNING, "Failed to save service");

		obs_service_update(service, settings);

		serviceType = obs_service_get_type(service);
	}
	obs_data_release(settings);
	obs_data_release(data);
}

bool OBS_service::isStreamingOutputActive(void)
{
	return obs_output_active(streamingOutput);
}

bool OBS_service::isRecordingOutputActive(void)
{
	return obs_output_active(recordingOutput);
}

bool OBS_service::isReplayBufferOutputActive(void)
{
	return obs_output_active(replayBufferOutput);
}

int OBS_service::GetSimpleAudioBitrate(void)
{
	int bitrate = (int)config_get_uint(ConfigManager::getInstance().getBasic(), "SimpleOutput", "ABitrate");

	return FindClosestAvailableAACBitrate(bitrate);
}

int OBS_service::GetAdvancedAudioBitrate(int i)
{
	static const char* names[] = {
	    "Track1Bitrate",
	    "Track2Bitrate",
	    "Track3Bitrate",
	    "Track4Bitrate",
	    "Track5Bitrate",
	    "Track6Bitrate",
	};
	int bitrate = (int)config_get_uint(ConfigManager::getInstance().getBasic(), "AdvOut", names[i]);
	return FindClosestAvailableAACBitrate(bitrate);
}

bool OBS_service::EncoderAvailable(const char* encoder)
{
	const char* val;
	int         i = 0;

	while (obs_enum_encoder_types(i++, &val)) {
		if (val == nullptr)
			continue;
		if (strcmp(val, encoder) == 0)
			return true;
	}

	return false;
}

void OBS_service::updateVideoStreamingEncoder(bool isSimpleMode)
{
	if (videoStreamingEncoder && obs_encoder_active(videoStreamingEncoder))
		return;

	if (isSimpleMode) {
		obs_data_t* h264Settings = obs_data_create();
		obs_data_t* aacSettings  = obs_data_create();

		int  videoBitrate = int(config_get_uint(ConfigManager::getInstance().getBasic(), "SimpleOutput", "VBitrate"));
		int  audioBitrate = GetSimpleAudioBitrate();
		bool advanced     = config_get_bool(ConfigManager::getInstance().getBasic(), "SimpleOutput", "UseAdvanced");
		bool enforceBitrate =
		    config_get_bool(ConfigManager::getInstance().getBasic(), "SimpleOutput", "EnforceBitrate");
		const char* custom = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "x264Settings");
		const char* encoder =
		    config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "StreamEncoder");
		const char* encoderID = nullptr;
		const char* presetType = nullptr;
		const char* preset = nullptr;

		if (encoder != NULL) {
			if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0 || strcmp(encoder, ADVANCED_ENCODER_QSV) == 0) {
				presetType = "QSVPreset";
				encoderID  = "obs_qsv11";
			} else if (strcmp(encoder, SIMPLE_ENCODER_AMD) == 0 || strcmp(encoder, ADVANCED_ENCODER_AMD) == 0) {
				presetType = "AMDPreset";
				UpdateStreamingSettings_amd(h264Settings, videoBitrate);
				encoderID = "amd_amf_h264";
			} else if (strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0 || strcmp(encoder, ADVANCED_ENCODER_NVENC) == 0) {
				presetType = "NVENCPreset";
				encoderID  = "ffmpeg_nvenc";
			} else if (strcmp(encoder, ENCODER_NEW_NVENC) == 0) {
				presetType = "NVENCPreset";
				encoderID  = "jim_nvenc";
			} else if (strcmp(encoder, APPLE_SOFTWARE_VIDEO_ENCODER) == 0)  {
				encoderID  = APPLE_SOFTWARE_VIDEO_ENCODER;
			} else if (strcmp(encoder, APPLE_HARDWARE_VIDEO_ENCODER) == 0)  {
				encoderID  = APPLE_HARDWARE_VIDEO_ENCODER;
			} else {
				presetType = "Preset";
				encoderID  = "obs_x264";
			}
			if (presetType)
				preset = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", presetType);

			if (videoStreamingEncoder != nullptr) {
				obs_encoder_release(videoStreamingEncoder);
				videoStreamingEncoder = nullptr;
			}
			videoStreamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", nullptr, nullptr);
		}

		if (videoBitrate == 0) {
			videoBitrate = 2500;
			config_set_uint(ConfigManager::getInstance().getBasic(), "SimpleOutput", "VBitrate", videoBitrate);
			config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
		}

		obs_data_set_string(h264Settings, "rate_control", "CBR");
		obs_data_set_int(h264Settings, "bitrate", videoBitrate);

		if (advanced) {
			obs_data_set_string(h264Settings, "preset", preset);
			obs_data_set_string(h264Settings, "x264opts", custom);
		}

		obs_data_set_string(aacSettings, "rate_control", "CBR");
		obs_data_set_int(aacSettings, "bitrate", audioBitrate);

		const char* url = obs_service_get_url(service);

		obs_service_apply_encoder_settings(service, h264Settings, aacSettings);

		if (advanced && !enforceBitrate) {
			obs_data_set_int(h264Settings, "bitrate", videoBitrate);
			obs_data_set_int(aacSettings, "bitrate", audioBitrate);
		}

		video_t*          video  = obs_get_video();
		enum video_format format = video_output_get_format(video);

		if (format != VIDEO_FORMAT_NV12 && format != VIDEO_FORMAT_I420)
			obs_encoder_set_preferred_video_format(videoStreamingEncoder, VIDEO_FORMAT_NV12);

		if (strcmp(encoder, APPLE_SOFTWARE_VIDEO_ENCODER) == 0 ||
				strcmp(encoder, APPLE_HARDWARE_VIDEO_ENCODER) == 0) {
			const char* profile = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "Profile");
			if (profile)
				obs_data_set_string(h264Settings, "profile", profile);
		}

		obs_encoder_update(videoStreamingEncoder, h264Settings);
		obs_encoder_update(audioSimpleStreamingEncoder, aacSettings);

		obs_data_release(h264Settings);
		obs_data_release(aacSettings);
	} else {
		const char* streamEncoder = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "Encoder");
		if (streamEncoder && strcmp(streamEncoder, ENCODER_NEW_NVENC) != 0) {
			unsigned int cx = 0;
			unsigned int cy = 0;

			bool        rescale    = config_get_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "Rescale");
			const char* rescaleRes = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RescaleRes");

			if (rescale && rescaleRes && *rescaleRes) {
				if (sscanf(rescaleRes, "%ux%u", &cx, &cy) != 2) {
					cx = 0;
					cy = 0;
				}
				obs_encoder_set_scaled_size(videoStreamingEncoder, cx, cy);
			}
		}
	}
	obs_encoder_set_video(videoStreamingEncoder, obs_get_video());
}

std::string OBS_service::GetDefaultVideoSavePath(void)
{
#ifdef WIN32
	wchar_t path_utf16[MAX_PATH];
	char    path_utf8[MAX_PATH] = {};

	SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT, path_utf16);

	os_wcs_to_utf8(path_utf16, wcslen(path_utf16), path_utf8, MAX_PATH);
	return std::string(path_utf8);
#else
    return g_util_osx_server->getDefaultVideoSavePath();
#endif
}

void OBS_service::updateService(void)
{
	obs_data_t* settings = obs_service_get_settings(service);
	const char* platform = obs_data_get_string(settings, "service");

	const char* server = obs_service_get_url(service);

	if (platform && strcmp(platform, "Twitch") == 0) {
		if (!server || strcmp(server, "") == 0) {
			server = "auto";
			obs_data_set_string(settings, "server", server);
			obs_service_update(service, settings);
		}
	}

	obs_data_release(settings);
	obs_output_set_service(streamingOutput, service);
}

void OBS_service::updateFfmpegOutput(bool isSimpleMode, obs_output_t* output)
{
	const char* path;
	const char* format;
	const char* mux;
	bool        noSpace;
	const char* fileNameFormat;
	bool        overwriteIfExists;

	if (isSimpleMode) {
		path    = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "FilePath");
		format  = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecFormat");
		mux     = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "MuxerCustom");
		noSpace = config_get_bool(ConfigManager::getInstance().getBasic(), "SimpleOutput", "FileNameWithoutSpace");
		fileNameFormat    = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "FilenameFormatting");
		overwriteIfExists = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "OverwriteIfExists");
	} else {
		path              = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecFilePath");
		format            = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecFormat");
		fileNameFormat    = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "FilenameFormatting");
		overwriteIfExists = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "OverwriteIfExists");
		noSpace  = config_get_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "RecFileNameWithoutSpace");
	}

	std::string initialPath;
	if (path != nullptr) {
		initialPath = path;
	}

	if (fileNameFormat == NULL) {
		fileNameFormat = "%CCYY-%MM-%DD %hh-%mm-%ss";
	}

	std::string strPath;
	strPath += initialPath;

	char lastChar = strPath.back();
	if (lastChar != '/' && lastChar != '\\')
		strPath += "/";

	if (fileNameFormat != NULL && format != NULL)
		strPath += GenerateSpecifiedFilename(ffmpegOutput ? "avi" : format, noSpace, fileNameFormat);

	if (!overwriteIfExists)
		FindBestFilename(strPath, noSpace);

	if (strPath.size() > 0) {
		obs_data_t* settings = obs_data_create();
		obs_data_set_string(settings, ffmpegOutput ? "url" : "path", strPath.c_str());
		obs_output_update(output, settings);
		obs_data_release(settings);
	}
}

void OBS_service::updateAudioTracks()
{
	static const char* configTracksNames[] = {
	    "Track1Name",
	    "Track2Name",
	    "Track3Name",
	    "Track4Name",
	    "Track5Name",
	    "Track6Name",
	};

	obs_data_t* settings[MAX_AUDIO_MIXES];

	for (size_t i = 0; i < MAX_AUDIO_MIXES; i++) {
		if (aacTracks[i] && !obs_encoder_active(aacTracks[i])) {
			settings[i] = obs_data_create();
			obs_data_set_int(settings[i], "bitrate", GetAdvancedAudioBitrate(i));

			const char* name =
			    config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", configTracksNames[i]);
			if (name)
				obs_encoder_set_name(aacTracks[i], name);

			obs_encoder_update(aacTracks[i], settings[i]);
			obs_data_release(settings[i]);
			obs_encoder_set_audio(aacTracks[i], obs_get_audio());
		}
	}
}

void OBS_service::LoadRecordingPreset_Lossless()
{
	if (recordingOutput != NULL) {
		obs_output_release(recordingOutput);
	}
	recordingOutput = obs_output_create("ffmpeg_output", "simple_ffmpeg_output", nullptr, nullptr);
	connectOutputSignals();
	if (!recordingOutput)
		throw "Failed to create recording FFmpeg output "
		      "(simple output)";

	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "format_name", "avi");
	obs_data_set_string(settings, "video_encoder", "utvideo");
	obs_data_set_string(settings, "audio_encoder", "pcm_s16le");

	obs_output_set_mixers(recordingOutput, 1);
	obs_output_update(recordingOutput, settings);
	obs_data_release(settings);
}

void OBS_service::LoadRecordingPreset_h264(const char* encoderId)
{
	if (videoRecordingEncoder != NULL) {
		obs_encoder_release(videoRecordingEncoder);
	}
	videoRecordingEncoder = obs_video_encoder_create(encoderId, "simple_h264_recording", nullptr, nullptr);
	if (!videoRecordingEncoder)
		throw "Failed to create h264 recording encoder (simple output)";
	// obs_encoder_release(videoRecordingEncoder);
}

static bool update_ffmpeg_output(config_t* config)
{
	if (config_has_user_value(config, "AdvOut", "FFOutputToFile"))
		return false;

	const char* url = config_get_string(config, "AdvOut", "FFURL");
	if (!url)
		return false;

	bool isActualURL = strstr(url, "://") != nullptr;
	if (isActualURL)
		return false;

	std::string urlStr = url;
	std::string extension;

	for (size_t i = urlStr.length(); i > 0; i--) {
		size_t idx = i - 1;

		if (urlStr[idx] == '.') {
			extension = &urlStr[i];
		}

		if (urlStr[idx] == '\\' || urlStr[idx] == '/') {
			urlStr[idx] = 0;
			break;
		}
	}

	if (urlStr.empty() || extension.empty())
		return false;

	config_remove_value(config, "AdvOut", "FFURL");
	config_set_string(config, "AdvOut", "FFFilePath", urlStr.c_str());
	config_set_string(config, "AdvOut", "FFExtension", extension.c_str());
	config_set_bool(config, "AdvOut", "FFOutputToFile", true);
	return true;
}

void OBS_service::UpdateFFmpegCustomOutput(void)
{
	update_ffmpeg_output(ConfigManager::getInstance().getBasic());

	if (recordingOutput != NULL) {
		obs_output_release(recordingOutput);
	}
	recordingOutput = obs_output_create("ffmpeg_output", "simple_ffmpeg_output", nullptr, nullptr);
	connectOutputSignals();

	const char* url        = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFURL");
	int         vBitrate   = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "FFVBitrate"));
	int         gopSize    = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "FFVGOPSize"));
	bool        rescale    = config_get_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "FFRescale");
	const char* rescaleRes = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFRescaleRes");
	const char* formatName = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFFormat");
	const char* mimeType   = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFFormatMimeType");
	const char* muxCustom  = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFMCustom");
	const char* vEncoder   = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFVEncoder");
	int         vEncoderId = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "FFVEncoderId"));
	const char* vEncCustom = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFVCustom");
	int         aBitrate   = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "FFABitrate"));
	int         aTrack     = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "FFAudioTrack"));
	const char* aEncoder   = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFAEncoder");
	int         aEncoderId = int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "FFAEncoderId"));
	const char* aEncCustom = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFACustom");

	obs_data_t* settings = obs_data_create();

	obs_data_set_string(settings, "url", url);
	obs_data_set_string(settings, "format_name", formatName);
	obs_data_set_string(settings, "format_mime_type", mimeType);
	obs_data_set_string(settings, "muxer_settings", muxCustom);
	obs_data_set_int(settings, "gop_size", gopSize);
	obs_data_set_int(settings, "video_bitrate", vBitrate);
	obs_data_set_string(settings, "video_encoder", vEncoder);
	obs_data_set_int(settings, "video_encoder_id", vEncoderId);
	obs_data_set_string(settings, "video_settings", vEncCustom);
	obs_data_set_int(settings, "audio_bitrate", aBitrate);
	obs_data_set_string(settings, "audio_encoder", aEncoder);
	obs_data_set_int(settings, "audio_encoder_id", aEncoderId);
	obs_data_set_string(settings, "audio_settings", aEncCustom);

	if (rescale && rescaleRes && *rescaleRes) {
		int width;
		int height;
		int val = sscanf(rescaleRes, "%dx%d", &width, &height);

		if (val == 2 && width && height) {
			obs_data_set_int(settings, "scale_width", width);
			obs_data_set_int(settings, "scale_height", height);
		}
	}

	obs_output_set_mixer(recordingOutput, aTrack - 1);
	obs_output_set_media(recordingOutput, obs_get_video(), obs_get_audio());
	obs_output_update(recordingOutput, settings);

	obs_data_release(settings);
}

static bool icq_available(obs_encoder_t* encoder)
{
	obs_properties_t* props     = obs_encoder_properties(encoder);
	obs_property_t*   p         = obs_properties_get(props, "rate_control");
	bool              icq_found = false;

	size_t num = obs_property_list_item_count(p);
	for (size_t i = 0; i < num; i++) {
		const char* val = obs_property_list_item_string(p, i);
		if (strcmp(val, "ICQ") == 0) {
			icq_found = true;
			break;
		}
	}

	obs_properties_destroy(props);
	return icq_found;
}

void OBS_service::UpdateRecordingSettings_qsv11(int crf)
{
	bool icq = icq_available(videoRecordingEncoder);

	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "profile", "high");

	if (icq) {
		obs_data_set_string(settings, "rate_control", "ICQ");
		obs_data_set_int(settings, "icq_quality", crf);
	} else {
		obs_data_set_string(settings, "rate_control", "CQP");
		obs_data_set_int(settings, "qpi", crf);
		obs_data_set_int(settings, "qpp", crf);
		obs_data_set_int(settings, "qpb", crf);
	}

	obs_encoder_update(videoRecordingEncoder, settings);

	obs_data_release(settings);
}

void OBS_service::UpdateRecordingSettings_nvenc(int cqp)
{
	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "rate_control", "CQP");
	obs_data_set_string(settings, "profile", "high");
	obs_data_set_string(settings, "preset", "hq");
	obs_data_set_int(settings, "cqp", cqp);
	obs_data_set_int(settings, "bitrate", 0);

	obs_encoder_update(videoRecordingEncoder, settings);

	obs_data_release(settings);
}

void OBS_service::UpdateStreamingSettings_amd(obs_data_t* settings, int bitrate)
{
	// Static Properties
	obs_data_set_int(settings, "Usage", 0);
	obs_data_set_int(settings, "Profile", 100); // High

	// Rate Control Properties
	obs_data_set_int(settings, "RateControlMethod", 3);
	obs_data_set_int(settings, "Bitrate.Target", bitrate);
	obs_data_set_int(settings, "FillerData", 1);
	obs_data_set_int(settings, "VBVBuffer", 1);
	obs_data_set_int(settings, "VBVBuffer.Size", bitrate);

	// Picture Control Properties
	obs_data_set_double(settings, "KeyframeInterval", 2.0);
	obs_data_set_int(settings, "BFrame.Pattern", 0);
}

void OBS_service::UpdateRecordingSettings_amd_cqp(int cqp)
{
	obs_data_t* settings = obs_data_create();

	// Static Properties
	obs_data_set_int(settings, "Usage", 0);
	obs_data_set_int(settings, "Profile", 100); // High

	// Rate Control Properties
	obs_data_set_int(settings, "RateControlMethod", 0);
	obs_data_set_int(settings, "QP.IFrame", cqp);
	obs_data_set_int(settings, "QP.PFrame", cqp);
	obs_data_set_int(settings, "QP.BFrame", cqp);
	obs_data_set_int(settings, "VBVBuffer", 1);
	obs_data_set_int(settings, "VBVBuffer.Size", 100000);

	// Picture Control Properties
	obs_data_set_double(settings, "KeyframeInterval", 2.0);
	obs_data_set_int(settings, "BFrame.Pattern", 0);

	// Update and release
	obs_encoder_update(videoRecordingEncoder, settings);
	obs_data_release(settings);
}

void OBS_service::UpdateRecordingSettings_x264_crf(int crf)
{
	obs_data_t* settings = obs_data_create();
	obs_data_set_int(settings, "crf", crf);
	obs_data_set_bool(settings, "use_bufsize", true);
	obs_data_set_string(settings, "rate_control", "CRF");
	obs_data_set_string(settings, "profile", "high");
	obs_data_set_string(settings, "preset", lowCPUx264 ? "ultrafast" : "veryfast");

	obs_encoder_update(videoRecordingEncoder, settings);

	obs_data_release(settings);
}

#define CROSS_DIST_CUTOFF 2000.0

int CalcCRF(int crf)
{
	uint64_t cx  = config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCX");
	uint64_t cy  = config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCY");
	double   fCX = double(cx);
	double   fCY = double(cy);

	if (lowCPUx264)
		crf -= 2;

	double crossDist       = sqrt(fCX * fCX + fCY * fCY);
	double crfResReduction = fmin(CROSS_DIST_CUTOFF, crossDist) / CROSS_DIST_CUTOFF;
	crfResReduction        = (1.0 - crfResReduction) * 10.0;

	return crf - int(crfResReduction);
}

void OBS_service::updateVideoRecordingEncoderSettings()
{
	bool ultra_hq = (videoQuality == "HQ");
	int  crf      = CalcCRF(ultra_hq ? 16 : 23);

	if (videoEncoder.compare(SIMPLE_ENCODER_X264) == 0 || videoEncoder.compare(ADVANCED_ENCODER_X264) == 0
	    || videoEncoder.compare(SIMPLE_ENCODER_X264_LOWCPU) == 0) {
		UpdateRecordingSettings_x264_crf(crf);

	} else if (videoEncoder.compare(SIMPLE_ENCODER_QSV) == 0 || videoEncoder.compare(ADVANCED_ENCODER_QSV) == 0) {
		UpdateRecordingSettings_qsv11(crf);

	} else if (videoEncoder.compare(SIMPLE_ENCODER_AMD) == 0 || videoEncoder.compare(ADVANCED_ENCODER_AMD) == 0) {
		UpdateRecordingSettings_amd_cqp(crf);

	} else if (videoEncoder.compare(SIMPLE_ENCODER_NVENC) == 0 || videoEncoder.compare(ADVANCED_ENCODER_NVENC) == 0) {
		UpdateRecordingSettings_nvenc(crf);
	} else if (videoEncoder.compare(ENCODER_NEW_NVENC) == 0) {
		UpdateRecordingSettings_nvenc(crf);
	}
}

obs_encoder_t* OBS_service::getStreamingEncoder(void)
{
	return videoStreamingEncoder;
}

void OBS_service::setStreamingEncoder(obs_encoder_t* encoder)
{
	if (videoStreamingEncoder)
		obs_encoder_release(videoStreamingEncoder);
	videoStreamingEncoder = encoder;
}

obs_encoder_t* OBS_service::getRecordingEncoder(void)
{
	return videoRecordingEncoder;
}

void OBS_service::setRecordingEncoder(obs_encoder_t* encoder)
{
	if (videoRecordingEncoder)
		obs_encoder_release(videoRecordingEncoder);
	videoRecordingEncoder = encoder;
}

obs_encoder_t* OBS_service::getAudioSimpleStreamingEncoder(void)
{
	return audioSimpleStreamingEncoder;
}

void OBS_service::setAudioSimpleStreamingEncoder(obs_encoder_t* encoder)
{
	obs_encoder_release(audioSimpleStreamingEncoder);
	audioSimpleStreamingEncoder = encoder;
}

obs_encoder_t* OBS_service::getAudioSimpleRecordingEncoder(void)
{
	return audioSimpleRecordingEncoder;
}

obs_encoder_t* OBS_service::getArchiveEncoder(void)
{
	return streamArchiveEncVod;
}

void OBS_service::setArchiveEncoder(obs_encoder_t* encoder)
{
	obs_encoder_release(streamArchiveEncVod);
	streamArchiveEncVod = encoder;
}

void OBS_service::setAudioSimpleRecordingEncoder(obs_encoder_t* encoder)
{
	obs_encoder_release(audioSimpleRecordingEncoder);
	audioSimpleRecordingEncoder = encoder;
}

obs_encoder_t* OBS_service::getAudioAdvancedStreamingEncoder(void)
{
	return audioAdvancedStreamingEncoder;
}

void OBS_service::setAudioAdvancedStreamingEncoder(obs_encoder_t* encoder)
{
	obs_encoder_release(audioAdvancedStreamingEncoder);
	audioAdvancedStreamingEncoder = encoder;
}

obs_output_t* OBS_service::getStreamingOutput(void)
{
	return streamingOutput;
}

void OBS_service::setStreamingOutput(obs_output_t* output)
{
	blog(LOG_INFO, "release streaming output 3");
	obs_output_release(streamingOutput);
	streamingOutput = output;
}

obs_output_t* OBS_service::getRecordingOutput(void)
{
	return recordingOutput;
}

void OBS_service::setRecordingOutput(obs_output_t* output)
{
	obs_output_release(recordingOutput);
	recordingOutput = output;
}

obs_output_t* OBS_service::getReplayBufferOutput(void)
{
	return replayBufferOutput;
}

void OBS_service::setReplayBufferOutput(obs_output_t* output)
{
	obs_output_release(replayBufferOutput);
	replayBufferOutput = output;
}

obs_output_t* OBS_service::getVirtualWebcamOutput(void)
{
	return virtualWebcamOutput;
}

void OBS_service::setVirtualWebcamOutput(obs_output_t* output)
{
	if (virtualWebcamOutput)
		obs_output_release(virtualWebcamOutput);

	virtualWebcamOutput = output;
}

void OBS_service::updateStreamingOutput()
{
	const char* currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");

	if (strcmp(currentOutputMode, "Advanced") == 0) {
		bool applyServiceSettings =
		    config_get_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "ApplyServiceSettings");

		if (applyServiceSettings) {
			obs_data_t* encoderSettings = obs_encoder_get_settings(videoStreamingEncoder);
			obs_service_apply_encoder_settings(OBS_service::getService(), encoderSettings, nullptr);
		}
	}

	bool reconnect  = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "Reconnect");
	int  retryDelay = config_get_uint(ConfigManager::getInstance().getBasic(), "Output", "RetryDelay");
	int  maxRetries = config_get_uint(ConfigManager::getInstance().getBasic(), "Output", "MaxRetries");

	bool    useDelay      = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "DelayEnable");
	int64_t delaySec      = config_get_int(ConfigManager::getInstance().getBasic(), "Output", "DelaySec");
	bool    preserveDelay = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "DelayPreserve");

	if (useDelay && delaySec < 0)
		delaySec = 0;

	const char* bindIP           = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "BindIP");
	bool        enableDynBitrate = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "DynamicBitrate");
	bool        enableNewSocketLoop =
	    config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "NewSocketLoopEnable");
	bool enableLowLatencyMode = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "LowLatencyEnable");

	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "bind_ip", bindIP);
	obs_data_set_bool(settings, "dyn_bitrate", enableDynBitrate);
	obs_data_set_bool(settings, "new_socket_loop_enabled", enableNewSocketLoop);
	obs_data_set_bool(settings, "low_latency_mode_enabled", enableLowLatencyMode);
	obs_output_update(streamingOutput, settings);
	obs_data_release(settings);

	if (!reconnect)
		maxRetries = 0;

	obs_output_set_delay(
	    streamingOutput, useDelay ? uint32_t(delaySec) : 0, preserveDelay ? OBS_OUTPUT_DELAY_PRESERVE : 0);

	obs_output_set_reconnect_settings(streamingOutput, maxRetries, retryDelay);
}

std::vector<SignalInfo*> streamingSignals;
std::vector<SignalInfo*> recordingSignals;
std::vector<SignalInfo*> replayBufferSignals;

void OBS_service::OBS_service_connectOutputSignals(signal_callback_t callback, void* jsThread)
{
	g_jsThread = jsThread;
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("start"),
		0,
		std::string(""),
		g_jsThread));
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("stop"),
		0,
		std::string(""),
		g_jsThread));
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("starting"),
		0,
		std::string(""),
		g_jsThread));
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("stopping"),
		0,
		std::string(""),
		g_jsThread));
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("activate"),
		0,
		std::string(""),
		g_jsThread));
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("deactivate"),
		0,
		std::string(""),
		g_jsThread));
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("reconnect"),
		0,
		std::string(""),
		g_jsThread));
	streamingSignals.push_back(new SignalInfo(
		std::string("streaming"),
		std::string("reconnect_success"),
		0,
		std::string(""),
		g_jsThread));

	recordingSignals.push_back(new SignalInfo(
		std::string("recording"),
		std::string("start"),
		0,
		std::string(""),
		g_jsThread));
	recordingSignals.push_back(new SignalInfo(
		std::string("recording"),
		std::string("stop"),
		0,
		std::string(""),
		g_jsThread));
	recordingSignals.push_back(new SignalInfo(
		std::string("recording"),
		std::string("stopping"),
		0,
		std::string(""),
		g_jsThread));

	replayBufferSignals.push_back(new SignalInfo(
		std::string("replay-buffer"),
		std::string("start"),
		0,
		std::string(""),
		g_jsThread));
	replayBufferSignals.push_back(new SignalInfo(
		std::string("replay-buffer"),
		std::string("stop"),
		0,
		std::string(""),
		g_jsThread));
	replayBufferSignals.push_back(new SignalInfo(
		std::string("replay-buffer"),
		std::string("stopping"),
		0,
		std::string(""),
		g_jsThread));

	replayBufferSignals.push_back(new SignalInfo(
		std::string("replay-buffer"),
		std::string("writing"),
		0,
		std::string(""),
		g_jsThread));
	replayBufferSignals.push_back(new SignalInfo(
		std::string("replay-buffer"),
		std::string("wrote"),
		0,
		std::string(""),
		g_jsThread));
	replayBufferSignals.push_back(new SignalInfo(
		std::string("replay-buffer"),
		std::string("writing_error"),
		0,
		std::string(""),
		g_jsThread));

	g_ouput_callback = callback;
	connectOutputSignals(g_ouput_callback);
}

void OBS_service::connectOutputSignals(signal_callback_t callback)
{
	if (streamingOutput) {
		signal_handler* streamingOutputSignalHandler = obs_output_get_signal_handler(streamingOutput);

		// Connect streaming output
		for (int i = 0; i < streamingSignals.size(); i++) {
			signal_handler_connect(
			    streamingOutputSignalHandler,
			    streamingSignals.at(i)->m_signal.c_str(),
			    callback == NULL ? g_ouput_callback : callback,
			    streamingSignals.at(i));
		}
	}

	if (recordingOutput) {
		signal_handler* recordingOutputSignalHandler = obs_output_get_signal_handler(recordingOutput);

		// Connect recording output
		for (int i = 0; i < recordingSignals.size(); i++) {
			signal_handler_connect(
			    recordingOutputSignalHandler,
			    recordingSignals.at(i)->m_signal.c_str(),
			    callback == NULL ? g_ouput_callback : callback,
			    recordingSignals.at(i));
		}
	}

	if (replayBufferOutput) {
		signal_handler* replayBufferOutputSignalHandler = obs_output_get_signal_handler(replayBufferOutput);

		// Connect replay buffer output
		for (int i = 0; i < replayBufferSignals.size(); i++) {
			signal_handler_connect(
			    replayBufferOutputSignalHandler,
			    replayBufferSignals.at(i)->m_signal.c_str(),
			    callback == NULL ? g_ouput_callback : callback,
			    replayBufferSignals.at(i));
		}
	}
}

struct HotkeyInfo
{
	std::string                objectName;
	obs_hotkey_registerer_type objectType;
	std::string                hotkeyName;
	std::string                hotkeyDesc;
	obs_hotkey_id              hotkeyId;
};

void OBS_service::OBS_service_processReplayBufferHotkey()
{
	obs_enum_hotkeys(
	    [](void* data, obs_hotkey_id id, obs_hotkey_t* key) {
		    if (obs_hotkey_get_registerer_type(key) == OBS_HOTKEY_REGISTERER_OUTPUT) {
			    std::string key_name = obs_hotkey_get_name(key);
			    if (key_name.compare("ReplayBuffer.Save") == 0) {
				    obs_hotkey_enable_callback_rerouting(true);
				    obs_hotkey_trigger_routed_callback(id, true);
			    }
		    }
		    return true;
	    },
	    nullptr);
}

std::string OBS_service::OBS_service_getLastReplay()
{
	calldata_t cd = {0};

	proc_handler_t* ph = obs_output_get_proc_handler(replayBufferOutput);

	proc_handler_call(ph, "get_last_replay", &cd);
	const char* path = calldata_string(&cd, "path");

	if (path == NULL)
		path = "";

	return std::string(path);
}

bool OBS_service::useRecordingPreset()
{
	return usingRecordingPreset;
}

void OBS_service::duplicate_encoder(obs_encoder_t** dst, obs_encoder_t* src, uint64_t trackIndex)
{
	if (!src)
		return;

	if (*dst != src && *dst)
		obs_encoder_release(*dst);

	std::string name = obs_encoder_get_name(src);
	name += "-duplicate";

	if (obs_encoder_get_type(src) == OBS_ENCODER_AUDIO) {
		*dst = obs_audio_encoder_create(
		    obs_encoder_get_id(src), name.c_str(), obs_encoder_get_settings(src), trackIndex, nullptr);
	} else if (obs_encoder_get_type(src) == OBS_ENCODER_VIDEO) {
		*dst = obs_video_encoder_create(obs_encoder_get_id(src), name.c_str(), obs_encoder_get_settings(src), nullptr);
	}
}

void OBS_service::releaseStreamingOutput()
{
	blog(LOG_INFO, "releaseStreamingOutput - 0");
	std::unique_lock<std::mutex> lock(
		mtx_output_stop);

	blog(LOG_INFO, "releaseStreamingOutput - 1");
	// cv_streaming.wait_for(lock, std::chrono::seconds(60));
	// while (!ready) cv_streaming.wait(lock);
	// ready = false;

	blog(LOG_INFO, "releaseStreamingOutput - 2");
	if (twitchSoundtrackEnabled)
		stopTwitchSoundtrackAudio();
	else
		clearArchiveVodEncoder();

	blog(LOG_INFO, "releaseStreamingOutput - 3");
	twitchSoundtrackEnabled = false;

	blog(LOG_INFO, "release streaming output 0");
	obs_output_release(streamingOutput);
	streamingOutput = nullptr;
	blog(LOG_INFO, "releaseStreamingOutput - 4");
}

void OBS_service::waitReleaseWorker(bool force)
{
	if (force) {
		std::unique_lock<std::mutex> lck(mtx_output_stop);
		ready = true;
		cv_streaming.notify_all();
	}

	if (releaseWorker.joinable()) {
		releaseWorker.join();
	}
}

void OBS_service::OBS_service_createVirtualWebcam(std::string name)
{
	virtualWebcamOutput = nullptr;
	if (name.empty())
		return;

	struct obs_video_info ovi;
	if (!obs_get_video_info(&ovi))
		return;

	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "name", name.c_str());
	obs_data_set_int(settings, "width", ovi.output_width);
	obs_data_set_int(settings, "height", ovi.output_height);
	obs_data_set_double(settings, "fps", ovi.fps_num);

	virtualWebcamOutput = obs_output_create("virtual_output", "Virtual Webcam", settings, NULL);
	obs_data_release(settings);
}

void OBS_service::OBS_service_removeVirtualWebcam()
{
	if (!virtualWebcamOutput)
		return;

	obs_output_release(virtualWebcamOutput);
	virtualWebcamOutput = nullptr;
}

void OBS_service::OBS_service_startVirtualWebcam()
{
	if (!virtualWebcamOutput)
		return;
	
	if (obs_output_start(virtualWebcamOutput))
		blog(LOG_INFO, "Successfully started the Virtual Webcam Output");
	else
		blog(LOG_ERROR, "Failed to start the Virtual Webcam Output");
}

void OBS_service::OBS_service_stopVirtualWebcam()
{
	if (!virtualWebcamOutput)
		return;
	
	obs_output_stop(virtualWebcamOutput);
}
void OBS_service::stopAllOutputs()
{
	if (streamingOutput && obs_output_active(streamingOutput))
		stopStreaming(true, NULL);

	if (replayBufferOutput && obs_output_active(replayBufferOutput))
		stopReplayBuffer(true);

	if (recordingOutput && obs_output_active(recordingOutput))
		stopRecording();
}

static inline uint32_t setMixer(obs_source_t *source, const int mixerIdx, const bool checked)
{
	uint32_t mixers = obs_source_get_audio_mixers(source);
	uint32_t new_mixers = mixers;
	if(checked) {
		new_mixers |= (1 << mixerIdx);
	} else {
		new_mixers &= ~(1 << mixerIdx);
	}
	obs_source_set_audio_mixers(source, new_mixers);
	return mixers;
}

uint32_t oldMixer_desktopSource1 = 0;
uint32_t oldMixer_desktopSource2 = 0;

bool OBS_service::startTwitchSoundtrackAudio(void) {
	bool sourceExists = false;

	if (!service)
		return false;

	obs_data_t *settings = obs_service_get_settings(service);
	const char *serviceName = obs_data_get_string(settings, "service");
	obs_data_release(settings);

	if (serviceName && strcmp(serviceName, "Twitch") != 0)
		return false;

	obs_enum_sources(
		[](void *param, obs_source_t *source) {
			auto id = obs_source_get_id(source);
			if(strcmp(id, "soundtrack_source") == 0) {
				*reinterpret_cast<bool *>(param) = true;
				return false;
			}
			return true;
		},
		&sourceExists);

	if (!sourceExists)
		return false;

	// These are magic ints provided by OBS for default sources:
	// 0 is the main scene/transition which you'd see on the main preview,
	// 1-2 are desktop audio 1 and 2 as you'd see in audio settings,
	// 2-4 are mic/aux 1-3 as you'd see in audio settings
	auto desktopSource1 = obs_get_output_source(1);
	auto desktopSource2 = obs_get_output_source(2);

	// Since our plugin duplicates all of the desktop sources, we want to ensure that both of the
	// default desktop sources, provided by OBS, are not set to mix on our custom encoder track.
	oldMixer_desktopSource1 = setMixer(
		desktopSource1, kSoundtrackArchiveTrackIdx, false);
	oldMixer_desktopSource2 = setMixer(
		desktopSource2, kSoundtrackArchiveTrackIdx, false);

	obs_source_release(desktopSource1);
	obs_source_release(desktopSource2);

	if(!streamArchiveEncST) {
		streamArchiveEncST = obs_audio_encoder_create("ffmpeg_aac",
			"Soundtrack by Twitch Archive Encoder",
			nullptr,
			kSoundtrackArchiveTrackIdx,
			nullptr);
		obs_encoder_set_audio(streamArchiveEncST, obs_get_audio());
	}

	obs_output_set_audio_encoder(streamingOutput,
		streamArchiveEncST,
		kSoundtrackArchiveEncoderIdx);

	std::string currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");
	bool        isSimpleMode      = currentOutputMode.compare("Simple") == 0;
	int bitrate = 0;

	if (isSimpleMode)
		bitrate = (int)config_get_uint(
			ConfigManager::getInstance().getBasic(), "SimpleOutput", "ABitrate");
	else
		bitrate = (int)config_get_uint(
			ConfigManager::getInstance().getBasic(), "AdvOut", "Track1Bitrate");

	obs_data_t *aacSettings = obs_data_create();
	obs_data_set_int(aacSettings, "bitrate", bitrate);
	obs_encoder_update(streamArchiveEncST, aacSettings);
	obs_data_release(aacSettings);
	return true;
}

void OBS_service::stopTwitchSoundtrackAudio(void) {
	if (!service)
		return;

	obs_data_t *settings = obs_service_get_settings(service);
	const char *serviceName = obs_data_get_string(settings, "service");
	obs_data_release(settings);

	if (serviceName && strcmp(serviceName, "Twitch") != 0)
		return;

	if (!streamArchiveEncST)
		return;

	if (obs_encoder_active(streamArchiveEncST))
		return;

	if (streamingOutput && obs_output_active(streamingOutput))
		return;

	obs_encoder_release(streamArchiveEncST);
	streamArchiveEncST = nullptr;

	auto desktopSource1 = obs_get_output_source(1);
	auto desktopSource2 = obs_get_output_source(2);

	obs_source_set_audio_mixers(desktopSource1, oldMixer_desktopSource1);
	obs_source_set_audio_mixers(desktopSource2, oldMixer_desktopSource2);

	obs_source_release(desktopSource1);
	obs_source_release(desktopSource2);
}

void OBS_service::clearArchiveVodEncoder()
{
	if (streamArchiveEncVod) {
		obs_encoder_release(streamArchiveEncVod);
		streamArchiveEncVod = nullptr;
	}
}

void OBS_service::setupVodTrack(bool isSimpleMode) {
	if (!service)
		return;

	obs_data_t *settings = obs_service_get_settings(service);
	const char *serviceName = obs_data_get_string(settings, "service");
	obs_data_release(settings);

	if (serviceName && strcmp(serviceName, "Twitch") != 0)
		return;

	if (streamArchiveEncVod && obs_encoder_active(streamArchiveEncVod))
		return;

	clearArchiveVodEncoder();

	int streamTrack = 0;
	bool vodTrackEnabled = false;
	int vodTrackIndex = 1;

	if (isSimpleMode) {
		bool advanced =
			config_get_bool(ConfigManager::getInstance().getBasic(), "SimpleOutput", "UseAdvanced");
		vodTrackEnabled = advanced ?
			config_get_bool(ConfigManager::getInstance().getBasic(), "SimpleOutput", "VodTrackEnabled") : false;
		blog(LOG_INFO, "vodTrackEnabled: %d", vodTrackEnabled);
	} else {
		streamTrack =
			int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "TrackIndex")) - 1;
		vodTrackEnabled =
			config_get_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "VodTrackEnabled");
		vodTrackIndex =
			int(config_get_int(ConfigManager::getInstance().getBasic(), "AdvOut", "VodTrackIndex")) - 1;
	}

	if (vodTrackEnabled && streamTrack != vodTrackIndex) {
		std::string id;
		if (createAudioEncoder(
			&streamArchiveEncVod,
			id,
			isSimpleMode ? GetSimpleAudioBitrate() : GetAdvancedAudioBitrate(vodTrackIndex),
			ARCHIVE_NAME,
			vodTrackIndex
		)) {
			obs_encoder_set_audio(streamArchiveEncVod, obs_get_audio());
			obs_output_set_audio_encoder(streamingOutput, streamArchiveEncVod, 1);
		}
	}
}