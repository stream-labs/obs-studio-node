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

#include "transition.hpp"
#include <condition_variable>
#include <mutex>
#include <string>
#include "error.hpp"
#include "shared.hpp"
#include "utility.hpp"
#include "server/osn-transition.hpp"

Napi::FunctionReference osn::Transition::constructor;

Napi::Object osn::Transition::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Transition",
		{
			StaticMethod("types", &osn::Transition::Types),
			StaticMethod("create", &osn::Transition::Create),
			StaticMethod("createPrivate", &osn::Transition::CreatePrivate),
			StaticMethod("fromName", &osn::Transition::FromName),

			InstanceMethod("getActiveSource", &osn::Transition::GetActiveSource),
			InstanceMethod("start", &osn::Transition::Start),
			InstanceMethod("set", &osn::Transition::Set),
			InstanceMethod("clear", &osn::Transition::Clear),

			InstanceAccessor("configurable", &osn::Transition::CallIsConfigurable, nullptr),
			InstanceAccessor("properties", &osn::Transition::CallGetProperties, nullptr),
			InstanceAccessor("settings", &osn::Transition::CallGetSettings, nullptr),
			InstanceAccessor("type", &osn::Transition::CallGetType, nullptr),
			InstanceAccessor("name", &osn::Transition::CallGetName, &osn::Transition::CallSetName),
			InstanceAccessor("outputFlags", &osn::Transition::CallGetOutputFlags, nullptr),
			InstanceAccessor("flags", &osn::Transition::CallGetFlags, &osn::Transition::CallSetFlags),
			InstanceAccessor("status", &osn::Transition::CallGetStatus, nullptr),
			InstanceAccessor("id", &osn::Transition::CallGetId, nullptr),
			InstanceAccessor("muted", &osn::Transition::CallGetMuted, &osn::Transition::CallSetMuted),
			InstanceAccessor("enabled", &osn::Transition::CallGetEnabled, &osn::Transition::CallSetEnabled),

			InstanceMethod("release", &osn::Transition::CallRelease),
			InstanceMethod("remove", &osn::Transition::CallRemove),
			InstanceMethod("update", &osn::Transition::CallUpdate),
			InstanceMethod("load", &osn::Transition::CallLoad),
			InstanceMethod("save", &osn::Transition::CallSave),
			InstanceMethod("sendMouseClick", &osn::Transition::CallSendMouseClick),
			InstanceMethod("sendMouseMove", &osn::Transition::CallSendMouseMove),
			InstanceMethod("sendMouseWheel", &osn::Transition::CallSendMouseWheel),
			InstanceMethod("sendFocus", &osn::Transition::CallSendFocus),
			InstanceMethod("sendKeyClick", &osn::Transition::CallSendKeyClick),
		});
	exports.Set("Transition", func);
	osn::Transition::constructor = Napi::Persistent(func);
	osn::Transition::constructor.SuppressDestruct();
	return exports;
}

osn::Transition::Transition(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Transition>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();

    if (length <= 0) {
        Napi::TypeError::New(env, "Too few arguments.").ThrowAsJavaScriptException();
        return;
    }

	auto externalItem = info[0].As<Napi::External<obs_source_t*>>();
	this->m_source = *externalItem.Data();
}

Napi::Value osn::Transition::Types(const Napi::CallbackInfo& info)
{
	auto res = obs::Transition::Types();
	Napi::Array types = Napi::Array::New(info.Env(), res.size());
	uint32_t index = 0;

	for (auto type: res)
		types.Set(index++, Napi::String::New(info.Env(), type));

	return types;
}

