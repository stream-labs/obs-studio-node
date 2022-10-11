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

#include "recording.hpp"
#include "utility.hpp"
#include "video-encoder.hpp"

Napi::Value osn::Recording::GetVideoEncoder(const Napi::CallbackInfo &info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response = conn->call_synchronous_helper(className, "GetVideoEncoder", {ipc::value(this->uid)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	auto instance = osn::VideoEncoder::constructor.New({Napi::Number::New(info.Env(), response[1].value_union.ui64)});

	return instance;
}

void osn::Recording::SetVideoEncoder(const Napi::CallbackInfo &info, const Napi::Value &value)
{
	osn::VideoEncoder *encoder = Napi::ObjectWrap<osn::VideoEncoder>::Unwrap(value.ToObject());

	if (!encoder) {
		Napi::TypeError::New(info.Env(), "Invalid encoder argument").ThrowAsJavaScriptException();
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(className, "SetVideoEncoder", {ipc::value(this->uid), ipc::value(encoder->uid)});
}

Napi::Value osn::Recording::GetSignalHandler(const Napi::CallbackInfo &info)
{
	if (this->cb.IsEmpty())
		return info.Env().Undefined();

	return this->cb.Value();
}

void osn::Recording::SetSignalHandler(const Napi::CallbackInfo &info, const Napi::Value &value)
{
	Napi::Function cb = value.As<Napi::Function>();
	if (cb.IsNull() || !cb.IsFunction())
		return;

	if (isWorkerRunning) {
		stopWorker();
	}

	this->cb = Napi::Persistent(cb);
	this->cb.SuppressDestruct();
}

void osn::Recording::Start(const Napi::CallbackInfo &info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return;

	if (!isWorkerRunning) {
		startWorker(info.Env(), this->cb.Value(), className, this->uid);
		isWorkerRunning = true;
	}

	conn->call(className, "Start", {ipc::value(this->uid)});
}

void osn::Recording::Stop(const Napi::CallbackInfo &info)
{
	bool force = false;
	if (info.Length() == 1)
		force = info[0].ToBoolean().Value();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(className, "Stop", {ipc::value(this->uid), ipc::value(force)});
}