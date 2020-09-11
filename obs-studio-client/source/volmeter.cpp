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

bool osn::Volmeter::m_all_workers_stop = false;

Napi::FunctionReference osn::Volmeter::constructor;

Napi::Object osn::Volmeter::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Volmeter",
		{
			StaticMethod("create", &osn::Volmeter::Create),

			InstanceAccessor("updateInterval", &osn::Volmeter::GetUpdateInterval, &osn::Volmeter::SetUpdateInterval),

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
	isWorkerRunning = false;
	worker_stop = true;
	sleepIntervalMS = info[1].ToNumber().Uint32Value();
	// sleepIntervalMS = 500;
	asyncWorker = nullptr;
	worker_thread = nullptr;
}

Napi::Value osn::Volmeter::Create(const Napi::CallbackInfo& info)
{
	int32_t type = info[0].ToNumber().Int32Value();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Volmeter",
	    "Create",
	    {
	        ipc::value(type),
	    });

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

    auto instance =
        osn::Volmeter::constructor.New({
            Napi::Number::New(info.Env(), response[1].value_union.ui64),
            Napi::Number::New(info.Env(), response[2].value_union.ui32)
            });
    return instance;
}

Napi::Value osn::Volmeter::GetUpdateInterval(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Volmeter",
	    "GetUpdateInterval",
	    {
	        ipc::value(this->m_uid),
	    });

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	this->sleepIntervalMS = response[1].value_union.ui32;
	return Napi::Number::New(info.Env(), this->sleepIntervalMS);
}

void osn::Volmeter::SetUpdateInterval(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("Volmeter", "SetUpdateInterval", {
		ipc::value(this->m_uid),
		ipc::value(value.ToNumber().Uint32Value())
		});
}

Napi::Value osn::Volmeter::Attach(const Napi::CallbackInfo& info)
{
	osn::Input* input = Napi::ObjectWrap<osn::Input>::Unwrap(info[0].ToObject());

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::cout << "Attaching volmeter " << this->m_uid << " to source " << input->sourceId << std::endl;
	conn->call("Volmeter", "Attach", {ipc::value(this->m_uid), ipc::value(input->sourceId)});
	return info.Env().Undefined();
}

Napi::Value osn::Volmeter::Detach(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("Volmeter", "Detach", {ipc::value(this->m_uid)});
	return info.Env().Undefined();
}

Napi::Value osn::Volmeter::AddCallback(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::cout << "Adding callback for " << this->m_uid	<< std::endl;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Volmeter", "AddCallback", {ipc::value(this->m_uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	worker_stop = false;
	std::cout << "creating asyncWorker for " << this->m_uid << std::endl;
	asyncWorker = new osn::Volmeter::Worker(async_callback, this);
	asyncWorker->SuppressDestruct();
	std::cout << "asyncWorker queue for " << this->m_uid << std::endl;
	asyncWorker->Queue();
	isWorkerRunning = true;
	
	std::cout << "return for " << this->m_uid << std::endl;
	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value osn::Volmeter::RemoveCallback(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::cout << "Remove callback for " << this->m_uid	<< std::endl;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Volmeter", "RemoveCallback", {ipc::value(this->m_uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	worker_stop = true;
	// asyncWorker->Cancel();
	// delete asyncWorker;
	std::cout << "Delete async worker for " << this->m_uid	<< std::endl;
	return Napi::Boolean::New(info.Env(), true);
}