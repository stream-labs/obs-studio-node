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
			InstanceMethod("buttonClicked", &osn::Filter::CallButtonClicked)
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
	auto source = *externalItem.Data();
	this->id = idSourcesCount++;
	sourcesStore.insert_or_assign(this->id, source);
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

	std::cout << "Creating: " << type.c_str() <<std::endl;
	if (info.Length() >= 3) {
		Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
		Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

		Napi::Object setobj = info[2].ToObject();
		settings = stringify.Call(json, { setobj }).As<Napi::String>();
	}

	std::string settings_str = settings.Utf8Value();
	auto source = obs::Filter::Create(type, name, settings_str);

    auto instance =
        osn::Filter::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::Filter::CallIsConfigurable(const Napi::CallbackInfo& info)
{
	return osn::ISource::IsConfigurable(info, this->id);
}

Napi::Value osn::Filter::CallGetProperties(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetProperties(info, this->id);
}

Napi::Value osn::Filter::CallGetSettings(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetSettings(info, this->id);
}


Napi::Value osn::Filter::CallGetType(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetType(info, this->id);
}

Napi::Value osn::Filter::CallGetName(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetName(info, this->id);
}

void osn::Filter::CallSetName(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetName(info, value, this->id);
}

Napi::Value osn::Filter::CallGetOutputFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetOutputFlags(info, this->id);
}

Napi::Value osn::Filter::CallGetFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetFlags(info, this->id);
}

void osn::Filter::CallSetFlags(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetFlags(info, value, this->id);
}

Napi::Value osn::Filter::CallGetStatus(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetStatus(info, this->id);
}

Napi::Value osn::Filter::CallGetId(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetId(info, this->id);
}

Napi::Value osn::Filter::CallGetMuted(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetMuted(info, this->id);
}

void osn::Filter::CallSetMuted(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetMuted(info, value, this->id);
}

Napi::Value osn::Filter::CallGetEnabled(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetEnabled(info, this->id);
}

void osn::Filter::CallSetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetEnabled(info, value, this->id);
}

Napi::Value osn::Filter::CallRelease(const Napi::CallbackInfo& info)
{
	// osn::ISource::Release(this->id);
	pushTask(osn::ISource::Release, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallRemove(const Napi::CallbackInfo& info)
{
	pushTask([&, this] {
		osn::ISource::Remove(this->id);
		this->id = UINT32_MAX;
	});

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallUpdate(const Napi::CallbackInfo& info)
{
	osn::ISource::Update(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallLoad(const Napi::CallbackInfo& info)
{
	osn::ISource::Load(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSave(const Napi::CallbackInfo& info)
{
	osn::ISource::Save(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendMouseClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseClick(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendMouseMove(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseMove(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendMouseWheel(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseWheel(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendFocus(const Napi::CallbackInfo& info)
{
	osn::ISource::SendFocus(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Filter::CallSendKeyClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendKeyClick(info, this->id);

	return info.Env().Undefined();
}

void osn::Filter::CallButtonClicked(const Napi::CallbackInfo& info)
{
	osn::ISource::ButtonClicked(info, this->id);
}