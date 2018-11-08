// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "input.hpp"
#include <condition_variable>
#include <mutex>
#include <string>
#include "controller.hpp"
#include "error.hpp"
#include "filter.hpp"
#include "ipc-value.hpp"
#include "shared.hpp"
#include "utility.hpp"

osn::Input::Input(uint64_t id)
{
	this->sourceId = id;
}

Nan::Persistent<v8::FunctionTemplate> osn::Input::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::Input::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Input").ToLocalChecked());

	// Function Template
	utilv8::SetTemplateField(fnctemplate, "types", Types);
	utilv8::SetTemplateField(fnctemplate, "create", Create);
	utilv8::SetTemplateField(fnctemplate, "createPrivate", CreatePrivate);
	utilv8::SetTemplateField(fnctemplate, "fromName", FromName);
	utilv8::SetTemplateField(fnctemplate, "getPublicSources", GetPublicSources);

	// Prototype Template

	// Instance Template
	v8::Local<v8::Template> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "duplicate", Duplicate);
	utilv8::SetTemplateAccessorProperty(objtemplate, "active", Active);
	utilv8::SetTemplateAccessorProperty(objtemplate, "showing", Showing);
	utilv8::SetTemplateAccessorProperty(objtemplate, "width", Width);
	utilv8::SetTemplateAccessorProperty(objtemplate, "height", Height);
	utilv8::SetTemplateAccessorProperty(objtemplate, "volume", GetVolume, SetVolume);
	utilv8::SetTemplateAccessorProperty(objtemplate, "syncOffset", GetSyncOffset, SetSyncOffset);
	utilv8::SetTemplateAccessorProperty(objtemplate, "audioMixers", GetAudioMixers, SetAudioMixers);
	utilv8::SetTemplateAccessorProperty(objtemplate, "monitoringType", GetMonitoringType, SetMonitoringType);
	utilv8::SetTemplateAccessorProperty(
	    objtemplate, "deinterlaceFieldOrder", GetDeinterlaceFieldOrder, SetDeinterlaceFieldOrder);
	utilv8::SetTemplateAccessorProperty(objtemplate, "deinterlaceMode", GetDeinterlaceMode, SetDeinterlaceMode);

	utilv8::SetTemplateAccessorProperty(objtemplate, "filters", Filters);
	utilv8::SetTemplateField(objtemplate, "addFilter", AddFilter);
	utilv8::SetTemplateField(objtemplate, "removeFilter", RemoveFilter);
	utilv8::SetTemplateField(objtemplate, "setFilterOrder", SetFilterOrder);
	utilv8::SetTemplateField(objtemplate, "findFilter", FindFilter);
	utilv8::SetTemplateField(objtemplate, "copyFilters", CopyFilters);

	// Stuff
	utilv8::SetObjectField(target, "Input", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Types(Nan::NAN_METHOD_ARGS_TYPE info)
{
	// Function takes no parameters.
	ASSERT_INFO_LENGTH(info, 0);

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

	info.GetReturnValue().Set(utilv8::ToValue<std::string>(types));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Create(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string           type;
	std::string           name;
	v8::Local<v8::String> settings = Nan::New<v8::String>("").ToLocalChecked();
	v8::Local<v8::String> hotkeys  = Nan::New<v8::String>("").ToLocalChecked();

	// Parameters: <string> Type, <string> Name[,<object> settings]
	ASSERT_INFO_LENGTH_AT_LEAST(info, 2);

	ASSERT_GET_VALUE(info[0], type);
	ASSERT_GET_VALUE(info[1], name);

	// Check if caller provided settings to send across.
	if (info.Length() >= 4) {
		ASSERT_INFO_LENGTH(info, 4);
		if (!info[2]->IsUndefined()) {
			v8::Local<v8::Object> hksobj;
			ASSERT_GET_VALUE(info[2], hksobj);
			hotkeys = v8::JSON::Stringify(info.GetIsolate()->GetCurrentContext(), hksobj).ToLocalChecked();
		}
	}
	if (info.Length() >= 3) {
		ASSERT_INFO_LENGTH_AT_LEAST(info, 3);
		if (!info[2]->IsUndefined()) {
			v8::Local<v8::Object> setobj;
			ASSERT_GET_VALUE(info[2], setobj);
			settings = v8::JSON::Stringify(info.GetIsolate()->GetCurrentContext(), setobj).ToLocalChecked();
		}
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
	if (hotkeys->Length() != 0) {
		std::string value;
		if (utilv8::FromValue(hotkeys, value)) {
			params.push_back(ipc::value(value));
		}
	}

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "Create", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Input* obj = new osn::Input(response[1].value_union.ui64);
	info.GetReturnValue().Set(osn::Input::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::CreatePrivate(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string           type;
	std::string           name;
	v8::Local<v8::String> settings = Nan::New<v8::String>("").ToLocalChecked();

	// Parameters: <string> Type, <string> Name[,<object> settings]
	ASSERT_INFO_LENGTH_AT_LEAST(info, 2);

	ASSERT_GET_VALUE(info[0], type);
	ASSERT_GET_VALUE(info[1], name);

	// Check if caller provided settings to send across.
	if (info.Length() >= 3) {
		ASSERT_INFO_LENGTH(info, 3);

		v8::Local<v8::Object> setobj;
		ASSERT_GET_VALUE(info[2], setobj);

		settings = v8::JSON::Stringify(info.GetIsolate()->GetCurrentContext(), setobj).ToLocalChecked();
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
	info.GetReturnValue().Set(osn::Input::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::FromName(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string name;

	// Parameters: <string> Name
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], name);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "FromName", {ipc::value(name)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Input* obj = new osn::Input(response[1].value_union.ui64);
	info.GetReturnValue().Set(osn::Input::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetPublicSources(Nan::NAN_METHOD_ARGS_TYPE info)
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

	info.GetReturnValue().Set(arr);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Duplicate(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	//parameters
	std::string name       = "";
	bool        is_private = false;
	ASSERT_INFO_LENGTH_AT_LEAST(info, 0);
	if (info.Length() >= 1) {
		ASSERT_GET_VALUE(info[0], name);
	}
	if (info.Length() >= 2) {
		ASSERT_INFO_LENGTH(info, 2);
		ASSERT_GET_VALUE(info[1], is_private);
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(obj->sourceId)};
	if (info.Length() >= 1) {
		params.push_back(ipc::value(name));
	}
	if (info.Length() >= 2) {
		params.push_back(ipc::value(is_private));
	}

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "Duplicate", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	osn::Input* nobj = new osn::Input(response[1].value_union.ui64);
	info.GetReturnValue().Set(osn::Input::Store(nobj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Active(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(response[1].value_union.i32);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Showing(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(response[1].value_union.i32);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Width(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Input", "GetWidth", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(response[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Height(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(response[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetVolume(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(response[1].value_union.fp32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetVolume(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	float_t volume = 0.0f;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], volume);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "SetVolume", {ipc::value(obj->sourceId), ipc::value(volume)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(response[1].value_union.fp32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetSyncOffset(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(tsobj);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetSyncOffset(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	v8::Local<v8::Object> tsobj;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], tsobj);

	int64_t sec, nsec;
	ASSERT_GET_OBJECT_FIELD(tsobj, "sec", sec);
	ASSERT_GET_OBJECT_FIELD(tsobj, "nsec", nsec);

	int64_t syncoffset = sec * 1000000000 + nsec;

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "SetSyncOffset", {ipc::value(obj->sourceId), ipc::value(syncoffset)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i64));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetAudioMixers(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetAudioMixers", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetAudioMixers(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	uint32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "SetAudioMixers", {ipc::value(obj->sourceId), ipc::value(audiomixers)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetMonitoringType(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetMonitoringType(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Input", "SetMonitoringType", {ipc::value(obj->sourceId), ipc::value(audiomixers)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetDeinterlaceFieldOrder(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetDeinterlaceFieldOrder(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Input", "SetDeInterlaceFieldOrder", {ipc::value(obj->sourceId), ipc::value(audiomixers)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetDeinterlaceMode(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
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

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetDeinterlaceMode(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Input", "SetDeInterlaceMode", {ipc::value(obj->sourceId), ipc::value(audiomixers)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Filters(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "GetFilters", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::Array> arr = Nan::New<v8::Array>(int(response.size()) - 1);
	for (size_t idx = 1; idx < response.size(); idx++) {
		osn::Filter* obj    = new osn::Filter(response[idx].value_union.ui64);
		auto         object = osn::Filter::Store(obj);
		Nan::Set(arr, uint32_t(idx) - 1, object);
	}

	info.GetReturnValue().Set(arr);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::AddFilter(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> objfilter;
	ASSERT_GET_VALUE(info[0], objfilter);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}
	osn::Filter* filter = dynamic_cast<osn::Filter*>(basefilter);
	if (!filter) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::TypeError(Nan::New<v8::String>("Source is not a filter.").ToLocalChecked()));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Input", "AddFilter", {ipc::value(obj->sourceId), ipc::value(filter->sourceId)});

	ValidateResponse(response);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::RemoveFilter(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> objfilter;
	ASSERT_GET_VALUE(info[0], objfilter);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Input", "RemoveFilter", {ipc::value(obj->sourceId), ipc::value(basefilter->sourceId)});

	ValidateResponse(response);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetFilterOrder(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 2);

	v8::Local<v8::Object> objfilter;
	uint32_t              movement;
	ASSERT_GET_VALUE(info[0], objfilter);
	ASSERT_GET_VALUE(info[1], movement);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Input", "MoveFilter", {ipc::value(obj->sourceId), ipc::value(basefilter->sourceId), ipc::value(movement)});

	ValidateResponse(response);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::FindFilter(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	std::string name;
	ASSERT_GET_VALUE(info[0], name);

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
		info.GetReturnValue().Set(osn::Input::Store(nobj));
	}
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::CopyFilters(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> objinput;
	ASSERT_GET_VALUE(info[0], objinput);

	osn::ISource* baseinput = nullptr;
	if (!osn::ISource::Retrieve(objinput, baseinput)) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::ReferenceError(Nan::New<v8::String>("Source is invalid.").ToLocalChecked()));
	}
	osn::Input* input = dynamic_cast<osn::Input*>(baseinput);
	if (!input) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::TypeError(Nan::New<v8::String>("Source is not a input.").ToLocalChecked()));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Input", "CopyFiltersTo", {ipc::value(obj->sourceId), ipc::value(input->sourceId)});

	ValidateResponse(response);
}
