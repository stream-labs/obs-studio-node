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
#include <nlohmann/json.hpp>
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
	NODEOBS_CATEGORY_TAB  = 1
};

enum ParameterTypes : uint32_t
{
	INVALID,
	BOOL,
	INT,
	UINT,
	DOUBLE,
	PATH,
	LIST,
	EDIT_PATH,
	EDIT_TEXT,
	COUNT
};

enum ParameterSubTypes : uint32_t
{
	INVALID,
	COMBO_INT,
	COMBO_FLOAT,
	COMBO_STRING,
	COUNT
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
	double            minVal             = -200;
	double            maxVal             = 200;
	double            stepVal            = 1;
	size_t            sizeOfCurrentValue = 0;
	std::vector<char> currentValue;
	size_t            sizeOfValues = 0;
	size_t            countValues  = 0;
	std::vector<char> values;

	std::vector<char> serialize()
	{
		std::vector<char> buffer;
		size_t            indexBuffer = 0;

		size_t sizeStruct = name.length() + description.length() + type.length() + subType.length() + sizeof(size_t) * 7
		                    + sizeof(bool) * 3 + sizeof(double) * 3 + sizeOfCurrentValue + sizeOfValues;
		buffer.resize(sizeStruct);

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = name.length();
		indexBuffer += sizeof(size_t);
		memcpy(buffer.data() + indexBuffer, name.data(), name.length());
		indexBuffer += name.length();

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = description.length();
		indexBuffer += sizeof(size_t);
		memcpy(buffer.data() + indexBuffer, description.data(), description.length());
		indexBuffer += description.length();

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = type.length();
		indexBuffer += sizeof(size_t);
		memcpy(buffer.data() + indexBuffer, type.data(), type.length());
		indexBuffer += type.length();

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = subType.length();
		indexBuffer += sizeof(size_t);
		memcpy(buffer.data() + indexBuffer, subType.data(), subType.length());
		indexBuffer += subType.length();

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
		*reinterpret_cast<bool*>(buffer.data() + indexBuffer) = stepVal;
		indexBuffer += sizeof(double);

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = sizeOfCurrentValue;
		indexBuffer += sizeof(size_t);

		memcpy(buffer.data() + indexBuffer, currentValue.data(), sizeOfCurrentValue);
		indexBuffer += sizeOfCurrentValue;

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = sizeOfValues;
		indexBuffer += sizeof(size_t);

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = countValues;
		indexBuffer += sizeof(size_t);

		memcpy(buffer.data() + indexBuffer, values.data(), sizeOfValues);
		indexBuffer += sizeOfValues;

		return buffer;
	}
};

struct SubCategory
{
	std::string            name;
	size_t                 paramsCount = 0;
	std::vector<Parameter> params;

	std::vector<char> serialize()
	{
		std::vector<char> buffer;
		size_t            indexBuffer = 0;

		size_t sizeStruct = name.length() + sizeof(size_t) + sizeof(size_t);
		buffer.resize(sizeStruct);

		*reinterpret_cast<size_t*>(buffer.data()) = name.length();
		indexBuffer += sizeof(size_t);
		memcpy(buffer.data() + indexBuffer, name.data(), name.length());
		indexBuffer += name.length();

		*reinterpret_cast<size_t*>(buffer.data() + indexBuffer) = paramsCount;
		indexBuffer += sizeof(size_t);

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

	private:
	// Exposed methods to the frontend
	static nlohmann::json getSettings(std::string nameCategory, CategoryTypes&);
	static void           saveSettings(std::string nameCategory, nlohmann::json);

	// Get each category
	static nlohmann::json getGeneralSettings();
	static nlohmann::json getStreamSettings();
	static nlohmann::json getOutputSettings(CategoryTypes&);
	static nlohmann::json getAudioSettings();
	static nlohmann::json getVideoSettings();
	static nlohmann::json getAdvancedSettings();

	// Save each category
	static void saveGeneralSettings(nlohmann::json generalSettings, std::string pathConfigDirectory);
	static void saveStreamSettings(nlohmann::json streamSettings);
	static void saveOutputSettings(nlohmann::json streamSettings);
	static void saveAudioSettings(nlohmann::json audioSettings);
	static void saveVideoSettings(nlohmann::json videoSettings);
	static void saveAdvancedSettings(nlohmann::json advancedSettings);

	static nlohmann::json serializeSettingsData(
	    std::string      nameSubCategory,
	    nlohmann::json&& entries,
	    config_t*        config,
	    std::string      section,
	    bool             isVisible,
	    bool             isEnabled);

	/****** Get Output Settings ******/

	// Simple Output mode
	static void getSimpleOutputSettings(nlohmann::json& outputSettings, config_t* config, bool isCategoryEnabled);

	// Advanced Output mode
	static void getAdvancedOutputSettings(nlohmann::json& outputSettings, config_t* config, bool isCategoryEnabled);

	static nlohmann::json getAdvancedOutputStreamingSettings(config_t* config, bool isCategoryEnabled);

	static nlohmann::json getAdvancedOutputRecordingSettings(config_t* config, bool isCategoryEnabled);
	static void
	    getStandardRecordingSettings(SubCategory* subCategoryParameters, config_t* config, bool isCategoryEnabled);
	static void
	    getFFmpegOutputRecordingSettings(SubCategory* subCategoryParameters, config_t* config, bool isCategoryEnabled);

	static nlohmann::json getAdvancedOutputAudioSettings(config_t* config, bool isCategoryEnabled);

	static nlohmann::json getReplayBufferSettings(config_t* config, bool advanced, bool isCategoryEnabled);

	/****** Save Output Settings ******/

	// Simple Output mode
	static void saveSimpleOutputSettings(std::vector<SubCategory> settings);

	// Advanced Output mode
	static void saveAdvancedOutputStreamingSettings(std::vector<SubCategory> settings);

	static void saveAdvancedOutputRecordingSettings(std::vector<SubCategory> settings);

	static void saveAdvancedOutputSettings(std::vector<SubCategory> settings);

	//Utility functions
	static void
	            getSimpleAvailableEncoders(std::vector<std::pair<std::string, ipc::value>>* streamEncode, bool recording);
	static void getAdvancedAvailableEncoders(std::vector<std::pair<std::string, ipc::value>>* streamEncode);
	static std::vector<std::pair<uint64_t, uint64_t>> getOutputResolutions(uint64_t base_cx, uint64_t base_cy);
	static void                                       getEncoderSettings(
	                                          const obs_encoder_t*    encoder,
	                                          obs_data_t*             settings,
	                                          std::vector<Parameter>* subCategoryParameters,
	                                          int                     index,
	                                          bool                    isCategoryEnabled,
	                                          bool                    recordEncoder);
};
