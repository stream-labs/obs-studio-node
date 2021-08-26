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
#include "controller.hpp"
#include "error.hpp"
#include "filter.hpp"
#include "ipc-value.hpp"
#include "shared.hpp"
#include "utility.hpp"
#include "server/osn-input.hpp"

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

    if (length <= 0 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }

	this->sourceId = (uint64_t)info[0].ToNumber().Int64Value();
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

	auto res = obs::Input::Create(type, name, settings.Utf8Value(), hotkeys.Utf8Value());

	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = type;
	sdi->id             = std::get<0>(res);
	sdi->setting        = std::get<1>(res);
	sdi->audioMixers    = std::get<2>(res);

	CacheManager<SourceDataInfo*>::getInstance().Store(std::get<0>(res), name, sdi);

    auto instance =
        osn::Input::constructor.New({
            Napi::Number::New(info.Env(), std::get<0>(res))
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

	auto uid = obs::Input::CreatePrivate(type, name, settings.Utf8Value());

	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = type;
	sdi->id             = uid;

	CacheManager<SourceDataInfo*>::getInstance().Store(uid, name, sdi);

    auto instance =
        osn::Input::constructor.New({
            Napi::Number::New(info.Env(), uid)
            });
    return instance;
}

Napi::Value osn::Input::FromName(const Napi::CallbackInfo& info)
{
	std::string name = info[0].ToString().Utf8Value();

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(name);

	if (sdi) {
		auto instance =
			osn::Input::constructor.New({
				Napi::Number::New(info.Env(), sdi->id)
				});
		return instance;
	}

	auto uid = obs::Input::FromName(name);

    auto instance =
        osn::Input::constructor.New({
            Napi::Number::New(info.Env(), uid)
            });
	instance.Set("sourceId", uid);
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
				Napi::Number::New(info.Env(), source)
				});
		arr[uint32_t(index++)] = object;
	}

	return arr;
}

Napi::Value osn::Input::Duplicate(const Napi::CallbackInfo& info)
{
	auto uid = obs::Input::Duplicate(this->sourceId);
    auto instance =
        osn::Input::constructor.New({
            Napi::Number::New(info.Env(), uid)
            });
    return instance;
}

Napi::Value osn::Input::Active(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::Input::GetActive(this->sourceId));
}

Napi::Value osn::Input::Showing(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::Input::GetShowing(this->sourceId));
}

Napi::Value osn::Input::Width(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetWidth(this->sourceId));
}

Napi::Value osn::Input::Height(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetHeight(this->sourceId));
}

Napi::Value osn::Input::GetVolume(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetVolume(this->sourceId));
}

void osn::Input::SetVolume(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	obs::Input::SetVolume(this->sourceId, value.ToNumber().FloatValue());
}

Napi::Value osn::Input::GetSyncOffset(const Napi::CallbackInfo& info)
{
	auto offset = obs::Input::GetSyncOffset(this->sourceId);

	Napi::Object tsobj = Napi::Object::New(info.Env());
	tsobj.Set("sec", offset / 1000000000);
	tsobj.Set("nsec", offset % 1000000000);
	return tsobj;
}

void osn::Input::SetSyncOffset(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object tsobj = info[0].ToObject();

	int64_t sec = tsobj.Get("sec").ToNumber().Int64Value();
	int64_t nsec = tsobj.Get("nsec").ToNumber().Int64Value();
	int64_t syncoffset = sec * 1000000000 + nsec;

	obs::Input::SetSyncOffset(this->sourceId, syncoffset);
}

Napi::Value osn::Input::GetAudioMixers(const Napi::CallbackInfo& info)
{
	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(this->sourceId);
	if (sdi && !sdi->audioMixersChanged && sdi->audioMixers != UINT32_MAX)
		return Napi::Number::New(info.Env(), sdi->audioMixers);

	auto mixers = obs::Input::GetAudioMixers(this->sourceId);
	if (sdi) {
		sdi->audioMixers        = mixers;
		sdi->audioMixersChanged = false;
	}

	return Napi::Number::New(info.Env(), mixers);
}

