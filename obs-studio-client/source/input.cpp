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

Nan::Persistent<v8::FunctionTemplate> osn::Input::prototype;

osn::Input::Input(uint64_t id)
{
	this->sourceId = id;
}

void osn::Input::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Input").ToLocalChecked());

	utilv8::SetTemplateField(fnctemplate, "types", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Types));
	utilv8::SetTemplateField(fnctemplate, "create", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Create));
	utilv8::SetTemplateField(fnctemplate, "createPrivate", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), CreatePrivate));
	utilv8::SetTemplateField(fnctemplate, "fromName", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), FromName));
	utilv8::SetTemplateField(fnctemplate, "getPublicSources", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetPublicSources));

	v8::Local<v8::Template> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "duplicate", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Duplicate));
	utilv8::SetTemplateAccessorProperty(objtemplate, "active", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Active));
	utilv8::SetTemplateAccessorProperty(objtemplate, "showing", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Showing));
	utilv8::SetTemplateAccessorProperty(objtemplate, "width", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Width));
	utilv8::SetTemplateAccessorProperty(objtemplate, "height", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Height));
	utilv8::SetTemplateAccessorProperty(objtemplate, "volume",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetVolume),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetVolume));
	utilv8::SetTemplateAccessorProperty(objtemplate, "syncOffset",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetSyncOffset),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetSyncOffset));
	utilv8::SetTemplateAccessorProperty(objtemplate, "audioMixers",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetAudioMixers),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetAudioMixers));
	utilv8::SetTemplateAccessorProperty(objtemplate, "monitoringType",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetMonitoringType),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetMonitoringType));
	utilv8::SetTemplateAccessorProperty(
	    objtemplate, "deinterlaceFieldOrder",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetDeinterlaceFieldOrder),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetDeinterlaceFieldOrder));
	utilv8::SetTemplateAccessorProperty(objtemplate, "deinterlaceMode",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetDeinterlaceMode),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetDeinterlaceMode));

	utilv8::SetTemplateAccessorProperty(objtemplate, "filters", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Filters));
	utilv8::SetTemplateField(objtemplate, "addFilter", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), AddFilter));
	utilv8::SetTemplateField(objtemplate, "removeFilter", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), RemoveFilter));
	utilv8::SetTemplateField(objtemplate, "setFilterOrder", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetFilterOrder));
	utilv8::SetTemplateField(objtemplate, "findFilter", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), FindFilter));
	utilv8::SetTemplateField(objtemplate, "copyFilters", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), CopyFilters));

	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "Input").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::Input::Types(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	// Function takes no parameters.
	ASSERT_INFO_LENGTH(args, 0);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "Types", {});

	if (!ValidateResponse(response))
		return;

	std::vector<std::string> types;

	for (size_t i = 1; i < response.size(); i++) {
		types.push_back(response[i].value_str);
	}

	args.GetReturnValue().Set(utilv8::ToValue<std::string>(types));
}

