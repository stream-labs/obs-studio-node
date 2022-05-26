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

#include "osn-advanced-replay-buffer.hpp"
#include "osn-video-encoder.hpp"
#include "osn-error.hpp"
#include "shared.hpp"
#include "osn-audio-track.hpp"

void osn::IAdvancedReplayBuffer::Register(ipc::server& srv)
{
    std::shared_ptr<ipc::collection> cls =
        std::make_shared<ipc::collection>("AdvancedReplayBuffer");
    cls->register_function(std::make_shared<ipc::function>(
        "Create", std::vector<ipc::type>{}, Create));
    cls->register_function(std::make_shared<ipc::function>(
        "GetDuration",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetDuration));
    cls->register_function(std::make_shared<ipc::function>(
        "SetDuration",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetDuration));
    cls->register_function(std::make_shared<ipc::function>(
        "GetPrefix",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetPrefix));
    cls->register_function(std::make_shared<ipc::function>(
        "SetPrefix",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        SetPrefix));
    cls->register_function(std::make_shared<ipc::function>(
        "GetSuffix",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetSuffix));
    cls->register_function(std::make_shared<ipc::function>(
        "SetSuffix",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        SetSuffix));
    cls->register_function(std::make_shared<ipc::function>(
        "GetUsesStream",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetUsesStream));
    cls->register_function(std::make_shared<ipc::function>(
        "SetUsesStream",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetUsesStream));
    cls->register_function(std::make_shared<ipc::function>(
        "GetMixer",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetMixer));
    cls->register_function(std::make_shared<ipc::function>(
        "SetMixer",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetMixer));
    cls->register_function(std::make_shared<ipc::function>(
        "Save",
        std::vector<ipc::type>{ipc::type::UInt64},
        Save));
    cls->register_function(std::make_shared<ipc::function>(
        "Start", std::vector<ipc::type>{ipc::type::UInt64}, Start));
    cls->register_function(std::make_shared<ipc::function>(
        "Stop", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, Stop));
    cls->register_function(std::make_shared<ipc::function>(
        "Query", std::vector<ipc::type>{ipc::type::UInt64}, Query));
    cls->register_function(std::make_shared<ipc::function>(
        "GetLegacySettings", std::vector<ipc::type>{}, GetLegacySettings));

    srv.register_collection(cls);
}

void osn::IAdvancedReplayBuffer::Create(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    uint64_t uid =
        osn::IAdvancedReplayBuffer::Manager::GetInstance().allocate(new AdvancedReplayBuffer());
    if (uid == UINT64_MAX) {
        PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
    }

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(uid));
    AUTO_DEBUG;
}

void osn::IAdvancedReplayBuffer::GetMixer(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    AdvancedReplayBuffer* replayBuffer =
        static_cast<AdvancedReplayBuffer*>(
            osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
    if (!replayBuffer) {
        PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Advanced replay buffer reference is not valid.");
    }

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(replayBuffer->mixer));
    AUTO_DEBUG;
}

