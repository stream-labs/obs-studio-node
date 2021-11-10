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
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"
#include "server/nodeobs_service-server.h"

#ifdef WIN32
#include <shellapi.h>
#endif

bool service::isWorkerRunning = false;
bool service::worker_stop = true;
uint32_t service::sleepIntervalMS = 33;
std::thread* service::worker_thread = nullptr;
Napi::ThreadSafeFunction service::js_thread;

void callJS(SignalInfo* data)
{
	Napi::ThreadSafeFunction& l_jsThread =
		*reinterpret_cast<Napi::ThreadSafeFunction*>(data->m_jsThread);

	auto callback = []( Napi::Env env, Napi::Function jsCallback, SignalInfo* data ) {
		Napi::Object result = Napi::Object::New(env);

		result.Set(
			Napi::String::New(env, "type"),
			Napi::String::New(env, data->m_outputType));
		result.Set(
			Napi::String::New(env, "signal"),
			Napi::String::New(env, data->m_signal));
		result.Set(
			Napi::String::New(env, "code"),
			Napi::Number::New(env, data->m_code));
		result.Set(
			Napi::String::New(env, "error"),
			Napi::String::New(env, data->m_errorMessage));

		jsCallback.Call({ result });
    };

	l_jsThread.BlockingCall( data, callback );
}

