#pragma once

#include <algorithm>
#include <iostream>
#include <ipc-server.hpp>
#include <map>
#include <mutex>
#include <obs.h>
#include <queue>
#include <string>
#include <thread>
#include <util/config-file.h>
#include <util/dstr.h>
#include <util/platform.h>
#include "nodeobs_api.h"

#include "nodeobs_audio_encoders.h"

#ifdef _WIN32

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#endif
#else
#include <unistd.h>
#endif

#define SIMPLE_ENCODER_X264 "x264"
#define SIMPLE_ENCODER_X264_LOWCPU "obs_x264"
#define SIMPLE_ENCODER_QSV "qsv"
#define SIMPLE_ENCODER_NVENC "nvenc"
#define SIMPLE_ENCODER_AMD "amd"

#define ADVANCED_ENCODER_X264 "obs_x264"
#define ADVANCED_ENCODER_QSV "obs_qsv11"
#define ADVANCED_ENCODER_NVENC "ffmpeg_nvenc"
#define ADVANCED_ENCODER_AMD "amd_amf_h264"

using namespace std;

class SignalInfo
{
	private:
	std::string m_outputType;
	std::string m_signal;
	int         m_code;
	std::string m_errorMessage;

	public:
	SignalInfo(){};
	SignalInfo(std::string outputType, std::string signal)
	{
		m_outputType   = outputType;
		m_signal       = signal;
		m_code         = 0;
		m_errorMessage = "";
	}
	std::string getOutputType(void)
	{
		return m_outputType;
	};
	std::string getSignal(void)
	{
		return m_signal;
	};

	int getCode(void)
	{
		return m_code;
	};
	void setCode(int code)
	{
		m_code = code;
	};
	std::string getErrorMessage(void)
	{
		return m_errorMessage;
	};
	void setErrorMessage(std::string errorMessage)
	{
		m_errorMessage = errorMessage;
	};
};

class OBS_service
{
	public:
	OBS_service();
	~OBS_service();

	static void Register(ipc::server&);

	static void OBS_service_resetAudioContext(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_resetVideoContext(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_createAudioEncoder(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_createVideoStreamingEncoder(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_createVideoRecordingEncoder(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_createService(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_createRecordingSettings(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_createStreamingOutput(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_createRecordingOutput(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_startStreaming(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_startRecording(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_stopStreaming(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_stopRecording(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_setServiceToTheStreamingOutput(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_setRecordingSettings(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_connectOutputSignals(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void Query(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

	private:
	static obs_data_t* createRecordingSettings(void);
	static bool        startStreaming(void);
	static bool        startRecording(void);
	static void        stopStreaming(bool forceStop);
	static void        stopRecording(void);
	static void        setRecordingSettings(void);

	static void LoadRecordingPreset_h264(const char* encoder);
	static void LoadRecordingPreset_Lossless(void);
	// static void LoadRecordingPreset(void);

	static void UpdateRecordingSettings_x264_crf(int crf);
	static void UpdateRecordingSettings_qsv11(int crf);
	static void UpdateRecordingSettings_nvenc(int cqp);
	static void UpdateStreamingSettings_amd(obs_data_t* settings, int bitrate);
	static void UpdateRecordingSettings_amd_cqp(int cqp);
	static void UpdateRecordingSettings(void);

	public:
	// Service
	static void           createService();
	static obs_service_t* getService(void);
	static void           setService(obs_service_t* newService);
	static void           saveService(void);
	static void           updateService(void);
	static void           setServiceToTheStreamingOutput(void);

	// Encoders
	static void           createAudioEncoder(obs_encoder_t** audioEncoder);
	static void           createVideoStreamingEncoder();
	static void           createVideoRecordingEncoder();
	static obs_encoder_t* getStreamingEncoder(void);
	static void           setStreamingEncoder(obs_encoder_t* encoder);
	static obs_encoder_t* getRecordingEncoder(void);
	static void           setRecordingEncoder(obs_encoder_t* encoder);
	static obs_encoder_t* getAudioStreamingEncoder(void);
	static void           setAudioStreamingEncoder(obs_encoder_t* encoder);
	static obs_encoder_t* getAudioRecordingEncoder(void);
	static void           setAudioRecordingEncoder(obs_encoder_t* encoder);

	// Outputs
	static void          createStreamingOutput(void);
	static void          createRecordingOutput(void);
	static obs_output_t* getStreamingOutput(void);
	static void          setStreamingOutput(obs_output_t* output);
	static obs_output_t* getRecordingOutput(void);
	static void          setRecordingOutput(obs_output_t* output);

	// Update settings
	static void updateStreamSettings(void);
	static void updateRecordSettings(void);

	// Update video encoders
	static void updateVideoStreamingEncoder(void);
	static void updateVideoRecordingEncoder(void);

	// Update outputs
	static void updateStreamingOutput(void);
	static void updateRecordingOutput(void);
	static void updateAdvancedRecordingOutput(void);
	static void UpdateFFmpegOutput(void);

	static std::string GetDefaultVideoSavePath(void);

	static bool isStreamingOutputActive(void);

	// Reset contexts
	static bool resetAudioContext(bool reload = false);
	static int  resetVideoContext(bool reload = false);

	static void associateAudioAndVideoToTheCurrentStreamingContext(void);
	static void associateAudioAndVideoToTheCurrentRecordingContext(void);
	static void associateAudioAndVideoEncodersToTheCurrentStreamingOutput(void);
	static void associateAudioAndVideoEncodersToTheCurrentRecordingOutput(void);

	static int GetAudioBitrate(void);

	// Output signals
	static void connectOutputSignals(void);
	static void JSCallbackOutputSignal(void* data, calldata_t*);
};
