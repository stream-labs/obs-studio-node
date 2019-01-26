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
#include "controller.hpp"
#include "error.hpp"
#include "ipc-value.hpp"
#include "shared.hpp"
#include "utility.hpp"

osn::Transition::Transition(uint64_t id)
{
	this->sourceId = id;
}

void osn::Transition::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
//    fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Transition").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "types", Types);
	utilv8::SetTemplateField(fnctemplate, "create", Create);
	utilv8::SetTemplateField(fnctemplate, "createPrivate", CreatePrivate);
	utilv8::SetTemplateField(fnctemplate, "fromName", FromName);

	// Object Template
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "getActiveSource", GetActiveSource);
	utilv8::SetTemplateField(objtemplate, "start", Start);
	utilv8::SetTemplateField(objtemplate, "set", Set);
	utilv8::SetTemplateField(objtemplate, "clear", Clear);

	// Stuff
	utilv8::SetObjectField(target, "Transition", fnctemplate->GetFunction());
//    prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Types(Nan::NAN_METHOD_ARGS_TYPE info)
{
	// Function takes no parameters.
	ASSERT_INFO_LENGTH(info, 0);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Transition", "Types", {});

	if (!ValidateResponse(response))
		return;

	std::vector<std::string> types;
	size_t                   count = response.size() - 1;

	for (size_t idx = 0; idx < count; idx++) {
		types.push_back(response[1 + idx].value_str);
	}

	info.GetReturnValue().Set(utilv8::ToValue<std::string>(types));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Create(Nan::NAN_METHOD_ARGS_TYPE info)
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

	std::vector<ipc::value> response = conn->call_synchronous_helper("Transition", "Create", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Transition* obj = new osn::Transition(response[1].value_union.ui64);

	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = type;
	sdi->id             = response[1].value_union.ui64;

	CacheManager<SourceDataInfo*>::getInstance().Store(response[1].value_union.ui64, name, sdi);

	info.GetReturnValue().Set(osn::Transition::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::CreatePrivate(Nan::NAN_METHOD_ARGS_TYPE info)
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

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Transition", "CreatePrivate", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Transition* obj = new osn::Transition(response[1].value_union.ui64);

	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = type;
	sdi->id             = response[1].value_union.ui64;

	CacheManager<SourceDataInfo*>::getInstance().Store(response[1].value_union.ui64, name, sdi);

	info.GetReturnValue().Set(osn::Transition::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::FromName(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string name;

	// Parameters: <string> Name
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], name);

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(name)};

	std::vector<ipc::value> response = conn->call_synchronous_helper("Transition", "FromName", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Transition* obj = new osn::Transition(response[1].value_union.ui64);
	info.GetReturnValue().Set(osn::Transition::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::GetActiveSource(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = static_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(obj->sourceId)};

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Transition", "GetActiveSource", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	if (response[2].value_union.ui32 == 0) {
		// Input
		osn::Input* obj = new osn::Input(response[1].value_union.ui64);
		info.GetReturnValue().Set(osn::Input::Store(obj));
	} else if (response[2].value_union.ui32 == 3) {
		// Scene
		osn::Scene* obj = new osn::Scene(response[1].value_union.ui64);
		info.GetReturnValue().Set(osn::Scene::Store(obj));
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Clear(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = static_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(obj->sourceId)};

	conn->call("Transition", "Clear", {std::move(params)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Set(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = static_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> targetbaseobj;
	ASSERT_GET_VALUE(info[0], targetbaseobj);

	osn::ISource* targetobj = nullptr;
	if (!osn::ISource::Retrieve(targetbaseobj, targetobj)) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::TypeError(Nan::New<v8::String>("Invalid type for target source.").ToLocalChecked()));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(obj->sourceId), ipc::value(targetobj->sourceId)};

	conn->call("Transition", "Set", {std::move(params)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Start(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = static_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Parameters
	ASSERT_INFO_LENGTH(info, 2);

	uint32_t ms = 0;
	ASSERT_GET_VALUE(info[0], ms);

	v8::Local<v8::Object> targetbaseobj;
	ASSERT_GET_VALUE(info[1], targetbaseobj);
	osn::ISource* targetobj = nullptr;
	if (!osn::ISource::Retrieve(targetbaseobj, targetobj)) {
		info.GetIsolate()->ThrowException(
		    v8::Exception::TypeError(Nan::New<v8::String>("Invalid type for target source.").ToLocalChecked()));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	auto params = std::vector<ipc::value>{ipc::value(obj->sourceId), ipc::value(ms), ipc::value(targetobj->sourceId)};

	std::vector<ipc::value> response = conn->call_synchronous_helper("Transition", "Start", {std::move(params)});

	if (!ValidateResponse(response))
		return;
	info.GetReturnValue().Set(!!response[1].value_union.i32);
}
