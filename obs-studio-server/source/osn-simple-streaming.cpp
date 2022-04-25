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
	    "GetReconnect",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetReconnect));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetReconnect",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetReconnect));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetNetwork",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetNetwork));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetNetwork",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64},
        SetNetwork));
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

void osn::ISimpleStreaming::GetReconnect(
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
        osn::IReconnect::Manager::GetInstance().find(simpleStreaming->reconnect);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetReconnect(
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

    Reconnect* reconnect =
        osn::IReconnect::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!reconnect) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Reconnect reference is not valid.");
	}

    simpleStreaming->reconnect = reconnect;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::GetNetwork(
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
        osn::INetwork::Manager::GetInstance().find(simpleStreaming->network);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleStreaming::SetNetwork(
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

    Network* network =
        osn::INetwork::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!network) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Network reference is not valid.");
	}

    simpleStreaming->network = network;

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

void osn::SimpleStreaming::ConnectSignals()
{
	if(!this->output)
		return;

	signal_handler* handler = obs_output_get_signal_handler(this->output);
	for (const auto &signal: this->signals) {
		osn::cbData* cd = new cbData();
		cd->signal = signal;
		cd->stream = this;
		signal_handler_connect(
			handler,
			signal.c_str(),
			calbback,
			cd);
	}
}

bool osn::SimpleStreaming::isTwitchVODSupported()
{
	if (!service)
		return false;

	obs_data_t *settings = obs_service_get_settings(service);
	const char *serviceName = obs_data_get_string(settings, "service");
	obs_data_release(settings);

	if (serviceName && strcmp(serviceName, "Twitch") != 0)
		return false;

	bool sourceExists = false;
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
}

static constexpr int kSoundtrackArchiveEncoderIdx = 1;
static constexpr int kSoundtrackArchiveTrackIdx = 5;

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

void osn::SimpleStreaming::SetupTwitchSoundtrackAudio()
{
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

	if (streamArchive && obs_encoder_active(streamArchive))
		return;

	if (!streamArchive) {
		streamArchive = obs_audio_encoder_create("ffmpeg_aac",
			"Soundtrack by Twitch Archive Encoder",
			nullptr,
			kSoundtrackArchiveTrackIdx,
			nullptr);
		obs_encoder_set_audio(streamArchive, obs_get_audio());
	}

	obs_output_set_audio_encoder(output, streamArchive, kSoundtrackArchiveEncoderIdx);

	obs_data_t *settings = obs_data_create();
	obs_data_set_int(settings, "bitrate", audioBitrate);
	obs_encoder_update(streamArchive, settings);
	obs_data_release(settings);
}

void osn::SimpleStreaming::StopTwitchSoundtrackAudio()
{
	if (streamArchive) {
		obs_encoder_release(streamArchive);
		streamArchive = nullptr;
	}

	auto desktopSource1 = obs_get_output_source(1);
	auto desktopSource2 = obs_get_output_source(2);

	obs_source_set_audio_mixers(desktopSource1, oldMixer_desktopSource1);
	obs_source_set_audio_mixers(desktopSource2, oldMixer_desktopSource2);

	obs_source_release(desktopSource1);
	obs_source_release(desktopSource2);
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

	if (!simpleStreaming->output) {
		simpleStreaming->output =
			obs_output_create(type, "stream", nullptr, nullptr);
		simpleStreaming->ConnectSignals();
	} else if (strcmp(obs_output_get_id(simpleStreaming->output), type) != 0) {
		if (obs_output_active(simpleStreaming->output)) {
			PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Current streaming output is active.");
		}
		obs_output_release(simpleStreaming->output);
		simpleStreaming->output =
			obs_output_create(type, "stream", nullptr, nullptr);
		simpleStreaming->ConnectSignals();
	}

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

	if (simpleStreaming->enableTwitchVOD) {
		simpleStreaming->twitchVODSupported = simpleStreaming->isTwitchVODSupported();
		if (simpleStreaming->twitchVODSupported)
			simpleStreaming->SetupTwitchSoundtrackAudio();
	}

	obs_output_set_service(simpleStreaming->output, simpleStreaming->service);

	if (!simpleStreaming->delay) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid delay.");
	}
	obs_output_set_delay(
	    simpleStreaming->output,
		simpleStreaming->delay->enabled ? uint32_t(simpleStreaming->delay->delaySec) : 0,
		simpleStreaming->delay->preserveDelay ? OBS_OUTPUT_DELAY_PRESERVE : 0);

	if (!simpleStreaming->reconnect) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid reconnect.");
	}
	uint32_t maxReties = 
		simpleStreaming->reconnect->enabled ?
		simpleStreaming->reconnect->maxRetries :
		0;
	obs_output_set_reconnect_settings(
		simpleStreaming->output,
		maxReties,
		simpleStreaming->reconnect->retryDelay);

	if (!simpleStreaming->network) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid network.");
	}

	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings,
		"bind_ip", simpleStreaming->network->bindIP.c_str());
	obs_data_set_bool(settings,
		"dyn_bitrate", simpleStreaming->network->enableDynamicBitrate);
	obs_data_set_bool(settings,
		"new_socket_loop_enabled", simpleStreaming->network->enableOptimizations);
	obs_data_set_bool(settings,
		"low_latency_mode_enabled", simpleStreaming->network->enableLowLatency);
	obs_output_update(simpleStreaming->output, settings);
	obs_data_release(settings);

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