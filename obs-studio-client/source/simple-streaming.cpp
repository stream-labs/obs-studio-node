/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

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

#include "simple-streaming.hpp"
#include "utility.hpp"
#include "encoder.hpp"
#include "service.hpp"
#include "delay.hpp"
#include "reconnect.hpp"

// This is to import SignalInfo
// Remove me when done
#include "nodeobs_service.hpp"


Napi::FunctionReference osn::SimpleStreaming::constructor;

Napi::Object osn::SimpleStreaming::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"SimpleStreaming",
		{
            StaticMethod("create", &osn::SimpleStreaming::Create),

            InstanceAccessor(
                "videoEncoder",
                &osn::SimpleStreaming::GetVideoEncoder,
                &osn::SimpleStreaming::SetVideoEncoder),
            InstanceAccessor(
                "service",
                &osn::SimpleStreaming::GetService,
                &osn::SimpleStreaming::SetService),
            InstanceAccessor(
                "enforceServiceBitrate",
                &osn::SimpleStreaming::GetEnforceServiceBirate,
                &osn::SimpleStreaming::SetEnforceServiceBirate),
            InstanceAccessor(
                "enableTwitchVOD",
                &osn::SimpleStreaming::GetEnableTwitchVOD,
                &osn::SimpleStreaming::SetEnableTwitchVOD),
            InstanceAccessor(
                "audioBitrate",
                &osn::SimpleStreaming::GetAudioBitrate,
                &osn::SimpleStreaming::SetAudioBitrate),
            InstanceAccessor(
                "signalHandler",
                &osn::SimpleStreaming::GetSignalHandler,
                &osn::SimpleStreaming::SetSignalHandler),
            InstanceAccessor(
                "delay",
                &osn::SimpleStreaming::GetDelay,
                &osn::SimpleStreaming::SetDelay),
            InstanceAccessor(
                "reconnect",
                &osn::SimpleStreaming::GetReconnect,
                &osn::SimpleStreaming::SetReconnect),

			InstanceMethod("start", &osn::SimpleStreaming::Start),
			InstanceMethod("stop", &osn::SimpleStreaming::Stop)
		});

	exports.Set("SimpleStreaming", func);
	osn::SimpleStreaming::constructor = Napi::Persistent(func);
	osn::SimpleStreaming::constructor.SuppressDestruct();

	return exports;
}

osn::SimpleStreaming::SimpleStreaming(const Napi::CallbackInfo& info)
	: Napi::ObjectWrap<osn::SimpleStreaming>(info), Worker("simpleStreaming") {
	Napi::Env env = info.Env();
	Napi::HandleScope scope(env);
	int length = info.Length();

	if (length <= 0 || !info[0].IsNumber()) {
		Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
		return;
	}

	this->uid = (uint64_t)info[0].ToNumber().Int64Value();
}

Napi::Value osn::SimpleStreaming::Create(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SimpleStreaming", "Create", {});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	auto instance =
		osn::SimpleStreaming::constructor.New({
			Napi::Number::New(info.Env(), response[1].value_union.ui64)
		});

	return instance;
}

Napi::Value osn::SimpleStreaming::GetService(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			"SimpleStreaming",
			"GetService",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	auto instance =
		osn::Service::constructor.New({
			Napi::Number::New(info.Env(), response[1].value_union.ui64)
		});

	return instance;
}

void osn::SimpleStreaming::SetService(const Napi::CallbackInfo& info, const Napi::Value& value) {
	osn::Service* service = Napi::ObjectWrap<osn::Service>::Unwrap(value.ToObject());

	if (!service) {
		Napi::TypeError::New(info.Env(), "Invalid service argument").ThrowAsJavaScriptException();
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(
		"SimpleStreaming",
		"SetService",
		{ipc::value(this->uid), ipc::value(service->uid)});
}

Napi::Value osn::SimpleStreaming::GetVideoEncoder(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			"SimpleStreaming",
			"GetVideoEncoder",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	auto instance =
		osn::Encoder::constructor.New({
			Napi::Number::New(info.Env(), response[1].value_union.ui64)
		});

	return instance;
}

void osn::SimpleStreaming::SetVideoEncoder(const Napi::CallbackInfo& info, const Napi::Value& value) {
	osn::Encoder* encoder = Napi::ObjectWrap<osn::Encoder>::Unwrap(value.ToObject());

	if (!encoder) {
		Napi::TypeError::New(info.Env(), "Invalid encoder argument").ThrowAsJavaScriptException();
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(
		"SimpleStreaming",
		"SetVideoEncoder",
		{ipc::value(this->uid), ipc::value(encoder->uid)});
}

Napi::Value osn::SimpleStreaming::GetEnforceServiceBirate(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			"SimpleStreaming",
			"GetEnforceServiceBirate",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), response[1].value_union.ui32);
}

void osn::SimpleStreaming::SetEnforceServiceBirate(const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		"SimpleStreaming",
		"SetEnforceServiceBirate",
		{ipc::value(this->uid), ipc::value(value.ToBoolean().Value())});
}

Napi::Value osn::SimpleStreaming::GetEnableTwitchVOD(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			"SimpleStreaming",
			"GetEnableTwitchVOD",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), response[1].value_union.ui32);
}

void osn::SimpleStreaming::SetEnableTwitchVOD(const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		"SimpleStreaming",
		"SetEnableTwitchVOD",
		{ipc::value(this->uid), ipc::value(value.ToBoolean().Value())});
}

Napi::Value osn::SimpleStreaming::GetAudioBitrate(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			"SimpleStreaming",
			"GetAudioBitrate",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), response[1].value_union.ui32);
}

