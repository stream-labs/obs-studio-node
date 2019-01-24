// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#pragma once
#include <nan.h>
#include <node.h>
#include <thread>
#include "utility-v8.hpp"

namespace osn
{
	struct VolMeterData
	{
		std::vector<float> magnitude;
		std::vector<float> peak;
		std::vector<float> input_peak;
		void*              param;
	};

	typedef utilv8::managed_callback<std::shared_ptr<osn::VolMeterData>> VolMeterCallback;

	class VolMeter : public Nan::ObjectWrap,
	                 public utilv8::InterfaceObject<osn::VolMeter>,
	                 public utilv8::ManagedObject<osn::VolMeter>
	{
		friend utilv8::InterfaceObject<osn::VolMeter>;
		friend utilv8::ManagedObject<osn::VolMeter>;
		friend utilv8::CallbackData<osn::VolMeterData, osn::VolMeter>;

		uint64_t m_uid;
		uint32_t m_sleep_interval = 33;

		std::thread m_worker;
		bool        m_worker_stop = true;
		std::mutex  m_worker_lock;

		osn::VolMeterCallback* m_async_callback = nullptr;
		Nan::Callback          m_callback_function;

		public:
		VolMeter(uint64_t uid);
		~VolMeter();

		uint64_t GetId();

		void start_async_runner();
		void stop_async_runner();
		void callback_handler(void* data, std::shared_ptr<osn::VolMeterData> item);

		void start_worker();
		void stop_worker();
		void worker();

		void set_keepalive(v8::Local<v8::Object>);

		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static Nan::NAN_METHOD_RETURN_TYPE Create(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Attach(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Detach(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE AddCallback(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info);
	};
} // namespace osn
