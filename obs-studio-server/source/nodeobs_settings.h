#pragma once
#include <obs.h>
#include <string>
#include <iostream>
#include <sstream>
#include "nodeobs_service.h"
#include <util/platform.h>
#include <util/lexer.h>

#include "nodeobs_audio_encoders.h"

using namespace std;

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

class OBS_settings
{
public:
	OBS_settings();
	~OBS_settings();

	static void OBS_settings_getSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_settings_saveSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_settings_getListCategories(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

private:
	static std::vector<std::string> getListCategories(void);

	// Exposed methods to the frontend
	static std::vector<SubCategory> getSettings(std::string nameCategory);
	static void saveSettings(std::string nameCategory, std::vector<SubCategory> settings);

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



	static void saveGenericSettings(std::vector<SubCategory> genericSettings, std::string section, std::string pathFile);

	static SubCategory serializeSettingsData(std::string nameSubCategory,
												std::vector<std::vector<std::pair<std::string, std::string>>> entries, 
												config_t* config, std::string section, bool isVisible, bool isEnabled);


	/****** Get Output Settings ******/

	// Simple Output mode
	static void getSimpleOutputSettings(std::vector<SubCategory> *outputSettings,
											config_t* config, bool isCategoryEnabled);

	// Advanced Output mode
	static void 				getAdvancedOutputSettings(std::vector<SubCategory>* outputSettings,
															config_t* config, bool isCategoryEnabled);

	static SubCategory	 		getAdvancedOutputStreamingSettings(config_t* config, bool isCategoryEnabled);

	static SubCategory 			getAdvancedOutputRecordingSettings(config_t* config, bool isCategoryEnabled);
	static void 				getStandardRecordingSettings(SubCategory* subCategoryParameters,
																config_t* config, bool isCategoryEnabled);
	static void					getFFmpegOutputRecordingSettings(SubCategory* subCategoryParameters,
																	config_t* config, bool isCategoryEnabled);

	static void 				getAdvancedOutputAudioSettings(std::vector<SubCategory>* outputSettings,
																config_t* config, bool isCategoryEnabled);


	/****** Save Output Settings ******/

	// Simple Output mode
	static void saveSimpleOutputSettings(std::vector<SubCategory> settings, std::string basicConfigFile);

	// Advanced Output mode
	static void saveAdvancedOutputStreamingSettings(std::vector<SubCategory> settings, std::string basicConfigFile);

	static void saveAdvancedOutputRecordingSettings(std::vector<SubCategory> settings, std::string basicConfigFile);

	static void saveAdvancedOutputSettings(std::vector<SubCategory> settings, std::string basicConfigFile);


	//Utility functions
	static void getAvailableEncoders (std::vector<std::pair<std::string, std::string>> *streamEncode);
	static std::vector<pair<uint32_t, uint32_t>> 	getOutputResolutions (int base_cx, int base_cy);
	static void 									getEncoderSettings(const obs_encoder_t *encoder,
																		obs_data_t *settings,
																		std::vector<Parameter>* subCategoryParameters,
																		int index, bool isCategoryEnabled);
};
