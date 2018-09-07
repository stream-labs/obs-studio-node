#include "nodeobs_settings.hpp"
#include "controller.hpp"
#include "utility-v8.hpp"
#include "error.hpp"

#include "shared.hpp"
#include "utility.hpp"
#include <string>
#include <sstream>
#include <node.h>

std::vector<settings::SubCategory> serializeCategory(
	uint32_t subCategoriesCount, uint32_t sizeStruct, std::vector<char> buffer) {
	std::vector<settings::SubCategory> category;

	uint32_t indexData = 0;
	for (int i = 0; i < subCategoriesCount; i++) {
		settings::SubCategory sc;

		size_t *sizeMessage = reinterpret_cast<size_t*>
			(buffer.data() + indexData);
		indexData += sizeof(size_t);

		std::string name(buffer.data() + indexData, *sizeMessage);
		indexData += *sizeMessage;

		size_t *paramsCount = reinterpret_cast<size_t*>
			(buffer.data() + indexData);
		indexData += sizeof(size_t);

		settings::Parameter param;
		for (size_t j = 0; j < *paramsCount; j++) {
			size_t *sizeName =
				reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string name(buffer.data() + indexData, *sizeName);
			indexData += *sizeName;

			size_t *sizeDescription =
				reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string description(buffer.data() + indexData, *sizeDescription);
			indexData += *sizeDescription;

			size_t *sizeType =
				reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string type(buffer.data() + indexData, *sizeType);
			indexData += *sizeType;

			size_t *sizeSubType =
				reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string subType(buffer.data() + indexData, *sizeSubType);
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

			size_t *sizeOfCurrentValue =
				reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::vector<char> currentValue;
			currentValue.resize(*sizeOfCurrentValue);
			memcpy(currentValue.data(), buffer.data() + indexData,
				*sizeOfCurrentValue);
			indexData += *sizeOfCurrentValue;

			size_t *sizeOfValues =
				reinterpret_cast<size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			size_t *countValues =
				reinterpret_cast<size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::vector<char> values;
			values.resize(*sizeOfValues);
			memcpy(values.data(), buffer.data() + indexData,
				*sizeOfValues);
			indexData += *sizeOfValues;

			param.name = name;
			param.description = description;
			param.type = type;
			param.subType = subType;
			param.enabled = *enabled;
			param.masked = *masked;
			param.visible = *visible;
			param.currentValue = currentValue;
			param.values = values;
			param.countValues = *countValues;

			sc.params.push_back(param);
		}
		sc.name = name;
		sc.paramsCount = *paramsCount;
		category.push_back(sc);
	}
	return category;
}

