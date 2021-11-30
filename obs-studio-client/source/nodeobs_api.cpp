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

#include "nodeobs_api.hpp"
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"
#include "volmeter.hpp"
#include "callback-manager.hpp"

#include "server/nodeobs_api-server.h"

Napi::ThreadSafeFunction js_thread;

Napi::Value api::OBS_API_initAPI(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	std::string path;
	std::string language;
	std::string version;
	std::string crashserverurl;

	ASSERT_GET_VALUE(info, info[0], language);
	ASSERT_GET_VALUE(info, info[1], path);
	ASSERT_GET_VALUE(info, info[2], version);
	if (info.Length()>3)
		ASSERT_GET_VALUE(info, info[3], crashserverurl);

	return Napi::Number::New(info.Env(), OBS_API::OBS_API_initAPI(
		path,
		language,
		version,
		crashserverurl
	));
}

Napi::Value api::OBS_API_destroyOBS_API(const Napi::CallbackInfo& info)
{
	profiny::Profiler::printStats("profiny_results");
	PROFINY_SCOPE
	OBS_API::OBS_API_destroyOBS_API();

#ifdef __APPLE__
	if (js_thread)
		js_thread.Release();
#endif
	return info.Env().Undefined();
}

Napi::Value api::OBS_API_getPerformanceStatistics(const Napi::CallbackInfo& info)
{
	Napi::Object statistics = Napi::Object::New(info.Env());

	statistics.Set(
		Napi::String::New(info.Env(), "CPU"),
		Napi::Number::New(info.Env(), OBS_API::getCPU_Percentage()));
	statistics.Set(
		Napi::String::New(info.Env(), "numberDroppedFrames"),
		Napi::Number::New(info.Env(), OBS_API::getNumberOfDroppedFrames()));
	statistics.Set(
		Napi::String::New(info.Env(), "percentageDroppedFrames"),
		Napi::Number::New(info.Env(), OBS_API::getDroppedFramesPercentage()));
	
	OBS_API::OutputStats streamStats = OBS_API::getCurrentOutputStats("stream");
	statistics.Set(
		Napi::String::New(info.Env(), "streamingBandwidth"),
		Napi::Number::New(info.Env(), streamStats.kbitsPerSec));
	statistics.Set(
		Napi::String::New(info.Env(), "streamingDataOutput"),
		Napi::Number::New(info.Env(), streamStats.dataOutput));

	OBS_API::OutputStats recordingStats = OBS_API::getCurrentOutputStats("recording");
	statistics.Set(
		Napi::String::New(info.Env(), "recordingBandwidth"),
		Napi::Number::New(info.Env(), recordingStats.kbitsPerSec));
	statistics.Set(
		Napi::String::New(info.Env(), "recordingDataOutput"),
		Napi::Number::New(info.Env(), recordingStats.dataOutput));

	statistics.Set(
		Napi::String::New(info.Env(), "frameRate"),
		Napi::Number::New(info.Env(), OBS_API::getCurrentFrameRate()));
	statistics.Set(
		Napi::String::New(info.Env(), "averageTimeToRenderFrame"),
		Napi::Number::New(info.Env(), OBS_API::getAverageTimeToRenderFrame()));
	statistics.Set(
		Napi::String::New(info.Env(), "memoryUsage"),
		Napi::Number::New(info.Env(), OBS_API::getMemoryUsage()));

	std::string diskSpaceAvailable = OBS_API::getDiskSpaceAvailable();
	if ( diskSpaceAvailable.c_str() == nullptr ||
		diskSpaceAvailable.empty())
		diskSpaceAvailable = "0 MB";

	statistics.Set(
		Napi::String::New(info.Env(), "diskSpaceAvailable"),
		Napi::String::New(info.Env(), diskSpaceAvailable));

	return statistics;
}

Napi::Value api::SetWorkingDirectory(const Napi::CallbackInfo& info)
{
	std::string path = info[0].ToString().Utf8Value();
	OBS_API::SetWorkingDirectory(path);

	return info.Env().Undefined();
}

Napi::Value api::InitShutdownSequence(const Napi::CallbackInfo& info)
{
	globalCallback::m_all_workers_stop = true;

	return info.Env().Undefined();
}

