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

#include "global.hpp"
#include <condition_variable>
#include <mutex>
#include "error.hpp"
#include "input.hpp"
#include "scene.hpp"
#include "transition.hpp"
#include "utility-v8.hpp"
#include "server/osn-global.hpp"

Napi::FunctionReference osn::Global::constructor;

Napi::Object osn::Global::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Global",
		{
			StaticMethod("getOutputSource", &osn::Global::getOutputSource),
			StaticMethod("setOutputSource", &osn::Global::setOutputSource),
			StaticMethod("getOutputFlagsFromId", &osn::Global::getOutputFlagsFromId),

			StaticAccessor("laggedFrames", &osn::Global::laggedFrames, nullptr),
			StaticAccessor("totalFrames", &osn::Global::totalFrames, nullptr),

			StaticAccessor("locale", &osn::Global::getLocale, &osn::Global::setLocale),
			StaticAccessor("multipleRendering", &osn::Global::getMultipleRendering,
				&osn::Global::setMultipleRendering),
		});
	exports.Set("Global", func);
	osn::Global::constructor = Napi::Persistent(func);
	osn::Global::constructor.SuppressDestruct();
	return exports;
}

osn::Global::Global(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Global>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
}

Napi::Value osn::Global::getOutputSource(const Napi::CallbackInfo& info)
{
	uint32_t channel = info[0].ToNumber().Uint32Value();
	auto res = obs::Global::GetOutputSource(channel);

	if (res.second == 0) {
		auto instance =
			osn::Input::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &res.first)
			});

		obs_source_release(res.first);
		return instance;
	} else if (res.second == 2) {
		auto instance =
			osn::Transition::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &res.first)
			});

		obs_source_release(res.first);
		return instance;
	} else if (res.second == 3) {
		auto instance =
			osn::Scene::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &res.first)
			});

		obs_source_release(res.first);
		return instance;
	}

	return info.Env().Undefined();
}

Napi::Value osn::Global::setOutputSource(const Napi::CallbackInfo& info)
{
	uint32_t channel = info[0].ToNumber().Uint32Value();
	osn::Input* input = nullptr;

	if (info[1].IsObject()) {
		input = Napi::ObjectWrap<osn::Input>::Unwrap(info[1].ToObject());
		obs::Global::SetOutputSource(channel, input ? input->m_source : nullptr);
	}

	return info.Env().Undefined();
}

Napi::Value osn::Global::getOutputFlagsFromId(const Napi::CallbackInfo& info)
{
	std::string id = info[0].ToString().Utf8Value();
	auto flags = obs::Global::GetOutputFlagsFromId(id);

	return Napi::Number::New(info.Env(), flags);
}

Napi::Value osn::Global::laggedFrames(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Global::LaggedFrames());
}

Napi::Value osn::Global::totalFrames(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Global::TotalFrames());
}

Napi::Value osn::Global::getLocale(const Napi::CallbackInfo& info)
{
	return Napi::String::New(info.Env(), obs::Global::GetLocale());
}

void osn::Global::setLocale(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	obs::Global::SetLocale(value.ToString().Utf8Value());
}

Napi::Value osn::Global::getMultipleRendering(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::Global::GetMultipleRendering());
}

void osn::Global::setMultipleRendering(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	obs::Global::SetMultipleRendering(value.ToBoolean().Value());
}