void osn::Input::Create(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::cout << "Create 0 " << std::endl;
	std::string           type;
	std::string           name;
	v8::Local<v8::String> settings = Nan::New<v8::String>("").ToLocalChecked();
	v8::Local<v8::String> hotkeys  = Nan::New<v8::String>("").ToLocalChecked();

	// Parameters: <string> Type, <string> Name[,<object> settings]
	ASSERT_INFO_LENGTH_AT_LEAST(args, 2);

	std::cout << "Create 1 " << std::endl;
	ASSERT_GET_VALUE(args[0], type);
	ASSERT_GET_VALUE(args[1], name);

	std::cout << "Create 2 " << std::endl;
	// Check if caller provided settings to send across.
	if (args.Length() >= 4) {
		ASSERT_INFO_LENGTH(args, 4);
		// if (!args[3]->IsUndefined()) {
			v8::Local<v8::Object> hksobj;
			ASSERT_GET_VALUE(args[3], hksobj);
			hotkeys = v8::JSON::Stringify(args.GetIsolate()->GetCurrentContext(), hksobj).ToLocalChecked();
		// }
	}
	std::cout << "Create 3 " << std::endl;
	if (args.Length() >= 3) {
		ASSERT_INFO_LENGTH_AT_LEAST(args, 3);
		// if (!args[2]->IsUndefined()) {
			v8::Local<v8::Object> setobj;
			ASSERT_GET_VALUE(args[2], setobj);
			settings = v8::JSON::Stringify(args.GetIsolate()->GetCurrentContext(), setobj).ToLocalChecked();
		// }
	}

	std::cout << "Create 4 " << std::endl;
	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(type), ipc::value(name)};
	if (settings->Length() != 0) {
		std::string value;
		if (utilv8::FromValue(settings, value)) {
			params.push_back(ipc::value(value));
		}
	}
	std::cout << "Create 5 " << std::endl;
	if (hotkeys->Length() != 0) {
		std::string value;
		if (utilv8::FromValue(hotkeys, value)) {
			params.push_back(ipc::value(value));
		}
	}

	std::cout << "Create 6 " << std::endl;
	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "Create", {std::move(params)});

	std::cout << "Create 6.1 " << std::endl;
	if (!ValidateResponse(response))
		return;

	std::cout << "Create 6.2 " << std::endl;
	// Create new Filter
	osn::Input* obj = new osn::Input(response[1].value_union.ui64);

	std::cout << "Create 6.3 " << std::endl;
	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = type;
	sdi->id             = response[1].value_union.ui64;
	sdi->setting        = response[2].value_str;
	sdi->audioMixers    = response[3].value_union.ui32;

	std::cout << "Create 6.4 " << std::endl;
	CacheManager<SourceDataInfo*>::getInstance().Store(response[1].value_union.ui64, name, sdi);

	std::cout << "Create 6.5 " << std::endl;
	args.GetReturnValue().Set(osn::Input::Store(obj));
	std::cout << "Create 7 " << std::endl;
}

void osn::Input::CreatePrivate(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string           type;
	std::string           name;
	v8::Local<v8::String> settings = Nan::New<v8::String>("").ToLocalChecked();

	// Parameters: <string> Type, <string> Name[,<object> settings]
	ASSERT_INFO_LENGTH_AT_LEAST(args, 2);

	ASSERT_GET_VALUE(args[0], type);
	ASSERT_GET_VALUE(args[1], name);

	// Check if caller provided settings to send across.
	if (args.Length() >= 3) {
		ASSERT_INFO_LENGTH(args, 3);

		v8::Local<v8::Object> setobj;
		ASSERT_GET_VALUE(args[2], setobj);

		settings = v8::JSON::Stringify(args.GetIsolate()->GetCurrentContext(), setobj).ToLocalChecked();
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(type), ipc::value(name)};
	if (settings->Length() != 0) {
		std::string value;
		if (utilv8::FromValue(settings, value)) {
			params.push_back(ipc::value(value));
		}
	}

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "CreatePrivate", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Input* obj = new osn::Input(response[1].value_union.ui64);

	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = type;
	sdi->id             = response[1].value_union.ui64;
	sdi->setting        = response[2].value_str;
	sdi->audioMixers    = response[3].value_union.ui32;

	CacheManager<SourceDataInfo*>::getInstance().Store(response[1].value_union.ui64, name, sdi);

	args.GetReturnValue().Set(osn::Input::Store(obj));
}

void osn::Input::FromName(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string name;

	// Parameters: <string> Name
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], name);

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(name);

	if (sdi) {
		osn::Input* obj = new osn::Input(sdi->id);
		args.GetReturnValue().Set(osn::Input::Store(obj));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "FromName", {ipc::value(name)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Input* obj = new osn::Input(response[1].value_union.ui64);
	args.GetReturnValue().Set(osn::Input::Store(obj));
}

void osn::Input::GetPublicSources(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "GetPublicSources", {});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::Array> arr = Nan::New<v8::Array>(int(response.size() - 1));
	for (size_t idx = 1; idx < response.size(); idx++) {
		osn::Input* obj    = new osn::Input(response[idx - 1].value_union.ui64);
		auto        object = osn::Input::Store(obj);
		Nan::Set(arr, uint32_t(idx), object);
	}

	args.GetReturnValue().Set(arr);
}

