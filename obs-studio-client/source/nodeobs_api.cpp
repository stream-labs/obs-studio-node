#include "nodeobs_api.hpp"
#include "controller.hpp"
#include "utility-v8.hpp"
#include "error.hpp"

#include "shared.hpp"
#include "utility.hpp"
#include <string>
#include <sstream>
#include <node.h>

void api::OBS_API_initAPI(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::string path;

	ASSERT_GET_VALUE(args[0], path);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response = 
		conn->call_synchronous_helper("API", "OBS_API_initAPI",
	{ ipc::value(path) });

	ValidateResponse(response);
}

void api::OBS_API_destroyOBS_API(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response = 
		conn->call_synchronous_helper("API", "OBS_API_destroyOBS_API", {});

	ValidateResponse(response);
} 

void api::OBS_API_getPerformanceStatistics(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("API", "OBS_API_getPerformanceStatistics", {});

	if (!ValidateResponse(response)) return;

	v8::Local<v8::Object> statistics = v8::Object::New(args.GetIsolate());

	statistics->Set(v8::String::NewFromUtf8(args.GetIsolate(), "CPU"),
		v8::Number::New(args.GetIsolate(), response[1].value_union.fp64));
	statistics->Set(v8::String::NewFromUtf8(args.GetIsolate(), "numberDroppedFrames"),
		v8::Number::New(args.GetIsolate(), response[2].value_union.i32));
	statistics->Set(v8::String::NewFromUtf8(args.GetIsolate(), "percentageDroppedFrames"),
		v8::Number::New(args.GetIsolate(), response[3].value_union.fp64));
	statistics->Set(v8::String::NewFromUtf8(args.GetIsolate(), "bandwidth"),
		v8::Number::New(args.GetIsolate(), response[4].value_union.fp64));
	statistics->Set(v8::String::NewFromUtf8(args.GetIsolate(), "frameRate"),
		v8::Number::New(args.GetIsolate(), response[5].value_union.fp64));
	
	args.GetReturnValue().Set(statistics);
	return;
}

void api::OBS_API_getOBS_existingProfiles(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("API", "OBS_API_getOBS_existingProfiles", {});

	if (!ValidateResponse(response)) return;

	v8::Local<v8::Array> existingProfiles = v8::Array::New(args.GetIsolate());

	for (int i = 1; i<response.size(); i++) {
		existingProfiles->Set(i, 
			v8::String::NewFromUtf8(args.GetIsolate(), response.at(i).value_str.c_str()));
	}

	args.GetReturnValue().Set(existingProfiles);
}

void api::OBS_API_getOBS_existingSceneCollections(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("API", "OBS_API_getOBS_existingSceneCollections", {});

	if (!ValidateResponse(response)) return;

	v8::Local<v8::Array> existingSceneCollections = v8::Array::New(args.GetIsolate());

	for (int i = 1; i<response.size(); i++) {
		existingSceneCollections->Set(i, 
			v8::String::NewFromUtf8(args.GetIsolate(), response.at(i).value_str.c_str()));
	}

	args.GetReturnValue().Set(existingSceneCollections);
	return;
}

void api::OBS_API_isOBS_installed(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("API", "OBS_API_isOBS_installed", {});

	if (!ValidateResponse(response)) return;
	
	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

void api::SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::string path;
	ASSERT_GET_VALUE(args[0], path);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("API", "SetWorkingDirectory", { ipc::value(path) });

	ValidateResponse(response);
}

INITIALIZER(nodeobs_api) {
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_API_initAPI", api::OBS_API_initAPI);
		NODE_SET_METHOD(exports, "OBS_API_destroyOBS_API", api::OBS_API_destroyOBS_API);
		NODE_SET_METHOD(exports, "OBS_API_getPerformanceStatistics", api::OBS_API_getPerformanceStatistics);
		NODE_SET_METHOD(exports, "OBS_API_getOBS_existingProfiles", api::OBS_API_getOBS_existingProfiles);
		NODE_SET_METHOD(exports, "OBS_API_getOBS_existingSceneCollections", api::OBS_API_getOBS_existingSceneCollections);
		NODE_SET_METHOD(exports, "OBS_API_isOBS_installed", api::OBS_API_isOBS_installed);
		NODE_SET_METHOD(exports, "SetWorkingDirectory", api::SetWorkingDirectory);
	});
}