Napi::Value service::OBS_service_resetAudioContext(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_resetAudioContext();

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_resetVideoContext(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_resetVideoContext();

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startStreaming(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_startStreaming(callJS);

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startRecording(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_startRecording(callJS);

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startReplayBuffer(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_startReplayBuffer(callJS);

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopStreaming(const Napi::CallbackInfo& info)
{
	bool forceStop = info[0].ToBoolean().Value();

	OBS_service::OBS_service_stopStreaming(forceStop);

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopRecording(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_stopRecording();

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopReplayBuffer(const Napi::CallbackInfo& info)
{
	bool forceStop = info[0].ToBoolean().Value();

	OBS_service::OBS_service_stopReplayBuffer(forceStop);

	return info.Env().Undefined();
}

static v8::Persistent<v8::Object> serviceCallbackObject;

void JSCallbackOutputSignal(void* data, calldata_t* params)
{
	SignalInfo* signal = reinterpret_cast<SignalInfo*>(data);

	if (signal->m_signal.compare("stop") == 0) {
		signal->m_code = (int)calldata_int(params, "code");

		obs_output_t* output;

		if (signal->m_outputType.compare("streaming") == 0) {
			output = streamingOutput;
			isStreaming = false;
		} else if (signal->m_outputType.compare("recording") == 0) {
			output = recordingOutput;
			isRecording = false;
		} else {
			output = replayBufferOutput;
			isReplayBufferActive = false;
		}

		const char* error = obs_output_get_last_error(output);
		if (error) {
			if (signal->m_outputType.compare("recording") == 0 && !signal->m_code)
				signal->m_code = OBS_OUTPUT_ERROR;
			signal->m_errorMessage = std::string(error);
		}
	}

	if (signal->m_jsThread)
		callJS(signal);
}

Napi::Value service::OBS_service_connectOutputSignals(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();
	js_thread = Napi::ThreadSafeFunction::New(
		info.Env(),
		async_callback,
		"Service",
		0,
		1,
		[]( Napi::Env ) {} );

	OBS_service::OBS_service_connectOutputSignals(JSCallbackOutputSignal, &js_thread);
	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value service::OBS_service_processReplayBufferHotkey(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_processReplayBufferHotkey();

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_getLastReplay(const Napi::CallbackInfo& info)
{
	return Napi::String::New(info.Env(), OBS_service::OBS_service_getLastReplay());
}

Napi::Value service::OBS_service_removeCallback(const Napi::CallbackInfo& info)
{
	// TODO
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_createVirtualWebcam(const Napi::CallbackInfo& info)
{
	std::string name = info[0].ToString().Utf8Value();

	OBS_service::OBS_service_createVirtualWebcam(name);

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_removeVirtualWebcam(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_removeVirtualWebcam();

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_startVirtualWebcam(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_startVirtualWebcam();

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_stopVirtualWebcam(const Napi::CallbackInfo& info)
{
	OBS_service::OBS_service_stopVirtualWebcam();

	return info.Env().Undefined();
}

Napi::Value service::OBS_service_installVirtualCamPlugin(const Napi::CallbackInfo& info) {
// #ifdef WIN32
// 	std::wstring pathToRegFile = L"/s /n /i:\"1\" \"" + utfWorkingDir;
// 	pathToRegFile += L"\\obs-virtualsource.dll\"";
// 	SHELLEXECUTEINFO ShExecInfo = {0};
// 	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
// 	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
// 	ShExecInfo.hwnd = NULL;
// 	ShExecInfo.lpVerb = L"runas";
// 	ShExecInfo.lpFile = L"regsvr32.exe";
// 	ShExecInfo.lpParameters = pathToRegFile.c_str();
// 	ShExecInfo.lpDirectory = NULL;
// 	ShExecInfo.nShow = SW_HIDE;
// 	ShExecInfo.hInstApp = NULL;
// 	ShellExecuteEx(&ShExecInfo);
// 	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
// 	CloseHandle(ShExecInfo.hProcess);

// 	std::wstring pathToRegFile32 = L"/s /n /i:\"1\" \"" + utfWorkingDir;
// 	pathToRegFile32 += L"\\data\\obs-plugins\\obs-virtualoutput\\obs-virtualsource_32bit\\obs-virtualsource.dll\"";
// 	SHELLEXECUTEINFO ShExecInfob = {0};
// 	ShExecInfob.cbSize = sizeof(SHELLEXECUTEINFO);
// 	ShExecInfob.fMask = SEE_MASK_NOCLOSEPROCESS;
// 	ShExecInfob.hwnd = NULL;
// 	ShExecInfob.lpVerb = L"runas";
// 	ShExecInfob.lpFile = L"regsvr32.exe";
// 	ShExecInfob.lpParameters = pathToRegFile32.c_str();
// 	ShExecInfob.lpDirectory = NULL;
// 	ShExecInfob.nShow = SW_HIDE;
// 	ShExecInfob.hInstApp = NULL;
// 	ShellExecuteEx(&ShExecInfob);
// 	WaitForSingleObject(ShExecInfob.hProcess, INFINITE);
// 	CloseHandle(ShExecInfob.hProcess);
// #elif __APPLE__
// 	g_util_osx->installPlugin();
// #endif
	return info.Env().Undefined();
}

Napi::Value service::OBS_service_uninstallVirtualCamPlugin(const Napi::CallbackInfo& info) {
#ifdef WIN32
	// std::wstring pathToRegFile = L"/u \"" + utfWorkingDir;
	// pathToRegFile += L"\\obs-virtualsource.dll\"";
	// SHELLEXECUTEINFO ShExecInfo = {0};
	// ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	// ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	// ShExecInfo.hwnd = NULL;
	// ShExecInfo.lpVerb = L"runas";
	// ShExecInfo.lpFile = L"regsvr32.exe";
	// ShExecInfo.lpParameters = pathToRegFile.c_str();
	// ShExecInfo.lpDirectory = NULL;
	// ShExecInfo.nShow = SW_HIDE;
	// ShExecInfo.hInstApp = NULL;
	// ShellExecuteEx(&ShExecInfo);
	// WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	// CloseHandle(ShExecInfo.hProcess);

	// std::wstring pathToRegFile32 = L"/u \"" + utfWorkingDir;
	// pathToRegFile32 += L"\\data\\obs-plugins\\obs-virtualoutput\\obs-virtualsource_32bit\\obs-virtualsource.dll\"";
	// SHELLEXECUTEINFO ShExecInfob = {0};
	// ShExecInfob.cbSize = sizeof(SHELLEXECUTEINFO);
	// ShExecInfob.fMask = SEE_MASK_NOCLOSEPROCESS;
	// ShExecInfob.hwnd = NULL;
	// ShExecInfob.lpVerb = L"runas";
	// ShExecInfob.lpFile = L"regsvr32.exe";
	// ShExecInfob.lpParameters = pathToRegFile32.c_str();
	// ShExecInfob.lpDirectory = NULL;
	// ShExecInfob.nShow = SW_HIDE;
	// ShExecInfob.hInstApp = NULL;
	// ShellExecuteEx(&ShExecInfob);
	// WaitForSingleObject(ShExecInfob.hProcess, INFINITE);
	// CloseHandle(ShExecInfob.hProcess);
#elif __APPLE__
	g_util_osx->uninstallPlugin();
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
		Napi::String::New(env, "OBS_service_uninstallVirtualCamPlugin"),
		Napi::Function::New(env, service::OBS_service_uninstallVirtualCamPlugin));
	exports.Set(
		Napi::String::New(env, "OBS_service_isVirtualCamPluginInstalled"),
		Napi::Function::New(env, service::OBS_service_isVirtualCamPluginInstalled));
}
