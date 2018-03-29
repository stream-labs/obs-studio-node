#include "nodeobs_service.hpp"
#include "controller.hpp"

void service::OBS_service_resetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_resetAudioContext", ipc_args, NULL, NULL);
}

void service::OBS_service_resetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_resetVideoContext", ipc_args, NULL, NULL);
}

void service::OBS_service_createAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_createAudioEncoder", ipc_args, NULL, NULL);
}

void service::OBS_service_createVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_createVideoStreamingEncoder", ipc_args, NULL, NULL);
}

void service::OBS_service_createVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_createVideoRecordingEncoder", ipc_args, NULL, NULL);
}

void service::OBS_service_createService(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_createService", ipc_args, NULL, NULL);
}

void service::OBS_service_createRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_createRecordingSettings", ipc_args, NULL, NULL);
}

void service::OBS_service_createStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_createStreamingOutput", ipc_args, NULL, NULL);
}

void service::OBS_service_createRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_createRecordingOutput", ipc_args, NULL, NULL);
}

void service::OBS_service_startStreaming(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_startStreaming", ipc_args, NULL, NULL);
}

void service::OBS_service_startRecording(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_startRecording", ipc_args, NULL, NULL);
}

void service::OBS_service_stopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;	
	bool forceStop = args[0]->ToBoolean()->BooleanValue();
	ipc_args.push_back(ipc::value(forceStop));

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_stopStreaming", ipc_args, NULL, NULL);
}

void service::OBS_service_stopRecording(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_stopRecording", ipc_args, NULL, NULL);
}

void service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_associateAudioAndVideoToTheCurrentStreamingContext", ipc_args, NULL, NULL);
}

void service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_associateAudioAndVideoToTheCurrentRecordingContext", ipc_args, NULL, NULL);
}

void service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput", ipc_args, NULL, NULL);
}

void service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput", ipc_args, NULL, NULL);
}

void service::OBS_service_setServiceToTheStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_setServiceToTheStreamingOutput", ipc_args, NULL, NULL);
}

void service::OBS_service_setRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::vector<ipc::value> ipc_args;

	Controller::GetInstance().GetConnection()->
		call("API", "OBS_service_setRecordingSettings", ipc_args, NULL, NULL);
}

/*static void OBS_service_connectOutputSignals(const v8::FunctionCallbackInfo<v8::Value>& args) {

}*/
