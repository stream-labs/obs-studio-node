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

#include "nodeobs_service.hpp"
#include "controller.hpp"
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

#ifdef WIN32
#include <shellapi.h>
#endif

bool service::isWorkerRunning = false;
bool service::worker_stop = true;
uint32_t service::sleepIntervalMS = 33;
std::thread* service::worker_thread = nullptr;
Napi::ThreadSafeFunction service::js_thread;
Napi::FunctionReference service::cb;

void service::start_worker(napi_env env, Napi::Function async_callback)
{
	if (!worker_stop)
		return;

	worker_stop = false;
	js_thread = Napi::ThreadSafeFunction::New(
		env,
		async_callback,
		"Service",
		0,
		1,
		[]( Napi::Env ) {} );
	worker_thread = new std::thread(&service::worker);
}

void service::stop_worker(void)
{
	if (worker_stop != false)
		return;

	worker_stop = true;
	if (worker_thread->joinable()) {
		worker_thread->join();
	}
}

Napi::Value service::OBS_service_resetAudioContext(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_resetAudioContext", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_resetVideoContext(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_resetVideoContext", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startStreaming(const Napi::CallbackInfo& info)
{
	if (!isWorkerRunning) {
		start_worker(info.Env(), cb.Value());
		isWorkerRunning = true;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_startStreaming", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startRecording(const Napi::CallbackInfo& info)
{
	if (!isWorkerRunning) {
		start_worker(info.Env(), cb.Value());
		isWorkerRunning = true;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_startRecording", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startReplayBuffer(const Napi::CallbackInfo& info)
{
	if (!isWorkerRunning) {
		start_worker(info.Env(), cb.Value());
		isWorkerRunning = true;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_startReplayBuffer", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopStreaming(const Napi::CallbackInfo& info)
{
	bool forceStop = info[0].ToBoolean().Value();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_stopStreaming", {ipc::value(forceStop)});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopRecording(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_stopRecording", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopReplayBuffer(const Napi::CallbackInfo& info)
{
	bool forceStop = info[0].ToBoolean().Value();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_stopReplayBuffer", {ipc::value(forceStop)});
	return info.Env().Undefined();
}

static v8::Persistent<v8::Object> serviceCallbackObject;

Napi::Value service::OBS_service_connectOutputSignals(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_connectOutputSignals", {});

	cb = Napi::Persistent(async_callback);
	cb.SuppressDestruct();
	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value service::OBS_service_processReplayBufferHotkey(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

    conn->call("Service", "OBS_service_processReplayBufferHotkey", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_getLastReplay(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Service", "OBS_service_getLastReplay", {});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response.at(1).value_str);
}

void service::worker()
{
    auto callback = []( Napi::Env env, Napi::Function jsCallback, SignalInfo* data ) {
		Napi::Object result = Napi::Object::New(env);

		result.Set(
			Napi::String::New(env, "type"),
			Napi::String::New(env, data->outputType));
		result.Set(
			Napi::String::New(env, "signal"),
			Napi::String::New(env, data->signal));
		result.Set(
			Napi::String::New(env, "code"),
			Napi::Number::New(env, data->code));
		result.Set(
			Napi::String::New(env, "error"),
			Napi::String::New(env, data->errorMessage));

		jsCallback.Call({ result });
    };
	size_t totalSleepMS = 0;

	while (!worker_stop) {
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
				SignalInfo* data = new SignalInfo{ "", "", 0, ""};
				data->outputType   = response[1].value_str;
				data->signal       = response[2].value_str;
				data->code         = response[3].value_union.i32;
				data->errorMessage = response[4].value_str;
				js_thread.BlockingCall( data, callback );
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

Napi::Value service::OBS_service_removeCallback(const Napi::CallbackInfo& info)
{
	if (isWorkerRunning) {
		stop_worker();
	}
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_createVirtualWebcam(const Napi::CallbackInfo& info) {
	std::string name = info[0].ToString().Utf8Value();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_createVirtualWebcam", {ipc::value(name)});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_removeVirtualWebcam(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_removeVirtualWebcam", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startVirtualWebcam(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_startVirtualWebcam", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopVirtualWebcam(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Service", "OBS_service_stopVirtualWebcam", {});
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_installVirtualCamPlugin(const Napi::CallbackInfo& info) {
#ifdef WIN32
	std::wstring pathToRegFile = L"/s /n /i:\"1\" \"" + utfWorkingDir;
	pathToRegFile += L"\\obs-virtualsource.dll\"";
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = L"runas";
	ShExecInfo.lpFile = L"regsvr32.exe";
	ShExecInfo.lpParameters = pathToRegFile.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	CloseHandle(ShExecInfo.hProcess);

	std::wstring pathToRegFile32 = L"/s /n /i:\"1\" \"" + utfWorkingDir;
	pathToRegFile32 += L"\\data\\obs-plugins\\obs-virtualoutput\\obs-virtualsource_32bit\\obs-virtualsource.dll\"";
	SHELLEXECUTEINFO ShExecInfob = {0};
	ShExecInfob.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfob.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfob.hwnd = NULL;
	ShExecInfob.lpVerb = L"runas";
	ShExecInfob.lpFile = L"regsvr32.exe";
	ShExecInfob.lpParameters = pathToRegFile32.c_str();
	ShExecInfob.lpDirectory = NULL;
	ShExecInfob.nShow = SW_HIDE;
	ShExecInfob.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfob);
	WaitForSingleObject(ShExecInfob.hProcess, INFINITE);
	CloseHandle(ShExecInfob.hProcess);
#elif __APPLE__
	g_util_osx->installPlugin();
#endif
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_isVirtualCamPluginInstalled(const Napi::CallbackInfo& info) {
#ifdef WIN32
	HKEY OpenResult;
	LONG err = RegOpenKeyEx(HKEY_CLASSES_ROOT, L"CLSID\\{27B05C2D-93DC-474A-A5DA-9BBA34CB2A9C}", 0, KEY_READ|KEY_WOW64_64KEY, &OpenResult);

	return Napi::Boolean::New(info.Env(), err == 0);
#elif __APPLE__
	// Not implemented
	return info.Env().Undefined();
#endif
}

void service::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		Napi::String::New(env, "OBS_service_resetAudioContext"),
		Napi::Function::New(env, service::OBS_service_resetAudioContext));
	exports.Set(
		Napi::String::New(env, "OBS_service_resetVideoContext"),
		Napi::Function::New(env, service::OBS_service_resetVideoContext));
	exports.Set(
		Napi::String::New(env, "OBS_service_startStreaming"),
		Napi::Function::New(env, service::OBS_service_startStreaming));
	exports.Set(
		Napi::String::New(env, "OBS_service_startRecording"),
		Napi::Function::New(env, service::OBS_service_startRecording));
	exports.Set(
		Napi::String::New(env, "OBS_service_startReplayBuffer"),
		Napi::Function::New(env, service::OBS_service_startReplayBuffer));
	exports.Set(
		Napi::String::New(env, "OBS_service_stopRecording"),
		Napi::Function::New(env, service::OBS_service_stopRecording));
	exports.Set(
		Napi::String::New(env, "OBS_service_stopStreaming"),
		Napi::Function::New(env, service::OBS_service_stopStreaming));
	exports.Set(
		Napi::String::New(env, "OBS_service_stopReplayBuffer"),
		Napi::Function::New(env, service::OBS_service_stopReplayBuffer));
	exports.Set(
		Napi::String::New(env, "OBS_service_connectOutputSignals"),
		Napi::Function::New(env, service::OBS_service_connectOutputSignals));
	exports.Set(
		Napi::String::New(env, "OBS_service_removeCallback"),
		Napi::Function::New(env, service::OBS_service_removeCallback));
	exports.Set(
		Napi::String::New(env, "OBS_service_processReplayBufferHotkey"),
		Napi::Function::New(env, service::OBS_service_processReplayBufferHotkey));
	exports.Set(
		Napi::String::New(env, "OBS_service_getLastReplay"),
		Napi::Function::New(env, service::OBS_service_getLastReplay));
	exports.Set(
		Napi::String::New(env, "OBS_service_createVirtualWebcam"),
		Napi::Function::New(env, service::OBS_service_createVirtualWebcam));
	exports.Set(
		Napi::String::New(env, "OBS_service_removeVirtualWebcam"),
		Napi::Function::New(env, service::OBS_service_removeVirtualWebcam));
	exports.Set(
		Napi::String::New(env, "OBS_service_startVirtualWebcam"),
		Napi::Function::New(env, service::OBS_service_startVirtualWebcam));
	exports.Set(
		Napi::String::New(env, "OBS_service_stopVirtualWebcam"),
		Napi::Function::New(env, service::OBS_service_stopVirtualWebcam));
	exports.Set(
		Napi::String::New(env, "OBS_service_installVirtualCamPlugin"),
		Napi::Function::New(env, service::OBS_service_installVirtualCamPlugin));
	exports.Set(
		Napi::String::New(env, "OBS_service_isVirtualCamPluginInstalled"),
		Napi::Function::New(env, service::OBS_service_isVirtualCamPluginInstalled));
}