void settings::OBS_settings_getSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::string category;
	ASSERT_GET_VALUE(args[0], category);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Settings", 
			"OBS_settings_getSettings", { ipc::value(category) });

	if (!ValidateResponse(response)) return;

	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Array> rval = v8::Array::New(isolate);

	std::vector<settings::SubCategory> categorySettings = 
		serializeCategory(response[1].value_union.ui64, 
						response[2].value_union.ui64, 
						response[3].value_bin);

	for (int i = 0; i < categorySettings.size(); i++) {
		v8::Local<v8::Object> subCategory = v8::Object::New(isolate);
		v8::Local<v8::Array> subCategoryParameters = v8::Array::New(isolate);

		std::vector<settings::Parameter> params =
			categorySettings.at(i).params;

		for (int j = 0; j < params.size(); j++) {
			v8::Local<v8::Object> parameter = v8::Object::New(isolate);

			parameter->Set(v8::String::NewFromUtf8(isolate, "name"),
				v8::String::NewFromUtf8(isolate, params.at(j).name.c_str()));

			parameter->Set(v8::String::NewFromUtf8(isolate, "type"),
				v8::String::NewFromUtf8(isolate, params.at(j).type.c_str()));

			parameter->Set(v8::String::NewFromUtf8(isolate, "description"),
				v8::String::NewFromUtf8(isolate, params.at(j).description.c_str()));

			parameter->Set(v8::String::NewFromUtf8(isolate, "subType"),
				v8::String::NewFromUtf8(isolate, params.at(j).subType.c_str()));

			// Current value
			if (params.at(j).currentValue.size() > 0) {
				if (params.at(j).type.compare("OBS_PROPERTY_LIST") == 0 ||
					params.at(j).type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 ||
					params.at(j).type.compare("OBS_PROPERTY_PATH") == 0 ||
					params.at(j).type.compare("OBS_PROPERTY_TEXT") == 0 ||
					params.at(j).type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {

					std::string value(params.at(j).currentValue.begin(), 
						params.at(j).currentValue.end());

					parameter->Set(v8::String::NewFromUtf8(isolate, "currentValue"),
						v8::String::NewFromUtf8(isolate, value.c_str()));
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_INT") == 0) {
					int64_t *value = reinterpret_cast<int64_t*>(params.at(j).currentValue.data());
					parameter->Set(v8::String::NewFromUtf8(isolate, "currentValue"),
						v8::Integer::New(isolate, *value));
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_UINT") == 0) {
					uint64_t *value = reinterpret_cast<uint64_t*>(params.at(j).currentValue.data());
					parameter->Set(v8::String::NewFromUtf8(isolate, "currentValue"),
						v8::Integer::New(isolate, *value));
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_BOOL") == 0) {
					bool *value = reinterpret_cast<bool*>(params.at(j).currentValue.data());
					parameter->Set(v8::String::NewFromUtf8(isolate, "currentValue"),
						v8::Boolean::New(isolate, (*value)));
				}
				else if (params.at(j).type.compare("OBS_PROPERTY_DOUBLE") == 0) {
					double *value = reinterpret_cast<double*>(params.at(j).currentValue.data());
					parameter->Set(v8::String::NewFromUtf8(isolate, "currentValue"),
						v8::Number::New(isolate, *value));
				}
			} else {
				parameter->Set(v8::String::NewFromUtf8(isolate, "currentValue"),
					v8::String::NewFromUtf8(isolate, ""));
			}

			// Values
			v8::Local<v8::Array> values = v8::Array::New(isolate);
			uint32_t indexData = 0;

			for (int k = 0; k < params.at(j).countValues; k++) {
				v8::Local<v8::Object> valueObject = v8::Object::New(isolate);

				if (params.at(j).subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
					size_t *sizeName =
						reinterpret_cast<std::size_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(size_t);
					std::string name(params.at(j).values.data() + indexData, *sizeName);
					indexData += *sizeName;


					int64_t *value = reinterpret_cast<int64_t*>(
						params.at(j).values.data() + indexData);

					indexData += sizeof(int64_t);
					
					valueObject->Set(v8::String::NewFromUtf8(isolate, name.c_str()),
						v8::String::NewFromUtf8(isolate, std::to_string(*value).c_str()));
				}
				else if (params.at(j).subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
					size_t *sizeName =
						reinterpret_cast<std::size_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(size_t);
					std::string name(params.at(j).values.data() + indexData, *sizeName);
					indexData += *sizeName;

					double *value = reinterpret_cast<double*>(
						params.at(j).values.data() + indexData);

					indexData += sizeof(double);

					valueObject->Set(v8::String::NewFromUtf8(isolate, name.c_str()),
						v8::String::NewFromUtf8(isolate, std::to_string(*value).c_str()));
				}
				else {
					size_t *sizeName =
						reinterpret_cast<std::size_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(size_t);
					std::string name(params.at(j).values.data() + indexData, *sizeName);
					indexData += *sizeName;

					size_t *sizeValue =
						reinterpret_cast<std::size_t*>(params.at(j).values.data() + indexData);
					indexData += sizeof(size_t);
					std::string value(params.at(j).values.data() + indexData, *sizeValue);
					indexData += *sizeValue;

					valueObject->Set(v8::String::NewFromUtf8(isolate, name.c_str()),
						v8::String::NewFromUtf8(isolate, value.c_str()));
				}
				values->Set(k, valueObject);
			}

			parameter->Set(
				v8::String::NewFromUtf8(isolate, "values"), 
				values);

			parameter->Set(v8::String::NewFromUtf8(isolate, "visible"),
				v8::Boolean::New(isolate, params.at(j).visible));

			parameter->Set(v8::String::NewFromUtf8(isolate, "enabled"),
				v8::Boolean::New(isolate, params.at(j).enabled));

			parameter->Set(v8::String::NewFromUtf8(isolate, "masked"),
				v8::Boolean::New(isolate, params.at(j).masked));

			subCategoryParameters->Set(j, parameter);
		}

		subCategory->Set(v8::String::NewFromUtf8(isolate, "nameSubCategory"), 
			v8::String::NewFromUtf8(isolate, categorySettings.at(i).name.c_str()));

		subCategory->Set(v8::String::NewFromUtf8(isolate, "parameters"), 
			subCategoryParameters);

		rval->Set(i, subCategory);
	}

	args.GetReturnValue().Set(rval);
	return;
}

