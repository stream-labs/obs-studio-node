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

#include "osn-simple-recording.hpp"
#include "osn-encoder.hpp"
#include "osn-audio-encoder.hpp"
#include "osn-service.hpp"
#include "osn-error.hpp"
#include "shared.hpp"
#include "nodeobs_audio_encoders.h"

void osn::ISimpleRecording::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("SimpleRecording");
	cls->register_function(std::make_shared<ipc::function>(
	    "Create", std::vector<ipc::type>{}, Create));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetPath",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetPath));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetPath",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        SetPath));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetFormat",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetFormat",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        SetFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetMuxerSettings",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetMuxerSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetMuxerSettings",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String},
        SetMuxerSettings));
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
	    "GetQuality",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetQuality));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetQuality",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetQuality));
	cls->register_function(std::make_shared<ipc::function>(
	    "Start", std::vector<ipc::type>{ipc::type::UInt64}, Start));
	cls->register_function(std::make_shared<ipc::function>(
	    "Stop", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, Stop));
	cls->register_function(std::make_shared<ipc::function>(
	    "Query", std::vector<ipc::type>{ipc::type::UInt64}, Query));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetFileFormat",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetFileFormat",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetOverwrite",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetOverwrite",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetFileFormat));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetNoSpace",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetNoSpace));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetNoSpace",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetNoSpace));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetLowCPU",
        std::vector<ipc::type>{ipc::type::UInt64},
        GetLowCPU));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetLowCPU",
        std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32},
        SetLowCPU));

	srv.register_collection(cls);
}

void osn::ISimpleRecording::Create(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	uint64_t uid =
        osn::ISimpleRecording::Manager::GetInstance().allocate(new Recording());
	if (uid == UINT64_MAX) {
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Index list is full.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleRecording::GetQuality(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::ISimpleRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value((uint32_t)recording->quality));
	AUTO_DEBUG;
}

void osn::ISimpleRecording::SetQuality(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::ISimpleRecording::Manager
			::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    recording->quality = (RecQuality)args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleRecording::GetAudioEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::ISimpleRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

	uint64_t uid =
        osn::AudioEncoder::Manager::GetInstance().find(recording->audioEncoder);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::ISimpleRecording::SetAudioEncoder(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::ISimpleRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Recording reference is not valid.");
	}

    obs_encoder_t* encoder =
        osn::AudioEncoder::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!encoder) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Encoder reference is not valid.");
	}

    recording->audioEncoder = encoder;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

static inline void LoadLosslessPreset(osn::Recording* recording)
{
	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "format_name", "avi");
	obs_data_set_string(settings, "video_encoder", "utvideo");
	obs_data_set_string(settings, "audio_encoder", "pcm_s16le");

	obs_output_set_mixers(recording->output, 1);
	obs_output_update(recording->output, settings);
	obs_data_release(settings);
}

static inline void calbback(void* data, calldata_t* params)
{
	auto info =
		reinterpret_cast<osn::cbDataRec*>(data);

	if (!info)
		return;

	std::string signal = info->signal;
	auto recording = info->recording;

	if (!recording->output)
		return;

	const char* error =
		obs_output_get_last_error(recording->output);

	std::unique_lock<std::mutex> ulock(recording->signalsMtx);
	recording->signalsReceived.push({
		signal,
		(int)calldata_int(params, "code"),
		error ? std::string(error) : ""
	});
}

void osn::Recording::ConnectSignals()
{
	if(!this->output)
		return;

	signal_handler* handler = obs_output_get_signal_handler(this->output);
	for (const auto &signal: this->signals) {
		osn::cbDataRec* cd = new cbDataRec();
		cd->signal = signal;
		cd->recording = this;
		signal_handler_connect(
			handler,
			signal.c_str(),
			calbback,
			cd);
	}
}

static inline obs_data_t* UpdateRecordingSettings_x264_crf(int crf, bool lowCPU)
{
	obs_data_t* settings = obs_data_create();
	obs_data_set_int(settings, "crf", crf);
	obs_data_set_bool(settings, "use_bufsize", true);
	obs_data_set_string(settings, "rate_control", "CRF");
	obs_data_set_string(settings, "profile", "high");
	obs_data_set_string(settings, "preset", lowCPU ? "ultrafast" : "veryfast");
	return settings;
}

static inline obs_data_t* UpdateRecordingSettings_amd_cqp(int cqp)
{
	obs_data_t* settings = obs_data_create();
	obs_data_set_int(settings, "Usage", 0);
	obs_data_set_int(settings, "Profile", 100); // High
	obs_data_set_int(settings, "RateControlMethod", 0);
	obs_data_set_int(settings, "QP.IFrame", cqp);
	obs_data_set_int(settings, "QP.PFrame", cqp);
	obs_data_set_int(settings, "QP.BFrame", cqp);
	obs_data_set_int(settings, "VBVBuffer", 1);
	obs_data_set_int(settings, "VBVBuffer.Size", 100000);
	obs_data_set_double(settings, "KeyframeInterval", 2.0);
	obs_data_set_int(settings, "BFrame.Pattern", 0);
	return settings;
}

static inline obs_data_t* UpdateRecordingSettings_nvenc(int cqp)
{
	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, "rate_control", "CQP");
	obs_data_set_string(settings, "profile", "high");
	obs_data_set_string(settings, "preset", "hq");
	obs_data_set_int(settings, "cqp", cqp);
	obs_data_set_int(settings, "bitrate", 0);
	return settings;
}

static inline bool icq_available(obs_encoder_t* encoder)
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

static inline obs_data_t* UpdateRecordingSettings_qsv11(int crf, obs_encoder_t* encoder)
{
	bool icq = icq_available(encoder);
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
	return settings;
}

