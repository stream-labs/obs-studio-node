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
#include <nan.h>
#include <node.h>
#include "utility-v8.hpp"

namespace osn
{
	class Fader : public Nan::ObjectWrap,
	              public utilv8::InterfaceObject<osn::Fader>,
	              public utilv8::ManagedObject<osn::Fader>
	{
		friend utilv8::InterfaceObject<osn::Fader>;
		friend utilv8::ManagedObject<osn::Fader>;

		private:
		uint64_t uid;

		public:
		Fader(uint64_t uid);
		~Fader();

		uint64_t GetId();

		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(v8::Local<v8::Object> exports);

		static void Create(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetDeziBel(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetDezibel(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetMultiplier(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetMultiplier(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Attach(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Detach(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
	};
} // namespace osn
