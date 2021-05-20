/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#pragma once
#include <iostream>
#include <obs.h>
#include <sstream>
#include <string>
#include <util/lexer.h>
#include <util/platform.h>
#include "nodeobs_service.h"

#include "nodeobs_audio_encoders.h"

enum CategoryTypes : uint32_t
{
	NODEOBS_CATEGORY_LIST = 0,
	NODEOBS_CATEGORY_TAB = 1
};

typedef std::vector<std::pair<std::string, ipc::value>> ipcpairvector;

struct OBSTypes
{
	inline static const char* PROPERTY_INPUT_RESOLUTION_LIST = "OBS_INPUT_RESOLUTION_LIST";
	inline static const char* PROPERTY_LIST         = "OBS_PROPERTY_LIST";
	inline static const char* PROPERTY_BOOL         = "OBS_PROPERTY_BOOL";
	inline static const char* PROPERTY_DOUBLE       = "OBS_PROPERTY_DOUBLE";
	inline static const char* PROPERTY_FLOAT        = "OBS_PROPERTY_FLOAT";
	inline static const char* PROPERTY_EDIT_TEXT    = "OBS_PROPERTY_EDIT_TEXT";
	inline static const char* PROPERTY_TEXT         = "OBS_PROPERTY_TEXT";
	inline static const char* PROPERTY_INT          = "OBS_PROPERTY_INT";
	inline static const char* PROPERTY_UINT         = "OBS_PROPERTY_UINT";
	inline static const char* PROPERTY_PATH			= "OBS_PROPERTY_PATH";
};

struct OBSSubTypes
{
	inline static const char* COMBO_FORMAT_STRING	= "OBS_COMBO_FORMAT_STRING";
	inline static const char* COMBO_FORMAT_INT		= "OBS_COMBO_FORMAT_INT";
	inline static const char* COMBO_FORMAT_FLOAT    = "OBS_COMBO_FORMAT_FLOAT";
};

struct Parameter
{
	std::string       name;
	std::string       description;
	std::string       type;
	std::string       subType;
	bool              enabled;
	bool              masked;
	bool              visible;
	double            minVal = -200;
	double            maxVal = 200;
	double            stepVal = 1;
	uint64_t          sizeOfCurrentValue = 0;
	std::vector<char> currentValue;
	uint64_t          sizeOfValues = 0;
	uint64_t          countValues  = 0;
	std::vector<char> values;

	Parameter(
	    std::string name = "",
	    std::string type = "",
	    std::string description = "",
	    std::string subType = "",
	    double minVal = 0,
	    double maxVal = 0,
	    double stepVal = 0,
	    uint64_t sizeOfCurrentValue = 0,
	    uint64_t sizeOfValues = 0,
	    uint64_t countValues = 0, 
	    bool enabled = false,
	    bool masked = false,
	    bool visible = false)
	    : name(name), type(type), description(description), subType(subType), minVal(minVal), maxVal(maxVal), 
		stepVal(stepVal), sizeOfCurrentValue(sizeOfCurrentValue), sizeOfValues(sizeOfValues),
	      countValues(countValues), enabled(enabled), masked(masked), visible(visible) {}

