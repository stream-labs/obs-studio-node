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

// Nan::Persistent<v8::FunctionTemplate> osn::VolMeter::prototype;

// osn::VolMeter::VolMeter(uint64_t p_uid)
// {
// 	m_uid = p_uid;
// }

// osn::VolMeter::~VolMeter()
// {
// }

// uint64_t osn::VolMeter::GetId() 
// {
// 	return m_uid;
// }

// void osn::VolMeter::start_async_runner()
// {
// 	if (m_async_callback)
// 		return;

// 	std::unique_lock<std::mutex> ul(m_worker_lock);

// 	// Start v8/uv asynchronous runner.
// 	m_async_callback = new osn::VolMeterCallback();
// 	m_async_callback->set_handler(std::bind(&VolMeter::callback_handler, this, std::placeholders::_1, std::placeholders::_2), nullptr);
// }

// void osn::VolMeter::stop_async_runner()
// {
// 	if (!m_async_callback)
// 		return;

// 	std::unique_lock<std::mutex> ul(m_worker_lock);

// 	// Stop v8/uv asynchronous runner.
// 	m_async_callback->clear();
// 	m_async_callback->finalize();
// 	m_async_callback = nullptr;
// }

// void osn::VolMeter::callback_handler(void* data, std::shared_ptr<osn::VolMeterData> item)
// {
// 	// utilv8::ToValue on a std::vector<> creates a v8::Local<v8::Array> automatically.
// 	v8::Local<v8::Value> args[] = {
// 	    utilv8::ToValue(item->magnitude), utilv8::ToValue(item->peak), utilv8::ToValue(item->input_peak)};

// 	Nan::Call(m_callback_function, 3, args);
// }

// void osn::VolMeter::start_worker()
// {
// 	if (!m_worker_stop)
// 		return;

// 	// Launch worker thread.
// 	m_worker_stop = false;
// 	m_worker      = std::thread(std::bind(&osn::VolMeter::worker, this));
// }

// void osn::VolMeter::stop_worker()
// {
// 	if (m_worker_stop != false)
// 		return;

// 	// Stop worker thread.
// 	m_worker_stop = true;
// 	if (m_worker.joinable()) {
// 		m_worker.join();
// 	}
// }

// void osn::VolMeter::worker()
// {
// 	size_t totalSleepMS = 0;

// 	while (!m_worker_stop) {
// 		auto tp_start = std::chrono::high_resolution_clock::now();

// 		// Validate Connection
// 		auto conn = Controller::GetInstance().GetConnection();
// 		if (!conn) {
// 			goto do_sleep;
// 		}

// 		// Call
// 		try {
// 			std::unique_lock<std::mutex> ul(m_worker_lock);

// 			if (!m_async_callback)
// 				goto do_sleep;

// 			std::vector<ipc::value> response = conn->call_synchronous_helper(
// 			    "VolMeter",
// 			    "Query",
// 			    {
// 			        ipc::value(m_uid),
// 			    });
// 			if (!response.size()) {
// 				goto do_sleep;
// 			}
// 			if ((response.size() == 1) && (response[0].type == ipc::type::Null)) {
// 				goto do_sleep;
// 			}

// 			ErrorCode error = (ErrorCode)response[0].value_union.ui64;
// 			if (error == ErrorCode::Ok) {
// 				std::shared_ptr<osn::VolMeterData> data     = std::make_shared<osn::VolMeterData>();
// 				size_t                             channels = response[1].value_union.i32;
// 				data->magnitude.resize(channels);
// 				data->peak.resize(channels);
// 				data->input_peak.resize(channels);
// 				data->param = this;
// 				for (size_t ch = 0; ch < channels; ch++) {
// 					data->magnitude[ch]  = response[2 + ch * 3 + 0].value_union.fp32;
// 					data->peak[ch]       = response[2 + ch * 3 + 1].value_union.fp32;
// 					data->input_peak[ch] = response[2 + ch * 3 + 2].value_union.fp32;
// 				}
// 				m_async_callback->queue(std::move(data));
// 			} else if(error == ErrorCode::InvalidReference) {
// 				goto do_sleep;
// 			}
// 			else
// 			{
// 				std::cerr << "Failed VolMeter" << std::endl;
// 				break;
// 			}
// 		} catch (std::exception e) {
// 			goto do_sleep;
// 		}

// 	do_sleep:
// 		auto tp_end  = std::chrono::high_resolution_clock::now();
// 		auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
// 		totalSleepMS = m_sleep_interval - dur.count();
// 		std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
// 	}
// }

// void osn::VolMeter::set_keepalive(v8::Local<v8::Object> obj)
// {
// 	if (!m_async_callback)
// 		return;
// 	m_async_callback->set_keepalive(obj);
// }

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
            Napi::Number::New(info.Env(), response[1].value_union.ui64)
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

	this->m_sleep_interval = response[1].value_union.ui32;
	return Napi::Number::New(info.Env(), this->m_sleep_interval);
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
	return Napi::Boolean::New(info.Env(), true);
	// osn::VolMeter*          self;
	// v8::Local<v8::Function> callback;

	// {
	// 	// Arguments
	// 	ASSERT_INFO_LENGTH(info, 1);
	// 	if (!Retrieve(info.This(), self)) {
	// 		return;
	// 	}

	// 	ASSERT_GET_VALUE(info[0], callback);
	// }

	// {
	// 	// Grab IPC Connection
	// 	std::shared_ptr<ipc::client> conn = nullptr;
	// 	if (!(conn = GetConnection())) {
	// 		return;
	// 	}

	// 	// Send request
	// 	std::vector<ipc::value> rval =
	// 	    conn->call_synchronous_helper("Volmeter", "AddCallback", {ipc::value(self->m_uid)});

	// 	if (!ValidateResponse(info, rval)) {
	// 		info.GetReturnValue().Set(Nan::Null());
	// 		return;
	// 	}
	// }

	// self->m_callback_function.Reset(callback);
	// self->start_async_runner();
	// self->set_keepalive(info.This());
	// self->start_worker();

	// info.GetReturnValue().Set(true);
}

Napi::Value osn::Volmeter::RemoveCallback(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), true);
	// osn::VolMeter* self;

	// {
	// 	ASSERT_INFO_LENGTH(info, 1);
	// 	if (!Retrieve(info.This(), self)) {
	// 		return;
	// 	}
	// }

	// self->stop_worker();
	// self->stop_async_runner();
	// self->m_callback_function.Reset();

	// // Grab IPC Connection
	// {
	// 	std::shared_ptr<ipc::client> conn = nullptr;
	// 	if (!(conn = GetConnection())) {
	// 		return;
	// 	}

	// 	// Send request
	// 	std::vector<ipc::value> rval =
	// 	    conn->call_synchronous_helper("Volmeter", "RemoveCallback", {ipc::value(self->m_uid)});

	// 	if (!ValidateResponse(info, rval)) {
	// 		info.GetReturnValue().Set(Nan::Null());
	// 		return;
	// 	}
	// }

	// info.GetReturnValue().Set(true);
}