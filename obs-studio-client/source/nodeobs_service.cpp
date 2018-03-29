#include "nodeobs_service.hpp"
#include "controller.hpp"
#include "utility-v8.hpp"
#include "error.hpp"

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_resetAudioContext(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_resetAudioContext",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_resetVideoContext(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_resetVideoContext",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_createAudioEncoder(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_createAudioEncoder",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_createVideoStreamingEncoder(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_createVideoStreamingEncoder",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_createVideoRecordingEncoder(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_createVideoRecordingEncoder",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_createService(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_createService",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_createRecordingSettings(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_createRecordingSettings",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_createStreamingOutput(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_createStreamingOutput",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_createRecordingOutput(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_createRecordingOutput",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_startStreaming(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_startStreaming",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_startRecording(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_startRecording",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_stopStreaming(Nan::NAN_METHOD_ARGS_TYPE info) {
	bool forceStop;
	ASSERT_GET_VALUE(info[0], forceStop);

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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_stopStreaming",
		std::vector<ipc::value>{ipc::value(forceStop)}, fnc, &rtd);
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

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_stopRecording(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_stopRecording",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_associateAudioAndVideoToTheCurrentStreamingContext(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_associateAudioAndVideoToTheCurrentStreamingContext",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_associateAudioAndVideoToTheCurrentRecordingContext(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_associateAudioAndVideoToTheCurrentRecordingContext",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_associateAudioAndVideoEncodersToTheCurrentStreamingOutput",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_associateAudioAndVideoEncodersToTheCurrentRecordingOutput",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_setServiceToTheStreamingOutput(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_setServiceToTheStreamingOutput",
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

	return;
}

Nan::NAN_METHOD_RETURN_TYPE service::OBS_service_setRecordingSettings(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Service", "OBS_service_setRecordingSettings",
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

	return;
}

/*static void OBS_service_connectOutputSignals(const v8::FunctionCallbackInfo<v8::Value>& args) {

}*/
