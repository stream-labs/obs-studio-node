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

std::mutex                         volmeters_lock;
std::map<uint64_t, osn::VolMeter*> volmeters;

osn::VolMeter::VolMeter(uint64_t p_uid)
{
	m_uid = p_uid;
}

osn::VolMeter::~VolMeter()
{
}

uint64_t osn::VolMeter::GetId() 
{
	return m_uid;
}

void osn::VolMeter::start_async_runner()
{
	if (m_async_callback)
		return;

	std::unique_lock<std::mutex> ul(m_worker_lock);

	// Start v8/uv asynchronous runner.
	m_async_callback = new osn::VolMeterCallback();
	m_async_callback->set_handler(std::bind(&VolMeter::callback_handler, this, std::placeholders::_1, std::placeholders::_2), nullptr);
}

void osn::VolMeter::stop_async_runner()
{
	if (!m_async_callback)
		return;

	std::unique_lock<std::mutex> ul(m_worker_lock);

	// Stop v8/uv asynchronous runner.
	m_async_callback->clear();
	m_async_callback->finalize();
	m_async_callback = nullptr;
}

void osn::VolMeter::callback_handler(void* data, std::shared_ptr<osn::VolMeterData> item)
{
	// utilv8::ToValue on a std::vector<> creates a v8::Local<v8::Array> automatically.
	v8::Local<v8::Value> args[] = {
	    utilv8::ToValue(item->magnitude), utilv8::ToValue(item->peak), utilv8::ToValue(item->input_peak)};

	Nan::Call(m_callback_function, 3, args);
}

void osn::VolMeter::set_keepalive(v8::Local<v8::Object> obj)
{
	if (!m_async_callback)
		return;
	m_async_callback->set_keepalive(obj);
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

	if (!ValidateResponse(rval)) {
		return;
	}

	// Return created Object
	auto* newVolmeter              = new osn::VolMeter(rval[1].value_union.ui64);
	newVolmeter->m_sleep_interval  = rval[2].value_union.ui32;
	info.GetReturnValue().Set(Store(newVolmeter));
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

	if (!ValidateResponse(rval)) {
		return;
	}

	self->m_sleep_interval = rval[1].value_union.ui32;

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
	conn->call("VolMeter", "SetUpdateInterval", {ipc::value(self->m_uid), ipc::value(interval)});
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
	conn->call("VolMeter", "Attach", {ipc::value(fader->m_uid), ipc::value(source->sourceId)});
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
	conn->call("VolMeter", "Detach", {ipc::value(fader->m_uid)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::AddCallback(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::VolMeter*               self;
	v8::Local<v8::Function>      callback;

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
			info.GetReturnValue().Set(Nan::Null());
			return;
		}
	}

	self->m_callback_function.Reset(callback);
	self->start_async_runner();

	std::unique_lock<std::mutex> lck(volmeters_lock);
	volmeters.emplace(std::make_pair(self->m_uid, self));
	info.GetReturnValue().Set(true);
}

Nan::NAN_METHOD_RETURN_TYPE osn::VolMeter::RemoveCallback(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::VolMeter*               self;

	{
		ASSERT_INFO_LENGTH(info, 1);
		if (!Retrieve(info.This(), self)) {
			return;
		}
	}

	self->stop_async_runner();
	self->m_callback_function.Reset();

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
			info.GetReturnValue().Set(Nan::Null());
			return;
		}
	}
	std::unique_lock<std::mutex> lck(volmeters_lock);
	volmeters.erase(self->m_uid);

	info.GetReturnValue().Set(true);
}

void osn::VolMeter::UpdateVolmeter(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::unique_lock<std::mutex>       lck(volmeters_lock);

	auto it = volmeters.find(args[0].value_union.ui64);
	if (it == volmeters.end())
		return;

	std::shared_ptr<osn::VolMeterData> item      = std::make_shared<osn::VolMeterData>();
	uint32_t                           indexData = 0;
	std::vector<char>                  buffer    = args[2].value_bin;

	for (size_t ch = 0; ch < args[1].value_union.i32; ch++) {
		item->magnitude.push_back(*reinterpret_cast<float*>(buffer.data() + indexData));
		indexData += sizeof(float);
		item->peak.push_back(*reinterpret_cast<float*>(buffer.data() + indexData));
		indexData += sizeof(float);
		item->input_peak.push_back(*reinterpret_cast<float*>(buffer.data() + indexData));
		indexData += sizeof(float);
	}

	it->second->m_async_callback->queue(item);
}

INITIALIZER(nodeobs_volmeter)
{
}