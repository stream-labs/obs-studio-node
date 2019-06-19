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

#include <condition_variable>
#include <mutex>
#include <string>
#include "controller.hpp"
#include "error.hpp"
#include "ipc-value.hpp"
#include "shared.hpp"
#include "utility.hpp"
#include "module.hpp"

osn::Module::Module(uint64_t id)
{
	this->moduleId = id;
}

Nan::Persistent<v8::FunctionTemplate> osn::Module::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::Module::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Module").ToLocalChecked());

	utilv8::SetTemplateField(fnctemplate, "open", Open);
	utilv8::SetTemplateField(fnctemplate, "modules", Modules);

	v8::Local<v8::Template> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "initialize", Initialize);

	utilv8::SetTemplateAccessorProperty(objtemplate, "name", Name);
	utilv8::SetTemplateAccessorProperty(objtemplate, "fileName", FileName);
	utilv8::SetTemplateAccessorProperty(objtemplate, "author", Author);
	utilv8::SetTemplateAccessorProperty(objtemplate, "description", Description);
	utilv8::SetTemplateAccessorProperty(objtemplate, "binaryPath", BinaryPath);
	utilv8::SetTemplateAccessorProperty(objtemplate, "dataPath", DataPath);

	utilv8::SetObjectField(target, "Module", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::Open(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string bin_path, data_path;

	ASSERT_INFO_LENGTH(info, 2);
	ASSERT_GET_VALUE(info[0], bin_path);
	ASSERT_GET_VALUE(info[1], data_path);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Module", "Open", {ipc::value(bin_path), ipc::value(data_path)});

	if (!ValidateResponse(response))
		return;

	osn::Module* obj = new osn::Module(response[1].value_union.ui64);
	info.GetReturnValue().Set(osn::Module::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::Modules(Nan::NAN_METHOD_ARGS_TYPE info)
{
	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "Modules", {});

	if (!ValidateResponse(response))
		return;

	v8::Isolate*         isolate    = v8::Isolate::GetCurrent();
	v8::Local<v8::Array> modules = v8::Array::New(isolate);

	uint64_t size = response[1].value_union.ui64;

	for (uint64_t i = 2; i < (size + 2); i++) {
		modules->Set(i - 2, v8::String::NewFromUtf8(isolate, response.at(i).value_str.c_str()));
	}

	info.GetReturnValue().Set(modules);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::Initialize(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(info, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "Initialize", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(response[1].value_union.i32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::Name(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(info, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetName", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::FileName(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(info, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetFileName", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::Description(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(info, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetDescription", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::Author(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(info, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetAuthor", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::BinaryPath(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(info, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetBinaryPath", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Module::DataPath(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(info, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetDataPath", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}