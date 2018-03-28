#include <node.h>

namespace service {
	static void OBS_service_resetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_resetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createService(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_startStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_startRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_stopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_stopRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_setServiceToTheStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_setRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	// static void OBS_service_connectOutputSignals(const v8::FunctionCallbackInfo<v8::Value>& args);
}