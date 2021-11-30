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

#include "input.hpp"
#include <condition_variable>
#include <mutex>
#include <string>
#include <algorithm>
#include <iterator>
#include "filter.hpp"
#include "shared.hpp"
#include "utility.hpp"

Napi::FunctionReference osn::Input::constructor;

Napi::Object osn::Input::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Input",
		{
			StaticMethod("types", &osn::Input::Types),
			StaticMethod("create", &osn::Input::Create),
			StaticMethod("createPrivate", &osn::Input::CreatePrivate),
			StaticMethod("fromName", &osn::Input::FromName),
			StaticMethod("getPublicSources", &osn::Input::GetPublicSources),

			InstanceMethod("duplicate", &osn::Input::Duplicate),
			InstanceMethod("addFilter", &osn::Input::AddFilter),
			InstanceMethod("removeFilter", &osn::Input::RemoveFilter),
			InstanceMethod("setFilterOrder", &osn::Input::SetFilterOrder),
			InstanceMethod("findFilter", &osn::Input::FindFilter),
			InstanceMethod("copyFilters", &osn::Input::CopyFilters),

			InstanceAccessor("active", &osn::Input::Active, nullptr),
			InstanceAccessor("showing", &osn::Input::Showing, nullptr),
			InstanceAccessor("width", &osn::Input::Width, nullptr),
			InstanceAccessor("height", &osn::Input::Height, nullptr),
			InstanceAccessor("volume", &osn::Input::GetVolume, &osn::Input::SetVolume),
			InstanceAccessor("syncOffset", &osn::Input::GetSyncOffset, &osn::Input::SetSyncOffset),
			InstanceAccessor("audioMixers", &osn::Input::GetAudioMixers, &osn::Input::SetAudioMixers),
			InstanceAccessor("monitoringType", &osn::Input::GetMonitoringType, &osn::Input::SetMonitoringType),
			InstanceAccessor("deinterlaceFieldOrder", &osn::Input::GetDeinterlaceFieldOrder, &osn::Input::SetDeinterlaceFieldOrder),
			InstanceAccessor("deinterlaceMode", &osn::Input::GetDeinterlaceMode, &osn::Input::SetDeinterlaceMode),
			InstanceAccessor("filters", &osn::Input::Filters, nullptr),
			InstanceAccessor("seek", &osn::Input::GetTime, &osn::Input::SetTime),

			InstanceAccessor("configurable", &osn::Input::CallIsConfigurable, nullptr),
			InstanceAccessor("properties", &osn::Input::CallGetProperties, nullptr),
			InstanceAccessor("settings", &osn::Input::CallGetSettings, nullptr),
			InstanceAccessor("type", &osn::Input::CallGetType, nullptr),
			InstanceAccessor("name", &osn::Input::CallGetName, &osn::Input::CallSetName),
			InstanceAccessor("outputFlags", &osn::Input::CallGetOutputFlags, nullptr),
			InstanceAccessor("flags", &osn::Input::CallGetFlags, &osn::Input::CallSetFlags),
			InstanceAccessor("status", &osn::Input::CallGetStatus, nullptr),
			InstanceAccessor("id", &osn::Input::CallGetId, nullptr),
			InstanceAccessor("muted", &osn::Input::CallGetMuted, &osn::Input::CallSetMuted),
			InstanceAccessor("enabled", &osn::Input::CallGetEnabled, &osn::Input::CallSetEnabled),

			InstanceMethod("release", &osn::Input::CallRelease),
			InstanceMethod("remove", &osn::Input::CallRemove),
			InstanceMethod("update", &osn::Input::CallUpdate),
			InstanceMethod("load", &osn::Input::CallLoad),
			InstanceMethod("save", &osn::Input::CallSave),
			InstanceMethod("sendMouseClick", &osn::Input::CallSendMouseClick),
			InstanceMethod("sendMouseMove", &osn::Input::CallSendMouseMove),
			InstanceMethod("sendMouseWheel", &osn::Input::CallSendMouseWheel),
			InstanceMethod("sendFocus", &osn::Input::CallSendFocus),
			InstanceMethod("sendKeyClick", &osn::Input::CallSendKeyClick),
			InstanceMethod("getDuration", &osn::Input::GetDuration),
			InstanceMethod("play", &osn::Input::Play),
			InstanceMethod("pause", &osn::Input::Pause),
			InstanceMethod("restart", &osn::Input::Restart),
			InstanceMethod("stop", &osn::Input::Stop),
			InstanceMethod("getMediaState", &osn::Input::GetMediaState)
		});
	exports.Set("Input", func);
	osn::Input::constructor = Napi::Persistent(func);
	osn::Input::constructor.SuppressDestruct();
	return exports;
}