void osn::IAdvancedReplayBuffer::SetMixer(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    AdvancedReplayBuffer* replayBuffer =
        static_cast<AdvancedReplayBuffer*>(
            osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
    if (!replayBuffer) {
        PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Advanced replay buffer reference is not valid.");
    }

    replayBuffer->mixer = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    AUTO_DEBUG;
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

void osn::IAdvancedReplayBuffer::Start(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    AdvancedReplayBuffer* replayBuffer =
        static_cast<AdvancedReplayBuffer*>(
            osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
    if (!replayBuffer) {
        PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple replay buffer reference is not valid.");
    }

    if (!replayBuffer->output)
        replayBuffer->createOutput("replay_buffer", "replay-buffer");

    int idx    = 0;
    for (int i = 0; i < MAX_AUDIO_MIXES; i++) {
        osn::AudioTrack* audioTrack =
            osn::IAudioTrack::audioTracks[i];
        if ((replayBuffer->mixer & (1 << i)) != 0 &&
                audioTrack && audioTrack->audioEnc) {
            obs_encoder_set_audio(audioTrack->audioEnc, obs_get_audio());
            obs_output_set_audio_encoder(replayBuffer->output, audioTrack->audioEnc, idx);
            idx++;
        }
    }

    obs_encoder_t* videoEncoder = nullptr;
    if (obs_get_multiple_rendering() && replayBuffer->usesStream) {
        if (!replayBuffer->streaming)
            return;
        videoEncoder = replayBuffer->streaming->videoEncoder;
    }
    else {
        if (!replayBuffer->recording)
        videoEncoder = replayBuffer->recording->videoEncoder;
    }

    if (!videoEncoder) {
        PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Invalid video encoder.");
    }

    obs_encoder_set_video(videoEncoder, obs_get_video());
    obs_output_set_video_encoder(replayBuffer->output, videoEncoder);

    if (!replayBuffer->path.size()) {
        PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Invalid recording path.");
    }

    const char* rbPrefix = replayBuffer->prefix.c_str();
    const char* rbSuffix = replayBuffer->suffix.c_str();
    int64_t rbSize = replayBuffer->usesStream ? 0 : 512;

    std::string f;
    if (rbPrefix && *rbPrefix) {
        f += rbPrefix;
        if (f.back() != ' ')
            f += " ";
    }
    f += replayBuffer->fileFormat.c_str();
    if (rbSuffix && *rbSuffix) {
        if (*rbSuffix != ' ')
            f += " ";
        f += rbSuffix;
    }
    remove_reserved_file_characters(f);

    obs_data_t* settings = obs_data_create();
    obs_data_set_string(settings, "directory", replayBuffer->path.c_str());
    obs_data_set_string(settings, "format", f.c_str());
    obs_data_set_string(settings, "extension", replayBuffer->format.c_str());
    obs_data_set_bool(settings, "allow_spaces", !replayBuffer->noSpace);
    obs_data_set_int(settings, "max_time_sec", replayBuffer->duration);
    obs_data_set_int(settings, "max_size_mb", rbSize);
    obs_output_update(replayBuffer->output, settings);
    obs_data_release(settings);

    replayBuffer->startOutput();

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    AUTO_DEBUG;
}

void osn::IAdvancedReplayBuffer::Stop(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    AdvancedReplayBuffer* replayBuffer =
        static_cast<AdvancedReplayBuffer*>(
            osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
    if (!replayBuffer) {
        PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple replay buffer reference is not valid.");
    }
    if (!replayBuffer->output) {
        PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid replay buffer output.");
    }

    obs_output_stop(replayBuffer->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    AUTO_DEBUG;
}

void osn::IAdvancedReplayBuffer::GetLegacySettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
    osn::AdvancedReplayBuffer* replayBuffer =
        new osn::AdvancedReplayBuffer();

    replayBuffer->path =
        config_get_string(
            ConfigManager::getInstance().getBasic(),
            "AdvOut", "RecFilePath");
    replayBuffer->format =
        config_get_string(
            ConfigManager::getInstance().getBasic(),
            "AdvOut", "RecFormat");
    replayBuffer->muxerSettings =
        config_get_string(
            ConfigManager::getInstance().getBasic(),
            "AdvOut", "RecMuxerCustom");
    replayBuffer->noSpace =
        config_get_bool(
            ConfigManager::getInstance().getBasic(),
            "AdvOut", "FileNameWithoutSpace");
    replayBuffer->fileFormat =
        config_get_string(
            ConfigManager::getInstance().getBasic(),
            "Output", "FilenameFormatting");
    replayBuffer->overwrite =
        config_get_bool(
            ConfigManager::getInstance().getBasic(),
            "Output", "OverwriteIfExists");

    replayBuffer->prefix =
        config_get_string(
            ConfigManager::getInstance().getBasic(),
            "SimpleOutput", "RecRBPrefix");
    replayBuffer->suffix =
        config_get_string(
            ConfigManager::getInstance().getBasic(),
            "SimpleOutput", "RecRBSuffix");
    replayBuffer->duration =
        config_get_int(
            ConfigManager::getInstance().getBasic(),
            "AdvOut", "RecRBTime");

    replayBuffer->mixer =
        config_get_int(
            ConfigManager::getInstance().getBasic(),
            "AdvOut", "RecTracks");

    replayBuffer->usesStream =
        config_get_bool(
            ConfigManager::getInstance().getBasic(),
            "AdvOut", "replayBufferUseStreamOutput");

    uint64_t uid =
        osn::IAdvancedReplayBuffer::Manager::GetInstance().
            allocate(replayBuffer);
    if (uid == UINT64_MAX) {
        PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
    }

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(uid));
    AUTO_DEBUG;
}