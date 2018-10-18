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
#include "callback_manager.hpp"

namespace osn
{
	struct VolMeterData
	{
		std::vector<float> magnitude;
		std::vector<float> peak;
		std::vector<float> input_peak;
		void*              param;
	};

	class VolMeter : public Nan::ObjectWrap,
	                 public utilv8::InterfaceObject<osn::VolMeter>,
	                 public utilv8::ManagedObject<osn::VolMeter>
	{
		friend utilv8::InterfaceObject<osn::VolMeter>;
		friend utilv8::ManagedObject<osn::VolMeter>;
		friend utilv8::CallbackData<osn::VolMeterData, osn::VolMeter>;

		uint64_t m_uid;
		CallbackManager<osn::VolMeterData> m_callback_manager;
		
		public:
		VolMeter(uint64_t uid);
		~VolMeter();

		void callback_update(CallbackManager<osn::VolMeterData>::DataCallback* dataCallback);

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
