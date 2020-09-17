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

namespace osn
{
	class Volmeter : public Napi::ObjectWrap<osn::Volmeter>
	{
		public:
		uint64_t m_uid;

		bool isWorkerRunning;
		bool worker_stop;
		uint32_t sleepIntervalMS;
		std::thread* worker_thread;
		Napi::ThreadSafeFunction js_thread;

		void worker(void);
		void start_worker(napi_env env, Napi::Function async_callback);
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
