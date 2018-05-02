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
#include <node.h>
#include <nan.h>
#include "utility-v8.hpp"

namespace osn {
	class Fader : public Nan::ObjectWrap,
		public utilv8::InterfaceObject<osn::Fader>,
		public utilv8::ManagedObject<osn::Fader> {
		friend utilv8::InterfaceObject<osn::Fader>;
		friend utilv8::ManagedObject<osn::Fader>;

		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static Nan::NAN_METHOD_RETURN_TYPE Create(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetDeziBel(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetDezibel(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetDeflection(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetDeflection(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetMul(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetMul(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Attach(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Detach(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE AddCallback(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info);
	};
}