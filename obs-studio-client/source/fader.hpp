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

		private:
		uint64_t uid;

		public:
		Fader(uint64_t uid);
		~Fader();
		
		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static void Create(Nan::NAN_METHOD_ARGS_TYPE info);
		static void GetDeziBel(Nan::NAN_METHOD_ARGS_TYPE info);
		static void SetDezibel(Nan::NAN_METHOD_ARGS_TYPE info);
		static void GetDeflection(Nan::NAN_METHOD_ARGS_TYPE info);
		static void SetDeflection(Nan::NAN_METHOD_ARGS_TYPE info);
		static void GetMultiplier(Nan::NAN_METHOD_ARGS_TYPE info);
		static void SetMultiplier(Nan::NAN_METHOD_ARGS_TYPE info);
		static void Attach(Nan::NAN_METHOD_ARGS_TYPE info);
		static void Detach(Nan::NAN_METHOD_ARGS_TYPE info);
		static void AddCallback(Nan::NAN_METHOD_ARGS_TYPE info);
		static void RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info);
	};
}