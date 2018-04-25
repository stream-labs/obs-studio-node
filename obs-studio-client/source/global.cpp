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

#include "global.hpp"
#include "utility-v8.hpp"
#include <condition_variable>
#include <mutex>
#include "error.hpp"
#include <ipc-value.hpp>
#include "controller.hpp"
#include "input.hpp"
#include "scene.hpp"
#include "transition.hpp"

void osn::Global::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto ObsGlobal = Nan::New<v8::Object>();

	utilv8::SetObjectField(ObsGlobal, "getOutputSource", getOutputSource);
	utilv8::SetObjectField(ObsGlobal, "setOutputSource", setOutputSource);
	utilv8::SetObjectAccessorProperty(ObsGlobal, "laggedFrames", laggedFrames);
	utilv8::SetObjectAccessorProperty(ObsGlobal, "totalFrames", totalFrames);

	Nan::Set(target, FIELD_NAME("Global"), ObsGlobal);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::getOutputSource(Nan::NAN_METHOD_ARGS_TYPE info) {
	ASSERT_INFO_LENGTH(info, 1);
	uint32_t channel;
	ASSERT_GET_VALUE(info[0], channel);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";

		uint64_t uid;
		int32_t type;
	} rtd;

	auto fnc = [](const void* data, const std::vector<ipc::value>& rval) {
		ThreadData* rtd = const_cast<ThreadData*>(static_cast<const ThreadData*>(data));

		if ((rval.size() == 1) && (rval[0].type == ipc::type::Null)) {
			// If there was an error, rval will only hold 1 element of type Null with a String value.
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

		rtd->uid = rval[1].value_union.ui64;
		rtd->type = rval[2].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Global", "GetOutputSource",
		std::vector<ipc::value>{ipc::value(channel)}, fnc, &rtd);
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
		osn::Input* scene = new osn::Input(rtd.uid);
		info.GetReturnValue().Set(osn::Input::Store(scene));
	} else if (rtd.type == 2) {
		// Transition
		osn::Transition* scene = new osn::Transition(rtd.uid);
		info.GetReturnValue().Set(osn::Transition::Store(scene));
	} else if (rtd.type == 3) {
		// Scene
		osn::Scene* scene = new osn::Scene(rtd.uid);
		info.GetReturnValue().Set(osn::Scene::Store(scene));
	}
}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::setOutputSource(Nan::NAN_METHOD_ARGS_TYPE info) {
	uint32_t channel;
	v8::Local<v8::Object> source_object;
	osn::ISource* source = nullptr;

	ASSERT_INFO_LENGTH(info, 2);
	ASSERT_GET_VALUE(info[0], channel);
	if (info[1]->IsObject()) {
		ASSERT_GET_VALUE(info[1], source_object);

		if (!osn::ISource::Retrieve(source_object, source)) {
			return;
		}
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
			// If there was an error, rval will only hold 1 element of type Null with a String value.
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

	bool suc = Controller::GetInstance().GetConnection()->call("Global", "SetOutputSource",
		std::vector<ipc::value>{ipc::value(channel), ipc::value(source ? source->sourceId : UINT64_MAX)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE osn::Global::laggedFrames(Nan::NAN_METHOD_ARGS_TYPE info) {
	//OBS_VALID

	//	info.GetReturnValue().Set(common::ToValue(obs::lagged_frames()));

}

Nan::NAN_METHOD_RETURN_TYPE osn::Global::totalFrames(Nan::NAN_METHOD_ARGS_TYPE info) {
	//OBS_VALID

	//	info.GetReturnValue().Set(common::ToValue(obs::total_frames()));

}
