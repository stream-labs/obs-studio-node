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

#include "nodeobs_settings.hpp"
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

#include "server/nodeobs_settings-server.h"
#include "servernodeobs_api-server.h"
#include "data-value.hpp"

enum CategoryTypes : uint32_t
{
	NODEOBS_CATEGORY_LIST = 0,
	NODEOBS_CATEGORY_TAB = 1
};

typedef std::pair<std::string, data::value> settings_value;

inline Napi::Object buildJSObject(
	const std::string& name, const std::string& type,
	const std::string& description, const std::string& subType,
	const data::value& value, const std::vector<settings_value>& values, Napi::Env env,
	const double& minVal = 0, const double& maxVal = 0,	const double& stepVal = 0,
	const bool& visible = true, const bool& enabled = true, const bool& masked = false
)
{
	Napi::Object jsObject;
	jsObject.Set("name", Napi::String::New(env, name));
	jsObject.Set("type", Napi::String::New(env, type));
	jsObject.Set("description", Napi::String::New(env, description));
	jsObject.Set("subType", Napi::String::New(env, subType));
	jsObject.Set("visible", Napi::Boolean::New(env, visible));
	jsObject.Set("enabled", Napi::Boolean::New(env, enabled));
	jsObject.Set("masked", Napi::Boolean::New(env, masked));


	if (type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || type.compare("OBS_PROPERTY_PATH") == 0 ||
		type.compare("OBS_PROPERTY_TEXT") == 0 ||type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
		jsObject.Set(
			"currentValue",
			Napi::String::New(env, value.value_str.c_str())
		);
	}
	else if (type.compare("OBS_PROPERTY_INT") == 0) {
		jsObject.Set("currentValue", Napi::Number::New(env, value.value_union.i64));
		jsObject.Set("minVal", Napi::Number::New(env, minVal));
		jsObject.Set("maxVal", Napi::Number::New(env, maxVal));
		jsObject.Set("stepVal", Napi::Number::New(env, stepVal));
	} else if (
		type.compare("OBS_PROPERTY_UINT") == 0 || type.compare("OBS_PROPERTY_BITMASK") == 0) {
		jsObject.Set("currentValue", Napi::Number::New(env, value.value_union.ui64));
		jsObject.Set("minVal", Napi::Number::New(env, minVal));
		jsObject.Set("maxVal", Napi::Number::New(env, maxVal));
		jsObject.Set("stepVal", Napi::Number::New(env, stepVal));
	}
	else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
		jsObject.Set("currentValue", Napi::Boolean::New(env, value.value_union.ui32));
	}
	else if (type.compare("OBS_PROPERTY_DOUBLE") == 0) {
		jsObject.Set("currentValue", Napi::Number::New(env, value.value_union.fp64));
		jsObject.Set("minVal", Napi::Number::New(env, minVal));
		jsObject.Set("maxVal", Napi::Number::New(env, maxVal));
		jsObject.Set("stepVal", Napi::Number::New(env, stepVal));
	}
	else if (type.compare("OBS_PROPERTY_LIST") == 0) {
		if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
			jsObject.Set("currentValue", Napi::Number::New(env, value.value_union.i64));
			jsObject.Set("minVal", Napi::Number::New(env, minVal));
			jsObject.Set("maxVal", Napi::Number::New(env, maxVal));
			jsObject.Set("stepVal", Napi::Number::New(env, stepVal));
		}
		else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
			jsObject.Set("currentValue", Napi::Number::New(env, value.value_union.fp32));
			jsObject.Set("minVal", Napi::Number::New(env, minVal));
			jsObject.Set("maxVal", Napi::Number::New(env, maxVal));
			jsObject.Set("stepVal", Napi::Number::New(env, stepVal));
		}
		else if (subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
			jsObject.Set(
				"currentValue",
				Napi::String::New(env, value.value_str.c_str())
			);
		}
	}

	return jsObject;
}

inline void getGeneralSettings(Napi::Object& settings, Napi::Env env)
{
	Napi::Array subCategories = Napi::Array::New(env);
	uint32_t indexSubCategories = 0;
	auto config = ConfigManager::getInstance().getGlobal();

	// Output
	Napi::Object output = Napi::Object::New(env);
	Napi::Array outputObjects = Napi::Array::New(env);
	uint32_t indexOutput = 0;

	outputObjects.Set(indexOutput++, buildJSObject(
		"WarnBeforeStartingStream", "OBS_PROPERTY_BOOL",
		"Show confirmation dialog when starting streams", "",
		data::value(config_get_bool(config, "BasicWindow", "WarnBeforeStartingStream")),
		{}, env
	));
	outputObjects.Set(indexOutput++, buildJSObject(
		"WarnBeforeStoppingStream", "OBS_PROPERTY_BOOL",
		"Show confirmation dialog when stopping streams", "",
		data::value(config_get_bool(config, "BasicWindow", "WarnBeforeStoppingStream")),
		{}, env
	));
	outputObjects.Set(indexOutput++, buildJSObject(
		"RecordWhenStreaming", "OBS_PROPERTY_BOOL",
		"Automatically record when streaming", "",
		data::value(config_get_bool(config, "BasicWindow", "RecordWhenStreaming")),
		{}, env
	));
	outputObjects.Set(indexOutput++, buildJSObject(
		"KeepRecordingWhenStreamStops", "OBS_PROPERTY_BOOL",
		"Keep recording when stream stops", "",
		data::value(config_get_bool(config, "BasicWindow", "KeepRecordingWhenStreamStops")),
		{}, env
	));
	outputObjects.Set(indexOutput++, buildJSObject(
		"ReplayBufferWhileStreaming", "OBS_PROPERTY_BOOL",
		"Automatically start replay buffer when streaming", "",
		data::value(config_get_bool(config, "BasicWindow", "ReplayBufferWhileStreaming")),
		{}, env
	));
	outputObjects.Set(indexOutput++, buildJSObject(
		"KeepReplayBufferStreamStops", "OBS_PROPERTY_BOOL",
		"Keep replay buffer active when stream stops", "",
		data::value(config_get_bool(config, "BasicWindow", "KeepReplayBufferStreamStops")),
		{}, env
	));

	output.Set("nameSubCategory", Napi::String::New(env, "Output"));
	output.Set("parameters", outputObjects);
	subCategories.Set(indexSubCategories++, output);

	// Source Alignement Snapping
	Napi::Object sourceAlignment = Napi::Object::New(env);
	Napi::Array sourceAlignmentObjects = Napi::Array::New(env);
	uint32_t indexSourceAlignment = 0;

	sourceAlignmentObjects.Set(indexSourceAlignment++, buildJSObject(
		"SnappingEnabled", "OBS_PROPERTY_BOOL",
		"Enable", "",
		data::value(config_get_bool(config, "BasicWindow", "SnappingEnabled")),
		{}, env
	));
	sourceAlignmentObjects.Set(indexSourceAlignment++, buildJSObject(
		"SnapDistance", "OBS_PROPERTY_DOUBLE",
		"Snap Sensitivity", "",
		data::value(config_get_double(config, "BasicWindow", "SnapDistance")),
		{}, env, 0, 100, 0.5
	));
	sourceAlignmentObjects.Set(indexSourceAlignment++, buildJSObject(
		"ScreenSnapping", "OBS_PROPERTY_BOOL",
		"Snap Sources to edge of screen", "",
		data::value(config_get_bool(config, "BasicWindow", "ScreenSnapping")),
		{}, env
	));
	sourceAlignmentObjects.Set(indexSourceAlignment++, buildJSObject(
		"SourceSnapping", "OBS_PROPERTY_BOOL",
		"Snap Sources to other sources", "",
		data::value(config_get_bool(config, "BasicWindow", "SourceSnapping")),
		{}, env
	));
	sourceAlignmentObjects.Set(indexSourceAlignment++, buildJSObject(
		"CenterSnapping", "OBS_PROPERTY_BOOL",
		"Snap Sources to horizontal and vertical center", "",
		data::value(config_get_bool(config, "BasicWindow", "CenterSnapping")),
		{}, env
	));

	sourceAlignment.Set("nameSubCategory", Napi::String::New(env, "Source Alignement Snapping"));
	sourceAlignment.Set("parameters", sourceAlignmentObjects);
	subCategories.Set(indexSubCategories++, sourceAlignment);

	// Projectors
	Napi::Object projectors = Napi::Object::New(env);
	Napi::Array projectorsObjects = Napi::Array::New(env);
	uint32_t indexProjectors = 0;

	projectorsObjects.Set(indexProjectors++, buildJSObject(
		"HideProjectorCursor", "OBS_PROPERTY_BOOL",
		"Hide cursor over projectors", "",
		data::value(config_get_bool(config, "BasicWindow", "HideProjectorCursor")),
		{}, env
	));
	projectorsObjects.Set(indexProjectors++, buildJSObject(
		"ProjectorAlwaysOnTop", "OBS_PROPERTY_BOOL",
		"Make projectors always on top", "",
		data::value(config_get_bool(config, "BasicWindow", "ProjectorAlwaysOnTop")),
		{}, env
	));
	projectorsObjects.Set(indexProjectors++, buildJSObject(
		"SaveProjectors", "OBS_PROPERTY_BOOL",
		"Save projectors on exit", "",
		data::value(config_get_bool(config, "BasicWindow", "SaveProjectors")),
		{}, env
	));

	projectors.Set("nameSubCategory", Napi::String::New(env, "Projectors"));
	projectors.Set("parameters", projectorsObjects);
	subCategories.Set(indexSubCategories++, projectors);

	// System Tray
	Napi::Object systemTray = Napi::Object::New(env);
	Napi::Array systemTrayObjects = Napi::Array::New(env);
	uint32_t indexSystemTray = 0;

	systemTrayObjects.Set(indexSystemTray++, buildJSObject(
		"SysTrayEnabled", "OBS_PROPERTY_BOOL",
		"Enable", "",
		data::value(config_get_bool(config, "BasicWindow", "SysTrayEnabled")),
		{}, env
	));
	systemTrayObjects.Set(indexSystemTray++, buildJSObject(
		"SysTrayWhenStarted", "OBS_PROPERTY_BOOL",
		"Minimize to system tray when started", "",
		data::value(config_get_bool(config, "BasicWindow", "SysTrayWhenStarted")),
		{}, env
	));
	systemTrayObjects.Set(indexSystemTray++, buildJSObject(
		"SysTrayMinimizeToTray", "OBS_PROPERTY_BOOL",
		"Always minimize to system tray instead of task bar", "",
		data::value(config_get_bool(config, "BasicWindow", "SysTrayMinimizeToTray")),
		{}, env
	));

	systemTray.Set("nameSubCategory", Napi::String::New(env, "Projectors"));
	systemTray.Set("parameters", systemTrayObjects);
	subCategories.Set(indexSubCategories++, systemTray);

	settings.Set("data", subCategories);
	settings.Set("type", Napi::Number::New(env, NODEOBS_CATEGORY_LIST));
}

