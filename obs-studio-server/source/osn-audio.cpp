/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

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

#include "osn-audio.hpp"
#include "osn-error.hpp"
#include "shared.hpp"

// DELETE ME WHEN REMOVING NODEOBS
#include "nodeobs_configManager.hpp"

void osn::Audio::Register(ipc::server &srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Audio");
	cls->register_function(std::make_shared<ipc::function>("GetAudioContext", std::vector<ipc::type>{}, GetAudioContext));
	cls->register_function(
		std::make_shared<ipc::function>("SetAudioContext", std::vector<ipc::type>{ipc::type::UInt32, ipc::type::UInt32}, SetAudioContext));
	cls->register_function(std::make_shared<ipc::function>("GetLegacySettings", std::vector<ipc::type>{}, GetLegacySettings));
	cls->register_function(
		std::make_shared<ipc::function>("SetLegacySettings", std::vector<ipc::type>{ipc::type::UInt32, ipc::type::UInt32}, SetLegacySettings));
	srv.register_collection(cls);
}

void osn::Audio::GetAudioContext(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	obs_audio_info audio;
	if (!obs_get_audio_info(&audio)) {
		PRETTY_ERROR_RETURN(ErrorCode::Error, "No audio context is currently set.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(audio.samples_per_sec));
	rval.push_back(ipc::value((uint32_t)audio.speakers));
	AUTO_DEBUG;
}

static const char *GetSpeakers(enum speaker_layout speakers)
{
	switch (speakers) {
	case SPEAKERS_UNKNOWN:
		return "Unknown";
	case SPEAKERS_MONO:
		return "Mono";
	case SPEAKERS_STEREO:
		return "Stereo";
	case SPEAKERS_2POINT1:
		return "2.1";
	case SPEAKERS_4POINT0:
		return "4.0";
	case SPEAKERS_4POINT1:
		return "4.1";
	case SPEAKERS_5POINT1:
		return "5.1";
	case SPEAKERS_7POINT1:
		return "7.1";
	default:
		return "Stereo";
	}
}

static enum speaker_layout GetSpeakersFromStr(const std::string &value)
{
	if (value.compare("Unknown") == 0)
		return SPEAKERS_UNKNOWN;
	else if (value.compare("Mono") == 0)
		return SPEAKERS_MONO;
	else if (value.compare("Stereo") == 0)
		return SPEAKERS_STEREO;
	else if (value.compare("2.1") == 0)
		return SPEAKERS_2POINT1;
	else if (value.compare("4.0") == 0)
		return SPEAKERS_4POINT0;
	else if (value.compare("4.1") == 0)
		return SPEAKERS_4POINT1;
	else if (value.compare("5.1") == 0)
		return SPEAKERS_5POINT1;
	else if (value.compare("7.1") == 0)
		return SPEAKERS_7POINT1;
	else
		return SPEAKERS_STEREO;
}

void osn::Audio::SetAudioContext(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	if (args.size() != 2) {
		PRETTY_ERROR_RETURN(ErrorCode::Error, "Invalid number of arguments to set the audio context.");
	}

	uint32_t sampleRate = args[0].value_union.ui32;
	uint32_t speakers = args[1].value_union.ui32;

	obs_audio_info audio;
	audio.samples_per_sec = sampleRate;
	audio.speakers = (enum speaker_layout)speakers;

	if (!obs_reset_audio(&audio)) {
		blog(LOG_ERROR, "Failed to reset audio context, sampleRate: %d and speakers: %d", audio.samples_per_sec, audio.speakers);
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

static void SaveAudioSettings(obs_audio_info audio)
{
	config_set_uint(ConfigManager::getInstance().getBasic(), "Audio", "SampleRate", audio.samples_per_sec);
	config_set_string(ConfigManager::getInstance().getBasic(), "Audio", "ChannelSetup", GetSpeakers(audio.speakers));

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
}

void osn::Audio::GetLegacySettings(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	uint32_t sampleRate = config_get_uint(ConfigManager::getInstance().getBasic(), "Audio", "SampleRate");
	std::string channelSetup = config_get_string(ConfigManager::getInstance().getBasic(), "Audio", "ChannelSetup");

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(sampleRate));
	rval.push_back(ipc::value((uint32_t)GetSpeakersFromStr(channelSetup)));
	AUTO_DEBUG;
}

void osn::Audio::SetLegacySettings(void *data, const int64_t id, const std::vector<ipc::value> &args, std::vector<ipc::value> &rval)
{
	uint32_t sampleRate = args[0].value_union.ui32;
	uint32_t channelSetup = args[1].value_union.ui32;

	config_set_uint(ConfigManager::getInstance().getBasic(), "Audio", "SampleRate", sampleRate);
	config_set_string(ConfigManager::getInstance().getBasic(), "Audio", "ChannelSetup", GetSpeakers((enum speaker_layout)channelSetup));

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}