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

bool osn::Volmeter::m_all_workers_stop = false;

void osn::Volmeter::start_worker(napi_env env, Napi::Function async_callback)
{
	if (!worker_stop)
		return;

	worker_stop = false;
	js_thread = Napi::ThreadSafeFunction::New(
      env,
      async_callback,
      "Volmeter " + this->m_uid,
      0,
      1,
      []( Napi::Env ) {} );
	worker_thread = new std::thread(&osn::Volmeter::worker, this);
}

void osn::Volmeter::stop_worker(void)
{
	if (worker_stop != false)
		return;

	worker_stop = true;
	if (worker_thread->joinable()) {
		worker_thread->join();
	}
}

void osn::Volmeter::worker()
{
    // auto callback = []( Napi::Env env, Napi::Function jsCallback, VolmeterData* data ) {
	// 	Napi::Array magnitude = Napi::Array::New(env);
	// 	Napi::Array peak = Napi::Array::New(env);
	// 	Napi::Array input_peak = Napi::Array::New(env);

	// 	for (size_t i = 0; i < data->magnitude.size(); i++) {
	// 		magnitude.Set(i, Napi::Number::New(env, data->magnitude[i]));
	// 	}
	// 	for (size_t i = 0; i < data->peak.size(); i++) {
	// 		peak.Set(i, Napi::Number::New(env, data->peak[i]));
	// 	}
	// 	for (size_t i = 0; i < data->input_peak.size(); i++) {
	// 		input_peak.Set(i, Napi::Number::New(env, data->input_peak[i]));
	// 	}

	// 	if (data->magnitude.size() > 0 && data->peak.size() > 0 && data->input_peak.size() > 0) {
	// 		jsCallback.Call({ magnitude, peak, input_peak });
	// 	}
    // };
	// size_t totalSleepMS = 0;

	// while (!worker_stop && !m_all_workers_stop) {
	// 	auto tp_start = std::chrono::high_resolution_clock::now();

	// 	auto conn = Controller::GetInstance().GetConnection();
	// 	if (!conn) {
	// 		goto do_sleep;
	// 	}

	// 	try {
	// 		std::vector<ipc::value> response = conn->call_synchronous_helper(
	// 		    "Volmeter",
	// 		    "Query",
	// 		    {
	// 		        ipc::value(m_uid),
	// 		    });
	// 		if (!response.size()) {
	// 			goto do_sleep;
	// 		}
	// 		if ((response.size() == 1) && (response[0].type == ipc::type::Null)) {
	// 			goto do_sleep;
	// 		}

	// 		ErrorCode error = (ErrorCode)response[0].value_union.ui64;
	// 		if (error == ErrorCode::Ok) {
	// 			VolmeterData* data     = new VolmeterData{{}, {}, {}};
	// 			size_t                             channels = response[1].value_union.i32;
	// 			data->magnitude.resize(channels);
	// 			data->peak.resize(channels);
	// 			data->input_peak.resize(channels);

	// 			for (size_t ch = 0; ch < channels; ch++) {
	// 				data->magnitude[ch]  = response[2 + ch * 3 + 0].value_union.fp32;
	// 				data->peak[ch]       = response[2 + ch * 3 + 1].value_union.fp32;
	// 				data->input_peak[ch] = response[2 + ch * 3 + 2].value_union.fp32;
	// 			}
	// 			js_thread.BlockingCall( data, callback );
	// 		} else if(error == ErrorCode::InvalidReference) {
	// 			goto do_sleep;
	// 		}
	// 		else
	// 		{
	// 			std::cerr << "Failed VolMeter" << std::endl;
	// 			break;
	// 		}
	// 	} catch (std::exception e) {
	// 		goto do_sleep;
	// 	}

	// do_sleep:
	// 	auto tp_end  = std::chrono::high_resolution_clock::now();
	// 	auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
	// 	totalSleepMS = sleepIntervalMS - dur.count();
	// 	std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	// }
	// js_thread.Release();
}

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

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Volmeter", "AddCallback", {ipc::value(this->m_uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	// start_worker(info.Env(), async_callback);
	// isWorkerRunning = true;

	globalCallback::add_volmeter(info.Env(), this->m_uid, async_callback);

	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value osn::Volmeter::RemoveCallback(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Volmeter", "RemoveCallback", {ipc::value(this->m_uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	// if (isWorkerRunning)
	// 	stop_worker();

	globalCallback::remove_volmeter(this->m_uid);

	return Napi::Boolean::New(info.Env(), true);
}