inline void getStreamSettings(Napi::Object& settings, Napi::Env env)
{
	Napi::Array subCategories = Napi::Array::New(env);
	uint32_t indexSubCategories = 0;

	Napi::Object service = Napi::Object::New(env);
	Napi::Array serviceObjects = Napi::Array::New(env);
	uint32_t indexService = 0;

	obs_data_t* settingsService = obs_service_get_settings(currentService);
	std::vector<settings_value> serviceTypes;
	uint32_t index = 0;
	const char* type;
	while (obs_enum_service_types(index++, &type)) {
		serviceTypes.push_back(std::make_pair(
			obs_service_get_display_name(type),
			data::value(type)
		))
	}
	serviceObjects.Set(indexService++, buildJSObject(
		"streamType", "OBS_PROPERTY_LIST",
		"Stream Type", "OBS_COMBO_FORMAT_STRING",
		data::value(obs_service_get_type(currentService)),
		serviceTypes, env
	));

	service.Set("nameSubCategory", Napi::String::New(env, "Untitled"));
	service.Set("parameters", serviceObjects);
	subCategories.Set(indexSubCategories++, service);

	Napi::Object serviceConfig = Napi::Object::New(env);
	Napi::Array serviceConfigObjects = Napi::Array::New(env);
	uint32_t indexserviceConfig = 0;

	obs_properties_t* properties = obs_service_properties(currentService);
	obs_property_t* property = obs_properties_first(properties);

	while (property) {
		std::string name = obs_property_name(property);
		std::string type = "";
		std::string subType = "";
		std::string description = obs_property_description(property);
		bool visible = obs_property_visible(property);
		bool enabled = !OBS_service::isStreamingOutputActive();
		bool masked = type.compare("OBS_PROPERTY_EDIT_TEXT") == 0
		               && obs_proprety_text_type(property) == OBS_TEXT_PASSWORD;

		int count = (int)obs_property_list_item_count(property);
		obs_combo_format format = obs_property_list_format(property);
		std::vector<settings_value> values;
		data::value value;
		for (int i = 0; i < count; i++) {
			switch(format) {
				case OBS_COMBO_FORMAT_INT: {
					type = "OBS_PROPERTY_INT";
					subType = "OBS_COMBO_FORMAT_INT";
					values.push_back(std::make_pair(
						obs_property_list_item_name(property, i),
						data::value(obs_property_list_item_int(property, i))
					));
					break;
				}
				case OBS_COMBO_FORMAT_FLOAT: {
					type  = "OBS_PROPERTY_DOUBLE";
					subType = "OBS_COMBO_FORMAT_FLOAT";
					values.push_back(std::make_pair(
						obs_property_list_item_name(property, i),
						data::value(obs_property_list_item_float(property, i))
					));
					break;
				}
				case OBS_COMBO_FORMAT_STRING: {
					type  = "OBS_PROPERTY_LIST";
					subType = "OBS_COMBO_FORMAT_STRING";
					values.push_back(std::make_pair(
						obs_property_list_item_name(property, i),
						data::value(obs_property_list_item_string(property, i))
					));
					break;
				}
			}
		}

		if (count == 0) {
			if (strcmp(obs_property_name(property), "key") == 0) {
				type = "OBS_PROPERTY_EDIT_TEXT";
				value = data::value(obs_service_get_key(currentService));
			} else if (strcmp(obs_property_name(property), "show_all") == 0) {
				type = "OBS_PROPERTY_BOOL";
				value = data::value(obs_data_get_bool(settingsService, "show_all"));
			} else if (strcmp(obs_property_name(property), "server") == 0) {
				type = strcmp(obs_service_get_type(currentService), "rtmp_common") == 0 ?
					"OBS_PROPERTY_LIST" : "OBS_PROPERTY_EDIT_TEXT";
				value = data::value(obs_service_get_url(currentService));
			} else if (strcmp(obs_property_name(property), "username") == 0) {
				type = "OBS_PROPERTY_EDIT_TEXT";
				value = data::value(obs_service_get_username(currentService));
			} else if (strcmp(obs_property_name(property), "password") == 0) {
				type = "OBS_PROPERTY_EDIT_TEXT";
				value = data::value(obs_service_get_password(currentService));
			} else if (strcmp(obs_property_name(property), "use_auth") == 0) {
				type = "OBS_PROPERTY_BOOL";
				value = data:value(obs_data_get_bool(settingsService, "use_auth"));
			}
		} else {
			switch(format) {
				case OBS_COMBO_FORMAT_INT: {
					value = data::value(obs_data_get_int(settingsService, obs_property_name(property)));
				}
				case OBS_COMBO_FORMAT_FLOAT: {
					value = data::value(obs_data_get_double(settingsService, obs_property_name(property)));
				}
				case OBS_COMBO_FORMAT_STRING: {
					value = data::value(obs_data_get_string(settingsService, obs_property_name(property)));
				}
			}
		}

		serviceConfigObjects.Set(indexserviceConfig++, buildJSObject(
			name, type, description, subType,
			value, values, env, 0, 0, 0, visible,
			enabled, masked
		));
		obs_property_next(&property);
	}

	serviceConfig.Set("nameSubCategory", Napi::String::New(env, "Untitled"));
	serviceConfig.Set("parameters", serviceConfigObjects);
	subCategories.Set(indexSubCategories++, serviceConfig);

	settings.Set("data", subCategories);
	settings.Set("type", Napi::Number::New(env, NODEOBS_CATEGORY_LIST));

	obs_data_release(settingsService);
	obs_properties_destroy(properties);
}

inline std::vector<settings_value> OBS_settings::getSimpleAvailableEncoders(bool recording = false)
{
	std::vector<settings_value> encoders;
	encoders->push_back(std::make_pair("Software (x264)", data::value(SIMPLE_ENCODER_X264)));

	if (recording)
		encoders->push_back(std::make_pair(
		    "Software (x264 low CPU usage preset, increases file size)", data::value(SIMPLE_ENCODER_X264_LOWCPU)));

	if (EncoderAvailable("obs_qsv11"))
		encoders->push_back(std::make_pair("Hardware (QSV)", data::value(SIMPLE_ENCODER_QSV)));

	if (EncoderAvailable("ffmpeg_nvenc"))
		encoders->push_back(std::make_pair("Hardware (NVENC)", data::value(SIMPLE_ENCODER_NVENC)));

	if (EncoderAvailable("amd_amf_h264"))
		encoders->push_back(std::make_pair("Hardware (AMD)", data::value(SIMPLE_ENCODER_AMD)));

	if (EncoderAvailable("jim_nvenc"))
		encoders->push_back(std::make_pair("Hardware (NVENC) (new)", data::value(ENCODER_NEW_NVENC)));

	if (EncoderAvailable(APPLE_SOFTWARE_VIDEO_ENCODER))
		encoders->push_back(std::make_pair("Apple VT H264 Software Encoder", data::value(APPLE_SOFTWARE_VIDEO_ENCODER)));

	if (EncoderAvailable(APPLE_HARDWARE_VIDEO_ENCODER))
		encoders->push_back(std::make_pair("Apple VT H264 Hardware Encoder", data::value(APPLE_HARDWARE_VIDEO_ENCODER)));

	return encoders;
}

