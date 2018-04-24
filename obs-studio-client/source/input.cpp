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

#include "input.hpp"
#include "shared.hpp"
#include "utility.hpp"
#include <string>
#include <condition_variable>
#include <mutex>
#include "error.hpp"
#include "controller.hpp"
#include "ipc-value.hpp"
#include "filter.hpp"

osn::Input::Input(uint64_t id) {
	this->sourceId = id;
}

Nan::Persistent<v8::FunctionTemplate> osn::Input::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::Input::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Input").ToLocalChecked());

	// Function Template
	utilv8::SetTemplateField(fnctemplate, "types", Types);
	utilv8::SetTemplateField(fnctemplate, "create", Create);
	utilv8::SetTemplateField(fnctemplate, "createPrivate", CreatePrivate);
	utilv8::SetTemplateField(fnctemplate, "fromName", FromName);
	utilv8::SetTemplateField(fnctemplate, "getPublicSources", GetPublicSources);

	// Prototype Template

	// Instance Template
	v8::Local<v8::Template> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "duplicate", Duplicate);
	utilv8::SetTemplateAccessorProperty(objtemplate, "active", Active);
	utilv8::SetTemplateAccessorProperty(objtemplate, "showing", Showing);
	utilv8::SetTemplateAccessorProperty(objtemplate, "width", Width);
	utilv8::SetTemplateAccessorProperty(objtemplate, "height", Height);
	utilv8::SetTemplateAccessorProperty(objtemplate, "volume", GetVolume, SetVolume);
	utilv8::SetTemplateAccessorProperty(objtemplate, "syncOffset", GetSyncOffset, SetSyncOffset);
	utilv8::SetTemplateAccessorProperty(objtemplate, "audioMixers", GetAudioMixers, SetAudioMixers);
	utilv8::SetTemplateAccessorProperty(objtemplate, "monitoringType", GetMonitoringType, SetMonitoringType);
	utilv8::SetTemplateAccessorProperty(objtemplate, "deinterlaceFieldOrder", GetDeinterlaceFieldOrder, SetDeinterlaceFieldOrder);
	utilv8::SetTemplateAccessorProperty(objtemplate, "deinterlaceMode", GetDeinterlaceMode, SetDeinterlaceMode);

	utilv8::SetTemplateAccessorProperty(objtemplate, "filters", Filters);
	utilv8::SetTemplateField(objtemplate, "addFilter", AddFilter);
	utilv8::SetTemplateField(objtemplate, "removeFilter", RemoveFilter);
	utilv8::SetTemplateField(objtemplate, "findFilter", FindFilter);
	utilv8::SetTemplateField(objtemplate, "copyFilters", CopyFilters);

	// Stuff
	utilv8::SetObjectField(target, "Input", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Types(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "Types",
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

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Create(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string type;
	std::string name;
	v8::Local<v8::String> settings = Nan::New<v8::String>("").ToLocalChecked();
	v8::Local<v8::String> hotkeys = Nan::New<v8::String>("").ToLocalChecked();

	// Parameters: <string> Type, <string> Name[,<object> settings]
	ASSERT_INFO_LENGTH_AT_LEAST(info, 2);

	ASSERT_GET_VALUE(info[0], type);
	ASSERT_GET_VALUE(info[1], name);

	// Check if caller provided settings to send across.
	if (info.Length() >= 4) {
		ASSERT_INFO_LENGTH(info, 4);

		v8::Local<v8::Object> hksobj;
		ASSERT_GET_VALUE(info[2], hksobj);

		hotkeys = v8::JSON::Stringify(info.GetIsolate()->GetCurrentContext(), hksobj).ToLocalChecked();
	}
	if (info.Length() >= 3) {
		ASSERT_INFO_LENGTH_AT_LEAST(info, 3);

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
	if (hotkeys->Length() != 0) {
		std::string value;
		if (utilv8::FromValue(hotkeys, value)) {
			params.push_back(ipc::value(value));
		}
	}
	bool suc = Controller::GetInstance().GetConnection()->call("Input", "Create",
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
	osn::Input* obj = new osn::Input(rtd.sourceId);
	info.GetReturnValue().Set(osn::Input::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::CreatePrivate(Nan::NAN_METHOD_ARGS_TYPE info) {
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
	bool suc = Controller::GetInstance().GetConnection()->call("Input", "CreatePrivate",
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
	osn::Input* obj = new osn::Input(rtd.sourceId);
	info.GetReturnValue().Set(osn::Input::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::FromName(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string name;

	// Parameters: <string> Name
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], name);

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

		rtd->sourceId = rval[1].value_union.ui64;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "FromName",
	{ ipc::value(name) }, fnc, &rtd);
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
	osn::Input* obj = new osn::Input(rtd.sourceId);
	info.GetReturnValue().Set(osn::Input::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetPublicSources(Nan::NAN_METHOD_ARGS_TYPE info) {
	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		std::vector<uint64_t> ids;
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

		size_t cnt = rval.size() - 1;
		rtd->ids.resize(cnt);
		if (cnt > 0) {
			for (size_t idx = 0; idx < cnt; idx++) {
				rtd->ids.push_back(rval[idx + 1].value_union.ui64);
			}
		}

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetPublicSources",
	{}, fnc, &rtd);
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

	v8::Local<v8::Array> arr = Nan::New<v8::Array>(rtd.ids.size());
	for (size_t idx = 0; idx < rtd.ids.size(); idx++) {
		osn::Input* obj = new osn::Input(rtd.ids[idx]);
		auto object = osn::Input::Store(obj);
		Nan::Set(arr, idx, object);
	}

	info.GetReturnValue().Set(arr);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Duplicate(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	//parameters
	std::string name = "";
	bool is_private = false;
	ASSERT_INFO_LENGTH_AT_LEAST(info, 0);
	if (info.Length() >= 1) {
		ASSERT_GET_VALUE(info[0], name);
	}
	if (info.Length() >= 2) {
		ASSERT_INFO_LENGTH(info, 2);
		ASSERT_GET_VALUE(info[1], is_private);
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

		rtd->sourceId = rval[1].value_union.ui64;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	auto params = std::vector<ipc::value>{ ipc::value(obj->sourceId) };
	if (info.Length() >= 1) {
		params.push_back(ipc::value(name));
	}
	if (info.Length() >= 2) {
		params.push_back(ipc::value(is_private));
	}
	bool suc = Controller::GetInstance().GetConnection()->call("Input", "Duplicate",
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

	osn::Input* nobj = new osn::Input(rtd.sourceId);
	info.GetReturnValue().Set(osn::Input::Store(nobj));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Active(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		bool result;
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

		rtd->result = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetActive",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Showing(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		bool result;
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

		rtd->result = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetShowing",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Width(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		uint32_t result;
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

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetWidth",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Height(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		uint32_t result;
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

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetHeight",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetVolume(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		float_t result;
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

		rtd->result = rval[1].value_union.fp32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetVolume",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetVolume(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	float_t volume = 0.0f;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], volume);

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		float_t result;
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

		rtd->result = rval[1].value_union.fp32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "SetVolume",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(volume) }, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetSyncOffset(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int64_t result;
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

		rtd->result = rval[1].value_union.i64;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetSyncOffset",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetSyncOffset(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int64_t syncoffset = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], syncoffset);

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int64_t result;
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

		rtd->result = rval[1].value_union.i64;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "SetSyncOffset",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(syncoffset) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetAudioMixers(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		uint32_t result;
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

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetAudioMixers",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetAudioMixers(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	uint32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		uint32_t result;
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

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "SetAudioMixers",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(audiomixers) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetMonitoringType(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int32_t result;
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

		rtd->result = rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetMonitoringType",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetMonitoringType(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int32_t result;
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

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "SetMonitoringType",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(audiomixers) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetDeinterlaceFieldOrder(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int32_t result;
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

		rtd->result = rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetDeInterlaceFieldOrder",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetDeinterlaceFieldOrder(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int32_t result;
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

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "SetDeInterlaceFieldOrder",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(audiomixers) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::GetDeinterlaceMode(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int32_t result;
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

		rtd->result = rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetDeInterlaceMode",
		std::vector<ipc::value>{ ipc::value(obj->sourceId) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::SetDeinterlaceMode(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	int32_t audiomixers = 0;
	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], audiomixers);

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		int32_t result;
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

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "SetDeInterlaceMode",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(audiomixers) }, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::Filters(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;

		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		std::vector<uint64_t> ids;
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

		size_t cnt = rval.size() - 1;
		rtd->ids.resize(cnt);
		if (cnt > 0) {
			for (size_t idx = 0; idx < cnt; idx++) {
				rtd->ids.push_back(rval[idx + 1].value_union.ui64);
			}
		}

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "GetFilters",
	{ obj->sourceId }, fnc, &rtd);
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

	v8::Local<v8::Array> arr = Nan::New<v8::Array>(rtd.ids.size());
	for (size_t idx = 0; idx < rtd.ids.size(); idx++) {
		osn::Filter* obj = new osn::Filter(rtd.ids[idx]);
		auto object = osn::Filter::Store(obj);
		Nan::Set(arr, idx, object);
	}

	info.GetReturnValue().Set(arr);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::AddFilter(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> objfilter;
	ASSERT_GET_VALUE(info[0], objfilter);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		info.GetIsolate()->ThrowException(
			v8::Exception::ReferenceError(Nan::New<v8::String>(
				"Source is invalid.").ToLocalChecked()));
	}
	osn::Filter* filter = dynamic_cast<osn::Filter*>(basefilter);
	if (!filter) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Source is not a filter.").ToLocalChecked()));
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
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

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "AddFilter",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(filter->sourceId) }, fnc, &rtd);
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
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::RemoveFilter(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> objfilter;
	ASSERT_GET_VALUE(info[0], objfilter);

	osn::ISource* basefilter = nullptr;
	if (!osn::ISource::Retrieve(objfilter, basefilter)) {
		info.GetIsolate()->ThrowException(
			v8::Exception::ReferenceError(Nan::New<v8::String>(
				"Source is invalid.").ToLocalChecked()));
	}
	osn::Filter* filter = dynamic_cast<osn::Filter*>(basefilter);
	if (!filter) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Source is not a filter.").ToLocalChecked()));
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
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

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "RemoveFilter",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(filter->sourceId) }, fnc, &rtd);
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
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::FindFilter(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	std::string name;
	ASSERT_GET_VALUE(info[0], name);

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
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

		rtd->sourceId = rval[1].value_union.ui64;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "FindFilter",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(name) }, fnc, &rtd);
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
	osn::Input* nobj = new osn::Input(rtd.sourceId);
	info.GetReturnValue().Set(osn::Input::Store(nobj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Input::CopyFilters(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Input* obj = dynamic_cast<osn::Input*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> objinput;
	ASSERT_GET_VALUE(info[0], objinput);

	osn::ISource* baseinput = nullptr;
	if (!osn::ISource::Retrieve(objinput, baseinput)) {
		info.GetIsolate()->ThrowException(
			v8::Exception::ReferenceError(Nan::New<v8::String>(
				"Source is invalid.").ToLocalChecked()));
	}
	osn::Input* input = dynamic_cast<osn::Input*>(baseinput);
	if (!input) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Source is not a input.").ToLocalChecked()));
		return;
	}

	// Perform the call
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
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

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Input", "CopyFiltersTo",
		std::vector<ipc::value>{ ipc::value(obj->sourceId), ipc::value(input->sourceId) }, fnc, &rtd);
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
}