osn::Input::Input(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Input>(info) {
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

Napi::Value osn::Input::Types(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return utilv8::ToValue<std::string>(info, obs::Input::Types());
}

Napi::Value osn::Input::Create(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	std::string type = info[0].ToString().Utf8Value();
	std::string name = info[1].ToString().Utf8Value();
	Napi::String settings = Napi::String::New(info.Env(), "");
	Napi::String hotkeys = Napi::String::New(info.Env(), "");

	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

	// Check if caller provided settings to send across.
	if (info.Length() >= 4) {
		if (!info[3].IsUndefined()) {
			Napi::Object hksobj = info[3].ToObject();
			hotkeys = stringify.Call(json, { hksobj }).As<Napi::String>();
		}
	}
	if (info.Length() >= 3) {
		if (!info[2].IsUndefined()) {
			Napi::Object setobj = info[2].ToObject();
			settings = stringify.Call(json, { setobj }).As<Napi::String>();
		}
	}

	auto source = obs::Input::Create(type, name, settings.Utf8Value(), hotkeys.Utf8Value());

    auto instance =
        osn::Input::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::Input::CreatePrivate(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	std::string type = info[0].ToString().Utf8Value();
	std::string name = info[1].ToString().Utf8Value();
	Napi::String settings = Napi::String::New(info.Env(), "");

	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

	if (info.Length() >= 3) {
		Napi::Object setobj = info[2].ToObject();
		settings = stringify.Call(json, { setobj }).As<Napi::String>();
	}

	auto source = obs::Input::CreatePrivate(type, name, settings.Utf8Value());

    auto instance =
        osn::Input::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});
    return instance;
}

Napi::Value osn::Input::FromName(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	std::string name = info[0].ToString().Utf8Value();

	auto source = obs::Input::FromName(name);
	if (!source)
		return info.Env().Undefined();

    auto instance =
        osn::Input::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

	obs_source_release(source);
    return instance;
}

Napi::Value osn::Input::GetPublicSources(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	auto sources = obs::Input::GetPublicSources();
	Napi::Array arr = Napi::Array::New(info.Env(), int(sources.size() - 1));

	uint32_t index = 0;
	for (auto source: sources) {
		auto object =
			osn::Input::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &source)
			});
		arr[uint32_t(index++)] = object;
	}

	return arr;
}

Napi::Value osn::Input::Duplicate(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	auto source = obs::Input::Duplicate(this->m_source);
    auto instance =
        osn::Input::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});
    return instance;
}

Napi::Value osn::Input::Active(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Boolean::New(info.Env(), obs::Input::GetActive(this->m_source));
}

Napi::Value osn::Input::Showing(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Boolean::New(info.Env(), obs::Input::GetShowing(this->m_source));
}

Napi::Value osn::Input::Width(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetWidth(this->m_source));
}

Napi::Value osn::Input::Height(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetHeight(this->m_source));
}

Napi::Value osn::Input::GetVolume(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetVolume(this->m_source));
}

void osn::Input::SetVolume(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	obs::Input::SetVolume(this->m_source, value.ToNumber().FloatValue());
}

Napi::Value osn::Input::GetSyncOffset(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	auto offset = obs::Input::GetSyncOffset(this->m_source);

	Napi::Object tsobj = Napi::Object::New(info.Env());
	tsobj.Set("sec", offset / 1000000000);
	tsobj.Set("nsec", offset % 1000000000);
	return tsobj;
}

void osn::Input::SetSyncOffset(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	Napi::Object tsobj = info[0].ToObject();

	int64_t sec = tsobj.Get("sec").ToNumber().Int64Value();
	int64_t nsec = tsobj.Get("nsec").ToNumber().Int64Value();
	int64_t syncoffset = sec * 1000000000 + nsec;

	obs::Input::SetSyncOffset(this->m_source, syncoffset);
}

Napi::Value osn::Input::GetAudioMixers(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	auto mixers = obs::Input::GetAudioMixers(this->m_source);

	return Napi::Number::New(info.Env(), mixers);
}

void osn::Input::SetAudioMixers(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	uint32_t audiomixers = info[0].ToNumber().Uint32Value();

	obs::Input::SetAudioMixers(this->m_source, audiomixers);
}

Napi::Value osn::Input::GetMonitoringType(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetMonitoringType(this->m_source));
}

void osn::Input::SetMonitoringType(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	int32_t audiomixers = info[0].ToNumber().Int32Value();

	obs::Input::SetMonitoringType(this->m_source, audiomixers);
}

Napi::Value osn::Input::GetDeinterlaceFieldOrder(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetDeInterlaceFieldOrder(this->m_source));
}

