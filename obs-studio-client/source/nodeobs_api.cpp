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
	std::string version;

	ASSERT_GET_VALUE(args[0], language);
	ASSERT_GET_VALUE(args[1], path);
	ASSERT_GET_VALUE(args[2], version);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "API", "OBS_API_initAPI", {ipc::value(path), ipc::value(language), ipc::value(version)});

	// The API init method will return a response error + graphical error
	// If there is a problem with the IPC the number of responses here will be zero so we must validate the
	// response.
	// If the method call was sucessfull we will have 2 arguments, also there is no need to validate the
	// response
	if (response.size() < 2) {
		if (!ValidateResponse(response)) {
			return;
		}
	}
	args.GetReturnValue().Set(v8::Number::New(args.GetIsolate(), response[1].value_union.i32));
}

void api::OBS_API_destroyOBS_API(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("API", "OBS_API_destroyOBS_API", {});
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
	    v8::String::NewFromUtf8(args.GetIsolate(), "CPU").ToLocalChecked(),
	    v8::Number::New(args.GetIsolate(), response[1].value_union.fp64));
	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "numberDroppedFrames").ToLocalChecked(),
	    v8::Number::New(args.GetIsolate(), response[2].value_union.i32));
	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "percentageDroppedFrames").ToLocalChecked(),
	    v8::Number::New(args.GetIsolate(), response[3].value_union.fp64));
	statistics->Set(
	    v8::String::NewFromUtf8(args.GetIsolate(), "streamingBandwidth").ToLocalChecked(),
	    v8::Number::New(args.GetIsolate(), response[4].value_union.fp64));
	statistics->Set(
            v8::String::NewFromUtf8(args.GetIsolate(), "streamingDataOutput").ToLocalChecked(),
            v8::Number::New(args.GetIsolate(), response[5].value_union.fp64));
	statistics->Set(
            v8::String::NewFromUtf8(args.GetIsolate(), "recordingBandwidth").ToLocalChecked(),
            v8::Number::New(args.GetIsolate(), response[6].value_union.fp64));
	statistics->Set(
            v8::String::NewFromUtf8(args.GetIsolate(), "recordingDataOutput").ToLocalChecked(),
            v8::Number::New(args.GetIsolate(), response[7].value_union.fp64));
	statistics->Set(
            v8::String::NewFromUtf8(args.GetIsolate(), "frameRate").ToLocalChecked(),
            v8::Number::New(args.GetIsolate(), response[8].value_union.fp64));
	statistics->Set(
            v8::String::NewFromUtf8(args.GetIsolate(), "averageTimeToRenderFrame").ToLocalChecked(),
            v8::Number::New(args.GetIsolate(), response[9].value_union.fp64));
	statistics->Set(
            v8::String::NewFromUtf8(args.GetIsolate(), "memoryUsage").ToLocalChecked(),
            v8::Number::New(args.GetIsolate(), response[10].value_union.fp64));
	statistics->Set(
            v8::String::NewFromUtf8(args.GetIsolate(), "diskSpaceAvailable").ToLocalChecked(),
            v8::String::NewFromUtf8(args.GetIsolate(), response[11].value_str.c_str()).ToLocalChecked());

	args.GetReturnValue().Set(statistics);
	return;
}

void api::SetWorkingDirectory(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	Nan::Utf8String param0(args[0]);
	std::string     path = *param0;

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("API", "SetWorkingDirectory", {ipc::value(path)});
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

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_QueryHotkeys(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("API", "OBS_API_QueryHotkeys", {});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::Array> hotkeyInfos = v8::Array::New(args.GetIsolate());

	// For each hotkey info that we need to fill
	for (int i = 0; i < (response.size() - 1) / 5; i++) {
		int                   responseIndex = i * 5 + 1;
		v8::Local<v8::Object> object        = v8::Object::New(args.GetIsolate());
		std::string           objectName    = response[responseIndex + 0].value_str;
		uint32_t              objectType    = response[responseIndex + 1].value_union.ui32;
		std::string           hotkeyName    = response[responseIndex + 2].value_str;
		std::string           hotkeyDesc    = response[responseIndex + 3].value_str;
		uint64_t              hotkeyId      = response[responseIndex + 4].value_union.ui64;

		object->Set(
		    v8::String::NewFromUtf8(args.GetIsolate(), "ObjectName").ToLocalChecked(),
		    v8::String::NewFromUtf8(args.GetIsolate(), objectName.c_str()).ToLocalChecked());

		object->Set(
		    v8::String::NewFromUtf8(args.GetIsolate(), "ObjectType").ToLocalChecked(),
		    v8::Number::New(args.GetIsolate(), objectType));

		object->Set(
		    v8::String::NewFromUtf8(args.GetIsolate(), "HotkeyName").ToLocalChecked(),
		    v8::String::NewFromUtf8(args.GetIsolate(), hotkeyName.c_str()).ToLocalChecked());

		object->Set(
		    v8::String::NewFromUtf8(args.GetIsolate(), "HotkeyDesc").ToLocalChecked(),
		    v8::String::NewFromUtf8(args.GetIsolate(), hotkeyDesc.c_str()).ToLocalChecked());

		object->Set(
		    v8::String::NewFromUtf8(args.GetIsolate(), "HotkeyId").ToLocalChecked(),
		    v8::Number::New(args.GetIsolate(), hotkeyId));

		hotkeyInfos->Set(i, object);
	}

	args.GetReturnValue().Set(hotkeyInfos);
}

Nan::NAN_METHOD_RETURN_TYPE api::OBS_API_ProcessHotkeyStatus(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	uint64_t    hotkeyId;
	bool        press;

	ASSERT_GET_VALUE(args[0], hotkeyId);
	ASSERT_GET_VALUE(args[1], press);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("API", "OBS_API_ProcessHotkeyStatus", {ipc::value(hotkeyId), ipc::value(press)});
}

Nan::NAN_METHOD_RETURN_TYPE api::SetUsername(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string username;

	ASSERT_GET_VALUE(args[0], username);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("API", "SetUsername", {ipc::value(username)});
}

INITIALIZER(nodeobs_api)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_API_initAPI", api::OBS_API_initAPI);
		NODE_SET_METHOD(exports, "OBS_API_destroyOBS_API", api::OBS_API_destroyOBS_API);
		NODE_SET_METHOD(exports, "OBS_API_getPerformanceStatistics", api::OBS_API_getPerformanceStatistics);
		NODE_SET_METHOD(exports, "SetWorkingDirectory", api::SetWorkingDirectory);
		NODE_SET_METHOD(exports, "StopCrashHandler", api::StopCrashHandler);
		NODE_SET_METHOD(exports, "OBS_API_QueryHotkeys", api::OBS_API_QueryHotkeys);
		NODE_SET_METHOD(exports, "OBS_API_ProcessHotkeyStatus", api::OBS_API_ProcessHotkeyStatus);
		NODE_SET_METHOD(exports, "SetUsername", api::SetUsername);
	});
}
