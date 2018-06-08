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

Nan::Persistent<v8::FunctionTemplate> AutoConfigCallback::prototype = Nan::Persistent<v8::FunctionTemplate>();
static v8::Persistent<v8::Object> autoConfigCallbackObject;

void autoConfig::InitializeAutoConfig(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Local<v8::Function> callback = args[0].As<v8::Function>();
	cb = new Nan::Callback(callback);

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

	// Callback
	autoConfigObject->callback = new AutoConfigCallback(autoConfigObject, AutoConfig::Callback, callback, 20);

	v8::Local<v8::Object> obj = AutoConfigCallback::Store(autoConfigObject->callback);
	autoConfigObject->callback->obj_ref.Reset(obj);
	autoConfigCallbackObject.Reset(args.GetIsolate(), obj);
}

void AutoConfig::Callback(AutoConfig* service, AutoConfigInfo* item) {
	if (!item) {
		return;
	}
	if (!service) {
		delete item;
		return;
	}

	/* We're in v8 context here */
	AutoConfigCallback *cb_binding = reinterpret_cast<AutoConfigCallback*>(item->param);
	if (!cb_binding) {
		delete item;
		return;
	}

	if (cb_binding->stopped) {
		delete item;
		return;
	}

	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Value> args[1];

	v8::Local<v8::Value> argv = v8::Object::New(isolate);
	argv->ToObject()->Set(v8::String::NewFromUtf8(isolate, "event"),
		v8::String::NewFromUtf8(isolate, item->event.c_str()));
	argv->ToObject()->Set(v8::String::NewFromUtf8(isolate,
		"description"), v8::String::NewFromUtf8(isolate, item->description.c_str()));
	
	if (item->event.compare("error") != 0) {
		argv->ToObject()->Set(v8::String::NewFromUtf8(isolate,
			"percentage"), v8::Number::New(isolate, item->percentage));
	}
	args[0] = argv;

	delete item;
	Nan::Call(cb_binding->cb, 1, args);
}

void autoConfig::StartBandwidthTest(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	AutoConfigInfo* data = new AutoConfigInfo();
	data->event = "starting_step";
	data->description = "bandwidth_test";
	data->percentage = 0;
	data->param = autoConfigObject->callback;
	autoConfigObject->callback->queue.send(data);
	
	
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		
	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {

		ErrorCode error = (ErrorCode)rval[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			AutoConfigInfo* data = new AutoConfigInfo();
			data->event = "stopping_step";
			data->description = "bandwidth_test";
			data->percentage = 100;
			data->param = autoConfigObject->callback;
			autoConfigObject->callback->queue.send(data);
		}
	};	
	
	bool success = conn->call("AutoConfig",
		"StartBandwidthTest", std::vector<ipc::value>{}, fnc, &rtd);
}

void autoConfig::StartStreamEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;
	
	AutoConfigInfo* data = new AutoConfigInfo();
	data->event = "starting_step";
	data->description = "streamingEncoder_test";
	data->percentage = 0;
	data->param = autoConfigObject->callback;
	autoConfigObject->callback->queue.send(data);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {

	ErrorCode error = (ErrorCode)rval[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			AutoConfigInfo* data = new AutoConfigInfo();
			data->event = "stopping_step";
			data->description = "streamingEncoder_test";
			data->percentage = 100;
			data->param = autoConfigObject->callback;
			autoConfigObject->callback->queue.send(data);
		}
	};

	bool success = conn->call("AutoConfig",
		"StartStreamEncoderTest", std::vector<ipc::value>{}, fnc, &rtd);
}

void autoConfig::StartRecordingEncoderTest(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	AutoConfigInfo* data = new AutoConfigInfo();
	data->event = "starting_step";
	data->description = "recordingEncoder_test";
	data->percentage = 0;
	data->param = autoConfigObject->callback;
	autoConfigObject->callback->queue.send(data);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {

		ErrorCode error = (ErrorCode)rval[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			AutoConfigInfo* data = new AutoConfigInfo();
			data->event = "stopping_step";
			data->description = "recordingEncoder_test";
			data->percentage = 100;
			data->param = autoConfigObject->callback;
			autoConfigObject->callback->queue.send(data);
		}
	};

	bool success = conn->call("AutoConfig",
		"StartRecordingEncoderTest", std::vector<ipc::value>{}, fnc, &rtd);
}

