#include "nodeobs_service.hpp"
#include "controller.hpp"
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

using namespace std::placeholders;

Service::Service(){};

Service::~Service(){};

void Service::start_async_runner()
{
	if (m_async_callback)
		return;
	std::unique_lock<std::mutex> ul(m_worker_lock);
	// Start v8/uv asynchronous runner.
	m_async_callback = new ServiceCallback();
	m_async_callback->set_handler(std::bind(&Service::callback_handler, this, _1, _2), nullptr);
}
void Service::stop_async_runner()
{
	if (!m_async_callback)
		return;
	std::unique_lock<std::mutex> ul(m_worker_lock);
	// Stop v8/uv asynchronous runner.
	m_async_callback->clear();
	m_async_callback->finalize();
	m_async_callback = nullptr;
}

void Service::callback_handler(void* data, std::shared_ptr<SignalInfo> item)
{
	v8::Isolate*         isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Value> args[1];

	v8::Local<v8::Value> argv = v8::Object::New(isolate);
	argv->ToObject()->Set(
	    v8::String::NewFromUtf8(isolate, "type"), v8::String::NewFromUtf8(isolate, item->outputType.c_str()));
	argv->ToObject()->Set(
	    v8::String::NewFromUtf8(isolate, "signal"), v8::String::NewFromUtf8(isolate, item->signal.c_str()));
	argv->ToObject()->Set(v8::String::NewFromUtf8(isolate, "code"), v8::Number::New(isolate, item->code));
	argv->ToObject()->Set(
	    v8::String::NewFromUtf8(isolate, "error"), v8::String::NewFromUtf8(isolate, item->errorMessage.c_str()));
	args[0] = argv;

	Nan::Call(m_callback_function, 1, args);
}
void Service::start_worker()
{
	if (!m_worker_stop)
		return;
	// Launch worker thread.
	m_worker_stop = false;
	m_worker      = std::thread(std::bind(&Service::worker, this));
}
void Service::stop_worker()
{
	if (m_worker_stop != false)
		return;
	// Stop worker thread.
	m_worker_stop = true;
	if (m_worker.joinable()) {
		m_worker.join();
	}
}

void service::OBS_service_resetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_resetAudioContext", {});

	ValidateResponse(response);
}

void service::OBS_service_resetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_resetVideoContext", {});

	ValidateResponse(response);
}

void service::OBS_service_createAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_createAudioEncoder", {});

	ValidateResponse(response);
}

void service::OBS_service_createVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_createVideoStreamingEncoder", {});

	ValidateResponse(response);
}

void service::OBS_service_createVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_createVideoRecordingEncoder", {});

	ValidateResponse(response);
}

void service::OBS_service_createService(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_createService", {});

	ValidateResponse(response);
}

void service::OBS_service_createRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_createRecordingSettings", {});

	ValidateResponse(response);
}

void service::OBS_service_createStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_createStreamingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_createRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_createRecordingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_startStreaming(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_startStreaming", {});

	ValidateResponse(response);
}

void service::OBS_service_startRecording(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_startRecording", {});

	ValidateResponse(response);
}

void service::OBS_service_stopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool forceStop;
	ASSERT_GET_VALUE(args[0], forceStop);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_stopStreaming", {ipc::value(forceStop)});

	ValidateResponse(response);
}

void service::OBS_service_stopRecording(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_stopRecording", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_associateAudioAndVideoToTheCurrentStreamingContext", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_associateAudioAndVideoToTheCurrentRecordingContext", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Service", "OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Service", "OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_setServiceToTheStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_setServiceToTheStreamingOutput", {});

	ValidateResponse(response);
}

void service::OBS_service_setRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "OBS_service_setRecordingSettings", {});

	ValidateResponse(response);
}

static v8::Persistent<v8::Object> serviceCallbackObject;

void service::OBS_service_connectOutputSignals(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Function> callback;
	ASSERT_GET_VALUE(args[0], callback);

	// Grab IPC Connection
	std::shared_ptr<ipc::client> conn = nullptr;
	if (!(conn = GetConnection())) {
		return;
	}

	// Send request
	std::vector<ipc::value> rval = conn->call_synchronous_helper("Service", "OBS_service_connectOutputSignals", {});
	if (!ValidateResponse(rval)) {
		return;
	}

	if (rval[0].value_union.ui64 != (uint64_t)ErrorCode::Ok) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	// Callback

	serviceObject = new Service();
	serviceObject->m_callback_function.Reset(callback);
	serviceObject->start_async_runner();
	serviceObject->set_keepalive(args.This());
	serviceObject->start_worker();
	args.GetReturnValue().Set(true);
}

