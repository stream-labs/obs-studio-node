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
#define SIMPLE_ENCODER_X264_LOWCPU "x264_lowcpu"
#define SIMPLE_ENCODER_QSV "qsv"
#define SIMPLE_ENCODER_NVENC "nvenc"
#define SIMPLE_ENCODER_AMD "amd"

#define ADVANCED_ENCODER_X264 "obs_x264"
#define ADVANCED_ENCODER_QSV "obs_qsv11"
#define ADVANCED_ENCODER_NVENC "ffmpeg_nvenc"
#define ADVANCED_ENCODER_AMD "amd_amf_h264"

#define ENCODER_NEW_NVENC "jim_nvenc"

#define MAX_AUDIO_MIXES 6

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
	static void OBS_service_startReplayBuffer(
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
	static void OBS_service_stopReplayBuffer(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_connectOutputSignals(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_processReplayBufferHotkey(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_service_getLastReplay(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void Query(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

	private:
	static bool startStreaming(void);
	static void stopStreaming(bool forceStop);
	static bool startRecording(void);
	static bool startReplayBuffer(void);
	static void stopReplayBuffer(bool forceStop);
	static void stopRecording(void);
	static void setRecordingSettings(void);

	static void LoadRecordingPreset_h264(const char* encoder);
	static void LoadRecordingPreset_Lossless(void);
	// static void LoadRecordingPreset(void);

	static void UpdateRecordingSettings_x264_crf(int crf);
	static void UpdateRecordingSettings_qsv11(int crf);
	static void UpdateRecordingSettings_nvenc(int cqp);
	static void UpdateStreamingSettings_amd(obs_data_t* settings, int bitrate);
	static void UpdateRecordingSettings_amd_cqp(int cqp);
	static void updateVideoRecordingEncoderSettings(void);

	public:
	// Service
	static bool           createService();
	static obs_service_t* getService(void);
	static void           setService(obs_service_t* newService);
	static void           saveService(void);
	static void           updateService(void);

	// Encoders
	static bool           createAudioEncoder(obs_encoder_t** audioEncoder, std::string& id, int bitrate, const char* name, size_t idx);
	static bool           createVideoStreamingEncoder();
	static void           createSimpleAudioStreamingEncoder();
	static bool           createVideoRecordingEncoder();
	static obs_encoder_t* getStreamingEncoder(void);
	static void           setStreamingEncoder(obs_encoder_t* encoder);
	static obs_encoder_t* getRecordingEncoder(void);
	static void           setRecordingEncoder(obs_encoder_t* encoder);
	static obs_encoder_t* getAudioSimpleStreamingEncoder(void);
	static void           setAudioSimpleStreamingEncoder(obs_encoder_t* encoder);
	static obs_encoder_t* getAudioSimpleRecordingEncoder(void);
	static void           setAudioSimpleRecordingEncoder(obs_encoder_t* encoder);
	static obs_encoder_t* getAudioAdvancedStreamingEncoder(void);
	static void           setAudioAdvancedStreamingEncoder(obs_encoder_t* encoder);
	static void           setupAudioEncoder(void);
	static void           clearAudioEncoder(void);

	// Outputs
	static bool          createStreamingOutput(void);
	static bool          createRecordingOutput(void);
	static void          createReplayBufferOutput(void);
	static obs_output_t* getStreamingOutput(void);
	static void          setStreamingOutput(obs_output_t* output);
	static obs_output_t* getRecordingOutput(void);
	static void          setRecordingOutput(obs_output_t* output);
	static obs_output_t* getReplayBufferOutput(void);
	static void          setReplayBufferOutput(obs_output_t* output);

	// Update settings
	static void updateStreamingOutput(bool isSimpleMode);
	static void updateRecordSettings(bool replayBuffer = false);
	static void updateAdvancedReplayBuffer(void);

	// Update video encoders
	static void updateVideoStreamingEncoder(bool isSimpleMode);
	static void updateAudioStreamingEncoder(bool isSimpleMode);
	static void updateAudioRecordingEncoder(bool isSimpleMode);
	static void updateVideoRecordingEncoder(bool isSimpleMode);
	static void updateAudioTracks(void);

	// Update outputs
	static void updateRecordingOutput(void);
	static void updateAdvancedRecordingOutput(void);
	static void UpdateFFmpegOutput(void);

	static std::string GetDefaultVideoSavePath(void);

	static bool isStreamingOutputActive(void);
	static bool isRecordingOutputActive(void);
	static bool isReplayBufferOutputActive(void);

	// Reset contexts
	static bool resetAudioContext(bool reload = false);
	static int  resetVideoContext(bool reload = false);

	static void associateAudioAndVideoToTheCurrentStreamingContext(void);
	static void associateAudioAndVideoToTheCurrentRecordingContext(void);
	static void associateAudioAndVideoEncodersToTheCurrentStreamingOutput(void);
	static void associateAudioAndVideoEncodersToTheCurrentRecordingOutput(bool useStreamingEncoder, bool replayBuffer);

	static int GetSimpleAudioBitrate(void);
	static int GetAdvancedAudioBitrate(int i);

	// Output signals
	static void connectOutputSignals(void);
	static void JSCallbackOutputSignal(void* data, calldata_t*);

	static bool useRecordingPreset();

	static void duplicate_encoder(obs_encoder_t** dst, obs_encoder_t* src, uint64_t trackIndex = 0);
};
