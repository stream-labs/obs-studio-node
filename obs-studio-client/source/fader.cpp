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

#include "fader.hpp"
#include <iterator>
#include <vector>
#include "controller.hpp"
#include "error.hpp"
#include "isource.hpp"
#include "shared.hpp"

Nan::Persistent<v8::FunctionTemplate> osn::Fader::prototype;

osn::Fader::Fader(uint64_t uid)
{
	this->uid = uid;
}

osn::Fader::~Fader()
{
}

uint64_t osn::Fader::GetId()
{
	return this->uid;
}

void osn::Fader::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Fader").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "create", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Create));

	// Object Template
	auto objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateAccessorProperty(objtemplate, "db",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetDeziBel),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetDezibel));
	utilv8::SetTemplateAccessorProperty(objtemplate, "deflection",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetDeflection),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetDeflection));
	utilv8::SetTemplateAccessorProperty(objtemplate, "mul",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetMultiplier),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetMultiplier));
	utilv8::SetTemplateField(objtemplate, "attach", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Attach));
	utilv8::SetTemplateField(objtemplate, "detach", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Detach));
	utilv8::SetTemplateField(objtemplate, "addCallback", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), AddCallback));
	utilv8::SetTemplateField(objtemplate, "removeCallback", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), RemoveCallback));

	// Stuff
	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "Fader").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::Fader::Create(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::cout << "Fader::Create 0" << std::endl;
	int32_t fader_type;

	// Validate and retrieve parameters.
	// ASSERT_INFO_LENGTH(args, 1);
	std::cout << "Fader::Create 0.1" << std::endl;
	ASSERT_GET_VALUE(args[0], fader_type);
	std::cout << "Fader::Create 0.2" << std::endl;

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	std::cout << "Fader::Create 0.3" << std::endl;
	if (!conn) {
		std::cout << "Fader::Create 0.4" << std::endl;
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	std::cout << "Fader::Create 1" << std::endl;
	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper(
	    "Fader",
	    "Create",
	    {
	        ipc::value(fader_type),
	    });

	if (!ValidateResponse(rval)) {
		return;
	}

	std::cout << "Fader::Create 2" << std::endl;
	// Return created Object
	auto* newFader = new osn::Fader(rval[1].value_union.ui64);
	std::cout << "Fader::Create 3" << std::endl;
	args.GetReturnValue().Set(Store(newFader));
	std::cout << "Fader::Create 4" << std::endl;
}

void osn::Fader::GetDeziBel(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Fader* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 0);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper(
	    "Fader",
	    "GetDeziBel",
	    {
	        ipc::value(fader->uid),
	    });

	if (!ValidateResponse(rval)) {
		return;
	}

	// Return DeziBel Value
	args.GetReturnValue().Set(rval[1].value_union.fp32);
}

void osn::Fader::SetDezibel(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	float_t     dezibel;
	osn::Fader* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], dezibel);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	conn->call("Fader", "SetDeziBel", {ipc::value(fader->uid), ipc::value(dezibel)});
}

void osn::Fader::GetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Fader* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 0);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper(
	    "Fader",
	    "GetDeflection",
	    {
	        ipc::value(fader->uid),
	    });

	if (!ValidateResponse(rval)) {
		return;
	}

	// Return DeziBel Value
	args.GetReturnValue().Set(rval[1].value_union.fp32);
}

void osn::Fader::SetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	float_t     dezibel;
	osn::Fader* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], dezibel);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	conn->call("Fader", "SetDeflection", {ipc::value(fader->uid), ipc::value(dezibel)});
}

void osn::Fader::GetMultiplier(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Fader* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 0);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper(
	    "Fader",
	    "GetMultiplier",
	    {
	        ipc::value(fader->uid),
	    });

	if (!ValidateResponse(rval)) {
		return;
	}

	// Return DeziBel Value
	args.GetReturnValue().Set(rval[1].value_union.fp32);
}

void osn::Fader::SetMultiplier(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	float_t     dezibel;
	osn::Fader* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], dezibel);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	conn->call("Fader", "SetMultiplier", {ipc::value(fader->uid), ipc::value(dezibel)});
}

void osn::Fader::Attach(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Fader*   fader;
	osn::ISource* source;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 1);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	v8::Local<v8::Object> sourceObj;
	ASSERT_GET_VALUE(args[0], sourceObj);
	if (!osn::ISource::Retrieve(sourceObj, source)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval =
	    conn->call_synchronous_helper("Fader", "Attach", {ipc::value(fader->uid), ipc::value(source->sourceId)});

	if (!ValidateResponse(rval)) {
		return;
	}
}

void osn::Fader::Detach(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Fader* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(args, 0);

	if (!Retrieve(args.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper("Fader", "Detach", {ipc::value(fader->uid)});

	if (!ValidateResponse(rval)) {
		return;
	}
}

void osn::Fader::AddCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	//obs::fader &handle = Fader::Object::GetHandle(args.Holder());
	//Fader* binding = Nan::ObjectWrap::Unwrap<Fader>(args.Holder());

	//ASSERT_INFO_LENGTH(info, 1);

	//v8::Local<v8::Function> callback;
	//ASSERT_GET_VALUE(info[0], callback);

	//FaderCallback *cb_binding =
	//	new FaderCallback(binding, Fader::Callback, callback);

	//handle.add_callback(fader_cb_wrapper, cb_binding);

	//auto object = FaderCallback::Object::GenerateObject(cb_binding);
	//cb_binding->obj_ref.Reset(object);
	//args.GetReturnValue().Set(object);
}

void osn::Fader::RemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	//obs::fader &handle = Fader::Object::GetHandle(args.Holder());

	//v8::Local<v8::Object> cb_object;
	//ASSERT_GET_VALUE(info[0], cb_object);

	//FaderCallback *cb_binding =
	//	FaderCallback::Object::GetHandle(cb_object);

	//cb_binding->stopped = true;

	//handle.remove_callback(fader_cb_wrapper, cb_binding);
	//cb_binding->obj_ref.Reset();

	///* What's this? A memory leak? Nope! The GC will decide when
	//* and where to destroy the object. */
}

INITIALIZER(nodeobs_fader)
{
}