/*void Service::Callback(Service* service, SignalInfo* item) {
	if (!item) {
		return;
	}
	if (!service) {
		delete item;
		return;
	}

	ServiceCallback *cb_binding = reinterpret_cast<ServiceCallback*>(item->param);
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
	argv->ToObject()->Set(v8::String::NewFromUtf8(isolate, "type"), 
		v8::String::NewFromUtf8(isolate, item->outputType.c_str()));
	argv->ToObject()->Set(v8::String::NewFromUtf8(isolate, 
		"signal"), v8::String::NewFromUtf8(isolate, item->signal.c_str()));
	argv->ToObject()->Set(v8::String::NewFromUtf8(isolate, 
		"code"), v8::Number::New(isolate, item->code));
	argv->ToObject()->Set(v8::String::NewFromUtf8(isolate, 
		"error"), v8::String::NewFromUtf8(isolate, item->errorMessage.c_str()));
	args[0] = argv;

	delete item;
	Nan::Call(cb_binding->cb, 1, args);
}*/

void Service::worker()
{
	size_t totalSleepMS = 0;

	while (!m_worker_stop) {
		auto tp_start = std::chrono::high_resolution_clock::now();

		// Validate Connection
		auto conn = Controller::GetInstance().GetConnection();
		if (!conn) {
			goto do_sleep;
		}

		// Call
		{
			std::vector<ipc::value> response = conn->call_synchronous_helper("Service", "Query", {});
			if (!response.size() || (response.size() == 1)) {
				goto do_sleep;
			}

			ErrorCode error = (ErrorCode)response[0].value_union.ui64;
			if (error == ErrorCode::Ok) {
				std::shared_ptr<SignalInfo> data = std::make_shared<SignalInfo>();

				data->outputType   = response[1].value_str;
				data->signal       = response[2].value_str;
				data->code         = response[3].value_union.i32;
				data->errorMessage = response[4].value_str;
				data->param        = this;

				m_async_callback->queue(std::move(data));
			}
		}

	do_sleep:
		auto tp_end  = std::chrono::high_resolution_clock::now();
		auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
		totalSleepMS = sleepIntervalMS - dur.count();
		std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	}
	return;
}

void Service::set_keepalive(v8::Local<v8::Object> obj)
{
	if (!m_async_callback)
		return;
	m_async_callback->set_keepalive(obj);
}

void service::OBS_service_removeCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	serviceObject->stop_worker();
	serviceObject->stop_async_runner();
}

INITIALIZER(nodeobs_service)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_service_resetAudioContext", service::OBS_service_resetAudioContext);

		NODE_SET_METHOD(exports, "OBS_service_resetVideoContext", service::OBS_service_resetVideoContext);

		NODE_SET_METHOD(exports, "OBS_service_createAudioEncoder", service::OBS_service_createAudioEncoder);

		NODE_SET_METHOD(
		    exports, "OBS_service_createVideoStreamingEncoder", service::OBS_service_createVideoStreamingEncoder);

		NODE_SET_METHOD(
		    exports, "OBS_service_createVideoRecordingEncoder", service::OBS_service_createVideoRecordingEncoder);

		NODE_SET_METHOD(exports, "OBS_service_createService", service::OBS_service_createService);

		NODE_SET_METHOD(exports, "OBS_service_createRecordingSettings", service::OBS_service_createRecordingSettings);

		NODE_SET_METHOD(exports, "OBS_service_createStreamingOutput", service::OBS_service_createStreamingOutput);

		NODE_SET_METHOD(exports, "OBS_service_createRecordingOutput", service::OBS_service_createRecordingOutput);

		NODE_SET_METHOD(exports, "OBS_service_startStreaming", service::OBS_service_startStreaming);

		NODE_SET_METHOD(exports, "OBS_service_startRecording", service::OBS_service_startRecording);

		NODE_SET_METHOD(exports, "OBS_service_stopRecording", service::OBS_service_stopRecording);

		NODE_SET_METHOD(exports, "OBS_service_stopStreaming", service::OBS_service_stopStreaming);

		NODE_SET_METHOD(
		    exports,
		    "OBS_service_associateAudioAndVideoToTheCurrentStreamingContext",
		    service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext);

		NODE_SET_METHOD(
		    exports,
		    "OBS_service_associateAudioAndVideoToTheCurrentRecordingContext",
		    service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext);

		NODE_SET_METHOD(
		    exports,
		    "OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput",
		    service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput);

		NODE_SET_METHOD(
		    exports,
		    "OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput",
		    service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput);

		NODE_SET_METHOD(
		    exports, "OBS_service_setServiceToTheStreamingOutput", service::OBS_service_setServiceToTheStreamingOutput);

		NODE_SET_METHOD(exports, "OBS_service_setRecordingSettings", service::OBS_service_setRecordingSettings);

		NODE_SET_METHOD(exports, "OBS_service_connectOutputSignals", service::OBS_service_connectOutputSignals);

		NODE_SET_METHOD(exports, "OBS_service_removeCallback", service::OBS_service_removeCallback);
	});
}