	std::vector<char> serialize()
	{
		std::vector<char> buffer;
		uint32_t          indexBuffer = 0;

		size_t sizeStruct = name.length() + description.length() + type.length() + subType.length()
		                    + sizeof(uint64_t) * 7 + sizeof(bool) * 3 + sizeof(double) * 3 + sizeOfCurrentValue
		                    + sizeOfValues;
		buffer.resize(sizeStruct);

		*reinterpret_cast<uint64_t*>(buffer.data() + indexBuffer) = name.length();
		indexBuffer += sizeof(uint64_t);
		memcpy(buffer.data() + indexBuffer, name.data(), name.length());
		indexBuffer += uint32_t(name.length());

		*reinterpret_cast<uint64_t*>(buffer.data() + indexBuffer) = description.length();
		indexBuffer += sizeof(uint64_t);
		memcpy(buffer.data() + indexBuffer, description.data(), description.length());
		indexBuffer += uint32_t(description.length());

		*reinterpret_cast<uint64_t*>(buffer.data() + indexBuffer) = type.length();
		indexBuffer += sizeof(uint64_t);
		memcpy(buffer.data() + indexBuffer, type.data(), type.length());
		indexBuffer += uint32_t(type.length());

		*reinterpret_cast<uint64_t*>(buffer.data() + indexBuffer) = subType.length();
		indexBuffer += sizeof(uint64_t);
		memcpy(buffer.data() + indexBuffer, subType.data(), subType.length());
		indexBuffer += uint32_t(subType.length());

		*reinterpret_cast<bool*>(buffer.data() + indexBuffer) = enabled;
		indexBuffer += sizeof(bool);
		*reinterpret_cast<bool*>(buffer.data() + indexBuffer) = masked;
		indexBuffer += sizeof(bool);
		*reinterpret_cast<bool*>(buffer.data() + indexBuffer) = visible;
		indexBuffer += sizeof(bool);

		*reinterpret_cast<double*>(buffer.data() + indexBuffer) = minVal;
		indexBuffer += sizeof(double);
		*reinterpret_cast<double*>(buffer.data() + indexBuffer) = maxVal;
		indexBuffer += sizeof(double);
		*reinterpret_cast<double*>(buffer.data() + indexBuffer) = stepVal;
		indexBuffer += sizeof(double);

		*reinterpret_cast<uint64_t*>(buffer.data() + indexBuffer) = sizeOfCurrentValue;
		indexBuffer += sizeof(uint64_t);

		memcpy(buffer.data() + indexBuffer, currentValue.data(), sizeOfCurrentValue);
		indexBuffer += uint32_t(sizeOfCurrentValue);

		*reinterpret_cast<uint64_t*>(buffer.data() + indexBuffer) = sizeOfValues;
		indexBuffer += sizeof(uint64_t);

		*reinterpret_cast<uint64_t*>(buffer.data() + indexBuffer) = countValues;
		indexBuffer += sizeof(uint64_t);

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
		uint64_t          indexBuffer = 0;

		size_t sizeStruct = name.length() + sizeof(uint64_t) + sizeof(uint32_t);
		buffer.resize(sizeStruct);

		*reinterpret_cast<uint64_t*>(buffer.data()) = name.length();
		indexBuffer += sizeof(uint64_t);
		memcpy(buffer.data() + indexBuffer, name.data(), name.length());
		indexBuffer += name.length();

		*reinterpret_cast<uint32_t*>(buffer.data() + indexBuffer) = paramsCount;
		indexBuffer += sizeof(uint32_t);

		for (int i = 0; i < params.size(); i++) {
			std::vector<char> serializedBuf = params.at(i).serialize();

			buffer.insert(buffer.end(), serializedBuf.begin(), serializedBuf.end());
		}

		return buffer;
	}
};

class OBS_settings
{
	public:
	OBS_settings();
	~OBS_settings();

	static void Register(ipc::server&);

