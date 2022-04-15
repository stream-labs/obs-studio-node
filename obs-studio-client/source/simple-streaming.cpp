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

			InstanceMethod("start", &osn::SimpleStreaming::Start),
			InstanceMethod("stop", &osn::SimpleStreaming::Stop)
		});

	exports.Set("SimpleStreaming", func);
	osn::SimpleStreaming::constructor = Napi::Persistent(func);
	osn::SimpleStreaming::constructor.SuppressDestruct();

	return exports;
}

osn::SimpleStreaming::SimpleStreaming(const Napi::CallbackInfo& info)
	: Napi::ObjectWrap<osn::SimpleStreaming>(info) {
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

void osn::SimpleStreaming::Start(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SimpleStreaming", "Start", {ipc::value(this->uid)});
}

void osn::SimpleStreaming::Stop(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SimpleStreaming", "Stop", {ipc::value(this->uid)});
}