void autoConfig::StartCheckSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	AutoConfigInfo* data = new AutoConfigInfo();
	data->event = "starting_step";
	data->description = "checking_settings";
	data->percentage = 0;
	data->param = autoConfigObject->callback;
	autoConfigObject->callback->queue.send(data);
	
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {
		ErrorCode error = (ErrorCode)rval[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			bool success = (bool)rval[1].value_union.ui32;
			if (!success) {
				AutoConfigInfo* error = new AutoConfigInfo();
				error->event = "stopping_step";
				error->description = "checking_settings";
				error->percentage = 100;
				error->param = autoConfigObject->callback;
				autoConfigObject->callback->queue.send(error);
			}

			AutoConfigInfo* data = new AutoConfigInfo();
			data->event = "stopping_step";
			data->description = "checking_settings";
			data->percentage = 100;
			data->param = autoConfigObject->callback;
			autoConfigObject->callback->queue.send(data);
		}
	};

	bool success = conn->call("AutoConfig",
		"StartCheckSettings", std::vector<ipc::value>{}, fnc, &rtd);
}

void autoConfig::StartSetDefaultSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;
	

	AutoConfigInfo* data = new AutoConfigInfo();
	data->event = "starting_step";
	data->description = "setting_default_settings";
	data->percentage = 0;
	data->param = autoConfigObject->callback;
	autoConfigObject->callback->queue.send(data);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {
		ErrorCode error = (ErrorCode)rval[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			AutoConfigInfo* data = new AutoConfigInfo();
			data->event = "stopping_step";
			data->description = "setting_default_settings";
			data->percentage = 100;
			data->param = autoConfigObject->callback;
			autoConfigObject->callback->queue.send(data);
		}
	};

	bool success = conn->call("AutoConfig",
		"StartSetDefaultSettings", std::vector<ipc::value>{}, fnc, &rtd);
}

void autoConfig::StartSaveStreamSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	AutoConfigInfo* data = new AutoConfigInfo();
	data->event = "starting_step";
	data->description = "saving_service";
	data->percentage = 0;
	data->param = autoConfigObject->callback;
	autoConfigObject->callback->queue.send(data);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {

		ErrorCode error = (ErrorCode)rval[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			AutoConfigInfo* data = new AutoConfigInfo();
			data->event = "stopping_step";
			data->description = "saving_service";
			data->percentage = 100;
			data->param = autoConfigObject->callback;
			autoConfigObject->callback->queue.send(data);
		}
	};

	bool success = conn->call("AutoConfig",
		"StartSaveStreamSettings", std::vector<ipc::value>{}, fnc, &rtd);
}

void autoConfig::StartSaveSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	AutoConfigInfo* data = new AutoConfigInfo();
	data->event = "starting_step";
	data->description = "saving_settings";
	data->percentage = 0;
	data->param = autoConfigObject->callback;
	autoConfigObject->callback->queue.send(data);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {

		ErrorCode error = (ErrorCode)rval[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			AutoConfigInfo* data = new AutoConfigInfo();
			data->event = "stopping_step";
			data->description = "saving_settings";
			data->percentage = 100;
			data->param = autoConfigObject->callback;
			autoConfigObject->callback->queue.send(data);

			AutoConfigInfo* dataDone = new AutoConfigInfo();
			dataDone->event = "done";
			dataDone->description = "";
			dataDone->percentage = 0;
			dataDone->param = cb;
			autoConfigObject->callback->queue.send(dataDone);
		}
	};

	bool success = conn->call("AutoConfig",
		"StartSaveSettings", std::vector<ipc::value>{}, fnc, &rtd);
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

		autoConfigObject = new AutoConfig();

		AutoConfigCallback::Init(exports);
	});
}