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

#include "osn-simple-streaming.hpp"
#include "osn-encoder.hpp"
#include "osn-service.hpp"
#include "error.hpp"
#include "shared.hpp"
#include "nodeobs_audio_encoders.h"

void osn::ISimpleStreaming::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("SimpleStreaming");
	cls->register_function(std::make_shared<ipc::function>(
	    "Create", std::vector<ipc::type>{}, Create));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetService",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetService));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetService",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetService));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetVideoEncoder",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetVideoEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetVideoEncoder",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetVideoEncoder));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetEnforceServiceBirate",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetEnforceServiceBirate));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetEnforceServiceBirate",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetEnforceServiceBirate));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetEnableTwitchVOD",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetEnableTwitchVOD));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetEnableTwitchVOD",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetEnableTwitchVOD));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetAudioBitrate",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetAudioBitrate));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetAudioBitrate",
        std::vector<ipc::type>{ipc::type::UInt64,ipc::type::UInt32},
        SetAudioBitrate));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetDelay",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetDelay));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetDelay",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetDelay));
	cls->register_function(std::make_shared<ipc::function>(
	    "Start", std::vector<ipc::type>{ipc::type::UInt64}, Start));
	cls->register_function(std::make_shared<ipc::function>(
	    "Stop", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, Stop));
	cls->register_function(std::make_shared<ipc::function>(
	    "Query", std::vector<ipc::type>{ipc::type::UInt64}, Query));

	srv.register_collection(cls);
}

void osn::ISimpleStreaming::Create(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	uint64_t uid =
        osn::ISimpleStreaming::Manager::GetInstance().allocate(new SimpleStreaming());
	if (uid == UINT64_MAX) {
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::GetService(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

	uint64_t uid =
        osn::Service::Manager::GetInstance().find(simpleStreaming->service);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetService(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    obs_service_t* service =
        osn::Service::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Service reference is not valid.");
	}

    simpleStreaming->service = service;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::GetVideoEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

	uint64_t uid =
        osn::Encoder::Manager::GetInstance().find(simpleStreaming->videoEncoder);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetVideoEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    obs_encoder_t* encoder =
        osn::Encoder::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!encoder) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Encoder reference is not valid.");
	}

    simpleStreaming->videoEncoder = encoder;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::GetEnforceServiceBirate(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(simpleStreaming->enforceServiceBitrate));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetEnforceServiceBirate(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    simpleStreaming->enforceServiceBitrate = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::GetEnableTwitchVOD(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(simpleStreaming->enableTwitchVOD));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetEnableTwitchVOD(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    simpleStreaming->enableTwitchVOD = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::GetAudioBitrate(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value(simpleStreaming->audioBitrate));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetAudioBitrate(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    simpleStreaming->audioBitrate = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::GetDelay(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Streaming reference is not valid.");
	}

	uint64_t uid =
        osn::IDelay::Manager::GetInstance().find(simpleStreaming->delay);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetDelay(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

    Delay* delay =
        osn::IDelay::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!delay) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Encoder reference is not valid.");
	}

    simpleStreaming->delay = delay;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

static inline obs_encoder_t* createAudioEncoder(uint32_t bitrate)
{
	obs_encoder_t* audioEncoder = nullptr;

	audioEncoder =
		obs_audio_encoder_create(GetAACEncoderForBitrate(bitrate), "audio", nullptr, 0, nullptr);

	return audioEncoder;
}

static inline void calbback(void* data, calldata_t* params)
{
	auto info =
		reinterpret_cast<osn::cbData*>(data);

	if (!info)
		return;

	std::string signal = info->signal;
	auto stream = info->stream;

	if (!stream->output)
		return;

	const char* error =
		obs_output_get_last_error(stream->output);

	std::unique_lock<std::mutex> ulock(stream->signalsMtx);
	stream->signalsReceived.push({
		signal,
		(int)calldata_int(params, "code"),
		error ? std::string(error) : ""
	});
}

void osn::ISimpleStreaming::Start(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

	if (!simpleStreaming->videoEncoder) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid video encoder.");
	}

	if (!simpleStreaming->service) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid service.");
	}

	const char* type =
		obs_service_get_output_type(simpleStreaming->service);
	if (!type)
		type = "rtmp_output";

	simpleStreaming->output =
		obs_output_create(type, "stream", nullptr, nullptr);

	if (!simpleStreaming->output) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Error while creating the streaming output.");
	}

	simpleStreaming->audioEncoder = createAudioEncoder(simpleStreaming->audioBitrate);
	if (!simpleStreaming->videoEncoder) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Error while creating the audio encoder.");
	}

	obs_encoder_set_audio(simpleStreaming->audioEncoder, obs_get_audio());
	obs_output_set_audio_encoder(simpleStreaming->output, simpleStreaming->audioEncoder, 0);
	obs_encoder_set_video(simpleStreaming->videoEncoder, obs_get_video());
	obs_output_set_video_encoder(simpleStreaming->output, simpleStreaming->videoEncoder);

	obs_output_set_service(simpleStreaming->output, simpleStreaming->service);

	signal_handler* handler = obs_output_get_signal_handler(simpleStreaming->output);

	for (const auto &signal: simpleStreaming->signals) {
		osn::cbData* cd = new cbData();
		cd->signal = signal;
		cd->stream = simpleStreaming;
		signal_handler_connect(
			handler,
			signal.c_str(),
			calbback,
			cd);
	}

	// Set delay
	if (!simpleStreaming->delay) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid delay.");
	}
	obs_output_set_delay(
	    simpleStreaming->output,
		simpleStreaming->delay->enabled ? uint32_t(simpleStreaming->delay->delaySec) : 0,
		simpleStreaming->delay->preserveDelay ? OBS_OUTPUT_DELAY_PRESERVE : 0);

	// Set network advanced settings

	obs_output_start(simpleStreaming->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::Stop(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

	if (!simpleStreaming->output) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid streaming output.");
	}

	bool force = args[1].value_union.ui32;

	if (force)
		obs_output_force_stop(simpleStreaming->output);
	else
		obs_output_stop(simpleStreaming->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::Query(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	SimpleStreaming* simpleStreaming =
		osn::ISimpleStreaming::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!simpleStreaming) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple streaming reference is not valid.");
	}

	std::unique_lock<std::mutex> ulock(simpleStreaming->signalsMtx);
	if (simpleStreaming->signalsReceived.empty()) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	auto signal = simpleStreaming->signalsReceived.front();
	rval.push_back(ipc::value("streaming"));
	rval.push_back(ipc::value(signal.signal));
	rval.push_back(ipc::value(signal.code));
	rval.push_back(ipc::value(signal.errorMessage));

	simpleStreaming->signalsReceived.pop();

	AUTO_DEBUG;
}

osn::ISimpleStreaming::Manager& osn::ISimpleStreaming::Manager::GetInstance()
{
	static osn::ISimpleStreaming::Manager _inst;
	return _inst;
}