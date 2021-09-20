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
#include <map>
#include <mutex>
#include <obs.h>
#include <queue>
#include <string>
#include <thread>
#include <util/config-file.h>
#include <util/dstr.h>
#include <util/platform.h>
#include "nodeobs_api-server.h"

#include "nodeobs_audio_encoders.h"

#ifdef _WIN32

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#endif
#else
#include <unistd.h>
#endif

#ifdef WIN32
#define SIMPLE_ENCODER_X264 "x264"
#elif __APPLE__
#define SIMPLE_ENCODER_X264 "obs_x264"
#endif
#define SIMPLE_ENCODER_X264_LOWCPU "x264_lowcpu"
#define SIMPLE_ENCODER_QSV "qsv"
#define SIMPLE_ENCODER_NVENC "nvenc"
#define SIMPLE_ENCODER_AMD "amd"

#define ADVANCED_ENCODER_X264 "obs_x264"
#define ADVANCED_ENCODER_QSV "obs_qsv11"
#define ADVANCED_ENCODER_NVENC "ffmpeg_nvenc"
#define ADVANCED_ENCODER_AMD "amd_amf_h264"

#define ENCODER_NEW_NVENC "jim_nvenc"

#define APPLE_SOFTWARE_VIDEO_ENCODER "com.apple.videotoolbox.videoencoder.h264"
#define APPLE_HARDWARE_VIDEO_ENCODER "com.apple.videotoolbox.videoencoder.h264.gva"

#define ARCHIVE_NAME "archive_aac"

#define MAX_AUDIO_MIXES 6

class SignalInfo {
	public:
	SignalInfo(
		std::string outputType,
		std::string signal,
		int code,
		std::string errorMessage,
		void* jsThread
	) {
		this->m_outputType = outputType;
		this->m_signal = signal;
		this->m_code = code;
		this->m_errorMessage = errorMessage;
		this->m_jsThread = jsThread;
	};
	~SignalInfo() {};


	std::string m_outputType;
	std::string m_signal;
	int m_code;
	std::string m_errorMessage;
	void* m_jsThread;
	bool sent;
	bool tosend;
};

typedef void (*callbackService)(SignalInfo* data);

extern obs_output_t* streamingOutput;
extern obs_output_t* recordingOutput;
extern obs_output_t* replayBufferOutput;
extern obs_output_t* virtualWebcamOutput;

extern bool        usingRecordingPreset;
extern bool        recordingConfigured;
extern bool        ffmpegOutput;
extern bool        lowCPUx264;
extern bool        isStreaming;
extern bool        isRecording;
extern bool        isReplayBufferActive;
extern bool        rpUsesRec;
extern bool        rpUsesStream;

extern void* g_jsThread;

extern signal_callback_t g_ouput_callback;

class OBS_service
{
	public:
	OBS_service();
	~OBS_service();

	static void OBS_service_resetAudioContext();
	static int OBS_service_resetVideoContext();
	static void OBS_service_startStreaming(callbackService callJS);
	static void OBS_service_startRecording(callbackService callJS);
	static void OBS_service_startReplayBuffer(callbackService callJS);
	static void OBS_service_stopStreaming(bool forceStop);
	static void OBS_service_stopRecording();
	static void OBS_service_stopReplayBuffer(bool forceStop);
	static void OBS_service_connectOutputSignals(signal_callback_t callbac, void* jsThread);
	static void OBS_service_processReplayBufferHotkey();
	static std::string OBS_service_getLastReplay();

	static void OBS_service_createVirtualWebcam(std::string name);
	static void OBS_service_removeVirtualWebcam();
	static void OBS_service_startVirtualWebcam();
	static void OBS_service_stopVirtualWebcam();

	private:
	static bool startStreaming(callbackService callJS);
	static void stopStreaming(bool forceStop);
	static bool startRecording(callbackService callJS);
	static bool startReplayBuffer(callbackService callJS);
	static void stopReplayBuffer(bool forceStop);
	static void stopRecording(void);

	static void releaseStreamingOutput(void);

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
	static obs_encoder_t* getArchiveEncoder(void);
	static void           setArchiveEncoder(obs_encoder_t* encoder);

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
	static obs_output_t* getVirtualWebcamOutput(void);
	static void          setVirtualWebcamOutput(obs_output_t* output);
	static void          waitReleaseWorker(void);

	// Update settings
	static void updateStreamingOutput();

	// Update video encoders
	static void updateStreamingEncoders(bool isSimpleMode);
	static bool updateRecordingEncoders(bool isSimpleMode);

	static void updateVideoStreamingEncoder(bool isSimpleMode);
	static void updateAudioStreamingEncoder(bool isSimpleMode);
	static void updateAudioRecordingEncoder(bool isSimpleMode);
	static void updateVideoRecordingEncoder(bool isSimpleMode);
	static void updateAudioTracks(void);

	// Update outputs
	static void updateFfmpegOutput(bool isSimpleMode, obs_output_t* output);
	static void UpdateFFmpegCustomOutput(void);
	static void updateReplayBufferOutput(bool isSimpleMode, bool useStreamEncoder);


	static std::string GetDefaultVideoSavePath(void);

	static bool isStreamingOutputActive(void);
	static bool isRecordingOutputActive(void);
	static bool isReplayBufferOutputActive(void);

	// Reset contexts
	static bool resetAudioContext(bool reload = false);
	static int  resetVideoContext(bool reload = false);

	static int GetSimpleAudioBitrate(void);
	static int GetAdvancedAudioBitrate(int i);

	// Output signals
	static void connectOutputSignals(signal_callback_t callback = NULL);
	static void JSCallbackOutputSignal(void* data, calldata_t*);

	static bool useRecordingPreset();

	static void duplicate_encoder(obs_encoder_t** dst, obs_encoder_t* src, uint64_t trackIndex = 0);

	static bool EncoderAvailable(const char* encoder);
	static void stopAllOutputs(void);

	static bool startTwitchSoundtrackAudio(void);
	static void stopTwitchSoundtrackAudio(void);
	static void setupVodTrack(bool isSimpleMode);
	static void clearArchiveVodEncoder();
};
