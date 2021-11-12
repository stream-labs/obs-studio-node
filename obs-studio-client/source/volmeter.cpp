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
#include "input.hpp"
#include "shared.hpp"
#include "utility-v8.hpp"
#include "utility.hpp"
#include "callback-manager.hpp"

Napi::FunctionReference osn::Volmeter::constructor;

Napi::Object osn::Volmeter::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Volmeter",
		{
			StaticMethod("create", &osn::Volmeter::Create),

			InstanceMethod("destroy", &osn::Volmeter::Destroy),
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

	auto res = obs::Volmeter::Create(type);

    auto instance =
        osn::Volmeter::constructor.New({
            Napi::Number::New(info.Env(), res.first),
            Napi::Number::New(info.Env(), res.second)
            });
    return instance;
}

Napi::Value osn::Volmeter::Destroy(const Napi::CallbackInfo& info)
{
	obs::Volmeter::Destroy(this->m_uid);

	return info.Env().Undefined();
}

Napi::Value osn::Volmeter::Attach(const Napi::CallbackInfo& info)
{
	osn::Input* input = Napi::ObjectWrap<osn::Input>::Unwrap(info[0].ToObject());

	obs::Volmeter::Attach(this->m_uid, input->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Volmeter::Detach(const Napi::CallbackInfo& info)
{
	obs::Volmeter::Detach(this->m_uid);

	return info.Env().Undefined();
}

auto volmeter_callback = []( Napi::Env env, Napi::Function jsCallback, VolmeterData* data ) {
	Napi::Array magnitude = Napi::Array::New(env);
	Napi::Array peak = Napi::Array::New(env);
	Napi::Array input_peak = Napi::Array::New(env);

	if (!data->magnitude.size() || !data->peak.size() || !data->input_peak.size())
		return;

	for (size_t i = 0; i < data->channels; i++) {
		magnitude.Set(i, Napi::Number::New(env, data->magnitude[i]));
		peak.Set(i, Napi::Number::New(env, data->peak[i]));
		input_peak.Set(i, Napi::Number::New(env, data->input_peak[i]));
	}

	jsCallback.Call({ magnitude, peak, input_peak });

	data->volmeter->cbReady = true;
	delete data;
};

void OBSCallback(
    void*       param,
    const float magnitude[MAX_AUDIO_CHANNELS],
    const float peak[MAX_AUDIO_CHANNELS],
    const float input_peak[MAX_AUDIO_CHANNELS])
{
	auto meter = reinterpret_cast<obs::Volmeter*>(param);
	if (!meter) {
		return;
	}

	bool cbReady = meter->cbReady;
	if (cbReady != true)
		return;

	auto now = std::chrono::high_resolution_clock::now();
	auto delta = now - meter->lastProcessed;

	if (std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() < 50)
		return;

	meter->lastProcessed = now;
	meter->cbReady = false;

	int channels = obs_volmeter_get_nr_channels(meter->self);
	VolmeterData* data     = new VolmeterData{
		{},
		{},
		{},
		obs_volmeter_get_nr_channels(meter->self),
		meter
	};

	data->magnitude.resize(channels);
	data->peak.resize(channels);
	data->input_peak.resize(channels);
	for (size_t ch = 0; ch < channels; ch++) {
		data->magnitude[ch]  = MAKE_FLOAT_SANE(magnitude[ch]);
		data->peak[ch]       = MAKE_FLOAT_SANE(peak[ch]);
		data->input_peak[ch] = MAKE_FLOAT_SANE(input_peak[ch]);
	}

	auto end = std::chrono::high_resolution_clock::now();

	Napi::ThreadSafeFunction& jsThread =
		*reinterpret_cast<Napi::ThreadSafeFunction*>(meter->m_jsThread);
	if (jsThread != NULL)
		jsThread.NonBlockingCall(data, volmeter_callback);
}

Napi::Value osn::Volmeter::AddCallback(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();

	m_jsThread = Napi::ThreadSafeFunction::New(
      info.Env(),
      async_callback,
      "Volmeter",
      0,
      1,
      []( Napi::Env ) {} );

	obs::Volmeter::AddCallback(this->m_uid, OBSCallback, &m_jsThread);

	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value osn::Volmeter::RemoveCallback(const Napi::CallbackInfo& info)
{
	obs::Volmeter::RemoveCallback(this->m_uid, OBSCallback);
	m_jsThread.Release();
	return Napi::Boolean::New(info.Env(), true);
}