#define CROSS_DIST_CUTOFF 2000.0
static inline int CalcCRF(int crf, bool lowCPU = false)
{
	obs_video_info ovi = {0};
	obs_get_video_info(&ovi);
	uint64_t cx  = ovi.output_width;
	uint64_t cy  = ovi.output_height;
	double   fCX = double(cx);
	double   fCY = double(cy);

	if (lowCPU)
		crf -= 2;

	double crossDist       = sqrt(fCX * fCX + fCY * fCY);
	double crfResReduction = fmin(CROSS_DIST_CUTOFF, crossDist) / CROSS_DIST_CUTOFF;
	crfResReduction        = (1.0 - crfResReduction) * 10.0;

	return crf - int(crfResReduction);
}

static inline void UpdateRecordingSettings_crf(
	const int& crf, osn::Recording* recording)
{
	std::string id =
		obs_encoder_get_id(recording->videoEncoder);

	obs_data_t* settings = nullptr;
	if (id.compare("obs_x264") == 0) {
		settings = UpdateRecordingSettings_x264_crf(
			CalcCRF(crf, recording->lowCPU),
			recording->lowCPU);
	} else if (id.compare("jim_nvenc") == 0 || id.compare("ffmpeg_nvenc") == 0) {
		settings = UpdateRecordingSettings_nvenc(CalcCRF(crf));
	} else if (id.compare("obs_qsv11") == 0) {
		settings = UpdateRecordingSettings_qsv11(CalcCRF(crf), recording->videoEncoder);
	} else if (id.compare("amd_amf_h264") == 0) {
		settings = UpdateRecordingSettings_amd_cqp(CalcCRF(crf));
	} else {
		return;
	}

	if (!settings)
		return;

	obs_encoder_update(recording->videoEncoder, settings);
	obs_data_release(settings);
}

static inline obs_encoder_t* duplicate_encoder(obs_encoder_t* src, uint64_t trackIndex = 0)
{
	if (!src)
		return nullptr;

	obs_encoder_t* dst = nullptr;
	std::string name = obs_encoder_get_name(src);
	name += "-duplicate";

	if (obs_encoder_get_type(src) == OBS_ENCODER_AUDIO) {
		dst = obs_audio_encoder_create(
		    obs_encoder_get_id(src),
			name.c_str(),
			obs_encoder_get_settings(src), trackIndex, nullptr);
	} else if (obs_encoder_get_type(src) == OBS_ENCODER_VIDEO) {
		dst = obs_video_encoder_create(
			obs_encoder_get_id(src),
			name.c_str(),
			obs_encoder_get_settings(src), nullptr);
	}

	return dst;
}

void osn::ISimpleRecording::Start(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

	const char* ffmpegMuxer = "ffmpeg_muxer";
	if (!recording->output ||
		strcmp(obs_output_get_id(recording->output), ffmpegMuxer) == 0) {
		if (recording->output)
			obs_output_release(recording->output);
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

	std::string format = recording->format;
	std::string pathProperty = "path";

    switch(recording->quality) {
        case RecQuality::Stream: {
			if (obs_get_multiple_rendering()) {
				obs_encoder_t* videoEncDup =
					duplicate_encoder(recording->videoEncoder);
				recording->videoEncoder = videoEncDup;
				obs_encoder_t* audioEncDup =
					duplicate_encoder(recording->audioEncoder);
				recording->audioEncoder = audioEncDup;
			}
            break;
        }
        case RecQuality::HighQuality: {
			UpdateRecordingSettings_crf(16, recording);
            break;
        }
        case RecQuality::HigherQuality: {
			UpdateRecordingSettings_crf(23, recording);
            break;
        }
        case RecQuality::Lossless: {
			obs_output_release(recording->output);
			recording->output =
				obs_output_create("ffmpeg_output", "recording", nullptr, nullptr);
			recording->ConnectSignals();
			LoadLosslessPreset(recording);
			format = "avi";
			pathProperty = "url";
			break;
		}
		default: {
			PRETTY_ERROR_RETURN(ErrorCode::InvalidReference,
				"Error while loading the recording pressets.");
		}
	}

	if (!recording->audioEncoder) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Invalid audio encoder.");
	}

	obs_encoder_set_audio(recording->audioEncoder, obs_get_audio());
	obs_output_set_audio_encoder(recording->output, recording->audioEncoder, 0);

	obs_encoder_set_video(recording->videoEncoder, obs_get_video());
	obs_output_set_video_encoder(recording->output, recording->videoEncoder);

	if (!recording->path.size()) {
		PRETTY_ERROR_RETURN(
            ErrorCode::InvalidReference, "Invalid recording path.");
	}

	std::string path = recording->path;

	char lastChar = path.back();
	if (lastChar != '/' && lastChar != '\\')
		path += "/";

	path += GenerateSpecifiedFilename(
		format, recording->noSpace, recording->fileFormat);

	if (!recording->overwrite)
		FindBestFilename(path, recording->noSpace);

	obs_data_t* settings = obs_data_create();
	obs_data_set_string(settings, pathProperty.c_str(), path.c_str());
	obs_output_update(recording->output, settings);
	obs_data_release(settings);

	obs_output_start(recording->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleRecording::Stop(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::IRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}
	if (!recording->output) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid recording output.");
	}

	obs_output_stop(recording->output);

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::ISimpleRecording::GetLowCPU(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::ISimpleRecording::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
    rval.push_back(ipc::value((uint32_t)recording->lowCPU));
	AUTO_DEBUG;
}

void osn::ISimpleRecording::SetLowCPU(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	Recording* recording =
		osn::ISimpleRecording::Manager
			::GetInstance().find(args[0].value_union.ui64);
	if (!recording) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Simple recording reference is not valid.");
	}

    recording->lowCPU = args[1].value_union.ui32;

    rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}