void osn::Input::SetAudioMixers(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t audiomixers = info[0].ToNumber().Uint32Value();
	obs::Input::SetAudioMixers(this->sourceId, audiomixers);

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(this->sourceId);
	if (sdi) {
		sdi->audioMixersChanged = true;
	}
}

Napi::Value osn::Input::GetMonitoringType(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetMonitoringType(this->sourceId));
}

void osn::Input::SetMonitoringType(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int32_t audiomixers = info[0].ToNumber().Int32Value();

	obs::Input::SetMonitoringType(this->sourceId, audiomixers);
}

Napi::Value osn::Input::GetDeinterlaceFieldOrder(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetDeInterlaceFieldOrder(this->sourceId));
}

void osn::Input::SetDeinterlaceFieldOrder(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int32_t deinterlaceOrder = info[0].ToNumber().Int32Value();

	obs::Input::SetDeInterlaceFieldOrder(this->sourceId, deinterlaceOrder);
}

Napi::Value osn::Input::GetDeinterlaceMode(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetDeInterlaceMode(this->sourceId));
}

void osn::Input::SetDeinterlaceMode(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int32_t deinterlaceMode = info[0].ToNumber().Int32Value();

	obs::Input::SetDeInterlaceMode(this->sourceId, deinterlaceMode);
}

Napi::Value osn::Input::Filters(const Napi::CallbackInfo& info)
{
	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(this->sourceId);

	if (sdi && !sdi->filtersOrderChanged) {
		std::vector<uint64_t>* filters = sdi->filters;
		Napi::Array array = Napi::Array::New(info.Env(), int(filters->size()));
		for (uint32_t i = 0; i < filters->size(); i++) {
			auto instance =
				osn::Filter::constructor.New({
					Napi::Number::New(info.Env(), filters->at(i))
					});
			array.Set(i, instance);
		}
		return array;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetFilters", {ipc::value(this->sourceId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	std::vector<uint64_t>* filters;
	if (sdi) {
		filters = sdi->filters;
		filters->clear();
	}

	auto filtersArray = obs::Input::GetFilters(this->sourceId);
	Napi::Array array = Napi::Array::New(info.Env(), response.size() - 1);
	uint32_t index = 0;

	for (auto filterId: filtersArray) {
		auto instance =
			osn::Filter::constructor.New({
				Napi::Number::New(info.Env(), filterId)
				});
		array.Set(index++, instance);

		if (sdi)
			filters->push_back(filterId);
	}

	if (sdi)
		sdi->filtersOrderChanged = false;

	return array;
}

Napi::Value osn::Input::AddFilter(const Napi::CallbackInfo& info)
{
	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());
	obs::Input::AddFilter(this->sourceId, objfilter->sourceId);

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(this->sourceId);
	if (sdi) {
		sdi->filtersOrderChanged = true;
	}

	return info.Env().Undefined();
}

Napi::Value osn::Input::RemoveFilter(const Napi::CallbackInfo& info)
{
	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());

	obs::Input::RemoveFilter(this->sourceId, objfilter->sourceId);

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(this->sourceId);
	if (sdi) {
		sdi->filtersOrderChanged = true;
	}
	return info.Env().Undefined();
}

Napi::Value osn::Input::SetFilterOrder(const Napi::CallbackInfo& info)
{
	osn::Filter* objfilter = Napi::ObjectWrap<osn::Filter>::Unwrap(info[0].ToObject());
	uint32_t movement = info[1].ToNumber().Uint32Value();
	
	obs::Input::MoveFilter(this->sourceId, objfilter->sourceId, movement);

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(this->sourceId);
	if (sdi) {
		sdi->filtersOrderChanged = true;
	}

	return info.Env().Undefined();
}

