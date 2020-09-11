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
#include <napi.h>
#include <thread>
#include "utility-v8.hpp"
#ifdef WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif

struct SignalInfo
{
	std::string outputType;
	std::string signal;
	int         code;
	std::string errorMessage;
};

extern const char* service_sem_name;
#ifdef WIN32
extern HANDLE service_sem;
#else
extern sem_t *service_sem;
#endif

namespace service
{
	class Worker: public Napi::AsyncWorker
    {
        public:
        std::shared_ptr<SignalInfo> data = nullptr;

        public:
        Worker(Napi::Function& callback) : AsyncWorker(callback){};
        // virtual ~Worker() {};

        void Execute() {
            if (!data)
                SetError("Invalid signal object");
        };
        void OnOK() {
            Napi::Object result = Napi::Object::New(Env());

            result.Set(
                Napi::String::New(Env(), "type"),
                Napi::String::New(Env(), data->outputType));
            result.Set(
                Napi::String::New(Env(), "signal"),
                Napi::String::New(Env(), data->signal));
            result.Set(
                Napi::String::New(Env(), "code"),
                Napi::Number::New(Env(), data->code));
            result.Set(
                Napi::String::New(Env(), "error"),
                Napi::String::New(Env(), data->errorMessage));

            std::cout << "calling " << data->signal.c_str() << std::endl;
            Callback().Call({ result });
			release_semaphore(service_sem);
        };
		void SetData(std::shared_ptr<SignalInfo> new_data) {
			data = new_data;
		};
    };

	extern bool isWorkerRunning;
	extern bool worker_stop;
	extern uint32_t sleepIntervalMS;
	extern Worker* asyncWorker;
	extern std::thread* worker_thread;
	extern std::vector<std::thread*> service_queue_task_workers;

	void worker(void);
	void start_worker(void);
	void stop_worker(void);
	void queueTask(std::shared_ptr<SignalInfo> data);

    void Init(Napi::Env env, Napi::Object exports);

	Napi::Value OBS_service_resetAudioContext(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_resetVideoContext(const Napi::CallbackInfo& info);

	Napi::Value OBS_service_startStreaming(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_startRecording(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_startReplayBuffer(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_stopStreaming(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_stopRecording(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_stopReplayBuffer(const Napi::CallbackInfo& info);

	Napi::Value OBS_service_connectOutputSignals(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_removeCallback(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_processReplayBufferHotkey(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_getLastReplay(const Napi::CallbackInfo& info);

	Napi::Value OBS_service_createVirtualWebcam(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_removeVirtualWebcam(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_startVirtualWebcam(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_stopVirtualWebcam(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_installVirtualCamPlugin(const Napi::CallbackInfo& info);
	Napi::Value OBS_service_isVirtualCamPluginInstalled(const Napi::CallbackInfo& info);
}