	static void OBS_settings_getSettings(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_settings_saveSettings(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);

	static void saveGenericSettings(std::vector<SubCategory> genericSettings, std::string section, config_t* config);

	static void OBS_settings_getInputAudioDevices(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_settings_getOutputAudioDevices(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_settings_getVideoDevices(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);

	private:
	// Exposed methods to the frontend
	static std::vector<SubCategory> getSettings(std::string nameCategory, CategoryTypes&);
	static bool                     saveSettings(std::string nameCategory, std::vector<SubCategory> settings);

	// Get each category
	static std::vector<SubCategory> getGeneralSettings();
	static std::vector<SubCategory> getStreamSettings();
	static std::vector<SubCategory> getOutputSettings(CategoryTypes&);
	static std::vector<SubCategory> getAudioSettings();
	static std::vector<SubCategory> getVideoSettings();
	static std::vector<SubCategory> getAdvancedSettings();

	// Save each category
	static void saveGeneralSettings(std::vector<SubCategory> generalSettings, std::string pathConfigDirectory);
	static bool saveStreamSettings(std::vector<SubCategory> streamSettings);
	static void saveOutputSettings(std::vector<SubCategory> streamSettings);
	static void saveAudioSettings(std::vector<SubCategory> audioSettings);
	static void saveVideoSettings(std::vector<SubCategory> videoSettings);
	static void saveAdvancedSettings(std::vector<SubCategory> advancedSettings);

	static SubCategory serializeSettingsData(
	    const std::string &                                           nameSubCategory,
	    std::vector<std::vector<std::pair<std::string, ipc::value>>>& entries,
	    config_t*                                                     config,
	    const std::string &                                           section,
	    bool                                                          isVisible,
	    bool                                                          isEnabled);

	/****** Get Output Settings ******/

	// Simple Output mode
	static void
	    getSimpleOutputSettings(std::vector<SubCategory>* outputSettings, config_t* config, bool isCategoryEnabled);

	// Advanced Output mode
	static void
	    getAdvancedOutputSettings(std::vector<SubCategory>* outputSettings, config_t* config, bool isCategoryEnabled);

	static SubCategory getAdvancedOutputStreamingSettings(config_t* config, bool isCategoryEnabled);

	static SubCategory getAdvancedOutputRecordingSettings(config_t* config, bool isCategoryEnabled);
	static void
	    getStandardRecordingSettings(SubCategory* subCategoryParameters, config_t* config, bool isCategoryEnabled);
	static void
	    getFFmpegOutputRecordingSettings(SubCategory* subCategoryParameters, config_t* config, bool isCategoryEnabled);

	static void getAdvancedOutputAudioSettings(
	    std::vector<SubCategory>* outputSettings,
	    config_t*                 config,
	    bool                      isCategoryEnabled);

	static ipcpairvector& addSubCategory(const Parameter& param, std::vector<ipcpairvector>& entries);
	static void getReplayBufferSettings(
	    std::vector<SubCategory>* outputSettings,
	    config_t*                 config,
	    bool                      advanced,
	    bool                      isCategoryEnabled);

	/****** Save Output Settings ******/

	// Simple Output mode
	static void saveSimpleOutputSettings(std::vector<SubCategory> settings);

	// Advanced Output mode
	static void saveAdvancedOutputStreamingSettings(std::vector<SubCategory> settings);

	static void saveAdvancedOutputRecordingSettings(std::vector<SubCategory> settings);

	static void saveAdvancedOutputSettings(std::vector<SubCategory> settings);

	//Utility functions
	static void getSimpleAvailableEncoders(std::vector<std::pair<std::string, ipc::value>>* streamEncode, bool recording);
	static void getAdvancedAvailableEncoders(std::vector<std::pair<std::string, ipc::value>>* streamEncode);
	static std::vector<std::pair<uint64_t, uint64_t>> getOutputResolutions(uint64_t base_cx, uint64_t base_cy);
	static void                                  getEncoderSettings(
	                                     const obs_encoder_t*    encoder,
	                                     obs_data_t*             settings,
	                                     std::vector<Parameter>* subCategoryParameters,
	                                     int                     index,
	                                     bool                    isCategoryEnabled,
	                                     bool                    recordEncoder);
};


template<typename T>
inline void config_set(config_t* config, const char* section, const char* name, T value){};

template<>
inline void config_set<uint64_t>(config_t* config, const char* section, const char* name, uint64_t value)
{
	config_set_uint64_t(config, section, name, value);
}

template<>
inline void config_set<int>(config_t* config, const char* section, const char* name, int value)
{
	config_set_int(config, section, name, value);
}

template<>
inline void config_set<int64_t>(config_t* config, const char* section, const char* name, int64_t value)
{
	config_set_int(config, section, name, value);
}

template<>
inline void config_set<bool>(config_t* config, const char* section, const char* name, bool value)
{
	config_set_bool(config, section, name, value);
}

template<>
inline void config_set<std::string>(config_t* config, const char* section, const char* name, std::string value)
{
	config_set_string(config, section, name, value.c_str());
}

template<>
inline void config_set<double>(config_t* config, const char* section, const char* name, double value)
{
	config_set_double(config, section, name, value);
}

template<typename T>
inline void obs_data_set(obs_data_t* config, const char* name, T value){};

template<>
inline void obs_data_set<std::string>(obs_data_t* config, const char* name, std::string value)
{
	obs_data_set_string(config, name, value.c_str());
}

template<>
inline void obs_data_set<int>(obs_data_t* config, const char* name, int value)
{
	obs_data_set_int(config, name, value);
}

template<>
inline void obs_data_set<int64_t>(obs_data_t* config, const char* name, int64_t value)
{
	obs_data_set_int(config, name, value);
}

template<>
inline void obs_data_set<bool>(obs_data_t* config, const char* name, bool value)
{
	obs_data_set_bool(config, name, value);
}

template<>
inline void obs_data_set<double>(obs_data_t* config, const char* name, double value)
{
	obs_data_set_double(config, name, value);
}

template<typename T>
inline void set_in_config(
    Parameter          param,
    obs_data_t*        encoderSettings,
    const std::string& section,
    const std::string& name,
    int                i,
    int                indexEncoderSettings)
{
	T* value = reinterpret_cast<T*>(param.currentValue.data());
	if (i < indexEncoderSettings) {
		config_set<T>(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
	} else {
		obs_data_set<T>(encoderSettings, name.c_str(), *value);
	}
}

template<>
inline void set_in_config<std::string>(
    Parameter          param,
    obs_data_t*        encoderSettings,
    const std::string& section,
    const std::string& name,
    int                i,
    int                indexEncoderSettings)
{
	std::string value(param.currentValue.data(), param.currentValue.size());
	if (i < indexEncoderSettings) {
		config_set<std::string>(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
	} else {
		obs_data_set<std::string>(encoderSettings, name.c_str(), value);
	}
}
