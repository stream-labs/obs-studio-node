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
#include "utility.hpp"
#include <iostream>

osn::VolMeter::VolMeter(uint64_t p_uid) {
	uid = p_uid;
	query_worker_close = false;
	query_worker = std::thread(std::bind(&osn::VolMeter::async_query, this));
}

osn::VolMeter::~VolMeter() {
	// Stop query thread
	query_worker_close = true;
	if (query_worker.joinable()) {
		query_worker.join();
	}

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

	uid = -1;
}

void osn::VolMeter::async_query() {
	size_t totalSleepMS = 0;

	while (!query_worker_close) {
		auto tp_start = std::chrono::high_resolution_clock::now();

		// Validate Connection
		auto conn = Controller::GetInstance().GetConnection();
		if (!conn) {
			goto do_sleep;
		}

		// Call
		{
			std::vector<ipc::value> response = conn->call_synchronous_helper("VolMeter", "Query", {
						ipc::value(uid),
			});
			if (!response.size()) {
				goto do_sleep;
			}
			if ((response.size() == 1) && (response[0].type == ipc::type::Null)) {
				goto do_sleep;
			}

			ErrorCode error = (ErrorCode)response[0].value_union.ui64;
			if (error == ErrorCode::Ok) {
				size_t channels = response[1].value_union.i32;
				for (auto cb : callbacks) {
					osn::VolMeterData* data = new osn::VolMeterData();
					data->magnitude.resize(channels);
					data->peak.resize(channels);
					data->input_peak.resize(channels);
					data->param = cb;
					for (size_t ch = 0; ch < channels; ch++) {
						data->magnitude[ch] = response[1 + ch * 3 + 0].value_union.fp32;
						data->peak[ch] = response[1 + ch * 3 + 0].value_union.fp32;
						data->input_peak[ch] = response[1 + ch * 3 + 0].value_union.fp32;
					}
					cb->queue.send(data);
				}
			} else {
				std::cerr << "Failed VolMeter" << std::endl;
				break;
			}
		}
		
	do_sleep:
		auto tp_end = std::chrono::high_resolution_clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
		totalSleepMS = sleepIntervalMS - dur.count();
		std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	}
	return;
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

	osn::VolMeterCallback::Init(target);
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

	self->sleepIntervalMS = rval[1].value_union.ui32;

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

	self->sleepIntervalMS = interval;

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
	osn::VolMeter* fader;

	// Arguments
	ASSERT_INFO_LENGTH(info, 1);
	if (!Retrieve(info.This(), fader)) {
		return;
	}

	v8::Local<v8::Function> callback;
	ASSERT_GET_VALUE(info[0], callback);

	// Grab IPC Connection
	std::shared_ptr<ipc::client> conn = nullptr;
	if (!(conn = GetConnection())) {
		return;
	}

	// Send request
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "AddCallback", {
		ipc::value(fader->uid)
	});
	if (!ValidateResponse(rval)) {
		return;
	}

	if (rval[0].value_union.ui64 != (uint64_t)ErrorCode::Ok) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	// Callback
	osn::VolMeterCallback *cb_binding = new osn::VolMeterCallback(fader, Callback, callback, 20);
	fader->callbacks.push_back(cb_binding);

	auto object = osn::VolMeterCallback::Store(cb_binding);
	cb_binding->obj_ref.Reset(object);
	info.GetReturnValue().Set(object);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info) {
	ASSERT_INFO_LENGTH(info, 1);

	osn::VolMeter* fader;
	if (!Retrieve(info.This(), fader)) {
		return;
	}

	v8::Local<v8::Object> cb_object;
	ASSERT_GET_VALUE(info[0], cb_object);
	
	osn::VolMeterCallback *cb_binding = nullptr;
	if (!osn::VolMeterCallback::Retrieve(cb_object, cb_binding)) {
		return;
	}

	// Grab IPC Connection
	std::shared_ptr<ipc::client> conn = nullptr;
	if (!(conn = GetConnection())) {
		return;
	}

	// Send request
	std::vector<ipc::value> rval = conn->call_synchronous_helper("VolMeter", "RemoveCallback", {
		ipc::value(fader->uid)
	});
	if (!ValidateResponse(rval)) {
		return;
	}

	if (rval[0].value_union.ui64 != (uint64_t)ErrorCode::Ok) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	cb_binding->stopped = true;
	cb_binding->obj_ref.Reset();

	fader->callbacks.remove(cb_binding);

	// /* What's this? A memory leak? Nope! The GC will automagically
	//  * destroy the CallbackData structure when it becomes weak. We
	//  * just need to make sure its in an unusable state. */
}

void osn::VolMeter::Callback(VolMeter* volmeter, VolMeterData* item) {
	if (!item) {
		return;
	}
	if (!volmeter) {
		delete item;
		return;
	}

	/* We're in v8 context here */
	VolMeterCallback *cb_binding = reinterpret_cast<VolMeterCallback*>(item->param);
	if (!cb_binding) {
		delete item;
		return;
	}

	if (cb_binding->stopped) {
		delete item;
		return;
	}

	// utilv8::ToValue on a std::vector<> creates a v8::Local<v8::Array> automatically.
	v8::Local<v8::Value> args[] = {
		utilv8::ToValue(item->magnitude),
		utilv8::ToValue(item->peak),
		utilv8::ToValue(item->input_peak)
	};
	delete item;
	Nan::Call(cb_binding->cb, 3, args);
}

Nan::Persistent<v8::FunctionTemplate> osn::VolMeterCallback::prototype = Nan::Persistent<v8::FunctionTemplate>();
