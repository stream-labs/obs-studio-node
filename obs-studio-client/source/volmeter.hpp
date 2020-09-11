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
#include <mutex>

struct VolmeterData
{
	std::vector<float> magnitude;
	std::vector<float> peak;
	std::vector<float> input_peak;
};

namespace osn
{
	class Volmeter : public Napi::ObjectWrap<osn::Volmeter>
	{
		class Worker: public Napi::AsyncProgressQueueWorker<VolmeterData>
		{
			public:
			osn::Volmeter* parent;

			public:
			Worker(Napi::Function& callback, osn::Volmeter* vol) :
				AsyncProgressQueueWorker<VolmeterData>(callback) {
				parent = vol;
				std::cout << "constructor for " << parent->m_uid << std::endl;
			};

			virtual ~Worker() {};

			void Execute(const ExecutionProgress& progress) {
				std::cout << "Execute - " << parent->m_uid << std::endl;
				size_t totalSleepMS = 0;

				while (!parent->worker_stop && !parent->m_all_workers_stop) {
					if (parent->m_uid == 4)
						std::cout << "looping for " << parent->m_uid << std::endl;
					auto tp_start = std::chrono::high_resolution_clock::now();

					// Validate Connection
					auto conn = Controller::GetInstance().GetConnection();
					if (!conn) {
						goto do_sleep;
					}

					// Call
					try {
						std::vector<ipc::value> response = conn->call_synchronous_helper(
							"Volmeter",
							"Query",
							{
								ipc::value(parent->m_uid),
							});
						if (!response.size()) {
							goto do_sleep;
						}
						if ((response.size() == 1) && (response[0].type == ipc::type::Null)) {
							goto do_sleep;
						}

						ErrorCode error = (ErrorCode)response[0].value_union.ui64;
						if (error == ErrorCode::Ok) {
							if (parent->m_uid == 4)
								std::cout << "receiving data for " << parent->m_uid << std::endl;
							VolmeterData* data = new VolmeterData { {}, {}, {}};
							size_t channels = response[1].value_union.i32;
							data->magnitude.resize(channels);
							data->peak.resize(channels);
							data->input_peak.resize(channels);

							for (size_t ch = 0; ch < channels; ch++) {
								data->magnitude[ch]  = response[2 + ch * 3 + 0].value_union.fp32;
								data->peak[ch]       = response[2 + ch * 3 + 1].value_union.fp32;
								data->input_peak[ch] = response[2 + ch * 3 + 2].value_union.fp32;
							}
							progress.Send(std::move(data), 1);
						} else if(error == ErrorCode::InvalidReference) {
							goto do_sleep;
						}
						else
						{
							std::cerr << "Failed VolMeter" << std::endl;
							break;
						}
					} catch (std::exception e) {
						goto do_sleep;
					}

				do_sleep:
					auto tp_end  = std::chrono::high_resolution_clock::now();
					auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
					totalSleepMS = parent->sleepIntervalMS - dur.count();
					std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
				}
				std::cout << "end for " << parent->m_uid << std::endl;
			};
			void OnProgress(const VolmeterData* data, size_t size) {
				if (parent->m_uid == 4)
					std::cout << "OnProgress for " << parent->m_uid << std::endl;
				if (!data)
					return;
				Napi::Array magnitude = Napi::Array::New(Env());
				Napi::Array peak = Napi::Array::New(Env());
				Napi::Array input_peak = Napi::Array::New(Env());

				for (size_t i = 0; i < data->magnitude.size(); i++)
					magnitude.Set(i, Napi::Number::New(Env(), data->magnitude[i]));
				for (size_t i = 0; i < data->peak.size(); i++)
					peak.Set(i, Napi::Number::New(Env(), data->peak[i]));
				for (size_t i = 0; i < data->input_peak.size(); i++)
					input_peak.Set(i, Napi::Number::New(Env(), data->input_peak[i]));

				if (data->magnitude.size() == 0 && data->peak.size() == 0 && data->input_peak.size() == 0) {

				} else {
					if (parent->m_uid == 4)
						std::cout << "Calling for " << parent->m_uid << std::endl;
					Callback().Call({ magnitude, peak, input_peak });
				}
			};
			void OnOk() {};
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
