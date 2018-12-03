#include <nan.h>
#include <node.h>

namespace settings
{
	struct Parameter
	{
		std::string       name;
		std::string       description;
		std::string       type;
		std::string       subType;
		bool              enabled;
		bool              masked;
		bool              visible;
		size_t            sizeOfCurrentValue = 0;
		std::vector<char> currentValue;
		size_t            sizeOfValues = 0;
		size_t            countValues  = 0;
		std::vector<char> values;

		std::vector<char> serialize()
		{
			std::vector<char> buffer;
			uint32_t          indexBuffer = 0;

			size_t sizeStruct = name.length() + description.length() + type.length() + subType.length()
			                    + sizeof(size_t) * 7 + sizeof(bool) * 3 + sizeOfCurrentValue + sizeOfValues;
			buffer.resize(sizeStruct);

			*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = name.length();
			indexBuffer += sizeof(size_t);
			memcpy(buffer.data() + indexBuffer, name.data(), name.length());
			indexBuffer += uint32_t(name.length());

			*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = description.length();
			indexBuffer += sizeof(size_t);
			memcpy(buffer.data() + indexBuffer, description.data(), description.length());
			indexBuffer += uint32_t(description.length());

			*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = type.length();
			indexBuffer += sizeof(size_t);
			memcpy(buffer.data() + indexBuffer, type.data(), type.length());
			indexBuffer += uint32_t(type.length());

			*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = subType.length();
			indexBuffer += sizeof(size_t);
			memcpy(buffer.data() + indexBuffer, subType.data(), subType.length());
			indexBuffer += uint32_t(subType.length());

			*reinterpret_cast<bool*>(buffer.data() + indexBuffer) = enabled;
			indexBuffer += sizeof(bool);
			*reinterpret_cast<bool*>(buffer.data() + indexBuffer) = masked;
			indexBuffer += sizeof(bool);
			*reinterpret_cast<bool*>(buffer.data() + indexBuffer) = visible;
			indexBuffer += sizeof(bool);

			*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = sizeOfCurrentValue;
			indexBuffer += sizeof(size_t);

			memcpy(buffer.data() + indexBuffer, currentValue.data(), sizeOfCurrentValue);
			indexBuffer += uint32_t(sizeOfCurrentValue);

			*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = sizeOfValues;
			indexBuffer += sizeof(size_t);

			*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = countValues;
			indexBuffer += sizeof(size_t);

			memcpy(buffer.data() + indexBuffer, values.data(), sizeOfValues);
			indexBuffer += uint32_t(sizeOfValues);

			return buffer;
		}
	};

	struct SubCategory
	{
		std::string            name;
		uint32_t               paramsCount = 0;
		std::vector<Parameter> params;

		std::vector<char> serialize()
		{
			std::vector<char> buffer;
			uint32_t          indexBuffer = 0;

			size_t sizeStruct = name.length() + sizeof(size_t) + sizeof(uint32_t);
			buffer.resize(sizeStruct);

			*reinterpret_cast<size_t*>(buffer.data()) = name.length();
			indexBuffer += sizeof(size_t);
			memcpy(buffer.data() + indexBuffer, name.data(), name.length());
			indexBuffer += uint32_t(name.length());

			*reinterpret_cast<uint32_t*>(buffer.data() + indexBuffer) = paramsCount;
			indexBuffer += sizeof(uint32_t);

			for (int i = 0; i < params.size(); i++) {
				std::vector<char> serializedBuf = params.at(i).serialize();

				buffer.insert(buffer.end(), serializedBuf.begin(), serializedBuf.end());
			}

			return buffer;
		}
	};

	static void OBS_settings_getSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_settings_saveSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void OBS_settings_getListCategories(const v8::FunctionCallbackInfo<v8::Value>& args);
} // namespace settings