inline std::vector<settings_value> getAdvancedAvailableEncoders()
{
	std::vector<settings_value> encoders;
	encoders->push_back(std::make_pair("Software (x264)", data::value(ADVANCED_ENCODER_X264)));

	if (EncoderAvailable("obs_qsv11"))
		encoders->push_back(std::make_pair("Hardware (QSV)", data::value(ADVANCED_ENCODER_QSV)));

	if (EncoderAvailable("ffmpeg_nvenc"))
		encoders->push_back(std::make_pair("Hardware (NVENC)", data::value(ADVANCED_ENCODER_NVENC)));

	if (EncoderAvailable("amd_amf_h264"))
		encoders->push_back(std::make_pair("AMD", data::value(ADVANCED_ENCODER_AMD)));

	if (EncoderAvailable("jim_nvenc"))
		encoders->push_back(std::make_pair("Hardware (NVENC) (new)", data::value(ENCODER_NEW_NVENC)));

	if (EncoderAvailable(APPLE_SOFTWARE_VIDEO_ENCODER))
		encoders->push_back(std::make_pair("Apple VT H264 Software Encoder", data::value(APPLE_SOFTWARE_VIDEO_ENCODER)));

	if (EncoderAvailable(APPLE_HARDWARE_VIDEO_ENCODER))
		encoders->push_back(std::make_pair("Apple VT H264 Hardware Encoder", data::value(APPLE_HARDWARE_VIDEO_ENCODER)));

	return encoders;
}

#ifdef __APPLE__
	std::string newValue;
static const char* translate_macvth264_encoder(std::string encoder)
{
	if (strcmp(encoder.c_str(), "vt_h264_hw") == 0) {
		newValue = "com.apple.videotoolbox.videoencoder.h264.gva";
	} else if (strcmp(encoder.c_str(), "vt_h264_sw") == 0) {
		newValue = "com.apple.videotoolbox.videoencoder.h264";
	} else {
		newValue = std::string(encoder);
	}

	return newValue.c_str();
}
#endif

inline void OBS_settings::getReplayBufferSettings(
    Napi::Array& subCategories, Napi::Env env,
    config_t* config, bool advanced,
    bool isCategoryEnabled, uint32_t& indexSubCategories)
{
	Napi::Object replayBuffer = Napi::Object::New(env);
	Napi::Array replayBufferObjects = Napi::Array::New(env);
	uint32_t indexReplayBuffer = 0;

	const char* section = advanced ? "AdvOut" : "SimpleOutput";

	replayBufferObjects.Set(indexReplayBuffer++, buildJSObject(
		"RecRB", "OBS_PROPERTY_BOOL",
		"Enable Replay Buffer", "",
		data::value(config_get_bool(config, section, "RecRB")),
		{}, env
	));

	bool currentRecRb = config_get_bool(config, section, "RecRB");

	if (currentRecRb) {
		replayBufferObjects.Set(indexReplayBuffer++, buildJSObject(
			"RecRBTime", "OBS_PROPERTY_INT",
			"Maximum Replay Time (Seconds)", "",
			data::value(config_get_bool(config, section, "RecRBTime")),
			{}, env, 0, 21599, 0
		));
	}

	if (obs_get_multiple_rendering()) {
		replayBufferObjects.Set(indexReplayBuffer++, buildJSObject(
			"replayBufferUseStreamOutput", "OBS_PROPERTY_BOOL",
			"Use stream output", "",
			data::value(config_get_bool(config, section, "replayBufferUseStreamOutput")),
			{}, env, 0, 21599, 0
		));
	}

	replayBuffer.Set("nameSubCategory", Napi::String::New(env, "Replay Buffer"));
	replayBuffer.Set("parameters", replayBufferObjects);
	subCategories.Set(indexReplayBuffer++, replayBuffer);
}

inline void getSimpleOutputSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, bool isCategoryEnabled)
{
	auto config = ConfigManager::getInstance().getBasic();

	// Streaming
	Napi::Object streaming = Napi::Object::New(env);
	Napi::Array streamingObjects = Napi::Array::New(env);
	uint32_t indexStreaming = 0;

	streamingObjects.Set(indexStreaming++, buildJSObject(
		"VBitrate", "OBS_PROPERTY_INT",
		"Video Bitrate", "",
		data::value(config_get_bool(config, "SimpleOutput", "VBitrate")),
		{}, env, 0, 1000000, 1
	));
	streamingObjects.Set(indexStreaming++, buildJSObject(
		"StreamEncoder", "OBS_PROPERTY_LIST",
		"Encoder", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_bool(config, "SimpleOutput", "StreamEncoder")),
		getSimpleAvailableEncoders(), env,
	));

#ifdef __APPLE__
	const char* sEncoder = config_get_string(config, "SimpleOutput", "StreamEncoder");
	config_set_string(config, "SimpleOutput", "StreamEncoder", translate_macvth264_encoder(std::string(sEncoder)));
	const char* rEncoder = config_get_string(config, "SimpleOutput", "RecEncoder");
	config_set_string(config, "SimpleOutput", "RecEncoder", translate_macvth264_encoder(std::string(rEncoder)));