void osn::Input::SetDeinterlaceFieldOrder(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	int32_t deinterlaceOrder = info[0].ToNumber().Int32Value();

	obs::Input::SetDeInterlaceFieldOrder(this->m_source, deinterlaceOrder);
}

Napi::Value osn::Input::GetDeinterlaceMode(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetDeInterlaceMode(this->m_source));
}

void osn::Input::SetDeinterlaceMode(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	int32_t deinterlaceMode = info[0].ToNumber().Int32Value();

	obs::Input::SetDeInterlaceMode(this->m_source, deinterlaceMode);
}

Napi::Value osn::Input::Filters(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	auto filtersArray = obs::Input::GetFilters(this->m_source);
	Napi::Array array = Napi::Array::New(info.Env(), filtersArray.size());
	uint32_t index = 0;

	for (auto filter: filtersArray)
		array.Set(
			index++,
			osn::Filter::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &filter)
			})
		);

	return array;
}

Napi::Value osn::Input::AddFilter(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());
	obs::Input::AddFilter(this->m_source, objfilter->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::RemoveFilter(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());

	obs::Input::RemoveFilter(this->m_source, objfilter->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::SetFilterOrder(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());
	uint32_t movement = info[1].ToNumber().Uint32Value();
	
	obs::Input::MoveFilter(this->m_source, objfilter->m_source, movement);

	return info.Env().Undefined();
}

Napi::Value osn::Input::FindFilter(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	std::string name = info[0].ToString().Utf8Value();

	auto filter = obs::Input::FindFilter(this->m_source, name);
	if (filter) {
		auto instance =
			osn::Filter::constructor.New({
				Napi::External<obs_source_t*>::New(info.Env(), &filter)
			});
		obs_source_release(filter);
		return instance;
	}

	return info.Env().Undefined();
}

Napi::Value osn::Input::CopyFilters(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::Input* objfilter = Napi::ObjectWrap<osn::Input>::Unwrap(info[0].ToObject());

	obs::Input::CopyFiltersTo(this->m_source, objfilter->m_source);
	return info.Env().Undefined();
}

Napi::Value osn::Input::CallIsConfigurable(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::IsConfigurable(info, this->m_source);
}

Napi::Value osn::Input::CallGetProperties(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetProperties(info, this->m_source);
}

Napi::Value osn::Input::CallGetSettings(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetSettings(info, this->m_source);
}


Napi::Value osn::Input::CallGetType(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetType(info, this->m_source);
}

Napi::Value osn::Input::CallGetName(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetName(info, this->m_source);
}

void osn::Input::CallSetName(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	osn::ISource::SetName(info, value, this->m_source);
}

Napi::Value osn::Input::CallGetOutputFlags(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetOutputFlags(info, this->m_source);
}

Napi::Value osn::Input::CallGetFlags(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetFlags(info, this->m_source);
}

void osn::Input::CallSetFlags(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	osn::ISource::SetFlags(info, value, this->m_source);
}

Napi::Value osn::Input::CallGetStatus(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetStatus(info, this->m_source);
}

Napi::Value osn::Input::CallGetId(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetId(info, this->m_source);
}

Napi::Value osn::Input::CallGetMuted(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetMuted(info, this->m_source);
}

void osn::Input::CallSetMuted(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	osn::ISource::SetMuted(info, value, this->m_source);
}

Napi::Value osn::Input::CallGetEnabled(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return osn::ISource::GetEnabled(info, this->m_source);
}

void osn::Input::CallSetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	osn::ISource::SetEnabled(info, value, this->m_source);
}

Napi::Value osn::Input::CallRelease(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::Release(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallRemove(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::Remove(info, this->m_source);
	this->m_source = nullptr;

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallUpdate(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::Update(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallLoad(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::Load(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSave(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::Save(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseClick(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::SendMouseClick(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseMove(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::SendMouseMove(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseWheel(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::SendMouseWheel(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendFocus(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::SendFocus(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendKeyClick(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	osn::ISource::SendKeyClick(info, this->m_source);

	return info.Env().Undefined();
}

Napi::Value osn::Input::GetDuration(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetDuration(this->m_source));
}

Napi::Value osn::Input::GetTime(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetTime(this->m_source));
}

void osn::Input::SetTime(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	PROFINY_SCOPE
	int64_t ms = info[0].ToNumber().Int64Value();

	obs::Input::SetTime(this->m_source, ms);
}

void osn::Input::Play(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	obs::Input::Play(this->m_source);
}

void osn::Input::Pause(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	obs::Input::Pause(this->m_source);
}

void osn::Input::Restart(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	obs::Input::Restart(this->m_source);
}

void osn::Input::Stop(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	obs::Input::Stop(this->m_source);
}

Napi::Value osn::Input::GetMediaState(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Input::GetMediaState(this->m_source));
}