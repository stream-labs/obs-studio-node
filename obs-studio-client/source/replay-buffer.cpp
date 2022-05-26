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
#include "video-encoder.hpp"
#include "simple-streaming.hpp"
#include "simple-recording.hpp"

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

Napi::Value osn::ReplayBuffer::GetStreaming(
    const Napi::CallbackInfo& info) {
    auto conn = GetConnection(info);
    if (!conn)
        return info.Env().Undefined();

    std::vector<ipc::value> response =
        conn->call_synchronous_helper(
            className,
            "GetStreaming",
            {ipc::value(this->uid)});

    if (!ValidateResponse(info, response))
        return info.Env().Undefined();

    auto instance =
        osn::SimpleStreaming::constructor.New({
            Napi::Number::New(info.Env(), response[1].value_union.ui64)
        });
    return instance;
}

void osn::ReplayBuffer::SetStreaming(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
    osn::SimpleStreaming* encoder =
        Napi::ObjectWrap<osn::SimpleStreaming>::Unwrap(value.ToObject());

    if (!encoder) {
        Napi::TypeError::New(info.Env(),
            "Invalid streaming argument").ThrowAsJavaScriptException();
        return;
    }

    auto conn = GetConnection(info);
    if (!conn)
        return;

    conn->call(
        className,
        "SetStreaming",
        {ipc::value(this->uid), ipc::value(encoder->uid)});
}

Napi::Value osn::ReplayBuffer::GetRecording(
    const Napi::CallbackInfo& info) {
    auto conn = GetConnection(info);
    if (!conn)
        return info.Env().Undefined();

    std::vector<ipc::value> response =
        conn->call_synchronous_helper(
            className,
            "GetRecording",
            {ipc::value(this->uid)});

    if (!ValidateResponse(info, response))
        return info.Env().Undefined();

    auto instance =
        osn::SimpleRecording::constructor.New({
            Napi::Number::New(info.Env(), response[1].value_union.ui64)
        });
    return instance;
}

void osn::ReplayBuffer::SetRecording(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
    osn::SimpleRecording* recording =
        Napi::ObjectWrap<osn::SimpleRecording>::Unwrap(value.ToObject());

    if (!recording) {
        Napi::TypeError::New(info.Env(),
            "Invalid streaming argument").ThrowAsJavaScriptException();
        return;
    }

    auto conn = GetConnection(info);
    if (!conn)
        return;

    conn->call(
        className,
        "SetRecording",
        {ipc::value(this->uid), ipc::value(recording->uid)});
}