#endif

	auto& bitrateMap = GetAACEncoderBitrateMap();
	std::vector<settings_value> aBitrates;
	for (auto& entry : bitrateMap)
		aBitrates.push_back(std::make_pair(std::to_string(entry.first), data::value(std::to_string(entry.first))));

	streamingObjects.Set(indexStreaming++, buildJSObject(
		"ABitrate", "OBS_PROPERTY_LIST",
		"Audio Bitrate", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "SimpleOutput", "ABitrate")),
		aBitrates, env,
	));
	bool useAdvanced = data::value(config_get_bool(config, "SimpleOutput", "UseAdvanced");
	streamingObjects.Set(indexStreaming++, buildJSObject(
		"UseAdvanced", "OBS_PROPERTY_BOOL",
		"Enable Advanced Encoder Settings", "",
		useAdvanced, {}, env,
	));

	if (useAdvanced) {
		streamingObjects.Set(indexStreaming++, buildJSObject(
			"EnforceBitrate", "OBS_PROPERTY_BOOL",
			"Enforce streaming service bitrate limits", "",
			data::value(config_get_bool(config, "SimpleOutput", "EnforceBitrate")),
			{}, env,
		));

		obs_data_t *settings = obs_service_get_settings(OBS_service::getService());
		const char *serviceName = obs_data_get_string(settings, "service");
		obs_data_release(settings);

		if (serviceName && strcmp(serviceName, "Twitch") == 0) {
			bool soundtrackSourceExists = false;
			obs_enum_sources(
				[](void *param, obs_source_t *source) {
					auto id = obs_source_get_id(source);
					if(strcmp(id, "soundtrack_source") == 0) {
						*reinterpret_cast<bool *>(param) = true;
						return false;
					}
					return true;
				},
				&soundtrackSourceExists
			);
			std::string twitchVODDesc = "Twitch VOD Track (Uses Track 2).";
			if (soundtrackSourceExists)
				twitchVODDesc += " Remove Twitch Soundtrack in order to enable this.";

			streamingObjects.Set(indexStreaming++, buildJSObject(
				"VodTrackEnabled", "OBS_PROPERTY_BOOL",
				twitchVODDesc.c_str(), "",
				data::value(config_get_bool(config, "SimpleOutput", "VodTrackEnabled")),
				{}, env,
			));
		}

		//Encoder Preset
		const char* defaultPreset;
		const char* encoder = config_get_string(config, "SimpleOutput", "StreamEncoder");

		if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0 || strcmp(encoder, ADVANCED_ENCODER_QSV) == 0) {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"QSVPreset", "OBS_PROPERTY_LIST",
				"Encoder Preset (higher = less CPU)", "OBS_COMBO_FORMAT_STRING",
				data::value(config_get_string(config, "SimpleOutput", "QSVPreset")),
				{}, env,
			));
			defaultPreset = "balanced";
		} else if (
			strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0 ||
			strcmp(encoder, ADVANCED_ENCODER_NVENC) == 0 ||
			strcmp(encoder, ENCODER_NEW_NVENC) == 0
		) {
			std::vector<settings_value> values;
			obs_properties_t* props = obs_get_encoder_properties("ffmpeg_nvenc");
			obs_property_t* p   = obs_properties_get(props, "preset");
			size_t          num = obs_property_list_item_count(p);

			for (size_t i = 0; i < num; i++) {
				const char* name = obs_property_list_item_name(p, i);
				const char* val  = obs_property_list_item_string(p, i);

				/* bluray is for ideal bluray disc recording settings,
				* not streaming */
				if (strcmp(val, "bd") == 0)
					continue;
				/* lossless should of course not be used to stream */
				if (astrcmp_n(val, "lossless", 8) == 0)
					continue;

				values.push_back(std::make_pair(name, data::value(val)));
			}

			obs_properties_destroy(props);

			streamingObjects.Set(indexStreaming++, buildJSObject(
				"NVENCPreset", "OBS_PROPERTY_LIST",
				"Encoder Preset (higher = less CPU)", "OBS_COMBO_FORMAT_STRING",
				data::value(config_get_string(config, "SimpleOutput", "NVENCPreset")),
				values, env,
			));
			defaultPreset = "default";
		} else if (
			strcmp(encoder, SIMPLE_ENCODER_AMD) == 0 ||
			strcmp(encoder, ADVANCED_ENCODER_AMD) == 0) {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"AMDPreset", "OBS_PROPERTY_LIST",
				"Encoder Preset (higher = less CPU)", "OBS_COMBO_FORMAT_STRING",
				data::value(config_get_string(config, "SimpleOutput", "AMDPreset")),
				{
					std::make_pair("Speed", data::value("speed")),
					std::make_pair("Balanced", data::value("balanced")),
					std::make_pair("Quality", data::value("quality"))
				}, env,
			));
			defaultPreset = "balanced";
		} else if (
			strcmp(encoder, APPLE_SOFTWARE_VIDEO_ENCODER) == 0 ||
			strcmp(encoder, APPLE_HARDWARE_VIDEO_ENCODER) == 0) {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"Profile", "OBS_PROPERTY_LIST",
				"", "OBS_COMBO_FORMAT_STRING",
				data::value(config_get_string(config, "SimpleOutput", "Profile")),
				{
					std::make_pair("(None)", data::value("")),
					std::make_pair("baseline", data::value("baseline")),
					std::make_pair("main", data::value("main")),
					std::make_pair("high", data::value("high"))
				}, env,
			));
		} else {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"Preset", "OBS_PROPERTY_LIST",
				"Encoder Preset (higher = less CPU)", "OBS_COMBO_FORMAT_STRING",
				data::value(config_get_string(config, "SimpleOutput", "Preset")),
				{
					std::make_pair("ultrafast", data::value("ultrafast")),
					std::make_pair("superfast", data::value("superfast")),
					std::make_pair("veryfast", data::value("veryfast")),
					std::make_pair("faster", data::value("faster")),
					std::make_pair("fast", data::value("fast")),
					std::make_pair("medium", data::value("medium")),
					std::make_pair("slow", data::value("slow")),
					std::make_pair("slower", data::value("slower")),
				}, env,
			));
			defaultPreset = "veryfast";

			streamingObjects.Set(indexStreaming++, buildJSObject(
				"x264Settings", "OBS_PROPERTY_LIST",
				"Custom Encoder Settings", "",
				data::value(config_get_string(config, "SimpleOutput", "x264Settings")),
				{}, env,
			));
		}
	}

	streaming.Set("nameSubCategory", Napi::String::New(env, "Streaming"));
	streaming.Set("parameters", streamingObjects);
	subCategories.Set(indexSubCategories++, streaming);

	// Recording
	Napi::Object recording = Napi::Object::New(env);
	Napi::Array recordingObjects = Napi::Array::New(env);
	uint32_t indexRecording = 0;

	recordingObjects.Set(indexRecording++, buildJSObject(
		"FilePath", "OBS_PROPERTY_PATH",
		"Recording Path", "",
		data::value(config_get_string(config, "SimpleOutput", "FilePath")),
		{}, env,
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"FileNameWithoutSpace", "OBS_PROPERTY_BOOL",
		"Generate File Name without Space", "",
		data::value(config_get_bool(config, "SimpleOutput", "FileNameWithoutSpace")),
		{}, env,
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecQuality", "OBS_PROPERTY_LIST",
		"Recording Quality", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "SimpleOutput", "RecQuality")),
		{
			std::make_pair("Same as stream", data::value("Stream")),
			std::make_pair("High Quality, Medium File Size", data::value("Small")),
			std::make_pair("Indistinguishable Quality, Large File Size", data::value("HQ")),
			std::make_pair("Lossless Quality, Tremendously Large File Size", data::value("Lossless")),
		}, env,
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecFormat", "OBS_PROPERTY_LIST",
		"Recording Format", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "SimpleOutput", "RecFormat")),
		{
			std::make_pair("flv", data::value("flv")),
			std::make_pair("mp4", data::value("mp4")),
			std::make_pair("mov", data::value("mov")),
			std::make_pair("mkv", data::value("mkv")),
			std::make_pair("ts", data::value("ts")),
			std::make_pair("m3u8", data::value("m3u8")),
		}, env,
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecFormat", "OBS_PROPERTY_LIST",
		"Recording Format", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "SimpleOutput", "RecFormat")),
		{
			std::make_pair("flv", data::value("flv")),
			std::make_pair("mp4", data::value("mp4")),
			std::make_pair("mov", data::value("mov")),
			std::make_pair("mkv", data::value("mkv")),
			std::make_pair("ts", data::value("ts")),
			std::make_pair("m3u8", data::value("m3u8")),
		}, env,
	));

	std::string currentRecQuality =
	    config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecQuality");
	if (currentRecQuality.compare("Small") == 0 || currentRecQuality.compare("HQ") == 0) {
		recordingObjects.Set(indexRecording++, buildJSObject(
			"RecEncoder", "OBS_PROPERTY_LIST",
			"Encoder", "OBS_COMBO_FORMAT_STRING",
			data::value(config_get_string(config, "SimpleOutput", "RecEncoder")),
			getSimpleAvailableEncoders(true), env,
		));
	}

	recordingObjects.Set(indexRecording++, buildJSObject(
		"MuxerCustom", "OBS_PROPERTY_EDIT_TEXT",
		"Custom Muxer Settings", "",
		data::value(config_get_string(config, "SimpleOutput", "MuxerCustom")),
		{}, env,
	));

	recording.Set("nameSubCategory", Napi::String::New(env, "Recording"));
	recording.Set("parameters", recordingObjects);
	subCategories.Set(indexRecording++, recording);

	getReplayBufferSettings(
		subCategories, env, config, false,
		isCategoryEnabled, indexSubCategories);
}

inline std::string ResString(uint64_t cx, uint64_t cy)
{
	std::ostringstream res;
	res << cx << "x" << cy;
	return res.str();
}

std::vector<settings_value> OBS_settings::getOutputResolutions(uint64_t base_cx, uint64_t base_cy)
{
	std::vector<settings_value> values;
	std::vector<std::pair<uint64_t, uint64_t>> outputResolutions;

	for (size_t idx = 0; idx < numVals; idx++) {
		uint64_t outDownscaleCX = uint64_t(double(base_cx) / vals[idx]);
		uint64_t outDownscaleCY = uint64_t(double(base_cy) / vals[idx]);

		outDownscaleCX &= 0xFFFFFFFE;
		outDownscaleCY &= 0xFFFFFFFE;

		outputResolutions.push_back(std::make_pair(outDownscaleCX, outDownscaleCY));
	}

	for (int i = 0; i < outputResolutions.size(); i++) {
		std::string outRes = ResString(outputResolutions.at(i).first, outputResolutions.at(i).second);
		values.push_back(outRes, data::value(outRes));
	}

	return values;
}

inline void OBS_settings::getEncoderSettings(
    const obs_encoder_t* encoder, obs_data_t* settings,
	Napi::Array& subCategories, Napi::Env env, uint32_t& indexSubCategories,
	Napi::Array& streamingObjects, uint32_t& indexStreaming,
	bool isCategoryEnabled, bool recordEncoder)
{
	obs_properties_t* encoderProperties = obs_encoder_properties(encoder);
	obs_property_t* property = obs_properties_first(encoderProperties);

	while (property) {
		std::string name = obs_property_name(property);
		obs_property_type typeProperty = obs_property_get_type(property);
		std::string description = obs_property_description(property);
		std::string type;
		data::value value;
		double minVal = 0;
		double maxVal = 0;
		double stepVal = 0;
		std::string subType = "";
		std::vector<settings_value> values;

		switch (typeProperty) {
			case OBS_PROPERTY_BOOL: {
				type = "OBS_PROPERTY_BOOL";
				value = data::value(obs_data_get_bool(settings, param.name.c_str()));
				break;
			}
			case OBS_PROPERTY_INT: {
				type = "OBS_PROPERTY_INT";
				value = data::value(obs_data_get_int(settings, param.name.c_str()));
				minVal = obs_property_int_min(property);
				maxVal = obs_property_int_max(property);
				stepVal = obs_property_int_step(property);
				break;
			}
			case OBS_PROPERTY_FLOAT: {
				type = "OBS_PROPERTY_DOUBLE";
				value = data::value(obs_data_get_double(settings, param.name.c_str()));
				minVal = obs_property_float_min(property);
				maxVal = obs_property_float_max(property);
				stepVal = obs_property_float_step(property);
				break;
			}
			case OBS_PROPERTY_TEXT: {
				type        = "OBS_PROPERTY_TEXT";
				const char* currentValue = obs_data_get_string(settings, param.name.c_str());
				if (currentValue == NULL) {
					currentValue = "";
				}
				value = data::value(currentValue);
				break;
			}
			case OBS_PROPERTY_PATH: {
				type        = "OBS_PROPERTY_PATH";
				const char* currentValue = obs_data_get_string(settings, param.name.c_str());
				if (currentValue == NULL) {
					currentValue = "";
				}
				value = data::value(currentValue);
				break;
			}
			case OBS_PROPERTY_LIST: {
				type        = "OBS_PROPERTY_LIST";
				obs_combo_format format = obs_property_list_format(property);
				switch(format) {
					case OBS_COMBO_FORMAT_INT: {
						value = data::value(obs_data_get_int(settings, param.name.c_str()));
						minVal = obs_property_int_min(property);
						maxVal = obs_property_int_max(property);
						stepVal = obs_property_int_step(property);
						break;
					}
					case OBS_COMBO_FORMAT_FLOAT: {
						value = data::value(obs_data_get_double(settings, param.name.c_str()));
						minVal = obs_property_float_min(property);
						maxVal = obs_property_float_max(property);
						stepVal = obs_property_float_step(property);
						break;
					}
					case OBS_COMBO_FORMAT_STRING: {
						const char* currentValue = obs_data_get_string(settings, param.name.c_str());
						if (currentValue == NULL) {
							currentValue = "";
						}
						value = data::value(currentValue);
						break;
					}
				}

				for (int i = 0; i < (int)obs_property_list_item_count(property); i++) {
					std::string itemName = obs_property_list_item_name(property, i);
					if (format == OBS_COMBO_FORMAT_INT) {
						subType = "OBS_COMBO_FORMAT_INT";
						values.push_back(itemName, data::value(obs_property_list_item_int(property, i)));
					} else if (format == OBS_COMBO_FORMAT_FLOAT) {
						subType = "OBS_COMBO_FORMAT_FLOAT";
						values.push_back(itemName, data::value(obs_property_list_item_float(property, i)));
					} else if (format == OBS_COMBO_FORMAT_STRING) {
						subType = "OBS_COMBO_FORMAT_STRING";
						values.push_back(itemName, data::value(obs_property_list_item_string(property, i)));
					}
				}
				break;
			}
			case OBS_PROPERTY_EDITABLE_LIST: {
				type        = "OBS_PROPERTY_EDITABLE_LIST";
				const char* currentValue = obs_data_get_string(settings, param.name.c_str());
				if (currentValue == NULL)
					currentValue = "";
				value = data::value(currentValue);
				break;
			}
		}
		bool visible = obs_property_visible(property);
		bool isEnabled = obs_property_enabled(property);
		if (!isCategoryEnabled)
			isEnabled = isCategoryEnabled;

		if (recordEncoder) {
			param.name.insert(0, "Rec");
		}

		subCategoryParameters->push_back(param);

		streamingObjects.Set(indexStreaming++, buildJSObject(
			name, type, description, subType,
			value, values, env, minVal, maxVal, stepVal, visible, isEnabled
		));

		obs_property_next(&property);
	}

	obs_properties_destroy(encoderProperties);
}

inline void getAdvancedOutputStreamingSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, config_t* config,
	bool isCategoryEnabled)
{
	// Streaming
	Napi::Object streaming = Napi::Object::New(env);
	Napi::Array streamingObjects = Napi::Array::New(env);
	uint32_t indexStreaming = 0;

	streamingObjects.Set(indexStreaming++, buildJSObject(
		"TrackIndex", "OBS_PROPERTY_LIST",
		"Audio Track", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "AdvOut", "TrackIndex")),
		{
			std::make_pair("1", data::value("1")),
			std::make_pair("2", data::value("2")),
			std::make_pair("3", data::value("3")),
			std::make_pair("4", data::value("4")),
			std::make_pair("5", data::value("5")),
			std::make_pair("6", data::value("6")),
		}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	obs_data_t *serviceSettings = obs_service_get_settings(OBS_service::getService());
	const char *serviceName = obs_data_get_string(serviceSettings, "service");
	obs_data_release(serviceSettings);

	if (serviceName && strcmp(serviceName, "Twitch") == 0) {
		bool soundtrackSourceExists = false;
		obs_enum_sources(
			[](void *param, obs_source_t *source) {
				auto id = obs_source_get_id(source);
				if(strcmp(id, "soundtrack_source") == 0) {
					*reinterpret_cast<bool *>(param) = true;
					return false;
				}
				return true;
			},
			&soundtrackSourceExists
		);
	
		bool doTwiwchVOD = config_get_bool(config, "AdvOut", "VodTrackEnabled");
		std::string twitchVODDesc = "Twitch VOD";
		if (soundtrackSourceExists)
			twitchVODDesc += ". Remove Twitch Soundtrack in order to enable this.";

		streamingObjects.Set(indexStreaming++, buildJSObject(
			"VodTrackEnabled", "OBS_PROPERTY_BOOL",
			twitchVODDesc, "", doTwiwchVOD,
			{}, env, 0, 0, 0, true, isCategoryEnabled, false
		));

		if (doTwiwchVOD) {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"VodTrackIndex", "OBS_PROPERTY_LIST",
				"Twitch VOD Track", "OBS_COMBO_FORMAT_STRING",
				data::value(config_get_string(config, "AdvOut", "VodTrackIndex")),
				{
					std::make_pair("1", data::value("1")),
					std::make_pair("2", data::value("2")),
					std::make_pair("3", data::value("3")),
					std::make_pair("4", data::value("4")),
					std::make_pair("5", data::value("5")),
					std::make_pair("6", data::value("6")),
				}, env, 0, 0, 0, true, isCategoryEnabled, false
			));
		}
	}

	const char* encoderCurrentValue = config_get_string(config, "AdvOut", "Encoder");
#ifdef __APPLE__
	encoderCurrentValue = translate_macvth264_encoder(std::string(encoderCurrentValue));
	config_set_string(config, "AdvOut", "Encoder", encoderCurrentValue);