Napi::Value osn::Input::FindFilter(const Napi::CallbackInfo& info)
{
	std::string name = info[0].ToString().Utf8Value();

	auto uid = obs::Input::FindFilter(this->sourceId, name);
	if (uid != UINT64_MAX) {
		auto instance =
			osn::Filter::constructor.New({
				Napi::Number::New(info.Env(), uid)
				});
		return instance;
	}
	return info.Env().Undefined();
}

Napi::Value osn::Input::CopyFilters(const Napi::CallbackInfo& info)
{
	osn::Input* objfilter = Napi::ObjectWrap<osn::Input>::Unwrap(info[0].ToObject());

	obs::Input::CopyFiltersTo(this->sourceId, objfilter->sourceId);
	return info.Env().Undefined();
}

Napi::Value osn::Input::CallIsConfigurable(const Napi::CallbackInfo& info)
{
	return osn::ISource::IsConfigurable(info, this->sourceId);
}

Napi::Value osn::Input::CallGetProperties(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetProperties(info, this->sourceId);
}

Napi::Value osn::Input::CallGetSettings(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetSettings(info, this->sourceId);
}


Napi::Value osn::Input::CallGetType(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetType(info, this->sourceId);
}

Napi::Value osn::Input::CallGetName(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetName(info, this->sourceId);
}

void osn::Input::CallSetName(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetName(info, value, this->sourceId);
}

Napi::Value osn::Input::CallGetOutputFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetOutputFlags(info, this->sourceId);
}

Napi::Value osn::Input::CallGetFlags(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetFlags(info, this->sourceId);
}

void osn::Input::CallSetFlags(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetFlags(info, value, this->sourceId);
}

Napi::Value osn::Input::CallGetStatus(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetStatus(info, this->sourceId);
}

Napi::Value osn::Input::CallGetId(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetId(info, this->sourceId);
}

Napi::Value osn::Input::CallGetMuted(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetMuted(info, this->sourceId);
}

void osn::Input::CallSetMuted(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetMuted(info, value, this->sourceId);
}

Napi::Value osn::Input::CallGetEnabled(const Napi::CallbackInfo& info)
{
	return osn::ISource::GetEnabled(info, this->sourceId);
}

void osn::Input::CallSetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	osn::ISource::SetEnabled(info, value, this->sourceId);
}

Napi::Value osn::Input::CallRelease(const Napi::CallbackInfo& info)
{
	osn::ISource::Release(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallRemove(const Napi::CallbackInfo& info)
{
	osn::ISource::Remove(info, this->sourceId);
	this->sourceId = UINT64_MAX;

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallUpdate(const Napi::CallbackInfo& info)
{
	osn::ISource::Update(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallLoad(const Napi::CallbackInfo& info)
{
	osn::ISource::Load(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSave(const Napi::CallbackInfo& info)
{
	osn::ISource::Save(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseClick(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseMove(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseMove(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendMouseWheel(const Napi::CallbackInfo& info)
{
	osn::ISource::SendMouseWheel(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendFocus(const Napi::CallbackInfo& info)
{
	osn::ISource::SendFocus(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::CallSendKeyClick(const Napi::CallbackInfo& info)
{
	osn::ISource::SendKeyClick(info, this->sourceId);

	return info.Env().Undefined();
}

Napi::Value osn::Input::GetDuration(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetDuration(this->sourceId));
}

Napi::Value osn::Input::GetTime(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetTime(this->sourceId));
}

void osn::Input::SetTime(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int64_t ms = info[0].ToNumber().Int64Value();

	obs::Input::SetTime(this->sourceId, ms);
}

void osn::Input::Play(const Napi::CallbackInfo& info)
{
	obs::Input::Play(this->sourceId);
}

void osn::Input::Pause(const Napi::CallbackInfo& info)
{
	obs::Input::Pause(this->sourceId);
}

void osn::Input::Restart(const Napi::CallbackInfo& info)
{
	obs::Input::Restart(this->sourceId);
}

void osn::Input::Stop(const Napi::CallbackInfo& info)
{
	obs::Input::Stop(this->sourceId);
}

Napi::Value osn::Input::GetMediaState(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::Input::GetMediaState(this->sourceId));
}