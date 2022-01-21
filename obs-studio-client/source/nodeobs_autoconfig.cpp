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

#include "nodeobs_autoconfig.hpp"
#include "shared.hpp"
#include "obs-wrapper/obs-nodeobs_autoconfig.h"

Napi::ThreadSafeFunction  autoConfig::js_thread;

void JSCallback(AutoConfigInfo* data, void* jsThread)
{
	auto sources_callback = [](
			Napi::Env env, 
			Napi::Function jsCallback,
			AutoConfigInfo* event_data) {
		try {
			Napi::Object result = Napi::Object::New(env);

			result.Set(
				Napi::String::New(env, "event"),
				Napi::String::New(env, event_data->m_event));
			result.Set(
				Napi::String::New(env, "description"),
				Napi::String::New(env, event_data->m_description));

			if (event_data->m_event.compare("error") != 0) {
				result.Set(
					Napi::String::New(env, "percentage"),
					Napi::Number::New(env, event_data->m_percentage));
			}
			result.Set(Napi::String::New(env, "continent"), Napi::String::New(env, ""));

			jsCallback.Call({result});
		} catch (...) {}
		delete event_data;
	};

	Napi::ThreadSafeFunction& l_jsThread =
		*reinterpret_cast<Napi::ThreadSafeFunction*>(jsThread);
	l_jsThread.BlockingCall(data, sources_callback);
}

Napi::Value autoConfig::InitializeAutoConfig(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();

	js_thread = Napi::ThreadSafeFunction::New(
		info.Env(),
		async_callback,
		"AutoConfig",
		0,
		1, 
		[](Napi::Env) {}
	);

	obs::autoConfig::InitializeAutoConfig(JSCallback, &js_thread);
	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value autoConfig::StartBandwidthTest(const Napi::CallbackInfo& info)
{
	obs::autoConfig::StartBandwidthTest();

	return info.Env().Undefined();
}

Napi::Value autoConfig::StartStreamEncoderTest(const Napi::CallbackInfo& info)
{
	obs::autoConfig::StartStreamEncoderTest();

	return info.Env().Undefined();
}

Napi::Value autoConfig::StartRecordingEncoderTest(const Napi::CallbackInfo& info)
{
	obs::autoConfig::StartRecordingEncoderTest();

	return info.Env().Undefined();
}

Napi::Value autoConfig::StartCheckSettings(const Napi::CallbackInfo& info)
{
	obs::autoConfig::StartCheckSettings();

	return info.Env().Undefined();
}

Napi::Value autoConfig::StartSetDefaultSettings(const Napi::CallbackInfo& info)
{
	obs::autoConfig::StartSetDefaultSettings();

	return info.Env().Undefined();
}

Napi::Value autoConfig::StartSaveStreamSettings(const Napi::CallbackInfo& info)
{
	obs::autoConfig::StartSaveStreamSettings();

	return info.Env().Undefined();
}

Napi::Value autoConfig::StartSaveSettings(const Napi::CallbackInfo& info)
{
	obs::autoConfig::StartSaveSettings();

	return info.Env().Undefined();
}

void autoConfig::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		Napi::String::New(env, "InitializeAutoConfig"),
		Napi::Function::New(env, autoConfig::InitializeAutoConfig));
	exports.Set(
		Napi::String::New(env, "StartBandwidthTest"),
		Napi::Function::New(env, autoConfig::StartBandwidthTest));
	exports.Set(
		Napi::String::New(env, "StartStreamEncoderTest"),
		Napi::Function::New(env, autoConfig::StartStreamEncoderTest));
	exports.Set(
		Napi::String::New(env, "StartRecordingEncoderTest"),
		Napi::Function::New(env, autoConfig::StartRecordingEncoderTest));
	exports.Set(
		Napi::String::New(env, "StartCheckSettings"),
		Napi::Function::New(env, autoConfig::StartCheckSettings));
	exports.Set(
		Napi::String::New(env, "StartSetDefaultSettings"),
		Napi::Function::New(env, autoConfig::StartSetDefaultSettings));
	exports.Set(
		Napi::String::New(env, "StartSaveStreamSettings"),
		Napi::Function::New(env, autoConfig::StartSaveStreamSettings));
	exports.Set(
		Napi::String::New(env, "StartSaveSettings"),
		Napi::Function::New(env, autoConfig::StartSaveSettings));
}
