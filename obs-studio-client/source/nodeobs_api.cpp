#include "controller.hpp"
#include "error.hpp"
#include "nodeobs_api.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

void api::OBS_API_initAPI(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string path;
	std::string language;

	ASSERT_GET_VALUE(args[0], language);
	ASSERT_GET_VALUE(args[1], path);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("API", "OBS_API_initAPI", {ipc::value(path), ipc::value(language)});

	ValidateResponse(response);
}

void api::OBS_API_destroyOBS_API(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("API", "OBS_API_destroyOBS_API", {});

	// There is no need to validate the response here since we are closing the app. If for any reason
	// the server crashes and we receive an error or an IPC timeout, this will throw an error. Throwing
	// an error makes no sense (for now) since this is a shutdown operation.
	// ValidateResponse(response);
}

void api::OBS_API_getPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("API", "OBS_API_getPerformanceStatistics", {});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::Object> statistics = v8::Object::New(args.GetIsolate());

	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "CPU"),
	    v8::Number::New(args.GetIsolate(), response[1].value_union.fp64));
	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "numberDroppedFrames"),
	    v8::Number::New(args.GetIsolate(), response[2].value_union.i32));
	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "percentageDroppedFrames"),
	    v8::Number::New(args.GetIsolate(), response[3].value_union.fp64));
	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "bandwidth"),
	    v8::Number::New(args.GetIsolate(), response[4].value_union.fp64));
	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "frameRate"),
	    v8::Number::New(args.GetIsolate(), response[5].value_union.fp64));

	args.GetReturnValue().Set(statistics);
	return;
}

void api::SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
	Nan::Utf8String param0(args[0]);
	std::string     path = *param0;

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("API", "SetWorkingDirectory", {ipc::value(path)});

	ValidateResponse(response);
}

void api::StopCrashHandler(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("API", "StopCrashHandler", {});

	// This is a shutdown operation, no response validation needed
	// ValidateResponse(response);
}

INITIALIZER(nodeobs_api)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_API_initAPI", api::OBS_API_initAPI);
		NODE_SET_METHOD(exports, "OBS_API_destroyOBS_API", api::OBS_API_destroyOBS_API);
		NODE_SET_METHOD(exports, "OBS_API_getPerformanceStatistics", api::OBS_API_getPerformanceStatistics);
		NODE_SET_METHOD(exports, "SetWorkingDirectory", api::SetWorkingDirectory);
		NODE_SET_METHOD(exports, "StopCrashHandler", api::StopCrashHandler);
	});
}
