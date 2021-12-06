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
	auto source = *externalItem.Data();
	this->id = idSourcesCount++;
	sources.insert_or_assign(this->id, source);
}

Napi::Value osn::Input::Types(const Napi::CallbackInfo& info)
{
	return utilv8::ToValue<std::string>(info, obs::Input::Types());
}

Napi::Value osn::Input::Create(const Napi::CallbackInfo& info)
{
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
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	auto input = obs::Input::Duplicate(source);
    auto instance =
        osn::Input::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &input)
		});
    return instance;
}

Napi::Value osn::Input::Active(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), obs::Input::GetActive(source));
}

Napi::Value osn::Input::Showing(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), obs::Input::GetShowing(source));
}

Napi::Value osn::Input::Width(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetWidth(source));
}

Napi::Value osn::Input::Height(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetHeight(source));
}

Napi::Value osn::Input::GetVolume(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetVolume(source));
}

void osn::Input::SetVolume(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto source = sources[this->id];
	if (!source)
		return;

	obs::Input::SetVolume(source, value.ToNumber().FloatValue());
}

Napi::Value osn::Input::GetSyncOffset(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	auto offset = obs::Input::GetSyncOffset(source);

	Napi::Object tsobj = Napi::Object::New(info.Env());
	tsobj.Set("sec", offset / 1000000000);
	tsobj.Set("nsec", offset % 1000000000);
	return tsobj;
}

void osn::Input::SetSyncOffset(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto source = sources[this->id];
	if (!source)
		return;

	Napi::Object tsobj = info[0].ToObject();

	int64_t sec = tsobj.Get("sec").ToNumber().Int64Value();
	int64_t nsec = tsobj.Get("nsec").ToNumber().Int64Value();
	int64_t syncoffset = sec * 1000000000 + nsec;

	obs::Input::SetSyncOffset(source, syncoffset);
}

Napi::Value osn::Input::GetAudioMixers(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	auto mixers = obs::Input::GetAudioMixers(source);

	return Napi::Number::New(info.Env(), mixers);
}

void osn::Input::SetAudioMixers(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto source = sources[this->id];
	if (!source)
		return;

	uint32_t audiomixers = info[0].ToNumber().Uint32Value();

	obs::Input::SetAudioMixers(source, audiomixers);
}

Napi::Value osn::Input::GetMonitoringType(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetMonitoringType(source));
}

void osn::Input::SetMonitoringType(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto source = sources[this->id];
	if (!source)
		return;

	int32_t audiomixers = info[0].ToNumber().Int32Value();

	obs::Input::SetMonitoringType(source, audiomixers);
}

Napi::Value osn::Input::GetDeinterlaceFieldOrder(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetDeInterlaceFieldOrder(source));
}

void osn::Input::SetDeinterlaceFieldOrder(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto source = sources[this->id];
	if (!source)
		return;

	int32_t deinterlaceOrder = info[0].ToNumber().Int32Value();

	obs::Input::SetDeInterlaceFieldOrder(source, deinterlaceOrder);
}

Napi::Value osn::Input::GetDeinterlaceMode(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetDeInterlaceMode(source));
}

void osn::Input::SetDeinterlaceMode(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto source = sources[this->id];
	if (!source)
		return;

	int32_t deinterlaceMode = info[0].ToNumber().Int32Value();

	obs::Input::SetDeInterlaceMode(source, deinterlaceMode);
}

Napi::Value osn::Input::Filters(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	auto filtersArray = obs::Input::GetFilters(source);
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
	auto input = sources[this->id];
	if (!input)
		return info.Env().Undefined();

	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());
	auto filter = sources[objfilter->id];
	if (!filter)
		return info.Env().Undefined();

	obs::Input::AddFilter(input, filter);

	return info.Env().Undefined();
}

Napi::Value osn::Input::RemoveFilter(const Napi::CallbackInfo& info)
{
	auto input = sources[this->id];
	if (!input)
		return info.Env().Undefined();

	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());
	auto filter = sources[objfilter->id];
	if (!filter)
		return info.Env().Undefined();

	obs::Input::RemoveFilter(input, filter);

	return info.Env().Undefined();
}