#endif
	streamingObjects.Set(indexStreaming++, buildJSObject(
		"Encoder", "OBS_PROPERTY_LIST",
		"Encoder", "OBS_COMBO_FORMAT_STRING",
		encoderCurrentValue,
		getAdvancedAvailableEncoders(), env, 0, 0, 0, true, isCategoryEnabled, false
	));

	streamingObjects.Set(indexStreaming++, buildJSObject(
		"ApplyServiceSettings", "OBS_PROPERTY_BOOL",
		"Enforce streaming service encoder settings", "",
		data::value(config_get_bool(config, "AdvOut", "ApplyServiceSettings")),
		{}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	bool doRescale = config_get_bool(config, "AdvOut", "Rescale");
	streamingObjects.Set(indexStreaming++, buildJSObject(
		"Rescale", "OBS_PROPERTY_BOOL",
		"Rescale Output", "",
		doRescale, {}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	if (doRescale) {
		uint64_t base_cx = config_get_uint(config, "Video", "BaseCX");
		uint64_t base_cy = config_get_uint(config, "Video", "BaseCY");
		const char* outputResString = config_get_string(config, "AdvOut", "RescaleRes");
		if (outputResString == NULL) {
			outputResString = "1280x720";
			config_set_string(config, "AdvOut", "RescaleRes", outputResString);
			config_save_safe(config, "tmp", nullptr);
		}

		streamingObjects.Set(indexStreaming++, buildJSObject(
			"RescaleRes", "OBS_INPUT_RESOLUTION_LIST",
			"Output Resolution", "OBS_COMBO_FORMAT_STRING",
			outputResString, getOutputResolutions(base_cx, base_cy),
			env, 0, 0, 0, true, isCategoryEnabled, false
		));
	}

	// Encoder settings
	const char* encoderID = config_get_string(config, "AdvOut", "Encoder");
	if (encoderID == NULL) {
		encoderID = "obs_x264";
		config_set_string(config, "AdvOut", "Encoder", encoderID);
		config_save_safe(config, "tmp", nullptr);
	}

	struct stat buffer;

	std::string streamName = ConfigManager::getInstance().getStream();
	bool        fileExist  = (os_stat(streamName.c_str(), &buffer) == 0);

	obs_data_t*    settings         = obs_encoder_defaults(encoderID);
	obs_encoder_t* streamingEncoder = OBS_service::getStreamingEncoder();
	obs_encoder_t* recordEncoder    = obs_output_get_video_encoder(OBS_service::getRecordingOutput());
	obs_output_t*  streamOutput     = OBS_service::getStreamingOutput();
	obs_output_t*  recordOutput     = OBS_service::getRecordingOutput();

	/*
		If the stream and recording outputs uses the same encoders, we need to check if both aren't active 
		before recreating the stream encoder to prevent releasing it when it's still being used.
		If they use differente encoders, just check for the stream output.
	*/
	bool streamOutputIsActive       = obs_output_active(streamOutput);
	bool recOutputIsActive          = obs_output_active(recordOutput);
	bool recStreamUsesSameEncoder   = streamingEncoder == recordEncoder;
	bool recOutputBlockStreamOutput = !(!recStreamUsesSameEncoder || (recStreamUsesSameEncoder && !recOutputIsActive));

	if ((!streamOutputIsActive && !recOutputBlockStreamOutput) || streamingEncoder == nullptr) {
		if (!fileExist) {
			streamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", nullptr, nullptr);
			OBS_service::setStreamingEncoder(streamingEncoder);

			if (!obs_data_save_json_safe(settings, streamName.c_str(), "tmp", "bak")) {
				blog(LOG_WARNING, "Failed to save encoder %s", streamName.c_str());
			}
		} else {
			obs_data_t* data = obs_data_create_from_json_file_safe(streamName.c_str(), "bak");
			obs_data_apply(settings, data);
			streamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", settings, nullptr);
			OBS_service::setStreamingEncoder(streamingEncoder);
		}

	} else {
		settings = obs_encoder_get_settings(streamingEncoder);
	}

	getEncoderSettings(
		streamingEncoder, settings, subCategories,
		env, indexSubCategories, streamingObjects,
		indexStreaming, isCategoryEnabled, false
	);

	streaming.Set("nameSubCategory", Napi::String::New(env, "Streaming"));
	streaming.Set("parameters", streamingObjects);
	subCategories.Set(indexSubCategories++, streaming);
}

void OBS_settings::getStandardRecordingSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, config_t* config,
    bool isCategoryEnabled)
{
	// Recording
	Napi::Object recording = Napi::Object::New(env);
	Napi::Array recordingObjects = Napi::Array::New(env);
	uint32_t indexRecording = 0;

	const char* RecFilePathCurrentValue = config_get_string(config, "AdvOut", "RecFilePath");
	if (!RecFilePathCurrentValue)
		RecFilePathCurrentValue = OBS_service::GetDefaultVideoSavePath().c_str();

	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecFilePath", "OBS_PROPERTY_PATH",
		"Recording Path", "", data::value(RecFilePathCurrentValue),
		{}, env, 0, 0, 0, true, isCategoryEnabled, false
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecFileNameWithoutSpace", "OBS_PROPERTY_BOOL",
		"Generate File Name without Space", "",
		data::value(config_get_bool(config, "AdvOut", "RecFileNameWithoutSpace")),
		{}, env, 0, 0, 0, true, isCategoryEnabled, false
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecFormat", "OBS_PROPERTY_LIST",
		"Recording Format", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "AdvOut", "RecFormat")),
		{
			std::make_pair("flv", data::value("flv")),
			std::make_pair("mp4", data::value("mp4")),
			std::make_pair("mov", data::value("mov")),
			std::make_pair("mkv", data::value("mkv")),
			std::make_pair("ts", data::value("ts")),
			std::make_pair("m3u8", data::value("m3u8"))
		}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	std::string recTracksDesc = std::string("Audio Track")
	    + (IsMultitrackAudioSupported(recFormatCurrentValue) ? 
		   "" : 
		   " (Format FLV does not support multiple audio tracks per recording)");
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecTracks", "OBS_PROPERTY_BITMASK",
		recTracksDesc, "",
		data::value(config_get_string(config, "AdvOut", "RecTracks")),
		{
			std::make_pair("flv", data::value("flv")),
			std::make_pair("mp4", data::value("mp4")),
			std::make_pair("mov", data::value("mov")),
			std::make_pair("mkv", data::value("mkv")),
			std::make_pair("ts", data::value("ts")),
			std::make_pair("m3u8", data::value("m3u8"))
		}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	const char* recEncoderCurrentValue = config_get_string(config, "AdvOut", "RecEncoder");
	if (!recEncoderCurrentValue)
		recEncoderCurrentValue = "none";
#ifdef __APPLE__
	recEncoderCurrentValue = translate_macvth264_encoder(std::string(recEncoderCurrentValue));
	config_set_string(config, "AdvOut", "RecEncoder", recEncoderCurrentValue);
#endif

	std::vector<settings_value> recEncoders;
	recEncoders.push_back(std::make_pair("Use stream encoder", data::value("none")));
	recEncoders.insert(getAdvancedAvailableEncoders);
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecEncoder", "OBS_PROPERTY_LIST",
		"Recording", "OBS_COMBO_FORMAT_STRING", data::value(recEncoderCurrentValue),
		getAdvancedAvailableEncoders(), env, 0, 0, 0, true, isCategoryEnabled, false
	));

	bool streamScaleAvailable = strcmp(recEncoderCurrentValue, "none") != 0;
	bool doRescale = config_get_bool(config, "AdvOut", "RecRescale");
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecRescale", "OBS_PROPERTY_BOOL",
		"Rescale Output", "", data::value(doRescale),
		{}, env, 0, 0, 0,
		strcmp(recEncoderCurrentValue, ENCODER_NEW_NVENC) != 0 && streamScaleAvailable,
		isCategoryEnabled, false
	));

	if (doRescale) {
		const char* outputResString = config_get_string(config, "AdvOut", "RecRescaleRes");
		if (!outputResString) {
			outputResString = "1280x720";
			config_set_string(config, "AdvOut", "RecRescaleRes", outputResString);
			config_save_safe(config, "tmp", nullptr);
		}
		uint64_t base_cx = config_get_uint(config, "Video", "BaseCX");
		uint64_t base_cy = config_get_uint(config, "Video", "BaseCY");
		recordingObjects.Set(indexRecording++, buildJSObject(
			"RecRescaleRes", "OBS_INPUT_RESOLUTION_LIST",
			"Output Resolution", "OBS_COMBO_FORMAT_STRING", data::value(outputResString),
			getOutputResolutions(base_cx, base_cy), env, 0, 0, 0,
			strcmp(recEncoderCurrentValue, ENCODER_NEW_NVENC) != 0 && streamScaleAvailable,
			isCategoryEnabled, false
		));
	}

	const char* RecMuxerCustomCurrentValue = config_get_string(config, "AdvOut", "RecMuxerCustom");
	if (RecMuxerCustomCurrentValue == NULL)
		RecMuxerCustomCurrentValue = "";
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecMuxerCustom", "OBS_PROPERTY_EDIT_TEXT",
		"Custom Muxer Settings", "", data::value(RecMuxerCustomCurrentValue),
		{}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	// Encoder settings
	struct stat buffer;

	bool fileExist = (os_stat(ConfigManager::getInstance().getRecord().c_str(), &buffer) == 0);

	obs_data_t*    settings = obs_encoder_defaults(recEncoderCurrentValue);
	obs_encoder_t* recordingEncoder;

	recordingEncoder           = OBS_service::getRecordingEncoder();
	obs_output_t* recordOutput = OBS_service::getRecordingOutput();

	if (recordOutput == NULL)
		return;

	if (obs_output_active(recordOutput)) {
		settings = obs_encoder_get_settings(recordingEncoder);
	} else if (!recordingEncoder || (recordingEncoder && !obs_encoder_active(recordingEncoder))) {
		if (!fileExist) {
			recordingEncoder = obs_video_encoder_create(recEncoderCurrentValue, "recording_h264", nullptr, nullptr);
			OBS_service::setRecordingEncoder(recordingEncoder);

			if (!obs_data_save_json_safe(settings, ConfigManager::getInstance().getRecord().c_str(), "tmp", "bak")) {
				blog(LOG_WARNING, "Failed to save encoder %s", ConfigManager::getInstance().getRecord().c_str());
			}
		} else if (strcmp(recEncoderCurrentValue, "none") != 0) {
			obs_data_t* data =
			    obs_data_create_from_json_file_safe(ConfigManager::getInstance().getRecord().c_str(), "bak");
			obs_data_apply(settings, data);
			recordingEncoder = obs_video_encoder_create(recEncoderCurrentValue, "recording_h264", settings, nullptr);
			OBS_service::setRecordingEncoder(recordingEncoder);
		}
	} else {
		settings = obs_encoder_get_settings(recordingEncoder);
	}

	if (strcmp(recEncoderCurrentValue, "none"))
		getEncoderSettings(
			recordingEncoder, settings, subCategories,
			env, indexSubCategories, recordingObjects,
			indexRecording, isCategoryEnabled, false
		);

	recording.Set("nameSubCategory", Napi::String::New(env, "Recording"));
	recording.Set("parameters", recordingObjects);
	subCategories.Set(indexSubCategories++, recording);
}

void OBS_settings::getAdvancedOutputAudioSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, config_t* config,
    bool                      isCategoryEnabled)
{
	Napi::Object audio = Napi::Object::New(env);
	Napi::Array audioObjects = Napi::Array::New(env);
	uint32_t indexAudio = 0;

	auto& bitrateMap = GetAACEncoderBitrateMap();
	UpdateAudioSettings(true);

	std::vector<settings_value> TrackBitrates;
	for (auto& entry : bitrateMap)
		TrackBitrates.push_back(
			std::make_pair(std::to_string(entry.first),
			data::value(std::to_string(entry.first))));

	for (int i = 0; i < 6, i++) {
		std::string track = "Track";
		track += std::to_string(i);
		track += "Bitrate";
		audioObjects.Set(indexRecording++, buildJSObject(
			track, "OBS_PROPERTY_LIST",
			"Audio Bitrate", "OBS_COMBO_FORMAT_STRING",
			data::value(config_get_string(config, "AdvOut", name)),
			TrackBitrates, env, 0, 0, 0, true, isCategoryEnabled, false
		));
		std::string name = "Track";
		name += std::to_string(i);
		name += "Name";
		audioObjects.Set(indexRecording++, buildJSObject(
			name, "OBS_PROPERTY_EDIT_TEXT",
			"Name", "",
			data::value(config_get_string(config, "AdvOut", name)),
			{}, env, 0, 0, 0, true, isCategoryEnabled, false
		));

		std::string subCategoryName = "Audio - Track ";
		subCategoryName += std::to_string(i);
		audio.Set("nameSubCategory", Napi::String::New(env, subCategoryName));
		audio.Set("parameters", audioObjects);
		subCategories.Set(indexSubCategories++, audio);
	}
}

void OBS_settings::getAdvancedOutputSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, config_t* config,
	bool isCategoryEnabled)
{
	auto config = ConfigManager::getInstance().getBasic();

	// Streaming
	getAdvancedOutputStreamingSettings(
		subCategories, env, indexSubCategories,
		config, isCategoryEnabled);

	// Recording
	getAdvancedOutputRecordingSettings(
		subCategories, env, indexSubCategories,
		config, isCategoryEnabled);

	// Audio
	getAdvancedOutputAudioSettings(
		subCategories, env, indexSubCategories,
		config, isCategoryEnabled);

	// Replay buffer
	getReplayBufferSettings(
		subCategories, env, config, true,
		isCategoryEnabled, indexSubCategories);
}

