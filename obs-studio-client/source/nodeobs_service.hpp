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

#include <mutex>
#include <nan.h>
#include <node.h>
#include <thread>
#include "utility-v8.hpp"

struct SignalInfo
{
	std::string outputType;
	std::string signal;
	int         code;
	std::string errorMessage;
	void*       param;
};

class Service;
typedef utilv8::managed_callback<std::shared_ptr<SignalInfo>> ServiceCallback;
Service*                                                      serviceObject;

class Service : public Nan::ObjectWrap, public utilv8::InterfaceObject<Service>, public utilv8::ManagedObject<Service>
{
	friend utilv8::InterfaceObject<Service>;
	friend utilv8::ManagedObject<Service>;
	friend utilv8::CallbackData<SignalInfo, Service>;

	uint32_t sleepIntervalMS = 33;

	public:
	std::thread m_worker;
	bool        m_worker_stop = true;
	std::mutex  m_worker_lock;

	ServiceCallback* m_async_callback = nullptr;
	Nan::Callback    m_callback_function;

	Service();
	~Service();

	void start_async_runner();
	void stop_async_runner();
	void callback_handler(void* data, std::shared_ptr<SignalInfo> item);
	void start_worker();
	void stop_worker();
	void worker();
	void set_keepalive(v8::Local<v8::Object>);

	std::list<ServiceCallback*> callbacks;
};

namespace service
{
	static void OBS_service_resetAudioContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_resetVideoContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createAudioEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createVideoStreamingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createVideoRecordingEncoder(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createService(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_createRecordingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_startStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_startRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_startReplayBuffer(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_stopStreaming(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_stopRecording(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_stopReplayBuffer(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void
	    OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void
	            OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(
	    const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(
	    const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_setServiceToTheStreamingOutput(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_setRecordingSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_connectOutputSignals(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_removeCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_processReplayBufferHotkey(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_service_getLastReplay(const v8::FunctionCallbackInfo<v8::Value>& args);
} // namespace service
