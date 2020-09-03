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
#include "utility-v8.hpp"

namespace osn
{
	class Module : public Napi::ObjectWrap<osn::Module>
	{
		public:
		uint64_t moduleId;

		public:
		static Napi::FunctionReference constructor;
		static Napi::Object Init(Napi::Env env, Napi::Object exports);
		Module(const Napi::CallbackInfo& info);

		static Napi::Value Open(const Napi::CallbackInfo& info);
		static Napi::Value Modules(const Napi::CallbackInfo& info);

		Napi::Value Initialize(const Napi::CallbackInfo& info);

		Napi::Value Name(const Napi::CallbackInfo& info);
		Napi::Value FileName(const Napi::CallbackInfo& info);
		Napi::Value Author(const Napi::CallbackInfo& info);
		Napi::Value Description(const Napi::CallbackInfo& info);
		Napi::Value BinaryPath(const Napi::CallbackInfo& info);
		Napi::Value DataPath(const Napi::CallbackInfo& info);
	};
}
