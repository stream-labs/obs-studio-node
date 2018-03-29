#include "nodeobs_display.hpp"
#include "controller.hpp"
#include "utility-v8.hpp"
#include "error.hpp"

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_createDisplay(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string windowHandle, key;

	ASSERT_GET_VALUE(info[0], windowHandle);
	ASSERT_GET_VALUE(info[1], key);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_createDisplay",
		std::vector<ipc::value>{ipc::value(windowHandle), ipc::value(key)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_destroyDisplay(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;

	ASSERT_GET_VALUE(info[0], key);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_destroyDisplay",
		std::vector<ipc::value>{ipc::value(key)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_getDisplayPreviewOffset(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;

	ASSERT_GET_VALUE(info[0], key);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		std::pair<int32_t, int32_t> result;
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

		rtd->result.first = rval[1].value_union.i32;
		rtd->result.second = rval[2].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_getDisplayPreviewOffset",
		std::vector<ipc::value>{ipc::value(key)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	v8::Local<v8::Object> previewOffset = v8::Object::New(info.GetIsolate());

	utilv8::SetObjectField(previewOffset, "x", rtd.result.first);
	utilv8::SetObjectField(previewOffset, "y", rtd.result.second);

	info.GetReturnValue().Set(previewOffset);

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_getDisplayPreviewSize(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;

	ASSERT_GET_VALUE(info[0], key);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		std::pair<int32_t, int32_t> result;
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

		rtd->result.first = rval[1].value_union.i32;
		rtd->result.second = rval[2].value_union.i32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_getDisplayPreviewSize",
		std::vector<ipc::value>{ipc::value(key)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	v8::Local<v8::Object> previewOffset = v8::Object::New(info.GetIsolate());

	utilv8::SetObjectField(previewOffset, "x", rtd.result.first);
	utilv8::SetObjectField(previewOffset, "y", rtd.result.second);

	info.GetReturnValue().Set(previewOffset);

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_createSourcePreviewDisplay(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string windowHandle, key;

	ASSERT_GET_VALUE(info[0], windowHandle);
	ASSERT_GET_VALUE(info[1], key); 
	
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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_createSourcePreviewDisplay",
		std::vector<ipc::value>{ipc::value(windowHandle), ipc::value(key)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_resizeDisplay(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t width, height;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], width);
	ASSERT_GET_VALUE(info[2], height);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_resizeDisplay",
		std::vector<ipc::value>{ipc::value(key), ipc::value(width), ipc::value(height)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_moveDisplay(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t x, y;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], x);
	ASSERT_GET_VALUE(info[2], y);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_moveDisplay",
		std::vector<ipc::value>{ipc::value(key), ipc::value(x), ipc::value(y)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setPaddingSize(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t paddingSize;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], paddingSize);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setPaddingSize",
		std::vector<ipc::value>{ipc::value(key), ipc::value(paddingSize)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setPaddingColor(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t r,g,b,a;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], r);
	ASSERT_GET_VALUE(info[1], g);
	ASSERT_GET_VALUE(info[1], b);
	ASSERT_GET_VALUE(info[1], a);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setPaddingColor",
		std::vector<ipc::value>{ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setOutlineColor(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t r, g, b, a;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], r);
	ASSERT_GET_VALUE(info[1], g);
	ASSERT_GET_VALUE(info[1], b);
	ASSERT_GET_VALUE(info[1], a);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setOutlineColor",
		std::vector<ipc::value>{ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setGuidelineColor(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t r, g, b, a;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], r);
	ASSERT_GET_VALUE(info[1], g);
	ASSERT_GET_VALUE(info[1], b);
	ASSERT_GET_VALUE(info[1], a);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setGuidelineColor",
		std::vector<ipc::value>{ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setResizeBoxInnerColor(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t r, g, b, a;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], r);
	ASSERT_GET_VALUE(info[1], g);
	ASSERT_GET_VALUE(info[1], b);
	ASSERT_GET_VALUE(info[1], a);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setResizeBoxInnerColor",
		std::vector<ipc::value>{ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setResizeBoxOuterColor(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	uint32_t r, g, b, a;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], r);
	ASSERT_GET_VALUE(info[1], g);
	ASSERT_GET_VALUE(info[1], b);
	ASSERT_GET_VALUE(info[1], a);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setResizeBoxOuterColor",
		std::vector<ipc::value>{ipc::value(key), ipc::value(r), ipc::value(g), ipc::value(b), ipc::value(a)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setShouldDrawUI(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	bool drawUI;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], drawUI);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setShouldDrawUI",
		std::vector<ipc::value>{ipc::value(key), ipc::value(drawUI)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_selectSource(Nan::NAN_METHOD_ARGS_TYPE info) {
	uint32_t x, y;

	ASSERT_GET_VALUE(info[0], x);
	ASSERT_GET_VALUE(info[1], y);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_selectSource",
		std::vector<ipc::value>{ipc::value(x), ipc::value(y)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_selectSources(Nan::NAN_METHOD_ARGS_TYPE info) {
	/*uint32_t size = 0;
	std::vector<std::string> sources;

	ASSERT_GET_VALUE(info[0], size);

	for (int i = 0; i < size; i++) {
		sources.push_back(0);
		ASSERT_GET_VALUE(info[i+1], sources.at(i));
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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_selectSources",
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;*/
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_dragSelectedSource(Nan::NAN_METHOD_ARGS_TYPE info) {
	uint32_t x, y;

	ASSERT_GET_VALUE(info[0], x);
	ASSERT_GET_VALUE(info[1], y);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_dragSelectedSource",
		std::vector<ipc::value>{ipc::value(x), ipc::value(y)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_getDrawGuideLines(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;

	ASSERT_GET_VALUE(info[0], key);

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
		}

		rtd->result = rval[1].value_union.ui32;

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_getDrawGuideLines",
		std::vector<ipc::value>{ipc::value(key)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}
	
	info.GetReturnValue().Set(Nan::New<v8::Boolean>(rtd.result));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE display::OBS_content_setDrawGuideLines(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string key;
	bool drawGuideLines;

	ASSERT_GET_VALUE(info[0], key);
	ASSERT_GET_VALUE(info[1], drawGuideLines);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Display", "OBS_content_setDrawGuideLines",
		std::vector<ipc::value>{ipc::value(key), ipc::value(drawGuideLines)}, fnc, &rtd);
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
		}
		else {
			info.GetIsolate()->ThrowException(
				v8::Exception::Error(Nan::New<v8::String>(
					rtd.error_string).ToLocalChecked()));
		}
		return;
	}

	return;
}



