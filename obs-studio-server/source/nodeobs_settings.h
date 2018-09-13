#pragma once
#include <obs.h>
#include <string>
#include <iostream>
#include <sstream>
#include "nodeobs_service.h"
#include <util/platform.h>
#include <util/lexer.h>
#include <categories_generated.h>

#include "nodeobs_audio_encoders.h"

using namespace std;

class OBS_settings
{
public:
	OBS_settings();
	~OBS_settings();

	static void Register(ipc::server&);

	static void OBS_settings_getSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_settings_saveSettings(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_settings_getListCategories(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

private:
	static no::Category getSettings(const std::string &nameCategory);

	//Utility functions
	static void getSimpleAvailableEncoders(std::vector<std::pair<std::string, std::string>> *streamEncode);
	static void getAdvancedAvailableEncoders(std::vector<std::pair<std::string, std::string>> *streamEncode);
	static std::vector<pair<uint32_t, uint32_t>> getOutputResolutions (int base_cx, int base_cy);

	static void getEncoderSettings(
	  const obs_encoder_t *encoder,
	  obs_data_t *settings,
	  const no::SubCategory &subCategoryParameters,
	  int index,
	  bool isCategoryEnabled
	);
};
