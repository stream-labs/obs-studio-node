#include "nodeobs_service.hpp"
#include "controller.hpp"
#include "utility-v8.hpp"
#include "error.hpp"

#include "shared.hpp"
#include "utility.hpp"
#include <string>
#include <sstream>
#include <node.h>

void service::OBS_service_resetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_resetAudioContext", {});

	ValidateResponse(response);
}

void service::OBS_service_resetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_resetVideoContext", {});

	ValidateResponse(response);
}

void service::OBS_service_createAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_createAudioEncoder", {});

	ValidateResponse(response);
}

void service::OBS_service_createVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_createVideoStreamingEncoder", {});

	ValidateResponse(response);
}

void service::OBS_service_createVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_createVideoRecordingEncoder", {});

	ValidateResponse(response);
}

void service::OBS_service_createService(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_createService", {});

	ValidateResponse(response);
}

void service::OBS_service_createRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_createRecordingSettings", {});

	ValidateResponse(response);
}

void service::OBS_service_createStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_createStreamingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_createRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_createRecordingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_startStreaming(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_startStreaming", {});

	ValidateResponse(response);
}

void service::OBS_service_startRecording(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_startRecording", {});

	ValidateResponse(response);
}

void service::OBS_service_stopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args) {
	bool forceStop;
	ASSERT_GET_VALUE(args[0], forceStop);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_stopStreaming", 
		{ ipc::value(forceStop) });

	ValidateResponse(response);
}

void service::OBS_service_stopRecording(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", "OBS_service_stopRecording", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service", 
			"OBS_service_associateAudioAndVideoToTheCurrentStreamingContext", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service",
			"OBS_service_associateAudioAndVideoToTheCurrentRecordingContext", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service",
			"OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service",
			"OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_setServiceToTheStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service",
			"OBS_service_setServiceToTheStreamingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_setRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Service",
			"OBS_service_setRecordingSettings", {});

	ValidateResponse(response);
	}

void service::OBS_service_connectOutputSignals(const v8::FunctionCallbackInfo<v8::Value>& args) {

}

INITIALIZER(nodeobs_service) {
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_service_resetAudioContext", 
			service::OBS_service_resetAudioContext);

		NODE_SET_METHOD(exports, "OBS_service_resetVideoContext", 
			service::OBS_service_resetVideoContext);

		NODE_SET_METHOD(exports, "OBS_service_createAudioEncoder", 
			service::OBS_service_createAudioEncoder);

		NODE_SET_METHOD(exports, "OBS_service_createVideoStreamingEncoder", 
			service::OBS_service_createVideoStreamingEncoder);

		NODE_SET_METHOD(exports, "OBS_service_createVideoRecordingEncoder", 
			service::OBS_service_createVideoRecordingEncoder);

		NODE_SET_METHOD(exports, "OBS_service_createService", 
			service::OBS_service_createService);

		NODE_SET_METHOD(exports, "OBS_service_createRecordingSettings", 
			service::OBS_service_createRecordingSettings);

		NODE_SET_METHOD(exports, "OBS_service_createStreamingOutput", 
			service::OBS_service_createStreamingOutput);

		NODE_SET_METHOD(exports, "OBS_service_createRecordingOutput", 
			service::OBS_service_createRecordingOutput);

		NODE_SET_METHOD(exports, "OBS_service_startStreaming", 
			service::OBS_service_startStreaming);

		NODE_SET_METHOD(exports, "OBS_service_startRecording", 
			service::OBS_service_startRecording);

		NODE_SET_METHOD(exports, "OBS_service_stopRecording", 
			service::OBS_service_stopRecording);

		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoToTheCurrentStreamingContext", 
			service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext);

		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoToTheCurrentRecordingContext",
			service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext);

		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput",
			service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput);

		NODE_SET_METHOD(exports, "OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput",
			service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput);

		NODE_SET_METHOD(exports, "OBS_service_setServiceToTheStreamingOutput",
			service::OBS_service_setServiceToTheStreamingOutput);

		NODE_SET_METHOD(exports, "OBS_service_setRecordingSettings",
			service::OBS_service_setRecordingSettings);

		NODE_SET_METHOD(exports, "OBS_service_connectOutputSignals",
			service::OBS_service_connectOutputSignals);
	});
}