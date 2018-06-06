#include "nodeobs_autoconfig.hpp"
#include "shared.hpp"

Nan::Callback* cb;

void call(std::string event, std::string description, int percentage) {
	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Value> args[1];

	auto result = Nan::New<v8::Object>();

	Nan::Set(result,
		Nan::New("event").ToLocalChecked(),
		Nan::New(event.c_str()).ToLocalChecked());

	Nan::Set(result,
		Nan::New("description").ToLocalChecked(),
		Nan::New(description.c_str()).ToLocalChecked());

	if (event.compare("error") != 0) {
		Nan::Set(result,
			Nan::New("percentage").ToLocalChecked(),
			Nan::New(percentage));
	}

	v8::Local<v8::Value> params[] = {
		result
	};
	cb->Call(1, params);
}

void autoConfig::GetListServer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::string service, continent;

	ASSERT_GET_VALUE(args[0], service);
	ASSERT_GET_VALUE(args[1], continent);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"GetListServer", {service, continent});

	ValidateResponse(response);

	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Array> listServer = v8::Array::New(isolate);

	for (int i = 1; i<response.size(); i++) {
		v8::Local<v8::Object> object = v8::Object::New(isolate);

		object->Set(v8::String::NewFromUtf8(isolate, "server_name"), 
			v8::String::NewFromUtf8(isolate, response[i].value_str.c_str()));

		object->Set(v8::String::NewFromUtf8(isolate, "server"), 
			v8::String::NewFromUtf8(isolate, response[i].value_str.c_str()));

		listServer->Set(i, object);
	}

	args.GetReturnValue().Set(listServer);
}

void autoConfig::InitializeAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Local<v8::Function> function = args[0].As<v8::Function>();
	cb = new Nan::Callback(function);

	v8::Isolate *isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::Object> serverInfo = args[1].As<v8::Object>();

	v8::String::Utf8Value param0(serverInfo->Get(v8::String::NewFromUtf8(isolate, "continent")));
	std::string continent = std::string(*param0);

	v8::String::Utf8Value param1(serverInfo->Get(v8::String::NewFromUtf8(isolate, "service_name")));
	std::string service = std::string(*param1);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"InitializeAutoConfig", {continent, service});

	ValidateResponse(response);
}

void autoConfig::StartBandwidthTest(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	call("starting_step", "bandwidth_test", 0);

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"StartBandwidthTest", {});

	ValidateResponse(response);

	call("progress", "bandwidth_test", 100);
	call("stopping_step", "bandwidth_test", 100);
}

void autoConfig::StartStreamEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	call("starting_step", "streamingEncoder_test", 0);

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"StartStreamEncoderTest", {});
	
	ValidateResponse(response);
	
	call("stopping_step", "streamingEncoder_test", 100);
}

void autoConfig::StartRecordingEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	call("starting_step", "recordingEncoder_test", 0);

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"StartRecordingEncoderTest", {});

	ValidateResponse(response);

	call("stopping_step", "recordingEncoder_test", 100);
}

void autoConfig::StartCheckSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	call("starting_step", "checking_settings", 0);

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"StartCheckSettings", {});

	ValidateResponse(response);

	call("stopping_step", "checking_settings", 100);
}

void autoConfig::StartSetDefaultSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	call("starting_step", "setting_default_settings", 0);

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"StartSetDefaultSettings", {});

	ValidateResponse(response);

	call("stopping_step", "setting_default_settings", 100);
}

void autoConfig::StartSaveStreamSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	call("starting_step", "saving_service", 0);

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"StartSaveStreamSettings", {});

	ValidateResponse(response);

	call("stopping_step", "saving_service", 100);
}

void autoConfig::StartSaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	call("starting_step", "saving_settings", 0);

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"StartSaveSettings", {});

	ValidateResponse(response);

	call("stopping_step", "saving_settings", 100);
	call("done", "", 0);
}

void autoConfig::TerminateAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("AutoConfig",
			"TerminateAutoConfig", {});

	ValidateResponse(response);
}

INITIALIZER(nodeobs_autoconfig) {
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "GetListServer", autoConfig::GetListServer);
		NODE_SET_METHOD(exports, "InitializeAutoConfig", autoConfig::InitializeAutoConfig);
		NODE_SET_METHOD(exports, "StartBandwidthTest", autoConfig::StartBandwidthTest);
		NODE_SET_METHOD(exports, "StartStreamEncoderTest", autoConfig::StartStreamEncoderTest);
		NODE_SET_METHOD(exports, "StartRecordingEncoderTest", autoConfig::StartRecordingEncoderTest);
		NODE_SET_METHOD(exports, "StartCheckSettings", autoConfig::StartCheckSettings);
		NODE_SET_METHOD(exports, "StartSetDefaultSettings", autoConfig::StartSetDefaultSettings);
		NODE_SET_METHOD(exports, "StartSaveStreamSettings", autoConfig::StartSaveStreamSettings);
		NODE_SET_METHOD(exports, "StartSaveSettings", autoConfig::StartSaveSettings);
		NODE_SET_METHOD(exports, "TerminateAutoConfig", autoConfig::TerminateAutoConfig);
	});
}