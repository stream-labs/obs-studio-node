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

#include "transition.hpp"
#include "shared.hpp"
#include "utility.hpp"
#include "error.hpp"
#include "controller.hpp"
#include "ipc-value.hpp"
#include <string>
#include <condition_variable>
#include <mutex>

osn::Transition::Transition(uint64_t id) {
	this->sourceId = id;
}

Nan::Persistent<v8::FunctionTemplate> osn::Transition::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::Transition::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->Inherit(Nan::New<v8::FunctionTemplate>(osn::ISource::prototype));
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("Transition").ToLocalChecked());

	// Class Template
	utilv8::SetTemplateField(fnctemplate, "types", Types);
	utilv8::SetTemplateField(fnctemplate, "create", Create);
	utilv8::SetTemplateField(fnctemplate, "createPrivate", CreatePrivate);
	utilv8::SetTemplateField(fnctemplate, "fromName", FromName);

	// Object Template
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "getActiveSource", GetActiveSource);
	utilv8::SetTemplateField(objtemplate, "start", Start);
	utilv8::SetTemplateField(objtemplate, "set", Set);
	utilv8::SetTemplateField(objtemplate, "clear", Clear);

	// Stuff
	utilv8::SetObjectField(target, "Transition", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Types(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
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

	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "Types",
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

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Create(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
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

		rtd->sourceId = rval[1].value_union.ui64;

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
	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "Create",
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
	osn::Transition* obj = new osn::Transition(rtd.sourceId);
	info.GetReturnValue().Set(osn::Transition::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::CreatePrivate(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
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

		rtd->sourceId = rval[1].value_union.ui64;

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
	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "CreatePrivate",
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
	osn::Transition* obj = new osn::Transition(rtd.sourceId);
	info.GetReturnValue().Set(osn::Transition::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::FromName(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
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

	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "FromName",
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
	osn::Transition* obj = new osn::Transition(rtd.sourceId);
	info.GetReturnValue().Set(osn::Transition::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::GetActiveSource(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = dynamic_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		// Results
		uint64_t sourceId;
		int32_t type;
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
		rtd->type = rval[2].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "GetActiveSource",
		std::vector<ipc::value>{ipc::value(obj->sourceId)}, fnc, &rtd);
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

	if (rtd.type == 0) {
		// Input
		osn::Input* obj = new osn::Input(rtd.sourceId);
		info.GetReturnValue().Set(osn::Input::Store(obj));
	} else if (rtd.type == 3) {
		// Scene
		osn::Scene* obj = new osn::Scene(rtd.sourceId);
		info.GetReturnValue().Set(osn::Scene::Store(obj));
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Clear(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = dynamic_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

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

	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "Clear",
		std::vector<ipc::value>{ipc::value(obj->sourceId)}, fnc, &rtd);
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Set(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = dynamic_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	ASSERT_INFO_LENGTH(info, 1);

	v8::Local<v8::Object> targetbaseobj;
	ASSERT_GET_VALUE(info[0], targetbaseobj);
	osn::ISource* targetobj = nullptr;
	if (!osn::ISource::Retrieve(targetbaseobj, targetobj)) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Invalid type for target source.").ToLocalChecked()));
		return;
	}

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

	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "Set",
		std::vector<ipc::value>{ipc::value(obj->sourceId), ipc::value(targetobj->sourceId)}, fnc, &rtd);
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Transition::Start(Nan::NAN_METHOD_ARGS_TYPE info) {
	shared::LogWarnTimer logwarntimer(__FUNCTION_NAME__);
	osn::ISource* baseobj = nullptr;
	if (!osn::ISource::Retrieve(info.This(), baseobj)) {
		return;
	}
	osn::Transition* obj = dynamic_cast<osn::Transition*>(baseobj);
	if (!obj) {
		// How did you even call this? o.o
		return;
	}

	// Parameters
	ASSERT_INFO_LENGTH(info, 2);

	uint32_t ms = 0;
	ASSERT_GET_VALUE(info[0], ms);

	v8::Local<v8::Object> targetbaseobj;
	ASSERT_GET_VALUE(info[1], targetbaseobj);
	osn::ISource* targetobj = nullptr;
	if (!osn::ISource::Retrieve(targetbaseobj, targetobj)) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Invalid type for target source.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

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

	bool suc = Controller::GetInstance().GetConnection()->call("Transition", "Start",
		std::vector<ipc::value>{ipc::value(obj->sourceId), ipc::value(ms), ipc::value(targetobj->sourceId)}, fnc, &rtd);
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
