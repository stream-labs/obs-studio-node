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
#ifdef WIN32
		const char* v_sem_name = nullptr; // Not used on Windows
		HANDLE v_sem;
#else
		const char* v_sem_name = "volmeter-semaphore";
		sem_t *v_sem;
#endif
		class Worker: public Napi::AsyncWorker
		{
#ifdef WIN32
		HANDLE sem;
#else
		sem_t *sem;
#endif
			public:
			std::shared_ptr<VolmeterData> data = nullptr;

			public:
#ifdef WIN32
			Worker(Napi::Function& callback, HANDLE sem) : AsyncWorker(callback){ this->sem = sem; };
#else
			Worker(Napi::Function& callback, sem_t *sem) : AsyncWorker(callback){ this->sem = sem; };
#endif
			// virtual ~Worker() {};

			void Execute() {
				if (!data)
					SetError("Invalid signal object");
			};
			void OnOK() {
				Napi::Array magnitude = Napi::Array::New(Env());
				Napi::Array peak = Napi::Array::New(Env());
				Napi::Array input_peak = Napi::Array::New(Env());

				for (size_t i = 0; i < data->magnitude.size(); i++) {
					magnitude.Set(i, Napi::Number::New(Env(), data->magnitude[i]));
				}
				for (size_t i = 0; i < data->peak.size(); i++) {
					peak.Set(i, Napi::Number::New(Env(), data->peak[i]));
				}
				for (size_t i = 0; i < data->input_peak.size(); i++) {
					input_peak.Set(i, Napi::Number::New(Env(), data->input_peak[i]));
				}

				if (data->magnitude.size() > 0 && data->peak.size() > 0 && data->input_peak.size() > 0) {
					Callback().Call({ magnitude, peak, input_peak });
				}
				release_semaphore(this->sem);
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
		void start_worker(Napi::Function async_callback);
		void stop_worker(void);

		static bool m_all_workers_stop;

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
