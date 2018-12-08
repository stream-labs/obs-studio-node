#pragma once
#include <iostream>
#include <obs.h>
#include <sstream>
#include <string>
#include <util/lexer.h>
#include <util/platform.h>
#include "nodeobs_service.h"

#include "nodeobs_audio_encoders.h"

using namespace std;

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
		size_t            indexBuffer = 0;

		size_t sizeStruct = name.length() + description.length() + type.length() + subType.length() + sizeof(size_t) * 7
		                    + sizeof(bool) * 3 + sizeOfCurrentValue + sizeOfValues;
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
	static void OBS_settings_getListCategories(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);

	private:
	static std::vector<std::string> getListCategories(void);

	// Exposed methods to the frontend
	static std::vector<SubCategory> getSettings(std::string nameCategory);
	static void                     saveSettings(std::string nameCategory, std::vector<SubCategory> settings);

	// Get each category
	static std::vector<SubCategory> getGeneralSettings();
	static std::vector<SubCategory> getStreamSettings();
	static std::vector<SubCategory> getOutputSettings();
	static std::vector<SubCategory> getAudioSettings();
	static std::vector<SubCategory> getVideoSettings();
	static std::vector<SubCategory> getAdvancedSettings();

	// Save each category
	static void saveGeneralSettings(std::vector<SubCategory> generalSettings, std::string pathConfigDirectory);
	static void saveStreamSettings(std::vector<SubCategory> streamSettings);
	static void saveOutputSettings(std::vector<SubCategory> streamSettings);
	static void saveAudioSettings(std::vector<SubCategory> audioSettings);
	static void saveVideoSettings(std::vector<SubCategory> videoSettings);
	static void saveAdvancedSettings(std::vector<SubCategory> advancedSettings);

	static void saveGenericSettings(std::vector<SubCategory> genericSettings, std::string section, config_t* config);

	static SubCategory serializeSettingsData(
	    std::string                                                   nameSubCategory,
	    std::vector<std::vector<std::pair<std::string, std::string>>> entries,
	    config_t*                                                     config,
	    std::string                                                   section,
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

	static void
	    getReplayBufferSettings(std::vector<SubCategory>* outputSettings, config_t* config, bool advanced = false);

	/****** Save Output Settings ******/

	// Simple Output mode
	static void saveSimpleOutputSettings(std::vector<SubCategory> settings);

	// Advanced Output mode
	static void saveAdvancedOutputStreamingSettings(std::vector<SubCategory> settings);

	static void saveAdvancedOutputRecordingSettings(std::vector<SubCategory> settings);

	static void saveAdvancedOutputSettings(std::vector<SubCategory> settings);

	//Utility functions
	static void getSimpleAvailableEncoders(std::vector<std::pair<std::string, std::string>>* streamEncode);
	static void getAdvancedAvailableEncoders(std::vector<std::pair<std::string, std::string>>* streamEncode);
	static std::vector<pair<uint64_t, uint64_t>> getOutputResolutions(uint64_t base_cx, uint64_t base_cy);
	static void                                  getEncoderSettings(
	                                     const obs_encoder_t*    encoder,
	                                     obs_data_t*             settings,
	                                     std::vector<Parameter>* subCategoryParameters,
	                                     int                     index,
	                                     bool                    isCategoryEnabled,
	                                     bool                    recordEncoder);
};
