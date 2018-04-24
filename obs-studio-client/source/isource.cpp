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
#include "shared.hpp"
#include "utility.hpp"
#include "utility-v8.hpp"
#include "controller.hpp"
#include "properties.hpp"
#include <error.hpp>

Nan::Persistent<v8::FunctionTemplate> osn::ISource::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::ISource::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Source").ToLocalChecked());
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);

	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "release", Release);
	utilv8::SetTemplateField(objtemplate, "remove", Remove);

	utilv8::SetTemplateAccessorProperty(objtemplate, "configurable", IsConfigurable);
	utilv8::SetTemplateAccessorProperty(objtemplate, "properties", GetProperties);
	utilv8::SetTemplateAccessorProperty(objtemplate, "settings", GetSettings);
	utilv8::SetTemplateField(objtemplate, "update", Update);
	utilv8::SetTemplateField(objtemplate, "load", Load);
	utilv8::SetTemplateField(objtemplate, "save", Save);

	utilv8::SetTemplateAccessorProperty(objtemplate, "type", GetType);
	utilv8::SetTemplateAccessorProperty(objtemplate, "name", GetName, SetName);
	utilv8::SetTemplateAccessorProperty(objtemplate, "outputFlags", GetOutputFlags);
	utilv8::SetTemplateAccessorProperty(objtemplate, "flags", GetFlags, SetFlags);
	utilv8::SetTemplateAccessorProperty(objtemplate, "status", GetStatus);
	utilv8::SetTemplateAccessorProperty(objtemplate, "id", GetId);
	utilv8::SetTemplateAccessorProperty(objtemplate, "muted", GetMuted, SetMuted);
	utilv8::SetTemplateAccessorProperty(objtemplate, "enabled", GetEnabled, SetEnabled);

	utilv8::SetObjectField(target, "Source", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Release(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
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
		std::vector<ipc::value>{ipc::value(obj->sourceId)}, fnc, &rtd);
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

	obj->sourceId = UINT64_MAX;
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Remove(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
		
		v8::Local<v8::Object> obj;
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

		// Parse the massive structure of properties we were just sent.
		osn::property_map_t pmap;
		for (size_t idx = 1; idx < rval.size(); ++idx) {
			size_t elementsize = 6;

			std::shared_ptr<osn::Property> prop;
			osn::Property::Type type = (osn::Property::Type)(rval[idx + 5].value_union.i32);
			switch (type) {
				default:
				{
					prop = std::make_shared<osn::Property>();
					break;
				}
				case osn::Property::Type::INT:
				{
					elementsize = 6 + 4;
					std::shared_ptr<osn::NumberProperty> nprop = std::make_shared<osn::NumberProperty>();

					nprop->Int.min = rval[idx + 6].value_union.i32;
					nprop->Int.max = rval[idx + 7].value_union.i32;
					nprop->Int.step = rval[idx + 8].value_union.i32;
					nprop->field_type = (osn::NumberProperty::Type)rval[idx + 9].value_union.i32;

					prop = std::static_pointer_cast<osn::Property>(nprop);
					break;
				}
				case osn::Property::Type::FLOAT:
				{
					elementsize = 6 + 4;
					std::shared_ptr<osn::NumberProperty> nprop = std::make_shared<osn::NumberProperty>();

					nprop->Float.min = rval[idx + 6].value_union.fp32;
					nprop->Float.max = rval[idx + 7].value_union.fp32;
					nprop->Float.step = rval[idx + 8].value_union.fp32;
					nprop->field_type = (osn::NumberProperty::Type)rval[idx + 9].value_union.i32;

					prop = std::static_pointer_cast<osn::Property>(nprop);
					break;
				}
				case osn::Property::Type::TEXT:
				{
					elementsize = 6 + 1;
					std::shared_ptr<osn::TextProperty> nprop = std::make_shared<osn::TextProperty>();

					nprop->field_type = (osn::TextProperty::Type)rval[idx + 6].value_union.i32;

					prop = std::static_pointer_cast<osn::Property>(nprop);
					break;
				}
				case osn::Property::Type::PATH:
				{
					elementsize = 6 + 3;
					std::shared_ptr<osn::PathProperty> nprop = std::make_shared<osn::PathProperty>();

					nprop->field_type = (osn::PathProperty::Type)rval[idx + 6].value_union.i32;
					nprop->filter = rval[idx + 7].value_str;
					nprop->default_path = rval[idx + 8].value_str;

					prop = std::static_pointer_cast<osn::Property>(nprop);
					break;
				}
				case osn::Property::Type::EDITABLELIST:
				{
					elementsize = 6 + 3;
					std::shared_ptr<osn::EditableListProperty> nprop = std::make_shared<osn::EditableListProperty>();

					nprop->field_type = (osn::EditableListProperty::Type)rval[idx + 6].value_union.i32;
					nprop->filter = rval[idx + 7].value_str;
					nprop->default_path = rval[idx + 8].value_str;

					prop = std::static_pointer_cast<osn::Property>(nprop);
					break;
				}
				case osn::Property::Type::LIST:
				{
					elementsize = 6 + 3;
					std::shared_ptr<osn::ListProperty> nprop = std::make_shared<osn::ListProperty>();

					nprop->field_type = (osn::ListProperty::Type)rval[idx + 6].value_union.i32;
					nprop->item_format = (osn::ListProperty::Format)rval[idx + 7].value_union.i32;
					size_t item_cnt = rval[idx + 8].value_union.ui64;

					for (size_t idx2 = 0; idx2 < item_cnt; idx2++) {
						osn::ListProperty::Item itm;
						itm.name = rval[idx + elementsize + idx2 * 3 + 0].value_str;
						itm.disabled = !!rval[idx + elementsize + idx2 * 3 + 1].value_union.i32;
						switch (nprop->item_format) {
							case osn::ListProperty::Format::INT:
								itm.value_int = rval[idx + elementsize + idx2 * 3 + 2].value_union.i64;
								break;
							case osn::ListProperty::Format::FLOAT:
								itm.value_float = rval[idx + elementsize + idx2 * 3 + 2].value_union.fp64;
								break;
							case osn::ListProperty::Format::STRING:
								itm.value_str = rval[idx + elementsize + idx2 * 3 + 2].value_str;
								break;
						}
					}
					elementsize += item_cnt * 3;

					prop = std::static_pointer_cast<osn::Property>(nprop);
					break;
				}
				case osn::Property::Type::FRAMERATE:
				{
					elementsize = 6 + 2;
					std::shared_ptr<osn::FrameRateProperty> nprop = std::make_shared<osn::FrameRateProperty>();
					
					nprop->ranges; nprop->items;

					size_t range_cnt, item_cnt;
					range_cnt = rval[idx + 6].value_union.ui64;
					item_cnt = rval[idx + 7].value_union.ui64;
										
					nprop->ranges.resize(range_cnt);
					for (size_t idx2 = 0; idx2 < range_cnt; idx2++) {
						osn::FrameRateProperty::FrameRate min;
						osn::FrameRateProperty::FrameRate max;
						
						min.numerator = rval[idx + elementsize + idx2 * 4 + 0].value_union.ui32;
						min.denominator = rval[idx + elementsize + idx2 * 4 + 1].value_union.ui32;
						max.numerator = rval[idx + elementsize + idx2 * 4 + 2].value_union.ui32;
						max.denominator = rval[idx + elementsize + idx2 * 4 + 3].value_union.ui32;
						nprop->ranges[idx2] = std::pair<osn::FrameRateProperty::FrameRate, osn::FrameRateProperty::FrameRate>(min, max);
					}
					elementsize += range_cnt * 4;
					
					for (size_t idx2 = 0; idx2 < item_cnt; idx2++) {
						osn::FrameRateProperty::Item itm;
						itm.name = rval[idx + elementsize + idx2 * 2 + 0].value_str;
						itm.description = rval[idx + elementsize + idx2 * 2 + 1].value_str;
						nprop->items.insert_or_assign(itm.name, std::move(itm));
					}
					elementsize += item_cnt * 2;

					prop = std::static_pointer_cast<osn::Property>(nprop);
					break;
				}
			}

			if (prop) {
				prop->name = rval[idx + 0].value_str;
				prop->description = rval[idx + 1].value_str;
				prop->long_description = rval[idx + 2].value_str;
				prop->enabled = !!rval[idx + 3].value_union.i32;
				prop->visible = !!rval[idx + 4].value_union.i32;

				pmap.insert_or_assign(prop->name, prop);
			}
						
			idx += elementsize;
		}

		osn::Properties* props = new osn::Properties(std::move(pmap));
		rtd->obj = osn::Properties::Store(props);
		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetProperties",
		std::vector<ipc::value>{ipc::value(hndl->sourceId)}, fnc, &rtd);
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

	info.GetReturnValue().Set(rtd.obj);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetSettings(Nan::NAN_METHOD_ARGS_TYPE info) {
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

		std::string json = "";
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

		rtd->json = rval[1].value_str;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Source", "GetSettings",
		std::vector<ipc::value>{ipc::value(hndl->sourceId)}, fnc, &rtd);
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

	v8::Local<v8::String> jsondata = Nan::New<v8::String>(rtd.json).ToLocalChecked();
	v8::Local<v8::Value> json = v8::JSON::Parse(jsondata);
	info.GetReturnValue().Set(json);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Update(Nan::NAN_METHOD_ARGS_TYPE info) {
	v8::Local<v8::Object> json;
	ASSERT_GET_VALUE(info[0], json);
	
	// Retrieve Object
	osn::ISource* hndl = nullptr;
	if (!Retrieve(info.This(), hndl)) {
		return;
	}
	
	// Turn json into string
	v8::Local<v8::String> jsondata = v8::JSON::Stringify(info.GetIsolate()->GetCurrentContext(), json).ToLocalChecked();
	v8::String::Utf8Value jsondatautf8(jsondata);

	// Call Information
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
	bool suc = Controller::GetInstance().GetConnection()->call("Source", "Update",
		std::vector<ipc::value>{ipc::value(hndl->sourceId), ipc::value(std::string(*jsondatautf8, (size_t)jsondatautf8.length()))}, fnc, &rtd);
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

	info.GetReturnValue().Set(true);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Load(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
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
			rtd->called = true;
			rtd->cv.notify_all();
			return;
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
