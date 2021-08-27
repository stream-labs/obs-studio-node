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

#include "volmeter.hpp"
#include <iostream>
#include <iterator>
#include <vector>
#include "controller.hpp"
#include "error.hpp"
#include "input.hpp"
#include "shared.hpp"
#include "utility-v8.hpp"
#include "utility.hpp"
#include "callback-manager.hpp"
#include "server/osn-volmeter.hpp"

Napi::FunctionReference osn::Volmeter::constructor;

Napi::Object osn::Volmeter::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Volmeter",
		{
			StaticMethod("create", &osn::Volmeter::Create),

			InstanceMethod("destroy", &osn::Volmeter::Destroy),
			InstanceMethod("attach", &osn::Volmeter::Attach),
			InstanceMethod("detach", &osn::Volmeter::Detach),
			InstanceMethod("addCallback", &osn::Volmeter::AddCallback),
			InstanceMethod("removeCallback", &osn::Volmeter::RemoveCallback),
		});
	exports.Set("Volmeter", func);
	osn::Volmeter::constructor = Napi::Persistent(func);
	osn::Volmeter::constructor.SuppressDestruct();
	return exports;
}

osn::Volmeter::Volmeter(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Volmeter>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();

    if (length <= 0 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }

	this->m_uid = (uint64_t)info[0].ToNumber().Int64Value();
}

Napi::Value osn::Volmeter::Create(const Napi::CallbackInfo& info)
{
	int32_t type = info[0].ToNumber().Int32Value();

	auto res = obs::Volmeter::Create(type);

    auto instance =
        osn::Volmeter::constructor.New({
            Napi::Number::New(info.Env(), res.first),
            Napi::Number::New(info.Env(), res.second)
            });
    return instance;
}

Napi::Value osn::Volmeter::Destroy(const Napi::CallbackInfo& info)
{
	obs::Volmeter::Destroy(this->m_uid);

	return info.Env().Undefined();
}

Napi::Value osn::Volmeter::Attach(const Napi::CallbackInfo& info)
{
	osn::Input* input = Napi::ObjectWrap<osn::Input>::Unwrap(info[0].ToObject());

	obs::Volmeter::Attach(this->m_uid, input->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Volmeter::Detach(const Napi::CallbackInfo& info)
{
	obs::Volmeter::Detach(this->m_uid);

	return info.Env().Undefined();
}

Napi::Value osn::Volmeter::AddCallback(const Napi::CallbackInfo& info)
{
	// std::unique_lock<std::mutex> lck(globalCallback::mtx_volmeters);
	// Napi::Function async_callback = info[0].As<Napi::Function>();

	// auto conn = GetConnection(info);
	// if (!conn)
	// 	return info.Env().Undefined();

	// std::vector<ipc::value> response =
	// 	conn->call_synchronous_helper("Volmeter", "AddCallback", {ipc::value(this->m_uid)});

	// if (!ValidateResponse(info, response))
	// 	return info.Env().Undefined();

	// globalCallback::add_volmeter(info.Env(), this->m_uid, async_callback);

	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value osn::Volmeter::RemoveCallback(const Napi::CallbackInfo& info)
{
	// std::unique_lock<std::mutex> lck(globalCallback::mtx_volmeters);
	// auto conn = GetConnection(info);
	// if (!conn)
	// 	return info.Env().Undefined();

	// std::vector<ipc::value> response =
	// 	conn->call_synchronous_helper("Volmeter", "RemoveCallback", {ipc::value(this->m_uid)});

	// if (!ValidateResponse(info, response))
	// 	return info.Env().Undefined();

	// globalCallback::remove_volmeter(this->m_uid);

	return Napi::Boolean::New(info.Env(), true);
}