void osn::Input::Duplicate(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	//parameters
	std::string name       = "";
	bool        is_private = false;
	ASSERT_INFO_LENGTH_AT_LEAST(args, 0);
	if (args.Length() >= 1) {
		ASSERT_GET_VALUE(args[0], name);
	}
	if (args.Length() >= 2) {
		ASSERT_INFO_LENGTH(args, 2);
		ASSERT_GET_VALUE(args[1], is_private);
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(obj->sourceId)};
	if (args.Length() >= 1) {
		params.push_back(ipc::value(name));
	}
	if (args.Length() >= 2) {
		params.push_back(ipc::value(is_private));
	}

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "Duplicate", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	osn::Input* nobj = new osn::Input(response[1].value_union.ui64);
	args.GetReturnValue().Set(osn::Input::Store(nobj));
}

void osn::Input::Active(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "GetActive", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(response[1].value_union.i32);
	return;
}

void osn::Input::Showing(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetShowing", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(response[1].value_union.i32);
	return;
}

void osn::Input::Width(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	std::cout << "Width 0" << std::endl;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		std::cout << "Width 1" << std::endl;
		return;
	}
	std::cout << "Width 2" << std::endl;
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		std::cout << "Width 3" << std::endl;
		// How did you even call this? o.o
		return;
	}

	std::cout << "Width 4" << std::endl;
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "GetWidth", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(response[1].value_union.ui32);
}

void osn::Input::Height(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "GetHeight", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(response[1].value_union.ui32);
}

void osn::Input::GetVolume(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "GetVolume", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(response[1].value_union.fp32);
}

void osn::Input::SetVolume(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	float_t volume = 0.0f;
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], volume);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "SetVolume", {ipc::value(obj->sourceId), ipc::value(volume)});
}

void osn::Input::GetSyncOffset(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// Don't call methods on the prototype itself,
		// but call them on the object
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetSyncOffset", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::Object> tsobj = Nan::New<v8::Object>();

	utilv8::SetObjectField(tsobj, "sec", response[1].value_union.i64 / 1000000000);

	utilv8::SetObjectField(tsobj, "nsec", response[1].value_union.i64 % 1000000000);

	args.GetReturnValue().Set(tsobj);
}

void osn::Input::SetSyncOffset(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	v8::Local<v8::Object> tsobj;
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], tsobj);

	int64_t sec, nsec;
	ASSERT_GET_OBJECT_FIELD(tsobj, "sec", sec);
	ASSERT_GET_OBJECT_FIELD(tsobj, "nsec", nsec);

	int64_t syncoffset = sec * 1000000000 + nsec;

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "SetSyncOffset", {ipc::value(obj->sourceId), ipc::value(syncoffset)});
}

void osn::Input::GetAudioMixers(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(baseobj->sourceId);
	if (sdi && !sdi->audioMixersChanged && sdi->audioMixers != UINT32_MAX) {
		args.GetReturnValue().Set(sdi->audioMixers);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetAudioMixers", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	if (sdi) {
		sdi->audioMixers        = response[1].value_union.ui32;
		sdi->audioMixersChanged = false;
	}

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

void osn::Input::SetAudioMixers(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	uint32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "SetAudioMixers", {ipc::value(obj->sourceId), ipc::value(audiomixers)});

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(baseobj->sourceId);
	if (sdi) {
		sdi->audioMixersChanged = true;
	}
}

void osn::Input::GetMonitoringType(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetMonitoringType", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
}

void osn::Input::SetMonitoringType(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "SetMonitoringType", {ipc::value(obj->sourceId), ipc::value(audiomixers)});
}

void osn::Input::GetDeinterlaceFieldOrder(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetDeInterlaceFieldOrder", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
}

void osn::Input::SetDeinterlaceFieldOrder(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "SetDeInterlaceFieldOrder", {ipc::value(obj->sourceId), ipc::value(audiomixers)});
}

void osn::Input::GetDeinterlaceMode(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetDeInterlaceMode", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
	return;
}

void osn::Input::SetDeinterlaceMode(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "SetDeInterlaceMode", {ipc::value(obj->sourceId), ipc::value(audiomixers)});
}

