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
#include <iostream>
#include <iterator>
#include <vector>
#include "controller.hpp"
#include "error.hpp"
#include "isource.hpp"
#include "shared.hpp"
#include "utility-v8.hpp"
#include "utility.hpp"

using namespace std::placeholders;

osn::VolMeter::VolMeter(uint64_t p_uid)
{
	m_uid = p_uid;
}

osn::VolMeter::~VolMeter()
{
	// Destroy VolMeter on Server
	{
		// Validate Connection
		auto conn = Controller::GetInstance().GetConnection();
		if (!conn) {
			return; // Well, we can't really do anything here then.
		}

		// Call
		std::vector<ipc::value> rval = conn->call_synchronous_helper(
		    "VolMeter",
		    "Destroy",
		    {
		        ipc::value(m_uid),
		    });
		if (!rval.size()) {
			return; // Nothing we can do.
		}
	}

	m_uid = -1;
}

void osn::VolMeter::callback_update(CallbackManager<osn::VolMeterData>::DataCallback* dataCallback)
{
	// Validate Connection
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn) {
		return;
	}

	// Call
	try {
		std::vector<ipc::value> response = conn->call_synchronous_helper(
			"VolMeter",
			"Query",
			{
			    ipc::value(m_uid),
			});
		if (!response.size()) {
			return;
		}
		if ((response.size() == 1) && (response[0].type == ipc::type::Null)) {
			return;
		}

		ErrorCode error = (ErrorCode)response[0].value_union.ui64;
		if (error == ErrorCode::Ok) {
			std::shared_ptr<osn::VolMeterData> data     = std::make_shared<osn::VolMeterData>();
			size_t                             channels = response[1].value_union.i32;
			data->magnitude.resize(channels);
			data->peak.resize(channels);
			data->input_peak.resize(channels);
			data->param = this;
			for (size_t ch = 0; ch < channels; ch++) {
				data->magnitude[ch]  = response[1 + ch * 3 + 0].value_union.fp32;
				data->peak[ch]       = response[1 + ch * 3 + 1].value_union.fp32;
				data->input_peak[ch] = response[1 + ch * 3 + 2].value_union.fp32;
			}
			dataCallback->queue(std::move(data));
		} else {
			std::cerr << "Failed VolMeter" << std::endl;
			return;
		}
	} catch (std::exception e) {
		return;
	}
}

Nan::Persistent<v8::FunctionTemplate> osn::VolMeter::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::VolMeter::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
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

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Create(Nan::NAN_METHOD_ARGS_TYPE info)
{
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
	std::vector<ipc::value> rval = conn->call_synchronous_helper(
	    "VolMeter",
	    "Create",
	    {
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
	osn::VolMeter* obj    = new osn::VolMeter(rval[1].value_union.ui64);
	obj->m_callback_manager.SetUpdateInterval(rval[2].value_union.ui32);

	info.GetReturnValue().Set(Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::GetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info)
{
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
	std::vector<ipc::value> rval = conn->call_synchronous_helper(
	    "VolMeter",
	    "GetUpdateInterval",
	    {
	        ipc::value(self->m_uid),
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

	self->m_callback_manager.SetUpdateInterval(rval[1].value_union.ui32);

	// Return DeziBel Value
	info.GetReturnValue().Set(rval[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::SetUpdateInterval(Nan::NAN_METHOD_ARGS_TYPE info)
{
	uint32_t       interval;
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
	std::vector<ipc::value> rval =
	    conn->call_synchronous_helper("VolMeter", "SetUpdateInterval", {ipc::value(self->m_uid), ipc::value(interval)});
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

	self->m_callback_manager.SetUpdateInterval(interval);

	// Return DeziBel Value
	info.GetReturnValue().Set(rval[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Attach(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::VolMeter* fader;
	osn::ISource*  source;

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
	std::vector<ipc::value> rval =
	    conn->call_synchronous_helper("VolMeter", "Attach", {ipc::value(fader->m_uid), ipc::value(source->sourceId)});
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

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::Detach(Nan::NAN_METHOD_ARGS_TYPE info)
{
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
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "Detach", {ipc::value(fader->m_uid)});
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

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::AddCallback(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::VolMeter*          self;
	v8::Local<v8::Function> callback;

	{
		// Arguments
		ASSERT_INFO_LENGTH(info, 1);
		if (!Retrieve(info.This(), self)) {
			return;
		}

		ASSERT_GET_VALUE(info[0], callback);
	}

	{
		// Grab IPC Connection
		std::shared_ptr<ipc::client> conn = nullptr;
		if (!(conn = GetConnection())) {
			return;
		}

		// Send request
		std::vector<ipc::value> rval =
		    conn->call_synchronous_helper("VolMeter", "AddCallback", {ipc::value(self->m_uid)});
		if (!ValidateResponse(rval)) {
			return;
		}

		if (rval[0].value_union.ui64 != (uint64_t)ErrorCode::Ok) {
			info.GetReturnValue().Set(Nan::Null());
			return;
		}
	}

	self->m_callback_manager.Initialize(callback, 
		info.This(),
		std::bind(&VolMeter::callback_update, self, _1), 
		[] (void* data, std::shared_ptr<osn::VolMeterData> item, Nan::Callback& callback) {

		    // utilv8::ToValue on a std::vector<> creates a v8::Local<v8::Array> automatically.
		    v8::Local<v8::Value> args[] = {
		        utilv8::ToValue(item->magnitude), utilv8::ToValue(item->peak), utilv8::ToValue(item->input_peak)};

		    Nan::Call(callback, 3, args);
		});

	info.GetReturnValue().Set(true);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::VolMeter* self;

	{
		ASSERT_INFO_LENGTH(info, 1);
		if (!Retrieve(info.This(), self)) {
			return;
		}
	}

	self->m_callback_manager.Shutdown();

	// Grab IPC Connection
	{
		std::shared_ptr<ipc::client> conn = nullptr;
		if (!(conn = GetConnection())) {
			return;
		}

		// Send request
		std::vector<ipc::value> rval =
		    conn->call_synchronous_helper("VolMeter", "RemoveCallback", {ipc::value(self->m_uid)});
		if (!ValidateResponse(rval)) {
			return;
		}
	}

	info.GetReturnValue().Set(true);
}
