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

Nan::Persistent<v8::FunctionTemplate> osn::Module::prototype;

void osn::Module::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Module").ToLocalChecked());

	utilv8::SetTemplateField(fnctemplate, "open", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Open));
	utilv8::SetTemplateField(fnctemplate, "modules", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Modules));

	v8::Local<v8::Template> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "initialize", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Initialize));

	utilv8::SetTemplateAccessorProperty(objtemplate, "name", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Name));
	utilv8::SetTemplateAccessorProperty(objtemplate, "fileName", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), FileName));
	utilv8::SetTemplateAccessorProperty(objtemplate, "author", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Author));
	utilv8::SetTemplateAccessorProperty(objtemplate, "description", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Description));
	utilv8::SetTemplateAccessorProperty(objtemplate, "binaryPath", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), BinaryPath));
	utilv8::SetTemplateAccessorProperty(objtemplate, "dataPath", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), DataPath));

	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "Module").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::Module::Open(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string bin_path, data_path;

	ASSERT_INFO_LENGTH(args, 2);
	ASSERT_GET_VALUE(args[0], bin_path);
	ASSERT_GET_VALUE(args[1], data_path);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Module", "Open", {ipc::value(bin_path), ipc::value(data_path)});

	if (!ValidateResponse(response))
		return;

	osn::Module* obj = new osn::Module(response[1].value_union.ui64);
	args.GetReturnValue().Set(osn::Module::Store(obj));
}

void osn::Module::Modules(const v8::FunctionCallbackInfo<v8::Value>& args)
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
		modules->Set(
		    Nan::GetCurrentContext(),
		    i - 2,
		    v8::String::NewFromUtf8(isolate, response.at(i).value_str.c_str()).ToLocalChecked());
	}

	args.GetReturnValue().Set(modules);
}

void osn::Module::Initialize(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(args, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "Initialize", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(response[1].value_union.i32);
}

void osn::Module::Name(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(args, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetName", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

void osn::Module::FileName(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(args, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetFileName", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

void osn::Module::Description(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(args, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetDescription", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

void osn::Module::Author(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(args, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetAuthor", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

void osn::Module::BinaryPath(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(args, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetBinaryPath", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

void osn::Module::DataPath(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Module* module;
	if (!utilv8::SafeUnwrap(args, module)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Module", "GetDataPath", {ipc::value(module->moduleId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}