void osn::Input::Filters(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(baseobj->sourceId);

	if (sdi && !sdi->filtersOrderChanged) {
		std::vector<uint64_t>* filters = sdi->filters;
		v8::Local<v8::Array>   arr     = Nan::New<v8::Array>(int(filters->size()));
		for (uint32_t i = 0; i < filters->size(); i++) {
			osn::Filter* obj    = new osn::Filter(filters->at(i));
			auto         object = osn::Filter::Store(obj);
			Nan::Set(arr, i, object);
		}
		args.GetReturnValue().Set(arr);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetFilters", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	std::vector<uint64_t>* filters;
	if (sdi) {
		filters = sdi->filters;
		filters->clear();
	}

	v8::Local<v8::Array> arr = Nan::New<v8::Array>(int(response.size()) - 1);
	for (size_t idx = 1; idx < response.size(); idx++) {
		osn::Filter* obj    = new osn::Filter(response[idx].value_union.ui64);
		auto         object = osn::Filter::Store(obj);
		Nan::Set(arr, uint32_t(idx) - 1, object);

		if (sdi) {
			filters->push_back(response[idx].value_union.ui64);
		}
	}

	args.GetReturnValue().Set(arr);

	if (sdi) {
		sdi->filtersOrderChanged = false;
	}
}

void osn::Input::AddFilter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(args, 1);

	v8::Local<v8::Object> objfilter;
	ASSERT_GET_VALUE(args[0], objfilter);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		args.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}
	osn::Filter* filter = static_cast<osn::Filter*>(basefilter);
	if (!filter) {
		args.GetIsolate()->ThrowException(
		    v8::Exception::TypeError(Nan::New<v8::String>("Source is not a filter.").ToLocalChecked()));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "AddFilter", {ipc::value(obj->sourceId), ipc::value(filter->sourceId)});

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(baseobj->sourceId);
	if (sdi) {
		sdi->filtersOrderChanged = true;
	}
}

void osn::Input::RemoveFilter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(args, 1);

	v8::Local<v8::Object> objfilter;
	ASSERT_GET_VALUE(args[0], objfilter);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		args.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "RemoveFilter", {ipc::value(obj->sourceId), ipc::value(basefilter->sourceId)});

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(baseobj->sourceId);
	if (sdi) {
		sdi->filtersOrderChanged = true;
	}
}

void osn::Input::SetFilterOrder(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(args, 2);

	v8::Local<v8::Object> objfilter;
	uint32_t              movement;
	ASSERT_GET_VALUE(args[0], objfilter);
	ASSERT_GET_VALUE(args[1], movement);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		args.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call(
	    "Input", "MoveFilter", {ipc::value(obj->sourceId), ipc::value(basefilter->sourceId), ipc::value(movement)});

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(baseobj->sourceId);
	if (sdi) {
		sdi->filtersOrderChanged = true;
	}
}

void osn::Input::FindFilter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(args, 1);

	std::string name;
	ASSERT_GET_VALUE(args[0], name);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "FindFilter", {ipc::value(obj->sourceId), ipc::value(name)});

	if (!ValidateResponse(response))
		return;

	if (response.size() > 1) {
		// Create new Filter
		osn::Input* nobj = new osn::Input(response[1].value_union.ui64);
		args.GetReturnValue().Set(osn::Input::Store(nobj));
	}
}

void osn::Input::CopyFilters(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(args.This(), baseobj)) {
		return;
	}
	osn::Input* obj = static_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(args, 1);

	v8::Local<v8::Object> objinput;
	ASSERT_GET_VALUE(args[0], objinput);

	osn::ISource* baseinput = nullptr;
	if (!osn::ISource::Retrieve(objinput, baseinput)) {
		args.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}
	osn::Input* input = static_cast<osn::Input*>(baseinput);
	if (!input) {
		args.GetIsolate()->ThrowException(
		    v8::Exception::TypeError(Nan::New<v8::String>("Source is not a input.").ToLocalChecked()));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Input", "CopyFiltersTo", {ipc::value(obj->sourceId), ipc::value(input->sourceId)});
}
