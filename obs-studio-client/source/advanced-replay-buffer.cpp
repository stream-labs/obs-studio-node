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

#include "advanced-replay-buffer.hpp"
#include "utility.hpp"
#include "audio-encoder.hpp"

Napi::FunctionReference osn::AdvancedReplayBuffer::constructor;

Napi::Object osn::AdvancedReplayBuffer::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func =
        DefineClass(env,
        "AdvancedReplayBuffer",
        {
            StaticMethod("create", &osn::AdvancedReplayBuffer::Create),

            InstanceAccessor(
                "path",
                &osn::AdvancedReplayBuffer::GetPath,
                &osn::AdvancedReplayBuffer::SetPath),
            InstanceAccessor(
                "format",
                &osn::AdvancedReplayBuffer::GetFormat,
                &osn::AdvancedReplayBuffer::SetFormat),
            InstanceAccessor(
                "muxerSettings",
                &osn::AdvancedReplayBuffer::GetMuxerSettings,
                &osn::AdvancedReplayBuffer::SetMuxerSettings),
            InstanceAccessor(
                "fileFormat",
                &osn::AdvancedReplayBuffer::GetFileFormat,
                &osn::AdvancedReplayBuffer::SetFileFormat),
            InstanceAccessor(
                "overwrite",
                &osn::AdvancedReplayBuffer::GetOverwrite,
                &osn::AdvancedReplayBuffer::SetOverwrite),
            InstanceAccessor(
                "noSpace",
                &osn::AdvancedReplayBuffer::GetNoSpace,
                &osn::AdvancedReplayBuffer::SetNoSpace),
            InstanceAccessor(
                "duration",
                &osn::AdvancedReplayBuffer::GetDuration,
                &osn::AdvancedReplayBuffer::SetDuration),
            InstanceAccessor(
                "prefix",
                &osn::AdvancedReplayBuffer::GetPrefix,
                &osn::AdvancedReplayBuffer::SetPrefix),
            InstanceAccessor(
                "suffix",
                &osn::AdvancedReplayBuffer::GetSuffix,
                &osn::AdvancedReplayBuffer::SetSuffix),
            InstanceAccessor(
                "videoEncoder",
                &osn::AdvancedReplayBuffer::GetVideoEncoder,
                &osn::AdvancedReplayBuffer::SetVideoEncoder),
            InstanceAccessor(
                "signalHandler",
                &osn::AdvancedReplayBuffer::GetSignalHandler,
                &osn::AdvancedReplayBuffer::SetSignalHandler),
            InstanceAccessor(
                "mixer",
                &osn::AdvancedReplayBuffer::GetMixer,
                &osn::AdvancedReplayBuffer::SetMixer),

            InstanceMethod("start", &osn::AdvancedReplayBuffer::Start),
            InstanceMethod("stop", &osn::AdvancedReplayBuffer::Stop),

            InstanceMethod("save", &osn::AdvancedReplayBuffer::Save),
            InstanceMethod("lastReplay", &osn::AdvancedReplayBuffer::GetLastReplay)
        });

    exports.Set("AdvancedReplayBuffer", func);
    osn::AdvancedReplayBuffer::constructor = Napi::Persistent(func);
    osn::AdvancedReplayBuffer::constructor.SuppressDestruct();

    return exports;
}

osn::AdvancedReplayBuffer::AdvancedReplayBuffer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::AdvancedReplayBuffer>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();

    if (length <= 0 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }

    this->uid = (uint64_t)info[0].ToNumber().Int64Value();
    this->className = std::string("AdvancedReplayBuffer");
}

Napi::Value osn::AdvancedReplayBuffer::Create(const Napi::CallbackInfo& info) {
    auto conn = GetConnection(info);
    if (!conn)
        return info.Env().Undefined();

    std::vector<ipc::value> response =
        conn->call_synchronous_helper("AdvancedReplayBuffer", "Create", {});

    if (!ValidateResponse(info, response))
        return info.Env().Undefined();

    auto instance =
        osn::AdvancedReplayBuffer::constructor.New({
            Napi::Number::New(info.Env(), response[1].value_union.ui64)
        });

    return instance;
}

Napi::Value osn::AdvancedReplayBuffer::GetMixer(const Napi::CallbackInfo& info) {
    auto conn = GetConnection(info);
    if (!conn)
        return info.Env().Undefined();

    std::vector<ipc::value> response =
        conn->call_synchronous_helper(
            className,
            "GetMixer",
            {ipc::value(this->uid)});

    if (!ValidateResponse(info, response))
        return info.Env().Undefined();

    return Napi::Number::New(info.Env(), response[1].value_union.ui32);
}

void osn::AdvancedReplayBuffer::SetMixer(
    const Napi::CallbackInfo& info, const Napi::Value& value) {
    auto conn = GetConnection(info);
    if (!conn)
        return;

    conn->call_synchronous_helper(
        className,
        "SetMixer",
        {ipc::value(this->uid), ipc::value(value.ToNumber().Uint32Value())});
}