Napi::Value api::OBS_API_QueryHotkeys(const Napi::CallbackInfo& info)
{
	Napi::Array hotkeyInfos = Napi::Array::New(info.Env());
	auto hotkeysArray = OBS_API::QueryHotkeys();

	// For each hotkey info that we need to fill
	uint32_t index = 0;
	for (auto hotkey: hotkeysArray) {
		Napi::Object object     = Napi::Object::New(info.Env());
		object.Set(
			Napi::String::New(info.Env(), "ObjectName"),
			Napi::String::New(info.Env(), hotkey.objectName));

		object.Set(
			Napi::String::New(info.Env(), "ObjectType"),
			Napi::Number::New(info.Env(), hotkey.objectType));

		object.Set(
			Napi::String::New(info.Env(), "HotkeyName"),
			Napi::String::New(info.Env(), hotkey.hotkeyName));

		object.Set(
			Napi::String::New(info.Env(), "HotkeyDesc"),
			Napi::String::New(info.Env(), hotkey.hotkeyDesc));

		object.Set(
			Napi::String::New(info.Env(), "HotkeyId"),
			Napi::Number::New(info.Env(), hotkey.hotkeyId));

		hotkeyInfos.Set(index++, object);
	}

	return hotkeyInfos;
}

Napi::Value api::OBS_API_ProcessHotkeyStatus(const Napi::CallbackInfo& info)
{
	uint64_t    hotkeyId;
	bool        pressed;

	ASSERT_GET_VALUE(info, info[0], hotkeyId);
	ASSERT_GET_VALUE(info, info[1], pressed);

	OBS_API::ProcessHotkeyStatus(hotkeyId, pressed);

	return info.Env().Undefined();
}

Napi::Value api::SetUsername(const Napi::CallbackInfo& info)
{
	std::string username;

	ASSERT_GET_VALUE(info, info[0], username);

	OBS_API::SetUsername(username);

	return info.Env().Undefined();
}

Napi::Value api::GetPermissionsStatus(const Napi::CallbackInfo& info)
{
#ifdef __APPLE__
	bool webcam, mic;
	g_util_osx->getPermissionsStatus(webcam, mic);

	Napi::Object perms = Napi::Object::New(info.Env());
	perms.Set(
		Napi::String::New(info.Env(), "webcamPermission"),
		Napi::Boolean::New(info.Env(), webcam));
	perms.Set(
		Napi::String::New(info.Env(), "micPermission"),
		Napi::Boolean::New(info.Env(), mic));

	return perms;
#endif
	return info.Env().Undefined();
}

Napi::Value api::RequestPermissions(const Napi::CallbackInfo& info)
{
#ifdef __APPLE__
	Napi::Function async_callback = info[0].As<Napi::Function>();
	js_thread = Napi::ThreadSafeFunction::New(
        info.Env(),
		async_callback,
		"RequestPermissionsThread",
		0,
		1,
		[]( Napi::Env ) {} );

	auto cb = [](void* data, bool webcam, bool mic) {
		Napi::ThreadSafeFunction* worker =
			reinterpret_cast<Napi::ThreadSafeFunction*>(data);

		auto callback = []( Napi::Env env, Napi::Function jsCallback, Permissions* data ) {
			Napi::Object result = Napi::Object::New(env);

			result.Set(
				Napi::String::New(env, "webcamPermission"),
				Napi::Boolean::New(env, data->webcam)
			);
			result.Set(
				Napi::String::New(env, "micPermission"),
				Napi::Boolean::New(env, data->mic)
			);

			jsCallback.Call({ result });
		};
		Permissions* perms_status = new Permissions();
		perms_status->webcam = webcam;
		perms_status->mic = mic;
		js_thread.BlockingCall( perms_status, callback );
	};

	g_util_osx->requestPermissions(js_thread, cb);
#endif
	return info.Env().Undefined();
}

void api::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(Napi::String::New(env, "OBS_API_initAPI"), Napi::Function::New(env, api::OBS_API_initAPI));
	exports.Set(Napi::String::New(env, "OBS_API_destroyOBS_API"), Napi::Function::New(env, api::OBS_API_destroyOBS_API));
	exports.Set(Napi::String::New(env, "OBS_API_getPerformanceStatistics"), Napi::Function::New(env, api::OBS_API_getPerformanceStatistics));
	exports.Set(Napi::String::New(env, "SetWorkingDirectory"), Napi::Function::New(env, api::SetWorkingDirectory));
	exports.Set(Napi::String::New(env, "InitShutdownSequence"), Napi::Function::New(env, api::InitShutdownSequence));
	exports.Set(Napi::String::New(env, "OBS_API_QueryHotkeys"), Napi::Function::New(env, api::OBS_API_QueryHotkeys));
	exports.Set(Napi::String::New(env, "OBS_API_ProcessHotkeyStatus"), Napi::Function::New(env, api::OBS_API_ProcessHotkeyStatus));
	exports.Set(Napi::String::New(env, "SetUsername"), Napi::Function::New(env, api::SetUsername));
	exports.Set(Napi::String::New(env, "GetPermissionsStatus"), Napi::Function::New(env, api::GetPermissionsStatus));
	exports.Set(Napi::String::New(env, "RequestPermissions"), Napi::Function::New(env, api::RequestPermissions));
}