inline void getOutputSettings(Napi::Object& settings, Napi::Env env)
{
	Napi::Array subCategories = Napi::Array::New(env);
	uint32_t indexSubCategories = 0;

	// Output mode
	Napi::Object outputMode = Napi::Object::New(env);
	Napi::Array outputModeObjects = Napi::Array::New(env);
	uint32_t indexOutputMode = 0;

	const char* currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");
	outputModeObjects.Set(indexOutputMode++, buildJSObject(
		"Mode", "OBS_PROPERTY_LIST",
		"Output Mode", "OBS_COMBO_FORMAT_STRING",
		data::value(currentOutputMode ? currentOutputMode : "Simple"),
		{
			std::make_pair("Simple", data::value("Simple")),
			std::make_pair("Advanced", data::value("Advanced"))
		}, env
	));

	outputMode.Set("nameSubCategory", Napi::String::New(env, "Output"));
	outputMode.Set("parameters", outputModeObjects);
	subCategories.Set(indexSubCategories++, outputMode);

	CategoryTypes type;
	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive() && !OBS_service::isRecordingOutputActive()
		&& !OBS_service::isReplayBufferOutputActive();
	if (strcmp(currentOutputMode, "Advanced") == 0) {
		getAdvancedOutputSettings(settings, env, isCategoryEnabled);
		type = NODEOBS_CATEGORY_TAB;
	} else {
		getSimpleOutputSettings(settings, env, isCategoryEnabled);
		type = NODEOBS_CATEGORY_LIST;
	}

	settings.Set("data", subCategories);
	settings.Set("type", Napi::Number::New(env, type));
	
}

Napi::Value settings::OBS_settings_getSettings(const Napi::CallbackInfo& info)
{
	std::string category = info[0].ToString().Utf8Value();
	std::vector<std::string> listSettings = getListCategories();
	std::vector<std::string>::iterator it = std::find(listSettings.begin(), listSettings.end(), category);

	if (it == listSettings.end())
		return Napi::Array::New(info.Env());

	Napi::Object settings = Napi::Object::New(info.Env());

	if (category.compare("General") == 0) {
		getGeneralSettings(settings, info.Env());
	} else if (category.compare("Stream") == 0) {
		getStreamSettings(settings, info.Env());
	} else if (category.compare("Output") == 0) {
		getOutputSettings(settings);
	} else if (category.compare("Audio") == 0) {
		getAudioSettings(settings);
	} else if (category.compare("Video") == 0) {
		getVideoSettings(settings);
	} else if (category.compare("Advanced") == 0) {
		getAdvancedSettings(settings);
	}

	return settings;

	// auto settingsData = OBS_settings::OBS_settings_getSettings(category);

	// Napi::Array array = Napi::Array::New(info.Env());
	// Napi::Object settings = Napi::Object::New(info.Env());

	// std::vector<settings::SubCategory> categorySettings = serializeCategory(
	//     settingsData.settingsSize, settingsData.bufferSize, settingsData.buffer);

	// for (int i = 0; i < categorySettings.size(); i++) {
	// 	Napi::Object subCategory = Napi::Object::New(info.Env());
	// 	Napi::Array subCategoryParameters = Napi::Array::New(info.Env());
	// 	std::vector<settings::Parameter> params = categorySettings.at(i).params;

	// 	for (int j = 0; j < params.size(); j++) {
	// 		Napi::Object parameter = Napi::Object::New(info.Env());

	// 		parameter.Set("name", Napi::String::New(info.Env(), params.at(j).name));
	// 		parameter.Set("type", Napi::String::New(info.Env(), params.at(j).type));
	// 		parameter.Set("description", Napi::String::New(info.Env(), params.at(j).description));
	// 		parameter.Set("subType", Napi::String::New(info.Env(), params.at(j).subType));

	// 		if (params.at(j).currentValue.size() > 0) {
	// 			if (params.at(j).type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 ||
	// 				params.at(j).type.compare("OBS_PROPERTY_PATH") == 0 ||
	// 				params.at(j).type.compare("OBS_PROPERTY_TEXT") == 0 ||
	// 				params.at(j).type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {

	// 				std::string value(params.at(j).currentValue.begin(), 
	// 					params.at(j).currentValue.end());
	// 				parameter.Set("currentValue", Napi::String::New(info.Env(), value));
	// 			}
	// 			else if (params.at(j).type.compare("OBS_PROPERTY_INT") == 0) {
	// 				int64_t value = *reinterpret_cast<int64_t*>(params.at(j).currentValue.data());
	// 				parameter.Set("currentValue", Napi::Number::New(info.Env(), value));
	// 				parameter.Set("minVal", Napi::Number::New(info.Env(), params.at(j).minVal));
	// 				parameter.Set("maxVal", Napi::Number::New(info.Env(), params.at(j).maxVal));
	// 				parameter.Set("stepVal", Napi::Number::New(info.Env(), params.at(j).stepVal));
	// 			} else if (
	// 			    params.at(j).type.compare("OBS_PROPERTY_UINT") == 0
	// 			    || params.at(j).type.compare("OBS_PROPERTY_BITMASK") == 0) {
	// 				uint64_t value = *reinterpret_cast<uint64_t*>(params.at(j).currentValue.data());
	// 				parameter.Set("currentValue", Napi::Number::New(info.Env(), value));
	// 				parameter.Set("minVal", Napi::Number::New(info.Env(), params.at(j).minVal));
	// 				parameter.Set("maxVal", Napi::Number::New(info.Env(), params.at(j).maxVal));
	// 				parameter.Set("stepVal", Napi::Number::New(info.Env(), params.at(j).stepVal));
	// 			}
	// 			else if (params.at(j).type.compare("OBS_PROPERTY_BOOL") == 0) {
	// 				bool value = *reinterpret_cast<bool*>(params.at(j).currentValue.data());
	// 				parameter.Set("currentValue", Napi::Boolean::New(info.Env(), value));
	// 			}
	// 			else if (params.at(j).type.compare("OBS_PROPERTY_DOUBLE") == 0) {
	// 				double value =* reinterpret_cast<double*>(params.at(j).currentValue.data());
	// 				parameter.Set("currentValue", Napi::Number::New(info.Env(), value));
	// 				parameter.Set("minVal", Napi::Number::New(info.Env(), params.at(j).minVal));
	// 				parameter.Set("maxVal", Napi::Number::New(info.Env(), params.at(j).maxVal));
	// 				parameter.Set("stepVal", Napi::Number::New(info.Env(), params.at(j).stepVal));
	// 			}
	// 			else if (params.at(j).type.compare("OBS_PROPERTY_LIST") == 0) {
	// 				if (params.at(j).subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
	// 					int64_t value = *reinterpret_cast<int64_t*>(params.at(j).currentValue.data());
	// 					parameter.Set("currentValue", Napi::Number::New(info.Env(), value));
	// 					parameter.Set("minVal", Napi::Number::New(info.Env(), params.at(j).minVal));
	// 					parameter.Set("maxVal", Napi::Number::New(info.Env(), params.at(j).maxVal));
	// 					parameter.Set("stepVal", Napi::Number::New(info.Env(), params.at(j).stepVal));
	// 				}
	// 				else if (params.at(j).subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
	// 					double value =* reinterpret_cast<double*>(params.at(j).currentValue.data());
	// 					parameter.Set("currentValue", Napi::Number::New(info.Env(), value));
	// 					parameter.Set("minVal", Napi::Number::New(info.Env(), params.at(j).minVal));
	// 					parameter.Set("maxVal", Napi::Number::New(info.Env(), params.at(j).maxVal));
	// 					parameter.Set("stepVal", Napi::Number::New(info.Env(), params.at(j).stepVal));
	// 				}
	// 				else if (params.at(j).subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
	// 					std::string value(params.at(j).currentValue.begin(), 
	// 						params.at(j).currentValue.end());
	// 					parameter.Set("currentValue", Napi::String::New(info.Env(), value));
	// 				}
	// 			}
	// 		} else {
	// 			parameter.Set("currentValue", Napi::String::New(info.Env(), ""));
	// 		}

	// 		// Values
	// 		Napi::Array values = Napi::Array::New(info.Env());
	// 		uint32_t indexData = 0;

	// 		for (int k = 0; k < params.at(j).countValues; k++) {
	// 			Napi::Object valueObject = Napi::Object::New(info.Env());

	// 			if (params.at(j).subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
	// 				uint64_t* sizeName = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
	// 				indexData += sizeof(uint64_t);
	// 				std::string name(params.at(j).values.data() + indexData, *sizeName);
	// 				indexData += uint32_t(*sizeName);

	// 				int64_t value = *reinterpret_cast<int64_t*>(params.at(j).values.data() + indexData);

	// 				indexData += sizeof(int64_t);

	// 				valueObject.Set(name, Napi::Number::New(info.Env(), value));
	// 			}
	// 			else if (params.at(j).subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
	// 				uint64_t* sizeName = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
	// 				indexData += sizeof(uint64_t);
	// 				std::string name(params.at(j).values.data() + indexData, *sizeName);
	// 				indexData += uint32_t(*sizeName);

	// 				double value = *reinterpret_cast<double*>(params.at(j).values.data() + indexData);

	// 				indexData += sizeof(double);

	// 				valueObject.Set(name, Napi::Number::New(info.Env(), value));
	// 			}
	// 			else {
	// 				uint64_t* sizeName = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
	// 				indexData += sizeof(uint64_t);
	// 				std::string name(params.at(j).values.data() + indexData, *sizeName);
	// 				indexData += uint32_t(*sizeName);

	// 				uint64_t* sizeValue = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
	// 				indexData += sizeof(uint64_t);
	// 				std::string value(params.at(j).values.data() + indexData, *sizeValue);
	// 				indexData += uint32_t(*sizeValue);

	// 				valueObject.Set(name, Napi::String::New(info.Env(), value));
	// 			}
	// 			values.Set(k, valueObject);
	// 		}
	// 		if (params.at(j).countValues > 0 && params.at(j).currentValue.size() == 0
	// 		    && params.at(j).type.compare("OBS_PROPERTY_LIST") == 0 && params.at(j).enabled) {
	// 			uint32_t indexData = 0;
	// 			uint64_t* sizeName  = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
	// 			indexData += sizeof(uint64_t);
	// 			std::string name(params.at(j).values.data() + indexData, *sizeName);
	// 			indexData += uint32_t(*sizeName);

	// 			uint64_t* sizeValue = reinterpret_cast<uint64_t*>(params.at(j).values.data() + indexData);
	// 			indexData += sizeof(uint64_t);
	// 			std::string value(params.at(j).values.data() + indexData, *sizeValue);
	// 			indexData += uint32_t(*sizeValue);

	// 			parameter.Set("currentValue", Napi::String::New(info.Env(), value));
	// 		}
	// 		parameter.Set("values", values);
	// 		parameter.Set("visible", Napi::Boolean::New(info.Env(), params.at(j).visible));
	// 		parameter.Set("enabled", Napi::Boolean::New(info.Env(), params.at(j).enabled));
	// 		parameter.Set("masked", Napi::Boolean::New(info.Env(), params.at(j).masked));
	// 		subCategoryParameters.Set(j, parameter);
	// 	}
	// 	subCategory.Set("nameSubCategory", Napi::String::New(info.Env(), categorySettings.at(i).name));
	// 	subCategory.Set("parameters", subCategoryParameters);
	// 	array.Set(i, subCategory);
	// 	settings.Set("data", array);
	// 	settings.Set("type", Napi::Number::New(info.Env(), settingsData.type));
	// }
	// return settings;
}

