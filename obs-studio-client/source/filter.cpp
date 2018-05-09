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

#include "filter.hpp"
#include "shared.hpp"
#include "utility.hpp"
#include <string>
#include <condition_variable>
#include <mutex>
#include "error.hpp"
#include "controller.hpp"
#include "ipc-value.hpp"

osn::Filter::Filter(uint64_t id) {
	this->sourceId = id;
}

Nan::Persistent<v8::FunctionTemplate> osn::Filter::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::Filter::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Filter").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "types", Types);
	utilv8::SetTemplateField(fnctemplate, "create", Create);

	// Stuff
	utilv8::SetObjectField(target, "Filter", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Filter::Types(Nan::NAN_METHOD_ARGS_TYPE info) {
	// Function takes no parameters.
	ASSERT_INFO_LENGTH(info, 0);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response = 
		conn->call_synchronous_helper("Filter", "Types", {});

	if (!ValidateResponse(response)) return;
	
	std::vector<std::string> types;
	size_t count = response.size() - 1;

	for (size_t idx = 0; idx < count; idx++) {
		types.push_back(response[1 + idx].value_str);
	}

	info.GetReturnValue().Set(utilv8::ToValue<std::string>(types));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Filter::Create(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string type;
	std::string name;
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
	if (!conn) return;

	auto params = std::vector<ipc::value>{ ipc::value(type), ipc::value(name) };
	if (settings->Length() != 0) {
		std::string value;
		if (utilv8::FromValue(settings, value)) {
			params.push_back(ipc::value(value));
		}
	}

	std::vector<ipc::value> response = conn->call_synchronous_helper("Filter", "Create",
	{ std::move(params) });

	if (!ValidateResponse(response)) return;

	// Create new Filter
	osn::Filter* obj = new osn::Filter(response[1].value_union.ui64);
	info.GetReturnValue().Set(osn::Filter::Store(obj));
}
