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
#include <thread>
#include "utility-v8.hpp"

struct VolmeterData
{
	std::vector<float> magnitude;
	std::vector<float> peak;
	std::vector<float> input_peak;
};

extern const char* v_sem_name;
#ifdef WIN32
extern HANDLE v_sem;
#else
extern sem_t *v_sem;
#endif

namespace osn
{
	class Volmeter : public Napi::ObjectWrap<osn::Volmeter>
	{
		class Worker: public Napi::AsyncWorker
		{
			public:
			std::shared_ptr<VolmeterData> data = nullptr;

			public:
			Worker(Napi::Function& callback) : AsyncWorker(callback){};
			virtual ~Worker() {};

			void Execute() {
				if (!data)
					SetError("Invalid signal object");
			};
			void OnOK() {
				Napi::Array magnitude;
				Napi::Array peak;
				Napi::Array input_peak;

				for (size_t i = 0; i < data->magnitude.size(); i++) {
					magnitude.Set(i, Napi::Number::New(Env(), data->magnitude[i]));
				}
				for (size_t i = 0; i < data->peak.size(); i++) {
					peak.Set(i, Napi::Number::New(Env(), data->peak[i]));
				}
				for (size_t i = 0; i < data->input_peak.size(); i++) {
					input_peak.Set(i, Napi::Number::New(Env(), data->input_peak[i]));
				}

				Callback().Call({ magnitude, peak, input_peak });
				release_semaphore(v_sem);
			};
			void SetData(std::shared_ptr<VolmeterData> new_data) {
				data = new_data;
			};
		};

		public:
		uint64_t m_uid;

		bool isWorkerRunning;
		bool worker_stop;
		uint32_t sleepIntervalMS;
		Worker* asyncWorker;
		std::thread* worker_thread;

		void worker(void);
		void start_worker(void);
		void stop_worker(void);
		void queueTask(std::shared_ptr<VolmeterData> data);
		std::vector<std::thread*> v_queue_task_workers;

		public:
		static Napi::FunctionReference constructor;
		static Napi::Object Init(Napi::Env env, Napi::Object exports);
		Volmeter(const Napi::CallbackInfo& info);

		static Napi::Value Create(const Napi::CallbackInfo& info);
		Napi::Value GetUpdateInterval(const Napi::CallbackInfo& info);
		void SetUpdateInterval(const Napi::CallbackInfo& info, const Napi::Value &value);
		Napi::Value Attach(const Napi::CallbackInfo& info);
		Napi::Value Detach(const Napi::CallbackInfo& info);
		Napi::Value AddCallback(const Napi::CallbackInfo& info);
		Napi::Value RemoveCallback(const Napi::CallbackInfo& info);
	};
}
