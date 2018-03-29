#include <node.h>
#include <nan.h>

namespace service {
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_resetAudioContext(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_resetVideoContext(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_createAudioEncoder(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_createVideoStreamingEncoder(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_createVideoRecordingEncoder(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_createService(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_createRecordingSettings(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_createStreamingOutput(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_createRecordingOutput(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_startStreaming(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_startRecording(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_stopStreaming(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_stopRecording(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_setServiceToTheStreamingOutput(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_service_setRecordingSettings(Nan::NAN_METHOD_ARGS_TYPE info);
	// static Nan::NAN_METHOD_RETURN_TYPE OBS_service_connectOutputSignals(Nan::NAN_METHOD_ARGS_TYPE info);
}