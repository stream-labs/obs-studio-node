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

#include "filter.hpp"
#include <condition_variable>
#include <mutex>
#include <string>
#include "controller.hpp"
#include "error.hpp"
#include "ipc-value.hpp"
#include "shared.hpp"
#include "utility.hpp"

Napi::FunctionReference osn::Filter::constructor;

Napi::Object osn::Filter::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Filter",
		{
			StaticMethod("types", &osn::Filter::Types),
			StaticMethod("create", &osn::Filter::Create),

			InstanceAccessor("configurable", &osn::Filter::CallIsConfigurable, nullptr),
			InstanceAccessor("properties", &osn::Filter::CallGetProperties, nullptr),
			InstanceAccessor("settings", &osn::Filter::CallGetSettings, nullptr),
			InstanceAccessor("type", &osn::Filter::CallGetType, nullptr),
			InstanceAccessor("name", &osn::Filter::CallGetName, &osn::Filter::CallSetName),
			InstanceAccessor("outputFlags", &osn::Filter::CallGetOutputFlags, nullptr),
			InstanceAccessor("flags", &osn::Filter::CallGetFlags, &osn::Filter::CallSetFlags),
			InstanceAccessor("status", &osn::Filter::CallGetStatus, nullptr),
			InstanceAccessor("id", &osn::Filter::CallGetId, nullptr),
			InstanceAccessor("muted", &osn::Filter::CallGetMuted, &osn::Filter::CallSetMuted),
			InstanceAccessor("enabled", &osn::Filter::CallGetEnabled, &osn::Filter::CallSetEnabled),

			InstanceMethod("release", &osn::Filter::CallRelease),
			InstanceMethod("remove", &osn::Filter::CallRemove),
			InstanceMethod("update", &osn::Filter::CallUpdate),
			InstanceMethod("load", &osn::Filter::CallLoad),
			InstanceMethod("save", &osn::Filter::CallSave),
			InstanceMethod("sendMouseClick", &osn::Filter::CallSendMouseClick),
			InstanceMethod("sendMouseMove", &osn::Filter::CallSendMouseMove),
			InstanceMethod("sendMouseWheel", &osn::Filter::CallSendMouseWheel),
			InstanceMethod("sendFocus", &osn::Filter::CallSendFocus),
			InstanceMethod("sendKeyClick", &osn::Filter::CallSendKeyClick),
		});
	exports.Set("Filter", func);
	osn::Filter::constructor = Napi::Persistent(func);
	osn::Filter::constructor.SuppressDestruct();
	return exports;
}

osn::Filter::Filter(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Filter>(info) {
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

Napi::Value osn::Filter::Types(const Napi::CallbackInfo& info)
{
	auto typesArray = obs::Filter::Types();
	Napi::Array types = Napi::Array::New(info.Env(), typesArray.size());
	uint32_t index = 0;

	for (auto type: typesArray)
		types.Set(index++, Napi::String::New(info.Env(), type));

	return types;
}

Napi::Value osn::Filter::Create(const Napi::CallbackInfo& info)
{
	std::string type = info[0].ToString().Utf8Value();
	std::string name = info[1].ToString().Utf8Value();
	Napi::String settings = Napi::String::New(info.Env(), "");

	if (info.Length() >= 3) {
		Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
		Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

		Napi::Object setobj = info[2].ToObject();
		settings = stringify.Call(json, { setobj }).As<Napi::String>();
	}

	auto params = std::vector<ipc::value>{ipc::value(type), ipc::value(name)};
	std::string settings_str = settings.Utf8Value();
	if (settings_str.size() != 0) {
		params.push_back(ipc::value(settings_str));
	}

	auto source = obs::Filter::Create(type, name, settings_str);

    auto instance =
        osn::Filter::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::Filter::CallIsConfigurable(const Napi::CallbackInfo& info)
{
	return osn::ISource::IsConfigurable(info, this->m_source);
}

Napi::Value osn::Filter::CallGetProperties(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetProperties(info, this->m_source);
}

Napi::Value osn::Filter::CallGetSettings(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetSettings(info, this->m_source);
}


Napi::Value osn::Filter::CallGetType(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetType(info, this->m_source);
}

Napi::Value osn::Filter::CallGetName(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetName(info, this->m_source);
}

void osn::Filter::CallSetName(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetName(info, value, this->m_source);
}

Napi::Value osn::Filter::CallGetOutputFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetOutputFlags(info, this->m_source);
}

Napi::Value osn::Filter::CallGetFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetFlags(info, this->m_source);
}

void osn::Filter::CallSetFlags(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetFlags(info, value, this->m_source);
}

Napi::Value osn::Filter::CallGetStatus(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetStatus(info, this->m_source);
}

Napi::Value osn::Filter::CallGetId(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetId(info, this->m_source);
}

Napi::Value osn::Filter::CallGetMuted(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetMuted(info, this->m_source);
}

void osn::Filter::CallSetMuted(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetMuted(info, value, this->m_source);
}

Napi::Value osn::Filter::CallGetEnabled(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetEnabled(info, this->m_source);
}

void osn::Filter::CallSetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetEnabled(info, value, this->m_source);
}

Napi::Value osn::Filter::CallRelease(const Napi::CallbackInfo& info)
{
	osn::ISource::Release(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallRemove(const Napi::CallbackInfo& info)
{
	osn::ISource::Remove(info, this->m_source);
	this->m_source = nullptr;

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallUpdate(const Napi::CallbackInfo& info)
{
	osn::ISource::Update(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallLoad(const Napi::CallbackInfo& info)
{
	osn::ISource::Load(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSave(const Napi::CallbackInfo& info)
{
	osn::ISource::Save(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendMouseClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseClick(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendMouseMove(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseMove(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendMouseWheel(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseWheel(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendFocus(const Napi::CallbackInfo& info)
{
	osn::ISource::SendFocus(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendKeyClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendKeyClick(info, this->m_source);

	return info.Env().Undefined();
}