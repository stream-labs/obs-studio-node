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

namespace osn
{
	struct VolmeterData
	{
		std::vector<float> magnitude;
		std::vector<float> peak;
		std::vector<float> input_peak;
		void*              param;
	};

	// typedef utilv8::managed_callback<std::shared_ptr<osn::VolMeterData>> VolMeterCallback;

	class Volmeter : public Napi::ObjectWrap<osn::Volmeter>
	{
		// friend utilv8::InterfaceObject<osn::VolMeter>;
		// friend utilv8::ManagedObject<osn::VolMeter>;
		// friend utilv8::CallbackData<osn::VolMeterData, osn::VolMeter>;

		public:
		uint64_t m_uid;
		uint32_t m_sleep_interval = 33;

		// std::thread m_worker;
		// bool        m_worker_stop = true;
		// std::mutex  m_worker_lock;

		// osn::VolMeterCallback* m_async_callback = nullptr;
		// Nan::Callback          m_callback_function;

		// VolMeter(uint64_t uid);
		// ~VolMeter();

		// uint64_t GetId();

		// void start_async_runner();
		// void stop_async_runner();
		// void callback_handler(void* data, std::shared_ptr<osn::VolMeterData> item);

		// void start_worker();
		// void stop_worker();
		// void worker();

		// void set_keepalive(v8::Local<v8::Object>);

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
