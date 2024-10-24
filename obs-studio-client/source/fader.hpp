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
#include "utility.hpp"
#include <thread>

namespace osn {
class Fader : public Napi::ObjectWrap<osn::Fader> {
public:
	uint64_t uid;

public:
	static Napi::FunctionReference constructor;
	static Napi::Object Init(Napi::Env env, Napi::Object exports);
	Fader(const Napi::CallbackInfo &info);

	static Napi::Value Create(const Napi::CallbackInfo &info);

	Napi::Value GetDeziBel(const Napi::CallbackInfo &info);
	void SetDezibel(const Napi::CallbackInfo &info, const Napi::Value &value);
	Napi::Value GetDeflection(const Napi::CallbackInfo &info);
	void SetDeflection(const Napi::CallbackInfo &info, const Napi::Value &value);
	Napi::Value GetMultiplier(const Napi::CallbackInfo &info);
	void SetMultiplier(const Napi::CallbackInfo &info, const Napi::Value &value);
	Napi::Value Destroy(const Napi::CallbackInfo &info);
	Napi::Value Attach(const Napi::CallbackInfo &info);
	Napi::Value Detach(const Napi::CallbackInfo &info);
};
}
