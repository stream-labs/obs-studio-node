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

#include "volmeter.hpp"
#include "controller.hpp"
#include <vector>
#include <iterator>
#include "shared.hpp"
#include "error.hpp"
#include "isource.hpp"
#include "utility-v8.hpp"

osn::VolMeter::VolMeter(uint64_t uid) {
	this->uid = uid;
}

osn::VolMeter::~VolMeter() {
	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		return; // Well, we can't really do anything here then.
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "Destroy", {
		ipc::value(uid),
	});
	if (!rval.size()) {
		return; // Nothing we can do.
	}
}

Nan::Persistent<v8::FunctionTemplate> osn::VolMeter::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::VolMeter::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Volmeter").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "create", Create);

	// Instance Template
	auto objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateAccessorProperty(objtemplate, "updateInterval", GetUpdateInterval, SetUpdateInterval);
	utilv8::SetTemplateField(objtemplate, "attach", Attach);
	utilv8::SetTemplateField(objtemplate, "detach", Detach);
	utilv8::SetTemplateField(objtemplate, "addCallback", AddCallback);
	utilv8::SetTemplateField(objtemplate, "removeCallback", RemoveCallback);

	// Stuff
	utilv8::SetObjectField(target, "Volmeter", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Create(Nan::NAN_METHOD_ARGS_TYPE info) {
	int32_t fader_type;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], fader_type);

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "Create", {
		ipc::value(fader_type),
	});
	if (!rval.size()) {
		Nan::ThrowError("Failed to make IPC call, verify IPC status.");
		return;
	}

	// Handle Unexpected Errors
	if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
		Nan::ThrowError(Nan::New(rval[0].value_str).ToLocalChecked());
		return;
	}

	// Handle Expected Errors
	ErrorCode ec = (ErrorCode)rval[0].value_union.ui64; 
	if (ec == ErrorCode::InvalidReference) {
		Nan::ThrowReferenceError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	} else if (ec != ErrorCode::Ok) {
		Nan::ThrowError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	}

	// Return created Object
	osn::VolMeter* obj = new osn::VolMeter(rval[1].value_union.ui64);
	info.GetReturnValue().Set(Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::GetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::VolMeter* self;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(info, 0);

	if (!Retrieve(info.This(), self)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "GetUpdateInterval", {
		ipc::value(self->uid),
	});
	if (!rval.size()) {
		Nan::ThrowError("Failed to make IPC call, verify IPC status.");
		return;
	}

	// Handle Unexpected Errors
	if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
		Nan::ThrowError(Nan::New(rval[0].value_str).ToLocalChecked());
		return;
	}

	// Handle Expected Errors
	ErrorCode ec = (ErrorCode)rval[0].value_union.ui64;
	if (ec == ErrorCode::InvalidReference) {
		Nan::ThrowReferenceError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	} else if (ec != ErrorCode::Ok) {
		Nan::ThrowError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	}

	// Return DeziBel Value
	info.GetReturnValue().Set(rval[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::SetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info) {
	uint32_t interval;
	osn::VolMeter* self;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], interval);

	if (!Retrieve(info.This(), self)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "SetUpdateInterval", {
		ipc::value(self->uid), ipc::value(interval)
	});
	if (!rval.size()) {
		Nan::ThrowError("Failed to make IPC call, verify IPC status.");
		return;
	}

	// Handle Unexpected Errors
	if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
		Nan::ThrowError(Nan::New(rval[0].value_str).ToLocalChecked());
		return;
	}

	// Handle Expected Errors
	ErrorCode ec = (ErrorCode)rval[0].value_union.ui64;
	if (ec == ErrorCode::InvalidReference) {
		Nan::ThrowReferenceError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	} else if (ec != ErrorCode::Ok) {
		Nan::ThrowError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	}

	// Return DeziBel Value
	info.GetReturnValue().Set(rval[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Attach(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::VolMeter* fader;
	osn::ISource* source;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(info, 1);

	if (!Retrieve(info.This(), fader)) {
		return;
	}

	v8::Local<v8::Object> sourceObj;
	ASSERT_GET_VALUE(info[0], sourceObj);
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
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "Attach", {
		ipc::value(fader->uid), ipc::value(source->sourceId)
	});
	if (!rval.size()) {
		Nan::ThrowError("Failed to make IPC call, verify IPC status.");
		return;
	}

	// Handle Unexpected Errors
	if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
		Nan::ThrowError(Nan::New(rval[0].value_str).ToLocalChecked());
		return;
	}

	// Handle Expected Errors
	ErrorCode ec = (ErrorCode)rval[0].value_union.ui64;
	if (ec == ErrorCode::InvalidReference) {
		Nan::ThrowReferenceError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	} else if (ec != ErrorCode::Ok) {
		Nan::ThrowError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	}
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Detach(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::VolMeter* fader;

	// Validate and retrieve parameters.
	ASSERT_INFO_LENGTH(info, 0);

	if (!Retrieve(info.This(), fader)) {
		return;
	}

	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		Nan::ThrowError("IPC is not connected.");
		return;
	}

	// Call
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "Detach", {
		ipc::value(fader->uid)
	});
	if (!rval.size()) {
		Nan::ThrowError("Failed to make IPC call, verify IPC status.");
		return;
	}

	// Handle Unexpected Errors
	if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
		Nan::ThrowError(Nan::New(rval[0].value_str).ToLocalChecked());
		return;
	}

	// Handle Expected Errors
	ErrorCode ec = (ErrorCode)rval[0].value_union.ui64;
	if (ec == ErrorCode::InvalidReference) {
		Nan::ThrowReferenceError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	} else if (ec != ErrorCode::Ok) {
		Nan::ThrowError(Nan::New(rval[1].value_str).ToLocalChecked());
		return;
	}
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::AddCallback(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());
	//Volmeter* binding = Nan::ObjectWrap::Unwrap<Volmeter>(info.Holder());

	//ASSERT_INFO_LENGTH(info, 1);

	//v8::Local<v8::Function> callback;
	//ASSERT_GET_VALUE(info[0], callback);

	//VolmeterCallback *cb_binding =
	//	new VolmeterCallback(binding, Volmeter::Callback, callback, 50);

	//cb_binding->user_data = new int;
	//*((int*)cb_binding->user_data) = binding->handle.nr_channels();

	//handle.add_callback(volmeter_cb_wrapper, cb_binding);

	//auto object = VolmeterCallback::Object::GenerateObject(cb_binding);
	//cb_binding->obj_ref.Reset(object);
	//info.GetReturnValue().Set(object);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info) {
	//obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

	//v8::Local<v8::Object> cb_object;
	//ASSERT_GET_VALUE(info[0], cb_object);

	//VolmeterCallback *cb_binding =
	//	VolmeterCallback::Object::GetHandle(cb_object);

	//cb_binding->stopped = true;
	//cb_binding->obj_ref.Reset();

	//handle.remove_callback(volmeter_cb_wrapper, cb_binding);

	//delete cb_binding->user_data;
	//cb_binding->user_data = 0;

	// /* What's this? A memory leak? Nope! The GC will automagically
	//  * destroy the CallbackData structure when it becomes weak. We
	//  * just need to make sure its in an unusable state. */
}
