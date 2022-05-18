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

#include "osn-simple-replay-buffer.hpp"
#include "osn-encoder.hpp"
#include "osn-audio-encoder.hpp"
#include "osn-error.hpp"
#include "shared.hpp"
#include "nodeobs_audio_encoders.h"
#include "osn-simple-recording.hpp"

void osn::ISimpleReplayBuffer::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls =
		std::make_shared<ipc::collection>("SimpleReplayBuffer");
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
	    "GetVideoEncoder",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetVideoEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetVideoEncoder",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetVideoEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetAudioEncoder",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetAudioEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetAudioEncoder",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetAudioEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "Save",
        std::vector<ipc::type>{ipc::type::UInt64},
        Save));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetLastReplay",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetLastReplay));
	cls->register_function(std::make_shared<ipc::function>(
	    "Start", std::vector<ipc::type>{ipc::type::UInt64}, Start));
	cls->register_function(std::make_shared<ipc::function>(
	    "Stop", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, Stop));
	cls->register_function(std::make_shared<ipc::function>(
	    "Query", std::vector<ipc::type>{ipc::type::UInt64}, Query));

	srv.register_collection(cls);
}

void osn::ISimpleReplayBuffer::Create(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	uint64_t uid =
        osn::ISimpleReplayBuffer::Manager::GetInstance().allocate(new ReplayBuffer());
	if (uid == UINT64_MAX) {
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleReplayBuffer::GetAudioEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	ReplayBuffer* replayBuffer =
		static_cast<ReplayBuffer*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!replayBuffer) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Replay buffer reference is not valid.");
	}

	uint64_t uid =
        osn::AudioEncoder::Manager::GetInstance().find(replayBuffer->audioEncoder);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleReplayBuffer::SetAudioEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	ReplayBuffer* replayBuffer =
		static_cast<ReplayBuffer*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!replayBuffer) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Replay buffer reference is not valid.");
	}

    obs_encoder_t* encoder =
        osn::AudioEncoder::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!encoder) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Encoder reference is not valid.");
	}

    replayBuffer->audioEncoder = encoder;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

static inline void calbback(void* data, calldata_t* params)
{
	auto info =
		reinterpret_cast<osn::cbDataRb*>(data);

	if (!info)
		return;

	std::string signal = info->signal;
	auto replayBuffer = info->replayBuffer;

	if (!replayBuffer->output)
		return;

	const char* error =
		obs_output_get_last_error(replayBuffer->output);

	std::unique_lock<std::mutex> ulock(replayBuffer->signalsMtx);
	replayBuffer->signalsReceived.push({
		signal,
		(int)calldata_int(params, "code"),
		error ? std::string(error) : ""
	});
}

void osn::ReplayBuffer::ConnectSignals()
{
	if(!this->output)
		return;

	signal_handler* handler = obs_output_get_signal_handler(this->output);
	for (const auto &signal: this->signals) {
		osn::cbDataRb* cd = new cbDataRb();
		cd->signal = signal;
		cd->replayBuffer = this;
		signal_handler_connect(
			handler,
			signal.c_str(),
			calbback,
			cd);
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

void osn::ISimpleReplayBuffer::Start(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	ReplayBuffer* replayBuffer =
		static_cast<ReplayBuffer*>(
			osn::IFileOutput::Manager::GetInstance().find(args[0].value_union.ui64));
	if (!replayBuffer) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple replay buffer reference is not valid.");
	}

	if (!replayBuffer->output) {
		replayBuffer->output =
			obs_output_create("replay_buffer", "replay-buffer", nullptr, nullptr);
		replayBuffer->ConnectSignals();
	}

	if (!replayBuffer->audioEncoder) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Invalid audio encoder.");
	}

	obs_encoder_set_audio(replayBuffer->audioEncoder, obs_get_audio());
	obs_output_set_audio_encoder(replayBuffer->output, replayBuffer->audioEncoder, 0);

	if (!replayBuffer->videoEncoder) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Invalid video encoder.");
	}

	obs_encoder_set_video(replayBuffer->videoEncoder, obs_get_video());
	obs_output_set_video_encoder(replayBuffer->output, replayBuffer->videoEncoder);

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

	obs_output_start(replayBuffer->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleReplayBuffer::Stop(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	ReplayBuffer* replayBuffer =
		static_cast<ReplayBuffer*>(
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