Napi::Value osn::Input::SetFilterOrder(const Napi::CallbackInfo& info)
{
	auto input = sources[this->id];
	if (!input)
		return info.Env().Undefined();

	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());
	auto filter = sources[objfilter->id];
	if (!filter)
		return info.Env().Undefined();

	uint32_t movement = info[1].ToNumber().Uint32Value();
	
	obs::Input::MoveFilter(input, filter, movement);

	return info.Env().Undefined();
}

Napi::Value osn::Input::FindFilter(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	std::string name = info[0].ToString().Utf8Value();

	auto filter = obs::Input::FindFilter(source, name);
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
	auto from = sources[this->id];
	if (!from)
		info.Env().Undefined();

	osn::Input* objfilter = Napi::ObjectWrap<osn::Input>::Unwrap(info[0].ToObject());
	auto to = sources[objfilter->id];
	if (!to)
		info.Env().Undefined();


	obs::Input::CopyFiltersTo(from, to);
	return info.Env().Undefined();
}

Napi::Value osn::Input::CallIsConfigurable(const Napi::CallbackInfo& info)
{
	return osn::ISource::IsConfigurable(info, this->id);
}

Napi::Value osn::Input::CallGetProperties(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetProperties(info, this->id);
}

Napi::Value osn::Input::CallGetSettings(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetSettings(info, this->id);
}


Napi::Value osn::Input::CallGetType(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetType(info, this->id);
}

Napi::Value osn::Input::CallGetName(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetName(info, this->id);
}

void osn::Input::CallSetName(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetName(info, value, this->id);
}

Napi::Value osn::Input::CallGetOutputFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetOutputFlags(info, this->id);
}

Napi::Value osn::Input::CallGetFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetFlags(info, this->id);
}

void osn::Input::CallSetFlags(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetFlags(info, value, this->id);
}

Napi::Value osn::Input::CallGetStatus(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetStatus(info, this->id);
}

Napi::Value osn::Input::CallGetId(const Napi::CallbackInfo& info)
{
	blog(LOG_INFO, "this->id: %d", this->id);
	return osn::ISource::GetId(info, this->id);
}

Napi::Value osn::Input::CallGetMuted(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetMuted(info, this->id);
}

void osn::Input::CallSetMuted(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetMuted(info, value, this->id);
}

Napi::Value osn::Input::CallGetEnabled(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetEnabled(info, this->id);
}

void osn::Input::CallSetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetEnabled(info, value, this->id);
}

Napi::Value osn::Input::CallRelease(const Napi::CallbackInfo& info)
{
	pushTask([&, this] {
		osn::ISource::Release(this->id);
	});

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallRemove(const Napi::CallbackInfo& info)
{
	pushTask([&, this] {
		osn::ISource::Remove(this->id);
		this->id = UINT32_MAX;
	});

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallUpdate(const Napi::CallbackInfo& info)
{
	osn::ISource::Update(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallLoad(const Napi::CallbackInfo& info)
{
	osn::ISource::Load(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSave(const Napi::CallbackInfo& info)
{
	osn::ISource::Save(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseClick(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseMove(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseMove(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseWheel(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseWheel(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendFocus(const Napi::CallbackInfo& info)
{
	osn::ISource::SendFocus(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendKeyClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendKeyClick(info, this->id);

	return info.Env().Undefined();
}

Napi::Value osn::Input::GetDuration(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetDuration(source));
}

Napi::Value osn::Input::GetTime(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetTime(source));
}

void osn::Input::SetTime(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto source = sources[this->id];
	if (!source)
		return;

	int64_t ms = info[0].ToNumber().Int64Value();

	obs::Input::SetTime(source, ms);
}

void osn::Input::Play(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return;

	obs::Input::Play(source);
}

void osn::Input::Pause(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return;

	obs::Input::Pause(source);
}

void osn::Input::Restart(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return;

	obs::Input::Restart(source);
}

void osn::Input::Stop(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return;

	obs::Input::Stop(source);
}

Napi::Value osn::Input::GetMediaState(const Napi::CallbackInfo& info)
{
	auto source = sources[this->id];
	if (!source)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::Input::GetMediaState(source));
}