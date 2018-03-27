#pragma once
#include <obs.h>
#include <string>
#include <iostream>
#include <sstream>
#include "nodeobs_service.h"
#include <util/platform.h>
#include <util/lexer.h>
// #include <libff/libff/ff-util.h>

#include "nodeobs_audio_encoders.h"

using namespace std;

class OBS_settings
{
public:
	OBS_settings();
	~OBS_settings();

	static void OBS_settings_getSettings(const FunctionCallbackInfo<Value>& args);
	static void OBS_settings_saveSettings(const FunctionCallbackInfo<Value>& args);
	static void OBS_settings_getListCategories(const FunctionCallbackInfo<Value>& args);

private:
	static Local<Array> getListCategories(void);

	// Exposed methods to the frontend
	static Local<Array> 		getSettings(std::string nameCategory);
	static void 				saveSettings(std::string nameCategory, Local<Array> settings);

	// Get each category
	static Local<Array> getGeneralSettings();
	static Local<Array> getStreamSettings();
	static Local<Array> getOutputSettings();
	static Local<Array> getAudioSettings();
	static Local<Array> getVideoSettings();
	static Local<Array> getAdvancedSettings();

	// Save each category
	static void saveGeneralSettings(Local<Array> generalSettings, std::string pathConfigDirectory);
	static void saveStreamSettings(Local<Array> streamSettings);
	static void saveOutputSettings(Local<Array> streamSettings);
	static void saveAudioSettings(Local<Array> audioSettings);
	static void saveVideoSettings(Local<Array> videoSettings);
	static void saveAdvancedSettings(Local<Array> advancedSettings);



	static void saveGenericSettings(Local<Array> genericSettings, std::string section, std::string pathFile);

	static Local<Object> serializeSettingsData(std::string nameSubCategory, 
												std::vector<std::vector<std::pair<std::string, std::string>>> entries, 
												config_t* config, std::string section, bool isVisible, bool isEnabled);


	/****** Get Output Settings ******/

	// Simple Output mode
	static void 				getSimpleOutputSettings(Local<Array> outputSettings,
														config_t* config, bool isCategoryEnabled);

	// Advanced Output mode
	static void 				getAdvancedOutputSettings(Local<Array> outputSettings,
															config_t* config, bool isCategoryEnabled);

	static Local<Object> 		getAdvancedOutputStreamingSettings(config_t* config, bool isCategoryEnabled);

	static Local<Object> 		getAdvancedOutputRecordingSettings(config_t* config, bool isCategoryEnabled);
	static void 				getStandardRecordingSettings(Local<Array>* subCategoryParameters,
																config_t* config, bool isCategoryEnabled);
	static void					getFFmpegOutputRecordingSettings(Local<Array>* subCategoryParameters,
																	config_t* config, bool isCategoryEnabled);

	static void 				getAdvancedOutputAudioSettings(Local<Array> outputSettings,
																config_t* config, bool isCategoryEnabled);


	/****** Save Output Settings ******/

	// Simple Output mode
	static void saveSimpleOutputSettings(Local<Array> settings, std::string basicConfigFile);

	// Advanced Output mode
	static void saveAdvancedOutputStreamingSettings(Local<Array> settings, std::string basicConfigFile);

	static void saveAdvancedOutputRecordingSettings(Local<Array> settings, std::string basicConfigFile);

	static void saveAdvancedOutputSettings(Local<Array> settings, std::string basicConfigFile);


	//Utility functions
	static void 									getAvailableEncoders (std::vector<std::pair<std::string,
																			std::string>>* streamEncoder);
	static std::vector<pair<uint32_t, uint32_t>> 	getOutputResolutions (int base_cx, int base_cy);
	static void 									getEncoderSettings(Isolate *isolate,
																		const obs_encoder_t *encoder,
																		obs_data_t *settings,
																		Local<Array>* subCategoryParameters,
																		int index, bool isCategoryEnabled);
};
