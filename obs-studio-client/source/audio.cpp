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

#include "audio.hpp"
#include "utility.hpp"

Napi::FunctionReference osn::Audio::constructor;

Napi::Object osn::Audio::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func =
        DefineClass(env,
        "Audio",
        {
            StaticAccessor("audioContext", &osn::Audio::getAudioContext, &osn::Audio::setAudioContext)
        });
    exports.Set("Audio", func);
    osn::Audio::constructor = Napi::Persistent(func);
    osn::Audio::constructor.SuppressDestruct();
    return exports;
}

osn::Audio::Audio(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Audio>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
}

Napi::Value osn::Audio::getAudioContext(const Napi::CallbackInfo& info)
{
    auto conn = GetConnection(info);
    if (!conn)
        return info.Env().Undefined();

    std::vector<ipc::value> response =
        conn->call_synchronous_helper("Audio", "GetAudioContext", {});

    if (!ValidateResponse(info, response))
        return info.Env().Undefined();

    if (response.size() != 3)
        return info.Env().Undefined();

    Napi::Object audio = Napi::Object::New(info.Env());
    audio.Set("sampleRate", response[1].value_union.ui32);
    audio.Set("speakers", response[2].value_union.ui32);

    return audio;
}

void osn::Audio::setAudioContext(const Napi::CallbackInfo& info, const Napi::Value &value)
{
    Napi::Object audio = value.ToObject();
    if (!audio || !audio.IsObject()) {
        Napi::Error::New(
            info.Env(),
            "The audio context object passed is invalid.").ThrowAsJavaScriptException();
        return;
    }

    auto conn = GetConnection(info);
    if (!conn)
        return;

    uint32_t sampleRate = audio.Get("sampleRate").ToNumber().Uint32Value();
    uint32_t speakers = audio.Get("speakers").ToNumber().Uint32Value();

    std::vector<ipc::value> response =
        conn->call_synchronous_helper(
            "Audio", "SetAudioContext", {ipc::value(sampleRate), ipc::value(speakers)});

    if (!ValidateResponse(info, response))
        return;
}