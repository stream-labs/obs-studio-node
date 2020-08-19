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

#include "filter.hpp"
#include <condition_variable>
#include <mutex>
#include <string>
#include "controller.hpp"
#include "error.hpp"
#include "ipc-value.hpp"
#include "shared.hpp"
#include "utility.hpp"

Nan::Persistent<v8::FunctionTemplate> osn::Filter::prototype;

osn::Filter::Filter(uint64_t id)
{
	this->sourceId = id;
}

void osn::Filter::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Filter").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "types", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Types));
	utilv8::SetTemplateField(fnctemplate, "create", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Create));

	// Stuff
	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "Filter").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::Filter::Types(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	// Function takes no parameters.
	ASSERT_INFO_LENGTH(args, 0);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Filter", "Types", {});

	if (!ValidateResponse(response))
		return;

	std::vector<std::string> types;
	size_t                   count = response.size() - 1;

	for (size_t idx = 0; idx < count; idx++) {
		types.push_back(response[1 + idx].value_str);
	}

	args.GetReturnValue().Set(utilv8::ToValue<std::string>(types));
	return;
}

void osn::Filter::Create(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string           type;
	std::string           name;
	v8::Local<v8::String> settings = Nan::New<v8::String>("").ToLocalChecked();

	// Parameters: <string> Type, <string> Name[,<object> settings]
	ASSERT_INFO_LENGTH_AT_LEAST(args, 2);

	ASSERT_GET_VALUE(args[0], type);
	ASSERT_GET_VALUE(args[1], name);

	// Check if caller provided settings to send across.
	if (args.Length() >= 3) {
		ASSERT_INFO_LENGTH(args, 3);

		v8::Local<v8::Object> setobj;
		ASSERT_GET_VALUE(args[2], setobj);

		settings = v8::JSON::Stringify(args.GetIsolate()->GetCurrentContext(), setobj).ToLocalChecked();
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

	std::vector<ipc::value> response = conn->call_synchronous_helper("Filter", "Create", {std::move(params)});

	if (!ValidateResponse(response))
		return;

	// Create new Filter
	osn::Filter* obj = new osn::Filter(response[1].value_union.ui64);

	SourceDataInfo* sdi = new SourceDataInfo;
	sdi->name           = name;
	sdi->obs_sourceId   = type;
	sdi->id             = response[1].value_union.ui64;

	CacheManager<SourceDataInfo*>::getInstance().Store(response[1].value_union.ui64, name, sdi);

	args.GetReturnValue().Set(osn::Filter::Store(obj));
}