// std::vector<char> deserializeCategory(uint32_t* subCategoriesCount, uint32_t* sizeStruct, Napi::Array settings)
// {
// 	std::vector<char> buffer;
// 	std::vector<settings::SubCategory> sucCategories;

// 	for (int i = 0; i < int(settings.Length()); i++) {
// 		settings::SubCategory sc;

// 		Napi::Object subCategoryObject = settings.Get(i).ToObject();

// 		sc.name = subCategoryObject.Get("nameSubCategory").ToString().Utf8Value();
// 		Napi::Array parameters = subCategoryObject.Get("parameters").As<Napi::Array>();

// 		sc.paramsCount = parameters.Length();
// 		for (int j = 0; j < sc.paramsCount; j++) {
// 			settings::Parameter param;
// 			Napi::Object parameterObject = parameters.Get(j).ToObject();

// 			param.name    = parameterObject.Get("name").ToString().Utf8Value();
// 			param.type    = parameterObject.Get("type").ToString().Utf8Value();
// 			param.subType = parameterObject.Get("subType").ToString().Utf8Value();

// 			if (param.type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || param.type.compare("OBS_PROPERTY_PATH") == 0
// 			    || param.type.compare("OBS_PROPERTY_TEXT") == 0
// 			    || param.type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
// 				std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();

// 				param.sizeOfCurrentValue = value.length();
// 				param.currentValue.resize(param.sizeOfCurrentValue);
// 				memcpy(param.currentValue.data(), value.c_str(), param.sizeOfCurrentValue);
// 			} else if (param.type.compare("OBS_PROPERTY_INT") == 0) {
// 				int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();

// 				param.sizeOfCurrentValue = sizeof(value);
// 				param.currentValue.resize(sizeof(value));
// 				memcpy(param.currentValue.data(), &value, sizeof(value));
// 			} else if (param.type.compare("OBS_PROPERTY_UINT") == 0 || param.type.compare("OBS_PROPERTY_BITMASK") == 0) {
// 				uint64_t value = uint64_t(parameterObject.Get("currentValue").ToNumber().Uint32Value());

// 				param.sizeOfCurrentValue = sizeof(value);
// 				param.currentValue.resize(sizeof(value));
// 				memcpy(param.currentValue.data(), &value, sizeof(value));
// 			} else if (param.type.compare("OBS_PROPERTY_BOOL") == 0) {
// 				bool value = parameterObject.Get("currentValue").ToBoolean().Value();

// 				param.sizeOfCurrentValue = sizeof(value);
// 				param.currentValue.resize(sizeof(value));
// 				memcpy(param.currentValue.data(), &value, sizeof(value));
// 			} else if (param.type.compare("OBS_PROPERTY_DOUBLE") == 0) {
// 				double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();

// 				param.sizeOfCurrentValue = sizeof(value);
// 				param.currentValue.resize(sizeof(value));
// 				memcpy(param.currentValue.data(), &value, sizeof(value));
// 			} else if (param.type.compare("OBS_PROPERTY_LIST") == 0) {
// 				std::string subType = parameterObject.Get("subType").ToString().Utf8Value();

// 				if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
// 					int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();

// 					param.sizeOfCurrentValue = sizeof(value);
// 					param.currentValue.resize(sizeof(value));
// 					memcpy(param.currentValue.data(), &value, sizeof(value));
// 				} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
// 					double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();

// 					param.sizeOfCurrentValue = sizeof(value);
// 					param.currentValue.resize(sizeof(value));
// 					memcpy(param.currentValue.data(), &value, sizeof(value));
// 				} else if (subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
// 					std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();

// 					param.sizeOfCurrentValue = value.length();
// 					param.currentValue.resize(param.sizeOfCurrentValue);
// 					memcpy(param.currentValue.data(), value.c_str(), param.sizeOfCurrentValue);
// 				}
// 			}
// 			sc.params.push_back(param);
// 		}
// 		sucCategories.push_back(sc);
// 	}

// 	for (int i = 0; i < sucCategories.size(); i++) {
// 		std::vector<char> serializedBuf = sucCategories.at(i).serialize();

// 		buffer.insert(buffer.end(), serializedBuf.begin(), serializedBuf.end());
// 	}

// 	*subCategoriesCount = uint32_t(sucCategories.size());
// 	*sizeStruct         = uint32_t(buffer.size());

// 	return buffer;
// }

void settings::OBS_settings_saveSettings(const Napi::CallbackInfo& info)
{
	std::string category = info[0].ToString().Utf8Value();
	Napi::Array settings = info[1].As<Napi::Array>();

	uint32_t subCategoriesCount, sizeStruct;

	// std::vector<char> buffer = deserializeCategory(&subCategoriesCount, &sizeStruct, settings);
	// OBS_settings::OBS_settings_saveSettings(category, subCategoriesCount, sizeStruct, buffer);
}

std::vector<std::string> settings::getListCategories(void)
{
	std::vector<std::string> categories;

	categories.push_back("General");
	categories.push_back("Stream");
	categories.push_back("Output");
	categories.push_back("Audio");
	categories.push_back("Video");
	categories.push_back("Hotkeys");
	categories.push_back("Advanced");

	return categories;
}

Napi::Value settings::OBS_settings_getListCategories(const Napi::CallbackInfo& info)
{
	Napi::Array categories = Napi::Array::New(info.Env());
	std::vector<std::string> settings = getListCategories();

	size_t index = 0;
	for (auto &category: settings)
		categories.Set(index++, Napi::String::New(info.Env(), category));

	return categories;
}

Napi::Array devices_to_js(const Napi::CallbackInfo& info, const std::vector<DeviceInfo> &data)
{
	Napi::Array devices = Napi::Array::New(info.Env());

	uint32_t js_array_index = 0;
	for (auto obj: data) {
		Napi::Object device = Napi::Object::New(info.Env());
		device.Set("description", Napi::String::New(info.Env(), obj.description));
		device.Set("id", Napi::String::New(info.Env(), obj.id));
		devices.Set(js_array_index++, device);
	}

	return devices;
}

Napi::Value settings::OBS_settings_getInputAudioDevices(const Napi::CallbackInfo& info)
{
	return devices_to_js(info, OBS_settings::OBS_settings_getInputAudioDevices());
}

Napi::Value settings::OBS_settings_getOutputAudioDevices(const Napi::CallbackInfo& info)
{
	return devices_to_js(info, OBS_settings::OBS_settings_getOutputAudioDevices());
}

Napi::Value settings::OBS_settings_getVideoDevices(const Napi::CallbackInfo& info)
{
	return devices_to_js(info, OBS_settings::OBS_settings_getVideoDevices());
}

void settings::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		Napi::String::New(env, "OBS_settings_getSettings"),
		Napi::Function::New(env, settings::OBS_settings_getSettings)
		);
	exports.Set(
		Napi::String::New(env, "OBS_settings_saveSettings"),
		Napi::Function::New(env, settings::OBS_settings_saveSettings)
		);
	exports.Set(
		Napi::String::New(env, "OBS_settings_getListCategories"),
		Napi::Function::New(env, settings::OBS_settings_getListCategories)
		);
	exports.Set(
		Napi::String::New(env, "OBS_settings_getInputAudioDevices"),
		Napi::Function::New(env, settings::OBS_settings_getInputAudioDevices)
		);
	exports.Set(
		Napi::String::New(env, "OBS_settings_getOutputAudioDevices"),
		Napi::Function::New(env, settings::OBS_settings_getOutputAudioDevices)
		);
	exports.Set(
		Napi::String::New(env, "OBS_settings_getVideoDevices"),
		Napi::Function::New(env, settings::OBS_settings_getVideoDevices)
		);
}