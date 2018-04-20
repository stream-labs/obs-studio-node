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

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		std::vector<std::string> types;
	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {
		ThreadData* rtd = const_cast<ThreadData*>(static_cast<const ThreadData*>(data));

		if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
			rtd->error_code = ErrorCode::Error;
			rtd->error_string = rval[0].value_str;
			rtd->called = true;
			rtd->cv.notify_all();
			return;
		}

		rtd->error_code = (ErrorCode)rval[0].value_union.ui64;
		if (rtd->error_code != ErrorCode::Ok) {
			rtd->error_string = rval[1].value_str;
			rtd->called = true;
			rtd->cv.notify_all();
			return;
		}

		size_t count = rval.size() - 1; // Skip ErrorCode
		rtd->types.resize(count);
		for (size_t idx = 0; idx < count; idx++) {
			rtd->types[idx] = rval[1 + idx].value_str;
		}

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Filter", "Types",
		std::vector<ipc::value>{}, fnc, &rtd);
	if (!suc) {
		info.GetIsolate()->ThrowException(
			v8::Exception::Error(
				Nan::New<v8::String>(
					"Failed to make IPC call, verify IPC status."
					).ToLocalChecked()
			));
		return;
	}

	std::unique_lock<std::mutex> ulock(rtd.mtx);
	rtd.cv.wait(ulock, [&rtd]() { return rtd.called; });

	if (rtd.error_code != ErrorCode::Ok) {
		if (rtd.error_code == ErrorCode::InvalidReference) {
			info.GetIsolate()->ThrowException(
				v8::Exception::ReferenceError(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		} else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	info.GetReturnValue().Set(utilv8::ToValue<std::string>(rtd.types));
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

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		uint64_t sourceId;
	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {
		ThreadData* rtd = const_cast<ThreadData*>(static_cast<const ThreadData*>(data));

		if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
			rtd->error_code = ErrorCode::Error;
			rtd->error_string = rval[0].value_str;
			rtd->called = true;
			rtd->cv.notify_all();
			return;
		}

		rtd->error_code = (ErrorCode)rval[0].value_union.ui64;
		if (rtd->error_code != ErrorCode::Ok) {
			rtd->error_string = rval[1].value_str;
			rtd->called = true;
			rtd->cv.notify_all();
			return;
		}

		rtd->sourceId = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	auto params = std::vector<ipc::value>{ ipc::value(type), ipc::value(name) };
	if (settings->Length() != 0) {
		std::string value;
		if (utilv8::FromValue(settings, value)) {
			params.push_back(ipc::value(value));
		}
	}
	bool suc = Controller::GetInstance().GetConnection()->call("Filter", "Create",
		std::move(params), fnc, &rtd);
	if (!suc) {
		info.GetIsolate()->ThrowException(
			v8::Exception::Error(
				Nan::New<v8::String>(
					"Failed to make IPC call, verify IPC status."
					).ToLocalChecked()
			));
		return;
	}

	std::unique_lock<std::mutex> ulock(rtd.mtx);
	rtd.cv.wait(ulock, [&rtd]() { return rtd.called; });

	if (rtd.error_code != ErrorCode::Ok) {
		if (rtd.error_code == ErrorCode::InvalidReference) {
			info.GetIsolate()->ThrowException(
				v8::Exception::ReferenceError(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		} else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	// Create new Filter
	osn::Filter* obj = new osn::Filter(rtd.sourceId);
	info.GetReturnValue().Set(osn::Filter::Store(obj));
}
