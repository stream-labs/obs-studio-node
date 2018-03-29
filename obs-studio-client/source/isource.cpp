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

#include "isource.hpp"
#include "utility-v8.hpp"
#include "controller.hpp"
#include <error.hpp>

static Nan::Persistent<v8::FunctionTemplate> prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::ISource::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Source").ToLocalChecked());
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();

	utilv8::SetObjectTemplateField(objtemplate, "release", Release);
	utilv8::SetObjectTemplateField(objtemplate, "remove", Remove);

	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "configurable", IsConfigurable);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "properties", GetProperties);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "settings", GetSettings);
	utilv8::SetObjectTemplateField(objtemplate, "update", Update);
	utilv8::SetObjectTemplateField(objtemplate, "load", Load);
	utilv8::SetObjectTemplateField(objtemplate, "save", Save);

	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "type", GetType);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "name", GetName, SetName);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "outputFlags", GetOutputFlags);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "flags", GetFlags, SetFlags);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "status", GetStatus);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "id", GetId);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "muted", GetMuted, SetMuted);
	utilv8::SetObjectTemplateAccessorProperty(objtemplate, "enabled", GetEnabled, SetEnabled);

	utilv8::SetObjectField(target, "Source", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Release(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
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
		// Grab a modifiable version of the pointer we passed to the other thread,
		//  which is const to allow the compiler to make some extra guarantees
		//  while optimizing. Make sure that any time you do this, no other thread
		//  will access the same values at the same time, you have to use a fence
		//  to guard against this (like condition_variable).
		ThreadData* rtd = const_cast<ThreadData*>(static_cast<const ThreadData*>(data));

		// Check if there was an IPC error calling this function.
		if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
			// If there was an error, rval will only hold 1 element of type Null with a String value.
			rtd->error_code = ErrorCode::Error;
			rtd->error_string = rval[0].value_str;
			rtd->called = true;
			rtd->cv.notify_all();
			return;
		}

		// There was no error from the IPC itself, so continue parsing the result.
		rtd->error_code = (ErrorCode)rval[0].value_union.ui64;
		if (rtd->error_code != ErrorCode::Ok) {
			rtd->error_string = rval[1].value_str;
		} /* else {} */

		// Finally, notify the caller that everything is complete.
		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "Release",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
	if (!suc) {
		// The IPC call failed due to one or more reasons:
		// - Not Connected or Host disconnected
		// - Out Of Memory
		// - Message Too Big for Socket
		// - Buffer currently full

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

	is->sourceId = UINT64_MAX;
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Remove(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
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
		}

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "Remove",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

	is->sourceId = UINT64_MAX;
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::IsConfigurable(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		bool is_configurable = false;
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
		}

		rtd->is_configurable = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "IsConfigurable",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.is_configurable);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetProperties(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* hndl = nullptr;
	if (!utilv8::SafeUnwrap<osn::ISource>(info, hndl)) {
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;

		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		struct Property {			
			std::string name, desc, long_desc;
			bool enabled, visible;

			int32_t prop_type, type;
			union {
				struct {
					double_t min, max, step;
				} floatData;
				struct {
					int64_t min, max, step;
				} intData;			
			};
			struct {
			} textData;
		};

		bool is_configurable = false;
	} rtd;
	
	/*obs::source handle = ISource::GetHandle(info.Holder());

	obs::properties props = handle.properties();

	if (props.status() != obs::properties::status_type::okay) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	Properties *bindings = new Properties(std::move(props));
	auto object = Properties::Object::GenerateObject(bindings);

	info.GetReturnValue().Set(object);*/
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetSettings(Nan::NAN_METHOD_ARGS_TYPE info) {
	/*obs::source handle = ISource::GetHandle(info.Holder());

	obs_data_t *data = handle.settings();

	info.GetReturnValue().Set(common::ToValue(data));*/
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Update(Nan::NAN_METHOD_ARGS_TYPE info) {
	/*obs::source handle = ISource::GetHandle(info.Holder());

	obs_data_t *data;

	ASSERT_GET_VALUE(info[0], data);

	handle.update(data);
	obs_data_release(data);*/
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Load(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
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
		}

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "Load",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Save(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
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
		}

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "Save",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetType(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		int32_t result = 0;
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
		}

		rtd->result = rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetType",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetName(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		std::string result = "";
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
		}

		rtd->result = rval[1].value_str;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetName",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetName(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string name;
	ASSERT_GET_VALUE(info[0], name);

	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		std::string result = "";
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
		}

		rtd->result = rval[1].value_str;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "SetName",
		std::vector<ipc::value>{ipc::value(is->sourceId), ipc::value(name)}, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result == name));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetOutputFlags(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		uint32_t result = 0;
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
		}

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetOutputFlags",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetFlags(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		uint32_t result = 0;
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
		}

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetFlags",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetFlags(Nan::NAN_METHOD_ARGS_TYPE info) {
	uint32_t flags;
	ASSERT_GET_VALUE(info[0], flags);

	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		uint32_t result = 0;
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
		}

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "SetFlags",
		std::vector<ipc::value>{ipc::value(is->sourceId), ipc::value(flags)}, fnc, &rtd);
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

	info.GetReturnValue().Set(utilv8::ToValue(rtd.result != flags));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetStatus(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		bool result = false;
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
		}

		rtd->result = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetStatus",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetId(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		std::string result = "";
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
		}

		rtd->result = rval[1].value_str;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetId",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetMuted(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		bool result = false;
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
		}

		rtd->result = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetMuted",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetMuted(Nan::NAN_METHOD_ARGS_TYPE info) {
	bool muted;

	ASSERT_GET_VALUE(info[0], muted);

	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		bool result = false;
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
		}

		rtd->result = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "SetMuted",
		std::vector<ipc::value>{ipc::value(is->sourceId), ipc::value(muted)}, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result == muted);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetEnabled(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		bool result = false;
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
		}

		rtd->result = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetEnabled",
		std::vector<ipc::value>{ipc::value(is->sourceId)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetEnabled(Nan::NAN_METHOD_ARGS_TYPE info) {
	bool enabled;

	ASSERT_GET_VALUE(info[0], enabled);

	osn::ISource* is = Nan::ObjectWrap::Unwrap<osn::ISource>(info.Holder());
	if (is == nullptr) {
		info.GetIsolate()->ThrowException(
			v8::Exception::TypeError(Nan::New<v8::String>(
				"Called without object.").ToLocalChecked()));
		return;
	}

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		bool result = false;
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
		}

		rtd->result = !!rval[1].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "SetEnabled",
		std::vector<ipc::value>{ipc::value(is->sourceId), ipc::value(enabled)}, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.result == enabled);
	return;
}
