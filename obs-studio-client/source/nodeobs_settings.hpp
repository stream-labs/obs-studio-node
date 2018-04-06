#include <node.h>
#include <nan.h>

namespace settings {
	struct Parameter {
		uint32_t sizeName = 0;
		std::string name;
		uint32_t sizeDescription = 0;
		std::string description;
		uint32_t sizeType = 0;
		std::string type;
		uint32_t sizeSubType = 0;
		std::string subType;
		bool enabled;
		bool masked;
		bool visible;
		uint32_t sizeOfCurrentValue = 0;
		void* currentValue;
		uint32_t sizeOfValues = 0;
		void* values;
	};

	struct SubCategory {
		uint32_t sizeName = 0;
		std::string name;
		uint32_t paramsCount = 0;
		uint32_t paramsSize = 0;
		std::vector<Parameter> params;
	};

	static Nan::NAN_METHOD_RETURN_TYPE OBS_settings_getSettings(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_settings_saveSettings(Nan::NAN_METHOD_ARGS_TYPE info);
	static Nan::NAN_METHOD_RETURN_TYPE OBS_settings_getListCategories(Nan::NAN_METHOD_ARGS_TYPE info);
}