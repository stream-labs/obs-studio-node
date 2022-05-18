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

#include "osn-advanced-recording.hpp"
#include "osn-error.hpp"
#include "shared.hpp"
#include "osn-audio-track.hpp"
#include "osn-file-output.hpp"

void osn::IAdvancedRecording::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("AdvancedRecording");
	cls->register_function(std::make_shared<ipc::function>(
	    "Create", std::vector<ipc::type>{}, Create));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetPath",
        std::vector<ipc::type>{ipc::type::UInt64},
        IFileOutput::GetPath));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetPath",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        IFileOutput::SetPath));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetFormat",
        std::vector<ipc::type>{ipc::type::UInt64},
        IFileOutput::GetFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetFormat",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        IFileOutput::SetFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetMuxerSettings",
        std::vector<ipc::type>{ipc::type::UInt64},
        IFileOutput::GetMuxerSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetMuxerSettings",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        IFileOutput::SetMuxerSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetFileFormat",
        std::vector<ipc::type>{ipc::type::UInt64},
        IFileOutput::GetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetFileFormat",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        IFileOutput::SetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetOverwrite",
        std::vector<ipc::type>{ipc::type::UInt64},
        IFileOutput::GetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetOverwrite",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        IFileOutput::SetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetNoSpace",
        std::vector<ipc::type>{ipc::type::UInt64},
        IFileOutput::GetNoSpace));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetNoSpace",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        IFileOutput::SetNoSpace));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetVideoEncoder",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetVideoEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetVideoEncoder",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetVideoEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetMixer",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetMixer));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetMixer",
        std::vector<ipc::type>{ipc::type::UInt64,ipc::type::UInt32},
        SetMixer));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetRescaling",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetRescaling));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetRescaling",
        std::vector<ipc::type>{ipc::type::UInt64,ipc::type::UInt32},
        SetRescaling));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetOutputWidth",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetOutputWidth));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetOutputWidth",
        std::vector<ipc::type>{ipc::type::UInt64,ipc::type::UInt32},
        SetOutputWidth));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetOutputHeight",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetOutputHeight));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetOutputHeight",
        std::vector<ipc::type>{ipc::type::UInt64,ipc::type::UInt32},
        SetOutputHeight));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetUseStreamEncoders",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetUseStreamEncoders));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetUseStreamEncoders",
        std::vector<ipc::type>{ipc::type::UInt64,ipc::type::UInt32},
        SetUseStreamEncoders));
	cls->register_function(std::make_shared<ipc::function>(
	    "Start", std::vector<ipc::type>{ipc::type::UInt64}, Start));
	cls->register_function(std::make_shared<ipc::function>(
	    "Stop", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, Stop));
	cls->register_function(std::make_shared<ipc::function>(
	    "Query", std::vector<ipc::type>{ipc::type::UInt64}, Query));

	srv.register_collection(cls);
}

void osn::IAdvancedRecording::Create(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	uint64_t uid =
        osn::IAdvancedRecording::Manager::GetInstance().allocate(new Recording());
	if (uid == UINT64_MAX) {
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::GetRescaling(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->rescaling));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::SetRescaling(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    recording->rescaling = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::GetOutputWidth(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->outputWidth));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::SetOutputWidth(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    recording->outputWidth = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::GetOutputHeight(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->outputHeight));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::SetOutputHeight(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference,"Simple recording reference is not valid.");
	}

    recording->outputHeight = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::Start(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

	if (!recording->output) {
		recording->output =
			obs_output_create("ffmpeg_muxer", "recording", nullptr, nullptr);
		recording->ConnectSignals();
	}

	if (!recording->output) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Error while creating the recording output.");
	}

	if (!recording->videoEncoder) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Invalid video encoder.");
	}

	int idx    = 0;
	for (int i = 0; i < MAX_AUDIO_MIXES; i++) {
		osn::AudioTrack* audioTrack =
			osn::IAudioTrack::audioTracks[i];
		if ((recording->mixer & (1 << i)) != 0 &&
				audioTrack && audioTrack->audioEnc) {
			obs_encoder_set_audio(audioTrack->audioEnc, obs_get_audio());
			obs_output_set_audio_encoder(recording->output, audioTrack->audioEnc, idx);
			idx++;
		}
	}

	if (recording->useStreamEncoders && obs_get_multiple_rendering()) {
		obs_encoder_t* videoEncDup =
			duplicate_encoder(recording->videoEncoder);
		recording->videoEncoder = videoEncDup;
	}

	obs_encoder_set_video(recording->videoEncoder, obs_get_video());
	obs_output_set_video_encoder(recording->output, recording->videoEncoder);

	std::string path = recording->path;

	char lastChar = path.back();
	if (lastChar != '/' && lastChar != '\\')
		path += "/";

	path += GenerateSpecifiedFilename(
		recording->format, recording->noSpace, recording->fileFormat);

	if (!recording->overwrite)
		FindBestFilename(path, recording->noSpace);

	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "path", path.c_str());
	obs_output_update(recording->output, settings);
	obs_data_release(settings);

	obs_output_start(recording->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::Stop(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

	obs_output_stop(recording->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::GetMixer(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->mixer));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::SetMixer(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    recording->mixer = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::GetUseStreamEncoders(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(recording->useStreamEncoders));
	AUTO_DEBUG;
}

void osn::IAdvancedRecording::SetUseStreamEncoders(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		static_cast<Recording*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!recording) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    recording->useStreamEncoders = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}