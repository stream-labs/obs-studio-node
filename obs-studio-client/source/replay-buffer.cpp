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

#include "replay-buffer.hpp"
#include "utility.hpp"
#include "encoder.hpp"

Napi::Value osn::ReplayBuffer::GetPath(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetPath",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::ReplayBuffer::SetPath(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetPath",
		{ipc::value(this->uid), ipc::value(value.ToString().Utf8Value())});
}

Napi::Value osn::ReplayBuffer::GetFormat(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetFormat",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::ReplayBuffer::SetFormat(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetFormat",
		{ipc::value(this->uid), ipc::value(value.ToString().Utf8Value())});
}

Napi::Value osn::ReplayBuffer::GetMuxerSettings(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetMuxerSettings",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::ReplayBuffer::SetMuxerSettings(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetMuxerSettings",
		{ipc::value(this->uid), ipc::value(value.ToString().Utf8Value())});
}

Napi::Value osn::ReplayBuffer::GetFileFormat(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetFileFormat",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::ReplayBuffer::SetFileFormat(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetFileFormat",
		{ipc::value(this->uid), ipc::value(value.ToString().Utf8Value())});
}

Napi::Value osn::ReplayBuffer::GetOverwrite(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetOverwrite",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), response[1].value_union.ui32);
}

void osn::ReplayBuffer::SetOverwrite(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetOverwrite",
		{ipc::value(this->uid), ipc::value((uint32_t)value.ToBoolean().Value())});
}

Napi::Value osn::ReplayBuffer::GetNoSpace(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetNoSpace",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), response[1].value_union.ui32);
}

void osn::ReplayBuffer::SetNoSpace(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetNoSpace",
		{ipc::value(this->uid), ipc::value((uint32_t)value.ToBoolean().Value())});
}

Napi::Value osn::ReplayBuffer::GetDuration(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetDuration",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), response[1].value_union.ui32);
}

void osn::ReplayBuffer::SetDuration(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetDuration",
		{ipc::value(this->uid), ipc::value(value.ToNumber().Uint32Value())});
}

Napi::Value osn::ReplayBuffer::GetPrefix(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetPrefix",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::ReplayBuffer::SetPrefix(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetPrefix",
		{ipc::value(this->uid), ipc::value(value.ToString().Utf8Value())});
}

Napi::Value osn::ReplayBuffer::GetSuffix(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetSuffix",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response[1].value_str);
}

void osn::ReplayBuffer::SetSuffix(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetSuffix",
		{ipc::value(this->uid), ipc::value(value.ToString().Utf8Value())});
}

Napi::Value osn::ReplayBuffer::GetUsesStream(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetUsesStream",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), response[1].value_union.ui32);
}

void osn::ReplayBuffer::SetUsesStream(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call_synchronous_helper(
		className,
		"SetUsesStream",
		{ipc::value(this->uid), ipc::value(value.ToBoolean().Value())});
}

Napi::Value osn::ReplayBuffer::GetVideoEncoder(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
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

void osn::ReplayBuffer::SetVideoEncoder(const Napi::CallbackInfo& info, const Napi::Value& value) {
	osn::Encoder* encoder = Napi::ObjectWrap<osn::Encoder>::Unwrap(value.ToObject());

	if (!encoder) {
		Napi::TypeError::New(info.Env(),
            "Invalid encoder argument").ThrowAsJavaScriptException();
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(
		className,
		"SetVideoEncoder",
		{ipc::value(this->uid), ipc::value(encoder->uid)});
}

Napi::Value osn::ReplayBuffer::GetSignalHandler(const Napi::CallbackInfo& info) {
	if (this->cb.IsEmpty())
		return info.Env().Undefined();

	return this->cb.Value();
}

void osn::ReplayBuffer::SetSignalHandler(const Napi::CallbackInfo& info, const Napi::Value& value) {
	Napi::Function cb = value.As<Napi::Function>();
	if (cb.IsNull() || !cb.IsFunction())
		return;

	if (isWorkerRunning) {
		stopWorker();
	}

	this->cb = Napi::Persistent(cb);
	this->cb.SuppressDestruct();
}

void osn::ReplayBuffer::Start(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	if (!isWorkerRunning) {
		startWorker(info.Env(), this->cb.Value(), className, this->uid);
		isWorkerRunning = true;
	}

	conn->call(className, "Start", {ipc::value(this->uid)});
}

void osn::ReplayBuffer::Stop(const Napi::CallbackInfo& info) {
	bool force = false;
	if (info.Length() == 1)
		force = info[0].ToBoolean().Value();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(className, "Stop", {ipc::value(this->uid), ipc::value(force)});
}

void osn::ReplayBuffer::Save(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(className, "Save", {ipc::value(this->uid)});
}

Napi::Value osn::ReplayBuffer::GetLastReplay(const Napi::CallbackInfo& info) {
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper(
			className,
			"GetLastReplay",
			{ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), response[1].value_str);
}