#include "nodeobs_settings.hpp"
#include "controller.hpp"
#include "utility-v8.hpp"
#include "error.hpp"

std::vector<settings::SubCategory> serializeCategory(
	uint32_t subCategoriesCount, uint32_t sizeStruct, std::vector<char> buffer) {
	std::vector<settings::SubCategory> category;

	for (int i = 0; i < subCategoriesCount; i++) {
		settings::SubCategory sc;
		uint32_t indexData = 0;

		uint32_t *sizeName =
			reinterpret_cast<uint32_t*>(buffer.data());
		indexData += sizeof(uint32_t);

		std::string *name =
			reinterpret_cast<std::string*>(buffer.data() + indexData);
		indexData += *sizeName;

		uint32_t *paramsCount =
			reinterpret_cast<uint32_t*>(buffer.data() + indexData);
		indexData += sizeof(uint32_t);

		uint32_t *paramsSize =
			reinterpret_cast<uint32_t*>(buffer.data() + indexData);
		indexData += sizeof(uint32_t);

		settings::Parameter param;
		for (uint32_t j = 0; j < *paramsCount; j++) {
			uint32_t *sizeName =
				reinterpret_cast<std::uint32_t*>(buffer.data() + indexData);
			indexData += sizeof(uint32_t);

			std::string *name =
				reinterpret_cast<std::string*>(buffer.data() + indexData);
			indexData += *sizeName;

			uint32_t *sizeDescription =
				reinterpret_cast<std::uint32_t*>(buffer.data() + indexData);
			indexData += sizeof(uint32_t);

			std::string *description =
				reinterpret_cast<std::string*>(buffer.data() + indexData);
			indexData += *sizeDescription;

			uint32_t *sizeType =
				reinterpret_cast<std::uint32_t*>(buffer.data() + indexData);
			indexData += sizeof(uint32_t);

			std::string *type =
				reinterpret_cast<std::string*>(buffer.data() + indexData);
			indexData += *sizeType;

			uint32_t *sizeSubType =
				reinterpret_cast<std::uint32_t*>(buffer.data() + indexData);
			indexData += sizeof(uint32_t);

			std::string *subType =
				reinterpret_cast<std::string*>(buffer.data() + indexData);
			indexData += *sizeSubType;

			bool *enabled =
				reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			bool *masked =
				reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			bool *visible =
				reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			uint32_t *sizeOfCurrentValue =
				reinterpret_cast<std::uint32_t*>(buffer.data() + indexData);
			indexData += sizeof(uint32_t);

			void *currentValue = malloc(*sizeOfCurrentValue);
			memcpy(currentValue, buffer.data() + indexData,
				*sizeOfCurrentValue);
			indexData += *sizeOfCurrentValue;

			uint32_t *sizeOfValues =
				reinterpret_cast<std::uint32_t*>(buffer.data() + indexData);
			indexData += sizeof(uint32_t);

			void *values = malloc(*sizeOfValues);
			memcpy(values, buffer.data() + indexData,
				*sizeOfValues);
			indexData += *sizeOfValues;

			param.name = *name;
			param.description = *description;
			param.type = *type;
			param.subType = *subType;
			param.enabled = enabled;
			param.masked = masked;
			param.visible = visible;
			param.currentValue = currentValue;
			param.values = values;

			sc.params.push_back(param);
		}
		sc.name = *name;
		category.push_back(sc);
	}
	return category;
}

Nan::NAN_METHOD_RETURN_TYPE settings::OBS_settings_getSettings(Nan::NAN_METHOD_ARGS_TYPE info) {
	std::string category;
	ASSERT_GET_VALUE(info[0], category);

	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		uint64_t subCategoriesCount = 0;
		uint64_t sizeStruct = 0;
		std::vector<char> *result;
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

		rtd->subCategoriesCount = rval[1].value_union.ui64;
		rtd->sizeStruct = rval[2].value_union.ui64;
		rtd->result = new std::vector<char>(rval[3].value_bin.data(),
			rval[2].value_bin.data() + rtd->sizeStruct);
		
		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Settings", "OBS_settings_getSettings",
		std::vector<ipc::value>{ipc::value(category)}, fnc, &rtd);
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

	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Array> rval = v8::Array::New(isolate);

	std::vector<settings::SubCategory> categorySettings = 
		serializeCategory(rtd.subCategoriesCount, rtd.sizeStruct, *rtd.result);

	for (int i = 0; i < categorySettings.size(); i++) {
		v8::Local<v8::Object> subCategory = v8::Object::New(isolate);
		v8::Local<v8::Array> subCategoryParameters = v8::Array::New(isolate);

		std::vector<settings::Parameter> params =
			categorySettings.at(i).params;

		for (int j = 0; j < params.size(); j++) {
			subCategory->Set(v8::String::NewFromUtf8(isolate, "name"),
				v8::String::NewFromUtf8(isolate, params.at(i).name.c_str()));

			subCategory->Set(v8::String::NewFromUtf8(isolate, "type"),
				v8::String::NewFromUtf8(isolate, params.at(i).type.c_str()));

			subCategory->Set(v8::String::NewFromUtf8(isolate, "description"),
				v8::String::NewFromUtf8(isolate, params.at(i).description.c_str()));

			subCategory->Set(v8::String::NewFromUtf8(isolate, "subType"),
				v8::String::NewFromUtf8(isolate, params.at(i).subType.c_str()));

			// Current value

			// Values

			subCategory->Set(v8::String::NewFromUtf8(isolate, "visible"), 
				v8::Boolean::New(isolate, params.at(i).visible));

			subCategory->Set(v8::String::NewFromUtf8(isolate, "enabled"), 
				v8::Boolean::New(isolate, params.at(i).enabled));

			subCategory->Set(v8::String::NewFromUtf8(isolate, "masked"), 
				v8::Boolean::New(isolate, params.at(i).masked));
		}

		subCategory->Set(v8::String::NewFromUtf8(isolate, "nameSubCategory"), 
			v8::String::NewFromUtf8(isolate, categorySettings.at(i).name.c_str()));

		subCategory->Set(v8::String::NewFromUtf8(isolate, "parameters"), 
			subCategoryParameters);
	}

	info.GetReturnValue().Set(rval);

	return;
}

Nan::NAN_METHOD_RETURN_TYPE settings::OBS_settings_saveSettings(Nan::NAN_METHOD_ARGS_TYPE info) {
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

	bool suc = Controller::GetInstance().GetConnection()->call("Settings", "OBS_settings_saveSettings",
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

Nan::NAN_METHOD_RETURN_TYPE settings::OBS_settings_getListCategories(Nan::NAN_METHOD_ARGS_TYPE info) {
	struct ThreadData {
		std::condition_variable cv;
		std::mutex mtx;
		bool called = false;
		ErrorCode error_code = ErrorCode::Ok;
		std::string error_string = "";
		uint32_t size = 0;
		std::vector<std::string> listCategories;
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

		rtd->size = rval[1].value_union.ui32;

		for (int i = 0; i < rtd->size; i++) {
			rtd->listCategories.push_back(rval[i+2].value_str);
		}

		rtd->called = true;
		rtd->cv.notify_all();
	};

	bool suc = Controller::GetInstance().GetConnection()->call("Settings", "OBS_settings_getListCategories",
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

	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Array> categories = v8::Array::New(isolate);

	for (int i = 0; i < rtd.listCategories.size(); i++) {
		categories->Set(i, v8::String::NewFromUtf8(isolate, rtd.listCategories.at(i).c_str()));
	}	

	info.GetReturnValue().Set(categories);

	return;
}