Napi::Value osn::Transition::Create(const Napi::CallbackInfo& info)
{
	std::string type = info[0].ToString().Utf8Value();
	std::string name = info[1].ToString().Utf8Value();
	Napi::String settings = Napi::String::New(info.Env(), "");

	// Check if caller provided settings to send across.
	if (info.Length() >= 3) {
		Napi::Object setobj = info[2].ToObject();
		Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
		Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

		settings = stringify.Call(json, { setobj }).As<Napi::String>();
	}

	std::string settings_str = settings.Utf8Value();
	auto source = obs::Transition::Create(type, name, settings_str);

    auto instance =
        osn::Transition::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::Transition::CreatePrivate(const Napi::CallbackInfo& info)
{
	std::string type = info[0].ToString().Utf8Value();
	std::string name = info[1].ToString().Utf8Value();
	Napi::String settings = Napi::String::New(info.Env(), "");

	// Check if caller provided settings to send across.
	if (info.Length() >= 3) {
		Napi::Object setobj = info[2].ToObject();
		Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
		Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

		settings = stringify.Call(json, { setobj }).As<Napi::String>();
	}

	std::string settings_str = settings.Utf8Value();
	auto source = obs::Transition::CreatePrivate(type, name, settings_str);

    auto instance =
        osn::Transition::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::Transition::FromName(const Napi::CallbackInfo& info)
{
	std::string name = info[0].ToString().Utf8Value();
	auto source = obs::Transition::FromName(name);

    auto instance =
        osn::Transition::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::Transition::GetActiveSource(const Napi::CallbackInfo& info)
{
	auto res = obs::Transition::GetActiveSource(this->m_source);
	if (!res.first)
		return info.Env().Undefined();

	if (res.second == 0) {
		// Input
		auto instance =
			osn::Input::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &res.first)
			});

		return instance;
	} else if (res.second == 3) {
		// Scene
		auto instance =
			osn::Scene::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &res.first)
			});

		return instance;
	}

	return info.Env().Undefined();
}

Napi::Value osn::Transition::Clear(const Napi::CallbackInfo& info)
{
	obs::Transition::Clear(this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::Set(const Napi::CallbackInfo& info)
{
	osn::Scene* scene = Napi::ObjectWrap<osn::Scene>::Unwrap(info[0].ToObject());

	obs::Transition::Set(this->m_source, scene->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::Start(const Napi::CallbackInfo& info)
{
	uint32_t ms = info[0].ToNumber().Uint32Value();
	osn::Scene* scene = Napi::ObjectWrap<osn::Scene>::Unwrap(info[1].ToObject());

	return Napi::Boolean::New(info.Env(), obs::Transition::Start(this->m_source, ms, scene->m_source));
}

Napi::Value osn::Transition::CallIsConfigurable(const Napi::CallbackInfo& info)
{
	return osn::ISource::IsConfigurable(info, this->m_source);
}

Napi::Value osn::Transition::CallGetProperties(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetProperties(info, this->m_source);
}

Napi::Value osn::Transition::CallGetSettings(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetSettings(info, this->m_source);
}


Napi::Value osn::Transition::CallGetType(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetType(info, this->m_source);
}

Napi::Value osn::Transition::CallGetName(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetName(info, this->m_source);
}

void osn::Transition::CallSetName(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetName(info, value, this->m_source);
}

Napi::Value osn::Transition::CallGetOutputFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetOutputFlags(info, this->m_source);
}

Napi::Value osn::Transition::CallGetFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetFlags(info, this->m_source);
}

void osn::Transition::CallSetFlags(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetFlags(info, value, this->m_source);
}

Napi::Value osn::Transition::CallGetStatus(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetStatus(info, this->m_source);
}

Napi::Value osn::Transition::CallGetId(const Napi::CallbackInfo& info)
{

	return osn::ISource::GetId(info, this->m_source);
}

Napi::Value osn::Transition::CallGetMuted(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetMuted(info, this->m_source);
}

void osn::Transition::CallSetMuted(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetMuted(info, value, this->m_source);
}

Napi::Value osn::Transition::CallGetEnabled(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetEnabled(info, this->m_source);
}

void osn::Transition::CallSetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetEnabled(info, value, this->m_source);
}

Napi::Value osn::Transition::CallRelease(const Napi::CallbackInfo& info)
{
	osn::ISource::Release(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallRemove(const Napi::CallbackInfo& info)
{
	osn::ISource::Remove(info, this->m_source);
	this->m_source = nullptr;

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallUpdate(const Napi::CallbackInfo& info)
{
	osn::ISource::Update(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallLoad(const Napi::CallbackInfo& info)
{
	osn::ISource::Load(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallSave(const Napi::CallbackInfo& info)
{
	osn::ISource::Save(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallSendMouseClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseClick(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallSendMouseMove(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseMove(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallSendMouseWheel(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseWheel(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallSendFocus(const Napi::CallbackInfo& info)
{
	osn::ISource::SendFocus(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Transition::CallSendKeyClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendKeyClick(info, this->m_source);

	return info.Env().Undefined();
}