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
#pragma once
#include <napi.h>
#include "utility-v8.hpp"
#ifdef WIN32
#include <windows.h>
#else
#endif

struct AutoConfigInfo
{
	std::string event;
	std::string description;
	double      percentage;
};


#ifdef WIN32
extern HANDLE sem;
#else

#endif

namespace autoConfig
{
	class Worker: public Napi::AsyncWorker
    {
        public:
        std::shared_ptr<AutoConfigInfo> data = nullptr;

        public:
        Worker(Napi::Function& callback) : AsyncWorker(callback){};
        virtual ~Worker() {};

        void Execute() {
            if (!data)
                SetError("Invalid signal object");
        };
        void OnOK() {
            Napi::Object result = Napi::Object::New(Env());

            result.Set(
                Napi::String::New(Env(), "event"),
                Napi::String::New(Env(), data->event));
            result.Set(
                Napi::String::New(Env(), "description"),
                Napi::String::New(Env(), data->description));

			if (data->event.compare("error") != 0) {
				result.Set(
					Napi::String::New(Env(), "percentage"),
					Napi::Number::New(Env(), data->percentage));
			}

            Callback().Call({ result });
			release_semaphore(sem);
        };
		void SetData(std::shared_ptr<AutoConfigInfo> new_data) {
			data = new_data;
		};
    };

	extern bool isWorkerRunning;
	extern bool worker_stop;
	extern uint32_t sleepIntervalMS;
	extern autoConfig::Worker* asyncWorker;
	extern std::thread* worker_thread;

	void worker(void);
	void start_worker(void);
	void stop_worker(void);
	void queueTask(std::shared_ptr<AutoConfigInfo> data);

    void Init(Napi::Env env, Napi::Object exports);

	Napi::Value InitializeAutoConfig(const Napi::CallbackInfo& info);
	Napi::Value StartBandwidthTest(const Napi::CallbackInfo& info);
	Napi::Value StartStreamEncoderTest(const Napi::CallbackInfo& info);
	Napi::Value StartRecordingEncoderTest(const Napi::CallbackInfo& info);
	Napi::Value StartCheckSettings(const Napi::CallbackInfo& info);
	Napi::Value StartSetDefaultSettings(const Napi::CallbackInfo& info);
	Napi::Value StartSaveStreamSettings(const Napi::CallbackInfo& info);
	Napi::Value StartSaveSettings(const Napi::CallbackInfo& info);
	Napi::Value TerminateAutoConfig(const Napi::CallbackInfo& info);
}