std::vector<char> deserializeCategory(uint32_t *subCategoriesCount, uint32_t *sizeStruct,
	v8::Local<v8::Array> settings) {
	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	std::vector<char> buffer;

	std::vector<settings::SubCategory> sucCategories;
	int sizeSettings = settings->Length();
	for (int i = 0; i < settings->Length(); i++) {
		settings::SubCategory sc;

		v8::Local<v8::Object> subCategoryObject = v8::Local<v8::Object>::Cast(settings->Get(i));

		v8::String::Utf8Value param0(subCategoryObject->Get(v8::String::NewFromUtf8(isolate, "nameSubCategory")));
		std::string test(*param0);
		sc.name = std::string(*param0);

		v8::Local<v8::Array> parameters = v8::Local<v8::Array>::Cast(
			subCategoryObject->Get(v8::String::NewFromUtf8(isolate, "parameters")));
		
		sc.paramsCount = parameters->Length();
		int sizeParams = parameters->Length();
		for (int j = 0; j< parameters->Length(); j++) {
			settings::Parameter param;

			v8::Local<v8::Object> parameterObject = v8::Local<v8::Object>::Cast(parameters->Get(j));

			v8::String::Utf8Value name(parameterObject->Get(v8::String::NewFromUtf8(isolate, "name")));
			v8::String::Utf8Value type(parameterObject->Get(v8::String::NewFromUtf8(isolate, "type")));
			v8::String::Utf8Value subType(parameterObject->Get(v8::String::NewFromUtf8(isolate, "subType")));

			param.name = std::string(*name);
			param.type = std::string(*type);
			param.subType = std::string(*subType);

			if (param.type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 ||
				param.type.compare("OBS_PROPERTY_PATH") == 0 ||
				param.type.compare("OBS_PROPERTY_TEXT") == 0 ||
				param.type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
				v8::String::Utf8Value value(parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue")));
				param.sizeOfCurrentValue = strlen(*value);
				param.currentValue.resize(strlen(*value));
				memcpy(param.currentValue.data(), *value, strlen(*value));
			}
			else if (param.type.compare("OBS_PROPERTY_INT") == 0) {
				int64_t value = parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue"))->NumberValue();

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			}
			else if (param.type.compare("OBS_PROPERTY_UINT") == 0) {
				uint64_t value = parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue"))->NumberValue();

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			}
			else if (param.type.compare("OBS_PROPERTY_BOOL") == 0) {
				uint64_t value = parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue"))->NumberValue();

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			}
			else if (param.type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double value = parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue"))->NumberValue();

				param.sizeOfCurrentValue = sizeof(value);
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
			}
			else if (param.type.compare("OBS_PROPERTY_LIST") == 0) {
				v8::String::Utf8Value paramSubType(parameterObject->Get(v8::String::NewFromUtf8(isolate, "subType")));
				
				std::string subType = *paramSubType;

				if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
					int64_t value = parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue"))->NumberValue();

					param.sizeOfCurrentValue = sizeof(value);
					param.currentValue.resize(sizeof(value));
					memcpy(param.currentValue.data(), &value, sizeof(value));
				}
				else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
					double value = parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue"))->NumberValue();

					param.sizeOfCurrentValue = sizeof(value);
					param.currentValue.resize(sizeof(value));
					memcpy(param.currentValue.data(), &value, sizeof(value));
				}
				else if (subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
					v8::String::Utf8Value value(parameterObject->Get(v8::String::NewFromUtf8(isolate, "currentValue")));
					param.sizeOfCurrentValue = strlen(*value);
					param.currentValue.resize(strlen(*value));
					memcpy(param.currentValue.data(), *value, strlen(*value));
				}
			}
			sc.params.push_back(param);
		}
		sucCategories.push_back(sc);
	}

	for (int i = 0; i < sucCategories.size(); i++) {
		std::vector<char> serializedBuf = sucCategories.at(i).serialize();

		buffer.insert(buffer.end(),
			serializedBuf.begin(),
			serializedBuf.end());
	}

	*subCategoriesCount = sucCategories.size();
	*sizeStruct = buffer.size();

	return buffer;
}

void settings::OBS_settings_saveSettings(const v8::FunctionCallbackInfo<v8::Value>& args) {
	std::string category;
	ASSERT_GET_VALUE(args[0], category);

	uint32_t subCategoriesCount, sizeStruct;
	v8::Local<v8::Array> settings = v8::Local<v8::Array>::Cast(args[1]);

	std::vector<char> buffer = 
		deserializeCategory(&subCategoriesCount, &sizeStruct,
			settings);

	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Settings", "OBS_settings_saveSettings",
		{ ipc::value(category), ipc::value(subCategoriesCount),
			ipc::value(sizeStruct), ipc::value(buffer) });

	ValidateResponse(response);
}

void settings::OBS_settings_getListCategories(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto conn = GetConnection();
	if (!conn) return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("Settings", "OBS_settings_getListCategories", {});

	if (!ValidateResponse(response)) return;

	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Array> categories = v8::Array::New(isolate);

	for (int i = 1; i < response.size(); i++) {
		categories->Set(i-1, 
			v8::String::NewFromUtf8(isolate, response.at(i).value_str.c_str()));
	}	

	args.GetReturnValue().Set(categories);

	return;
}

INITIALIZER(nodeobs_settings) {
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "OBS_settings_getSettings", settings::OBS_settings_getSettings);
		NODE_SET_METHOD(exports, "OBS_settings_saveSettings", settings::OBS_settings_saveSettings);
		NODE_SET_METHOD(exports, "OBS_settings_getListCategories", settings::OBS_settings_getListCategories);
	});
}