void osn::SimpleStreaming::SetAudioBitrate(const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		"SimpleStreaming",
		"SetAudioBitrate",
		{ipc::value(this->uid), ipc::value(value.ToNumber().Uint32Value())});
}

Napi::Value osn::SimpleStreaming::GetDelay(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			"SimpleStreaming",
			"GetDelay",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	auto instance =
		osn::Delay::constructor.New({
			Napi::Number::New(info.Env(), response[1].value_union.ui64)
		});

	return instance;
}

void osn::SimpleStreaming::SetDelay(const Napi::CallbackInfo& info, const Napi::Value& value) {
	osn::Delay* delay = Napi::ObjectWrap<osn::Delay>::Unwrap(value.ToObject());

	if (!delay) {
		Napi::TypeError::New(
			info.Env(),
			"Invalid delay argument").ThrowAsJavaScriptException();
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(
		"SimpleStreaming",
		"SetDelay",
		{ipc::value(this->uid), ipc::value(delay->uid)});
}

Napi::Value osn::SimpleStreaming::GetReconnect(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			"SimpleStreaming",
			"GetReconnect",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	auto instance =
		osn::Reconnect::constructor.New({
			Napi::Number::New(info.Env(), response[1].value_union.ui64)
		});

	return instance;
}

void osn::SimpleStreaming::SetReconnect(const Napi::CallbackInfo& info, const Napi::Value& value) {
	osn::Reconnect* reconnect =
		Napi::ObjectWrap<osn::Reconnect>::Unwrap(value.ToObject());

	if (!reconnect) {
		Napi::TypeError::New(
			info.Env(),
			"Invalid reconnect argument").ThrowAsJavaScriptException();
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(
		"SimpleStreaming",
		"SetReconnect",
		{ipc::value(this->uid), ipc::value(reconnect->uid)});
}

Napi::Value osn::SimpleStreaming::GetSignalHandler(const Napi::CallbackInfo& info) {
	if (this->cb.IsEmpty())
		return info.Env().Undefined();

	return this->cb.Value();
}
void osn::SimpleStreaming::SetSignalHandler(const Napi::CallbackInfo& info, const Napi::Value& value) {
	Napi::Function cb = value.As<Napi::Function>();
	if (cb.IsNull() || !cb.IsFunction())
		return;

	if (isWorkerRunning) {
		stopWorker();
	}

	this->cb = Napi::Persistent(cb);
	this->cb.SuppressDestruct();
}

void osn::SimpleStreaming::Start(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	if (!isWorkerRunning) {
		startWorker(info.Env(), this->cb.Value());
		isWorkerRunning = true;
	}

	conn->call("SimpleStreaming", "Start", {ipc::value(this->uid)});
}

void osn::SimpleStreaming::Stop(const Napi::CallbackInfo& info) {
	bool force = false;
	if (info.Length() == 1)
		force = info[0].ToBoolean().Value();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SimpleStreaming", "Stop", {ipc::value(this->uid), ipc::value(force)});
}

void osn::SimpleStreaming::worker()
{
	const static int maximum_signals_in_queue = 100;
	auto callback = []( Napi::Env env, Napi::Function jsCallback, SignalInfo* data ) {
		try {
			Napi::Object result = Napi::Object::New(env);

			result.Set(
				Napi::String::New(env, "type"),
				Napi::String::New(env, data->outputType));
			result.Set(
				Napi::String::New(env, "signal"),
				Napi::String::New(env, data->signal));
			result.Set(
				Napi::String::New(env, "code"),
				Napi::Number::New(env, data->code));
			result.Set(
				Napi::String::New(env, "error"),
				Napi::String::New(env, data->errorMessage));

			jsCallback.Call({ result });
		} catch (...) {
			data->tosend = true;
			return;
		}
		data->sent = true;

	};
	size_t totalSleepMS = 0;
	std::vector<SignalInfo*> signalsList;
	while (!workerStop) {
		auto tp_start = std::chrono::high_resolution_clock::now();

		// Validate Connection
		auto conn = Controller::GetInstance().GetConnection();
		if (conn) {
			std::vector<ipc::value> response =
				conn->call_synchronous_helper("SimpleStreaming", "Query", {ipc::value(this->uid)});
			if (response.size() && (response.size() == 5) && signalsList.size() < maximum_signals_in_queue) {
				ErrorCode error = (ErrorCode)response[0].value_union.ui64;
				if (error == ErrorCode::Ok) {
					SignalInfo* data = new SignalInfo{ "", "", 0, ""};
					data->outputType   = response[1].value_str;
					data->signal       = response[2].value_str;
					data->code         = response[3].value_union.i32;
					data->errorMessage = response[4].value_str;
					data->sent         = false;
					data->tosend       = true;
					signalsList.push_back(data);
				}
			}

			std::vector<SignalInfo*>::iterator i = signalsList.begin();
			while (i != signalsList.end()) {
				if ((*i)->tosend) {
					(*i)->tosend = false;
					napi_status status = jsThread.BlockingCall((*i), callback);
					if (status != napi_ok) {
						(*i)->tosend = true;
						break;
					}
				}
				if ((*i)->sent) {
					i = signalsList.erase(i);
				} else {
					i++;
				}
			}
		}

		auto tp_end  = std::chrono::high_resolution_clock::now();
		auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
		totalSleepMS = sleepIntervalMS - dur.count();
		std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	}

	for (auto & signalData : signalsList) {
		delete signalData;
	}
	return;
}
