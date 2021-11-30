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
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

#include "server/nodeobs_api-server.h"
#include "server/memory-manager.h"
#include "data-value.hpp"
#include <util/lexer.h>

enum CategoryTypes : uint32_t
{
	NODEOBS_CATEGORY_LIST = 0,
	NODEOBS_CATEGORY_TAB = 1
};

typedef std::pair<std::string, data::value> settings_value;

Napi::Array* currentAudioSettings = nullptr;

inline Napi::Object buildJSObject(
	const std::string& name, const std::string& type,
	const std::string& description, const std::string& subType,
	const data::value& value, const std::vector<settings_value>& values, Napi::Env env,
	const double& minVal = 0, const double& maxVal = 0,	const double& stepVal = 0,
	const bool& visible = true, const bool& enabled = true, const bool& masked = false
)
{
	Napi::Object jsObject = Napi::Object::New(env);
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

	Napi::Array valuesObject = Napi::Array::New(env);
	uint32_t indexValues = 0;
	for (auto settingValue: values) {
		Napi::Object valueObject = Napi::Object::New(env);
		if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
			valueObject.Set(
				settingValue.first, Napi::Number::New(env, settingValue.second.value_union.i64));
		}
		else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
			valueObject.Set(
				settingValue.first, Napi::Number::New(env, settingValue.second.value_union.fp64));
		}
		else if (subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
			valueObject.Set(
				settingValue.first, Napi::String::New(env, settingValue.second.value_str));
		}
		valuesObject.Set(indexValues++, valueObject);
	}

	jsObject.Set("values", valuesObject);
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

	obs_service_t* currentService = OBS_service::getService();
	obs_data_t* settingsService = obs_service_get_settings(currentService);
	std::vector<settings_value> serviceTypes;
	uint32_t index = 0;
	const char* type;
	while (obs_enum_service_types(index++, &type)) {
		serviceTypes.push_back(std::make_pair(
			obs_service_get_display_name(type),
			data::value(type)
		));
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
					std::string value = obs_property_list_item_string(property, i);
					uint64_t sizeValue = value.length();

					if (value[sizeValue-1] == '/') {
						sizeValue--;
						value.resize(sizeValue);
					}
					values.push_back(std::make_pair(
						obs_property_list_item_name(property, i),
						data::value(value)
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
				value = data::value(obs_data_get_bool(settingsService, "use_auth"));
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
					value = data::value(getSafeOBSstr(obs_data_get_string(settingsService, obs_property_name(property))));
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

inline bool EncoderAvailable(const char* encoder)
{
	const char* val;
	int         i = 0;

	while (obs_enum_encoder_types(i++, &val)) {
		if (val == nullptr)
			continue;
		if (strcmp(val, encoder) == 0)
			return true;
	}

	return false;
}

inline std::vector<settings_value> getSimpleAvailableEncoders(bool recording = false)
{
	std::vector<settings_value> encoders;
	encoders.push_back(std::make_pair("Software (x264)", data::value(SIMPLE_ENCODER_X264)));

	if (recording)
		encoders.push_back(std::make_pair(
		    "Software (x264 low CPU usage preset, increases file size)", data::value(SIMPLE_ENCODER_X264_LOWCPU)));

	if (EncoderAvailable("obs_qsv11"))
		encoders.push_back(std::make_pair("Hardware (QSV)", data::value(SIMPLE_ENCODER_QSV)));

	if (EncoderAvailable("ffmpeg_nvenc"))
		encoders.push_back(std::make_pair("Hardware (NVENC)", data::value(SIMPLE_ENCODER_NVENC)));

	if (EncoderAvailable("amd_amf_h264"))
		encoders.push_back(std::make_pair("Hardware (AMD)", data::value(SIMPLE_ENCODER_AMD)));

	if (EncoderAvailable("jim_nvenc"))
		encoders.push_back(std::make_pair("Hardware (NVENC) (new)", data::value(ENCODER_NEW_NVENC)));

	if (EncoderAvailable(APPLE_SOFTWARE_VIDEO_ENCODER))
		encoders.push_back(std::make_pair("Apple VT H264 Software Encoder", data::value(APPLE_SOFTWARE_VIDEO_ENCODER)));

	if (EncoderAvailable(APPLE_HARDWARE_VIDEO_ENCODER))
		encoders.push_back(std::make_pair("Apple VT H264 Hardware Encoder", data::value(APPLE_HARDWARE_VIDEO_ENCODER)));

	return encoders;
}

inline std::vector<settings_value> getAdvancedAvailableEncoders()
{
	std::vector<settings_value> encoders;
	encoders.push_back(std::make_pair("Software (x264)", data::value(ADVANCED_ENCODER_X264)));

	if (EncoderAvailable("obs_qsv11"))
		encoders.push_back(std::make_pair("Hardware (QSV)", data::value(ADVANCED_ENCODER_QSV)));

	if (EncoderAvailable("ffmpeg_nvenc"))
		encoders.push_back(std::make_pair("Hardware (NVENC)", data::value(ADVANCED_ENCODER_NVENC)));

	if (EncoderAvailable("amd_amf_h264"))
		encoders.push_back(std::make_pair("AMD", data::value(ADVANCED_ENCODER_AMD)));

	if (EncoderAvailable("jim_nvenc"))
		encoders.push_back(std::make_pair("Hardware (NVENC) (new)", data::value(ENCODER_NEW_NVENC)));

	if (EncoderAvailable(APPLE_SOFTWARE_VIDEO_ENCODER))
		encoders.push_back(std::make_pair("Apple VT H264 Software Encoder", data::value(APPLE_SOFTWARE_VIDEO_ENCODER)));

	if (EncoderAvailable(APPLE_HARDWARE_VIDEO_ENCODER))
		encoders.push_back(std::make_pair("Apple VT H264 Hardware Encoder", data::value(APPLE_HARDWARE_VIDEO_ENCODER)));

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

inline void getReplayBufferSettings(
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
			data::value(config_get_int(config, section, "RecRBTime")),
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
	subCategories.Set(indexSubCategories++, replayBuffer);
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
		data::value(config_get_int(config, "SimpleOutput", "VBitrate")),
		{}, env, 0, 1000000, 1
	));
	streamingObjects.Set(indexStreaming++, buildJSObject(
		"StreamEncoder", "OBS_PROPERTY_LIST",
		"Encoder", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "SimpleOutput", "StreamEncoder")),
		getSimpleAvailableEncoders(), env
	));

#ifdef __APPLE__
	config_set_string(
		config, "SimpleOutput", "StreamEncoder",
		translate_macvth264_encoder(getSafeOBSstr(config_get_string(config, "SimpleOutput", "StreamEncoder"))));
	config_set_string(
		config, "SimpleOutput", "RecEncoder",
		translate_macvth264_encoder(getSafeOBSstr(config_get_string(config, "SimpleOutput", "RecEncoder"))));
#endif

	auto& bitrateMap = GetAACEncoderBitrateMap();
	std::vector<settings_value> aBitrates;
	for (auto& entry : bitrateMap)
		aBitrates.push_back(std::make_pair(std::to_string(entry.first), data::value(std::to_string(entry.first))));

	streamingObjects.Set(indexStreaming++, buildJSObject(
		"ABitrate", "OBS_PROPERTY_LIST",
		"Audio Bitrate", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_string(config, "SimpleOutput", "ABitrate")),
		aBitrates, env
	));
	bool useAdvanced = config_get_bool(config, "SimpleOutput", "UseAdvanced");
	streamingObjects.Set(indexStreaming++, buildJSObject(
		"UseAdvanced", "OBS_PROPERTY_BOOL",
		"Enable Advanced Encoder Settings", "",
		data::value(useAdvanced), {}, env
	));

	if (useAdvanced) {
		streamingObjects.Set(indexStreaming++, buildJSObject(
			"EnforceBitrate", "OBS_PROPERTY_BOOL",
			"Enforce streaming service bitrate limits", "",
			data::value(config_get_bool(config, "SimpleOutput", "EnforceBitrate")),
			{}, env
		));

		obs_data_t *settings = obs_service_get_settings(OBS_service::getService());
		std::string serviceName = getSafeOBSstr(obs_data_get_string(settings, "service"));
		obs_data_release(settings);

		if (serviceName.compare("Twitch") == 0) {
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
				{}, env
			));
		}

		//Encoder Preset
		const char* defaultPreset;
		std::string encoder = getSafeOBSstr(config_get_string(config, "SimpleOutput", "StreamEncoder"));

		if (encoder.compare(SIMPLE_ENCODER_QSV) == 0 || encoder.compare(ADVANCED_ENCODER_QSV) == 0) {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"QSVPreset", "OBS_PROPERTY_LIST",
				"Encoder Preset (higher = less CPU)", "OBS_COMBO_FORMAT_STRING",
				data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "QSVPreset"))),
				{
					std::make_pair("Speed", data::value("speed")),
					std::make_pair("Balanced", data::value("balanced")),
					std::make_pair("Quality", data::value("quality"))
				}, env
			));
			defaultPreset = "balanced";
		} else if (
			encoder.compare(SIMPLE_ENCODER_NVENC) == 0 ||
			encoder.compare(ADVANCED_ENCODER_NVENC) == 0 ||
			encoder.compare(ENCODER_NEW_NVENC) == 0
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
				data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "NVENCPreset"))),
				values, env
			));
			defaultPreset = "default";
		} else if (
			encoder.compare(SIMPLE_ENCODER_AMD) == 0 ||
			encoder.compare(ADVANCED_ENCODER_AMD) == 0) {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"AMDPreset", "OBS_PROPERTY_LIST",
				"Encoder Preset (higher = less CPU)", "OBS_COMBO_FORMAT_STRING",
				data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "AMDPreset"))),
				{
					std::make_pair("Speed", data::value("speed")),
					std::make_pair("Balanced", data::value("balanced")),
					std::make_pair("Quality", data::value("quality"))
				}, env
			));
			defaultPreset = "balanced";
		} else if (
			encoder.compare(APPLE_SOFTWARE_VIDEO_ENCODER) == 0 ||
			encoder.compare(APPLE_HARDWARE_VIDEO_ENCODER) == 0) {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"Profile", "OBS_PROPERTY_LIST",
				"", "OBS_COMBO_FORMAT_STRING",
				data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "Profile"))),
				{
					std::make_pair("(None)", data::value("")),
					std::make_pair("baseline", data::value("baseline")),
					std::make_pair("main", data::value("main")),
					std::make_pair("high", data::value("high"))
				}, env
			));
		} else {
			streamingObjects.Set(indexStreaming++, buildJSObject(
				"Preset", "OBS_PROPERTY_LIST",
				"Encoder Preset (higher = less CPU)", "OBS_COMBO_FORMAT_STRING",
				data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "Preset"))),
				{
					std::make_pair("ultrafast", data::value("ultrafast")),
					std::make_pair("superfast", data::value("superfast")),
					std::make_pair("veryfast", data::value("veryfast")),
					std::make_pair("faster", data::value("faster")),
					std::make_pair("fast", data::value("fast")),
					std::make_pair("medium", data::value("medium")),
					std::make_pair("slow", data::value("slow")),
					std::make_pair("slower", data::value("slower")),
				}, env
			));
			defaultPreset = "veryfast";

			streamingObjects.Set(indexStreaming++, buildJSObject(
				"x264Settings", "OBS_PROPERTY_LIST",
				"Custom Encoder Settings", "",
				data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "x264Settings"))),
				{}, env
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
		data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "FilePath"))),
		{}, env
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"FileNameWithoutSpace", "OBS_PROPERTY_BOOL",
		"Generate File Name without Space", "",
		data::value(config_get_bool(config, "SimpleOutput", "FileNameWithoutSpace")),
		{}, env
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecQuality", "OBS_PROPERTY_LIST",
		"Recording Quality", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "RecQuality"))),
		{
			std::make_pair("Same as stream", data::value("Stream")),
			std::make_pair("High Quality, Medium File Size", data::value("Small")),
			std::make_pair("Indistinguishable Quality, Large File Size", data::value("HQ")),
			std::make_pair("Lossless Quality, Tremendously Large File Size", data::value("Lossless")),
		}, env
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecFormat", "OBS_PROPERTY_LIST",
		"Recording Format", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "RecFormat"))),
		{
			std::make_pair("flv", data::value("flv")),
			std::make_pair("mp4", data::value("mp4")),
			std::make_pair("mov", data::value("mov")),
			std::make_pair("mkv", data::value("mkv")),
			std::make_pair("ts", data::value("ts")),
			std::make_pair("m3u8", data::value("m3u8")),
		}, env
	));

	std::string currentRecQuality =
	    getSafeOBSstr(config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecQuality"));
	if (currentRecQuality.compare("Small") == 0 || currentRecQuality.compare("HQ") == 0) {
		recordingObjects.Set(indexRecording++, buildJSObject(
			"RecEncoder", "OBS_PROPERTY_LIST",
			"Encoder", "OBS_COMBO_FORMAT_STRING",
			data::value(config_get_string(config, "SimpleOutput", "RecEncoder")),
			getSimpleAvailableEncoders(true), env
		));
	}

	recordingObjects.Set(indexRecording++, buildJSObject(
		"MuxerCustom", "OBS_PROPERTY_EDIT_TEXT",
		"Custom Muxer Settings", "",
		data::value(getSafeOBSstr(config_get_string(config, "SimpleOutput", "MuxerCustom"))),
		{}, env
	));

	recording.Set("nameSubCategory", Napi::String::New(env, "Recording"));
	recording.Set("parameters", recordingObjects);
	subCategories.Set(indexSubCategories++, recording);

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

/* some nice default output resolution vals */
static const double vals[] = {1.0, 1.25, (1.0 / 0.75), 1.5, (1.0 / 0.6), 1.75, 2.0, 2.25, 2.5, 2.75, 3.0};

static const size_t numVals = sizeof(vals) / sizeof(double);

std::vector<settings_value> getOutputResolutions(uint64_t base_cx, uint64_t base_cy)
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
		values.push_back(std::make_pair(outRes, data::value(outRes)));
	}

	return values;
}

inline void getEncoderSettings(
    const obs_encoder_t* encoder, obs_data_t* settings,
	Napi::Env env, Napi::Array& streamingObjects, uint32_t& indexStreaming,
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
				value = data::value(obs_data_get_bool(settings, name.c_str()));
				break;
			}
			case OBS_PROPERTY_INT: {
				type = "OBS_PROPERTY_INT";
				value = data::value(obs_data_get_int(settings, name.c_str()));
				minVal = obs_property_int_min(property);
				maxVal = obs_property_int_max(property);
				stepVal = obs_property_int_step(property);
				break;
			}
			case OBS_PROPERTY_FLOAT: {
				type = "OBS_PROPERTY_DOUBLE";
				value = data::value(obs_data_get_double(settings, name.c_str()));
				minVal = obs_property_float_min(property);
				maxVal = obs_property_float_max(property);
				stepVal = obs_property_float_step(property);
				break;
			}
			case OBS_PROPERTY_TEXT: {
				type        = "OBS_PROPERTY_TEXT";
				value = data::value(getSafeOBSstr(obs_data_get_string(settings, name.c_str())));
				break;
			}
			case OBS_PROPERTY_PATH: {
				type        = "OBS_PROPERTY_PATH";
				value = data::value(getSafeOBSstr(obs_data_get_string(settings, name.c_str())));
				break;
			}
			case OBS_PROPERTY_LIST: {
				type        = "OBS_PROPERTY_LIST";
				obs_combo_format format = obs_property_list_format(property);
				switch(format) {
					case OBS_COMBO_FORMAT_INT: {
						value = data::value(obs_data_get_int(settings, name.c_str()));
						minVal = obs_property_int_min(property);
						maxVal = obs_property_int_max(property);
						stepVal = obs_property_int_step(property);
						break;
					}
					case OBS_COMBO_FORMAT_FLOAT: {
						value = data::value(obs_data_get_double(settings, name.c_str()));
						minVal = obs_property_float_min(property);
						maxVal = obs_property_float_max(property);
						stepVal = obs_property_float_step(property);
						break;
					}
					case OBS_COMBO_FORMAT_STRING: {
						value = data::value(getSafeOBSstr(obs_data_get_string(settings, name.c_str())));
						break;
					}
				}

				for (int i = 0; i < (int)obs_property_list_item_count(property); i++) {
					std::string itemName = obs_property_list_item_name(property, i);
					if (format == OBS_COMBO_FORMAT_INT) {
						subType = "OBS_COMBO_FORMAT_INT";
						values.push_back(std::make_pair(itemName, data::value(obs_property_list_item_int(property, i))));
					} else if (format == OBS_COMBO_FORMAT_FLOAT) {
						subType = "OBS_COMBO_FORMAT_FLOAT";
						values.push_back(std::make_pair(itemName, data::value(obs_property_list_item_float(property, i))));
					} else if (format == OBS_COMBO_FORMAT_STRING) {
						subType = "OBS_COMBO_FORMAT_STRING";
						values.push_back(std::make_pair(itemName, data::value(obs_property_list_item_string(property, i))));
					}
				}
				break;
			}
			case OBS_PROPERTY_EDITABLE_LIST: {
				type        = "OBS_PROPERTY_EDITABLE_LIST";
				value = data::value(getSafeOBSstr(obs_data_get_string(settings, name.c_str())));
				break;
			}
		}
		bool visible = obs_property_visible(property);
		bool isEnabled = obs_property_enabled(property);
		isEnabled = isCategoryEnabled ?
			obs_property_enabled(property) : isCategoryEnabled;

		if (recordEncoder) {
			name.insert(0, "Rec");
		}

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
		data::value(getSafeOBSstr(config_get_string(config, "AdvOut", "TrackIndex"))),
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
	std::string serviceName = getSafeOBSstr(obs_data_get_string(serviceSettings, "service"));
	obs_data_release(serviceSettings);

	if (serviceName.compare("Twitch") == 0) {
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
				data::value(getSafeOBSstr(config_get_string(config, "AdvOut", "VodTrackIndex"))),
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

	std::string encoderCurrentValue = getSafeOBSstr(config_get_string(config, "AdvOut", "Encoder"));
#ifdef __APPLE__
	encoderCurrentValue = translate_macvth264_encoder(encoderCurrentValue);
	config_set_string(config, "AdvOut", "Encoder", encoderCurrentValue);
#endif
	streamingObjects.Set(indexStreaming++, buildJSObject(
		"Encoder", "OBS_PROPERTY_LIST",
		"Encoder", "OBS_COMBO_FORMAT_STRING",
		data::value(encoderCurrentValue),
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
			data::value(outputResString), getOutputResolutions(base_cx, base_cy),
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
		streamingEncoder, settings,
		env, streamingObjects,
		indexStreaming, isCategoryEnabled, false
	);

	streaming.Set("nameSubCategory", Napi::String::New(env, "Streaming"));
	streaming.Set("parameters", streamingObjects);
	subCategories.Set(indexSubCategories++, streaming);
}

void getStandardRecordingSettings(
	Napi::Array& recordingObjects, Napi::Env env, 
	uint32_t& indexRecording, config_t* config,
    bool isCategoryEnabled)
{
	// Recording
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
		data::value(getSafeOBSstr(config_get_string(config, "AdvOut", "RecFormat"))),
		{
			std::make_pair("flv", data::value("flv")),
			std::make_pair("mp4", data::value("mp4")),
			std::make_pair("mov", data::value("mov")),
			std::make_pair("mkv", data::value("mkv")),
			std::make_pair("ts", data::value("ts")),
			std::make_pair("m3u8", data::value("m3u8"))
		}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	const char* recFormatCurrentValue = config_get_string(config, "AdvOut", "RecFormat");
	if (recFormatCurrentValue == NULL) recFormatCurrentValue = "";

	std::string recTracksDesc = std::string("Audio Track")
	    + (IsMultitrackAudioSupported(recFormatCurrentValue) ? 
		   "" : 
		   " (Format FLV does not support multiple audio tracks per recording)");
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecTracks", "OBS_PROPERTY_BITMASK",
		recTracksDesc, "",
		data::value(config_get_uint(config, "AdvOut", "RecTracks")),
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
	auto recoEncodersAvailable = getAdvancedAvailableEncoders();
	recEncoders.insert(recEncoders.end(), recoEncodersAvailable.begin(), recoEncodersAvailable.end());
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
			recordingEncoder, settings,
			env, recordingObjects,
			indexRecording, isCategoryEnabled, true
		);
}

void getAdvancedOutputRecordingSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, config_t* config,
	bool isCategoryEnabled)
{
	// Recording
	Napi::Object recording = Napi::Object::New(env);
	Napi::Array recordingObjects = Napi::Array::New(env);
	uint32_t indexRecording = 0;

	const char* RecTypeCurrentValue = config_get_string(config, "AdvOut", "RecType");
	if (RecTypeCurrentValue == NULL) {
		RecTypeCurrentValue = "Standard";
		config_set_string(config, "AdvOut", "RecType", "Standard");
	}
	recordingObjects.Set(indexRecording++, buildJSObject(
		"RecType", "OBS_PROPERTY_LIST",
		"Type", "OBS_COMBO_FORMAT_STRING",
		data::value(RecTypeCurrentValue),
		{
			std::make_pair("Standard", data::value("Standard")),
		}, env, 0, 0, 0, true, isCategoryEnabled, false
	));

	getStandardRecordingSettings(recordingObjects, env, indexRecording, config, isCategoryEnabled);

	recording.Set("nameSubCategory", Napi::String::New(env, "Recording"));
	recording.Set("parameters", recordingObjects);
	subCategories.Set(indexSubCategories++, recording);
}

void saveGenericSettings(const Napi::Array& settings, std::string section, config_t* config)
{
	for (int i = 0; i < settings.Length(); i++) {
		Napi::Object subCategoryObject = settings.Get(i).ToObject();
		Napi::Array parameters = subCategoryObject.Get("parameters").As<Napi::Array>();
		auto nameCateogry = subCategoryObject.Get("nameSubCategory").As<Napi::String>().ToString().Utf8Value();

		for (int j = 0; j < parameters.Length(); j++) {
			Napi::Object parameterObject = parameters.Get(j).ToObject();
			auto name = parameterObject.Get("name").ToString().Utf8Value();
			auto type = parameterObject.Get("type").ToString().Utf8Value();
			auto subType = parameterObject.Get("subType").ToString().Utf8Value();

			if (type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || type.compare("OBS_PROPERTY_PATH") == 0
			    || type.compare("OBS_PROPERTY_TEXT") == 0 || type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
				std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();
				config_set_string(config, section.c_str(), name.c_str(), value.c_str());
			} else if (type.compare("OBS_PROPERTY_INT") == 0) {
				int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();
				config_set_int(config, section.c_str(), name.c_str(), value);
			} else if (type.compare("OBS_PROPERTY_UINT") == 0) {
				uint64_t value = uint64_t(parameterObject.Get("currentValue").ToNumber().Uint32Value());
				config_set_uint(config, section.c_str(), name.c_str(), value);
			} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
				bool value = parameterObject.Get("currentValue").ToBoolean().Value();
				config_set_bool(config, section.c_str(), name.c_str(), value);

				if (name.compare("replayBufferUseStreamOutput") == 0) {
					if (value)
						obs_set_replay_buffer_rendering_mode(OBS_STREAMING_REPLAY_BUFFER_RENDERING);
					else
						obs_set_replay_buffer_rendering_mode(OBS_RECORDING_REPLAY_BUFFER_RENDERING);
				}
			} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();
				config_set_double(config, section.c_str(), name.c_str(), value);
			} else if (type.compare("OBS_PROPERTY_LIST") == 0) {
				if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
					int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();
					config_set_int(config, section.c_str(), name.c_str(), value);
				} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
					double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();
					config_set_double(config, section.c_str(), name.c_str(), value);
				} else if (subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
					std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();

					if (name.compare("MonitoringDeviceName") == 0) {
						std::string monDevName;
						std::string monDevId;

						if (value.compare("Default") != 0) {
							std::vector<std::pair<std::string, std::string>> monitoringDevice;

							auto enum_devices = [](void* param, const char* name, const char* id) {
								std::vector<std::pair<std::string, std::string>>* monitoringDevice =
								    (std::vector<std::pair<std::string, std::string>>*)param;
								monitoringDevice->push_back(std::make_pair(name, id));
								return true;
							};
							obs_enum_audio_monitoring_devices(enum_devices, &monitoringDevice);

							std::vector<std::pair<std::string, std::string>>::iterator it = std::find_if(
							    monitoringDevice.begin(),
							    monitoringDevice.end(),
							    [&value](const std::pair<std::string, std::string> device) {
								    return (device.first.compare(value) == 0);
							    });

							if (it != monitoringDevice.end()) {
								monDevName = it->first;
								monDevId   = it->second;
							} else {
								monDevName = "Default";
								monDevId   = "default";
							}
						} else {
							monDevName = value;
							monDevId   = "default";
						}
						config_set_string(config, section.c_str(), "MonitoringDeviceName", monDevName.c_str());
						config_set_string(config, section.c_str(), "MonitoringDeviceId", monDevId.c_str());
					} else {
						config_set_string(config, section.c_str(), name.c_str(), value.c_str());
					}
				}
			} else {
				std::cout << "type not found ! " << type.c_str() << std::endl;
			}
		}
	}
	config_save_safe(config, "tmp", nullptr);
}

void UpdateAudioSettings(const bool& saveOnlyIfLimitApplied, Napi::Env env)
{
	if (!currentAudioSettings) return;

	// Do nothing if there is no info
	if (!currentAudioSettings->IsArray() || currentAudioSettings->Length() == 0)
		return;

	auto currentChannelSetup =
	    getSafeOBSstr(config_get_string(ConfigManager::getInstance().getBasic(), "Audio", "ChannelSetup"));
	bool isSurround = IsSurround(currentChannelSetup.c_str());
	bool limitApplied = false;

	for (int i = 0; i < currentAudioSettings->Length(); i++) {
		Napi::Object audioObject = currentAudioSettings->Get(i).ToObject();
		Napi::Array parametersAudio = audioObject.Get("parameters").As<Napi::Array>();
		auto nameCateogry = audioObject.Get("nameSubCategory").As<Napi::String>().ToString().Utf8Value();
		Napi::Object parameterAudioObject = parametersAudio.Get((uint32_t)0).ToObject();
		auto name = parameterAudioObject.Get("name").ToString().Utf8Value();

		if (name.compare("Track1Bitrate") == 0 ||
		    name.compare("Track2Bitrate") == 0 ||
		    name.compare("Track3Bitrate") == 0 ||
		    name.compare("Track4Bitrate") == 0 ||
		    name.compare("Track5Bitrate") == 0 ||
		    name.compare("Track6Bitrate") == 0) {
			std::string valueStr = parameterAudioObject.Get("currentValue").ToString().Utf8Value();
			int value = std::atoi(valueStr.c_str());

			// Limit the value if not surround
			if (!isSurround && value > 320) {
				auto maxValue = std::to_string(320);
				parameterAudioObject.Set("currentValue", Napi::String::New(env, maxValue));
				limitApplied = true;
			}
		}
	}

	if ((!saveOnlyIfLimitApplied || (saveOnlyIfLimitApplied && limitApplied))) {
		saveGenericSettings(*currentAudioSettings, "AdvOut", ConfigManager::getInstance().getBasic());
	}
}

void getAdvancedOutputAudioSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, config_t* config,
    bool isCategoryEnabled)
{
	auto& bitrateMap = GetAACEncoderBitrateMap();
	UpdateAudioSettings(true, env);

	std::vector<settings_value> TrackBitrates;
	for (auto& entry : bitrateMap)
		TrackBitrates.push_back(
			std::make_pair(std::to_string(entry.first),
			data::value(std::to_string(entry.first))));

	Napi::Array audioObjectsArray = Napi::Array::New(env);
	
	for (int i = 0; i < 6; i++) {
		Napi::Object audio = Napi::Object::New(env);
		Napi::Array audioObjects = Napi::Array::New(env);
		uint32_t indexAudio = 0;
		std::string currentIndex = std::to_string(i + 1);

		std::string track = "Track";
		track += currentIndex;
		track += "Bitrate";
		audioObjects.Set(indexAudio++, buildJSObject(
			track, "OBS_PROPERTY_LIST",
			"Audio Bitrate", "OBS_COMBO_FORMAT_STRING",
			data::value(getSafeOBSstr(config_get_string(config, "AdvOut", track.c_str()))),
			TrackBitrates, env, 0, 0, 0, true, isCategoryEnabled, false
		));
		std::string name = "Track";
		name += currentIndex;
		name += "Name";
		audioObjects.Set(indexAudio++, buildJSObject(
			name, "OBS_PROPERTY_EDIT_TEXT",
			"Name", "",
			data::value(getSafeOBSstr(config_get_string(config, "AdvOut", name.c_str()))),
			{}, env, 0, 0, 0, true, isCategoryEnabled, false
		));

		std::string subCategoryName = "Audio - Track ";
		subCategoryName += currentIndex;
		audio.Set("nameSubCategory", Napi::String::New(env, subCategoryName));
		audio.Set("parameters", audioObjects);
		subCategories.Set(indexSubCategories++, audio);
		audioObjectsArray.Set(i, audio);
	}
	currentAudioSettings = new Napi::Array(env, audioObjectsArray);
}

void getAdvancedOutputSettings(
	Napi::Array& subCategories, Napi::Env env, 
	uint32_t& indexSubCategories, bool isCategoryEnabled)
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

	outputMode.Set("nameSubCategory", Napi::String::New(env, "Untitled"));
	outputMode.Set("parameters", outputModeObjects);
	subCategories.Set(indexSubCategories++, outputMode);

	CategoryTypes type;
	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive() && !OBS_service::isRecordingOutputActive()
		&& !OBS_service::isReplayBufferOutputActive();
	if (strcmp(currentOutputMode, "Advanced") == 0) {
		getAdvancedOutputSettings(subCategories, env, indexSubCategories, isCategoryEnabled);
		type = NODEOBS_CATEGORY_TAB;
	} else {
		getSimpleOutputSettings(subCategories, env, indexSubCategories, isCategoryEnabled);
		type = NODEOBS_CATEGORY_LIST;
	}

	settings.Set("data", subCategories);
	settings.Set("type", Napi::Number::New(env, type));
	
}

inline void getAudioSettings(Napi::Object& settings, Napi::Env env)
{
	Napi::Array subCategories = Napi::Array::New(env);
	uint32_t indexSubCategories = 0;
	auto config = ConfigManager::getInstance().getBasic();

	Napi::Object audio = Napi::Object::New(env);
	Napi::Array audioObjects = Napi::Array::New(env);
	uint32_t indexAudio = 0;

	audioObjects.Set(indexAudio++, buildJSObject(
		"SampleRate", "OBS_PROPERTY_LIST",
		"Sample Rate (requires a restart)", "OBS_COMBO_FORMAT_INT",
		data::value(config_get_uint(config, "Audio", "SampleRate")),
		{
			std::make_pair("44.1khz", data::value(44100)),
			std::make_pair("48khz", data::value(48000))
		}, env
	));
	audioObjects.Set(indexAudio++, buildJSObject(
		"ChannelSetup", "OBS_PROPERTY_LIST",
		"Channels (requires a restart)", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(config, "Audio", "ChannelSetup"))),
		{
			std::make_pair("Mono", data::value("Mono")),
			std::make_pair("Stereo", data::value("Stereo")),
			std::make_pair("2.1", data::value("2.1")),
			std::make_pair("4.0", data::value("4.0")),
			std::make_pair("4.1", data::value("4.1")),
			std::make_pair("5.1", data::value("5.1")),
			std::make_pair("7.1", data::value("7.1")),
		}, env
	));

	audio.Set("nameSubCategory", Napi::String::New(env, "Untitled"));
	audio.Set("parameters", audioObjects);
	subCategories.Set(indexSubCategories++, audio);

	settings.Set("data", subCategories);
	settings.Set("type", Napi::Number::New(env, NODEOBS_CATEGORY_LIST));
}

void getVideoSettings(Napi::Object& settings, Napi::Env env)
{
	Napi::Array subCategories = Napi::Array::New(env);
	uint32_t indexSubCategories = 0;
	auto config = ConfigManager::getInstance().getBasic();

	Napi::Object video = Napi::Object::New(env);
	Napi::Array videoObjects = Napi::Array::New(env);
	uint32_t indexVideo = 0;

	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive() && !OBS_service::isRecordingOutputActive()
	                         && !OBS_service::isReplayBufferOutputActive();

	uint64_t base_cx = config_get_uint(config, "Video", "BaseCX");
	uint64_t base_cy = config_get_uint(config, "Video", "BaseCY");
	auto currentBaseRes = ResString(base_cx, base_cy);

	std::vector<settings_value> baseResValues;
	baseResValues.push_back(std::make_pair("1920x1080", data::value("1920x1080")));
	baseResValues.push_back(std::make_pair("1280x720", data::value("1280x720")));

	std::vector<std::pair<uint32_t, uint32_t>> resolutions =
		OBS_API::availableResolutions();

	// Fill available display resolutions
	for (int i = 0; i < resolutions.size(); i++) {
		std::string baseResolutionString;
		baseResolutionString = std::to_string(resolutions.at(i).first);
		baseResolutionString += "x";
		baseResolutionString += std::to_string(resolutions.at(i).second);

		auto newBaseResolution =
		    std::make_pair(baseResolutionString.c_str(), data::value(baseResolutionString.c_str()));

		auto it = std::find_if(
		    baseResValues.begin(),
		    baseResValues.end(),
		    [&baseResolutionString](const settings_value value) {
			    return (value.second.value_str.compare(baseResolutionString) == 0);
		    });

		if (baseResValues.size() == 7 || it == baseResValues.end()) {
			baseResValues.push_back(newBaseResolution);
		}
	}

	// Check if the current resolution is in the available ones
	auto it = std::find_if(
	    baseResValues.begin(),
	    baseResValues.end(),
	    [&currentBaseRes](const settings_value value) {
		    return (value.second.value_str.compare(currentBaseRes) == 0);
	    });

	if (it == baseResValues.end())
		baseResValues.push_back(std::make_pair(currentBaseRes, data::value(currentBaseRes)));

	videoObjects.Set(indexVideo++, buildJSObject(
		"Base", "OBS_INPUT_RESOLUTION_LIST",
		"Base (Canvas) Resolution", "OBS_COMBO_FORMAT_STRING",
		data::value(currentBaseRes), baseResValues, env
	));

	uint64_t out_cx = config_get_uint(config, "Video", "OutputCX");
	uint64_t out_cy = config_get_uint(config, "Video", "OutputCY");
	auto currentOutputRes = ResString(out_cx, out_cy);

	videoObjects.Set(indexVideo++, buildJSObject(
		"Output", "OBS_INPUT_RESOLUTION_LIST",
		"Output (Scaled) Resolution", "OBS_COMBO_FORMAT_STRING",
		data::value(currentOutputRes), getOutputResolutions(base_cx, base_cy), env
	));
	videoObjects.Set(indexVideo++, buildJSObject(
		"ScaleType", "OBS_PROPERTY_LIST",
		"Downscale Filter", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(config, "Video", "ScaleType"))), {
			std::make_pair("Bilinear (Fastest, but blurry if scaling)", data::value("bilinear")),
			std::make_pair("Bicubic (Sharpened scaling, 16 samples)", data::value("bicubic")),
			std::make_pair("Lanczos (Sharpened scaling, 32 samples)", data::value("lanczos")),
		}, env
	));
	uint64_t fpsTypeValue = config_get_uint(config, "Video", "FPSType");
	switch (fpsTypeValue) {
		case 0: {
			videoObjects.Set(indexVideo++, buildJSObject(
				"FPSType", "OBS_PROPERTY_LIST",
				"FPS Type", "OBS_COMBO_FORMAT_STRING",
				data::value("Common FPS Values"), {
					std::make_pair("Common FPS Values", data::value("Common FPS Values")),
					std::make_pair("Integer FPS Value", data::value("Integer FPS Value")),
					std::make_pair("Fractional FPS Value", data::value("Fractional FPS Value")),
				}, env
			));
			videoObjects.Set(indexVideo++, buildJSObject(
				"FPSCommon", "OBS_PROPERTY_LIST",
				"Common FPS Values", "OBS_COMBO_FORMAT_STRING",
				data::value(getSafeOBSstr(config_get_string(config, "Video", "FPSCommon"))), {
					std::make_pair("10", data::value("10")),
					std::make_pair("20", data::value("20")),
					std::make_pair("24 NTSC", data::value("24 NTSC")),
					std::make_pair("29.97", data::value("29.97")),
					std::make_pair("30", data::value("30")),
					std::make_pair("48", data::value("48")),
					std::make_pair("59.94", data::value("59.94")),
					std::make_pair("60", data::value("60")),
				}, env
			));
			break;
		}
		case 1: {
			videoObjects.Set(indexVideo++, buildJSObject(
				"FPSType", "OBS_PROPERTY_LIST",
				"FPS Type", "OBS_COMBO_FORMAT_STRING",
				data::value("Integer FPS Value"), {
					std::make_pair("Common FPS Values", data::value("Common FPS Values")),
					std::make_pair("Integer FPS Value", data::value("Integer FPS Value")),
					std::make_pair("Fractional FPS Value", data::value("Fractional FPS Value")),
				}, env
			));
			videoObjects.Set(indexVideo++, buildJSObject(
				"FPSInt", "OBS_PROPERTY_UINT",
				"Integer FPS Value", "",
				data::value(config_get_uint(config, "Video", "FPSInt")), {}, env, 0, 120, 0
			));
			break;
		}
		case 2: {
			videoObjects.Set(indexVideo++, buildJSObject(
				"FPSType", "OBS_PROPERTY_LIST",
				"FPS Type", "OBS_COMBO_FORMAT_STRING",
				data::value("Fractional FPS Value"), {
					std::make_pair("Common FPS Values", data::value("Common FPS Values")),
					std::make_pair("Integer FPS Value", data::value("Integer FPS Value")),
					std::make_pair("Fractional FPS Value", data::value("Fractional FPS Value")),
				}, env
			));
			videoObjects.Set(indexVideo++, buildJSObject(
				"FPSNum", "OBS_PROPERTY_UINT",
				"FPSNum", "",
				data::value(config_get_uint(config, "Video", "FPSNum")), {}, env, 0, 1000000, 0
			));
			videoObjects.Set(indexVideo++, buildJSObject(
				"FPSDen", "OBS_PROPERTY_UINT",
				"FPSDen", "",
				data::value(config_get_uint(config, "Video", "FPSDen")), {}, env, 0, 1000000, 0
			));
			break;
		}
	}

	video.Set("nameSubCategory", Napi::String::New(env, "Untitled"));
	video.Set("parameters", videoObjects);
	subCategories.Set(indexSubCategories++, video);
	settings.Set("data", subCategories);
	settings.Set("type", Napi::Number::New(env, NODEOBS_CATEGORY_LIST));
}

void getAdvancedSettings(Napi::Object& settings, Napi::Env env)
{
	Napi::Array subCategories = Napi::Array::New(env);
	uint32_t indexSubCategories = 0;
	auto global_config = ConfigManager::getInstance().getGlobal();
	auto basic_config = ConfigManager::getInstance().getBasic();

#ifdef WIN32
	// General
	Napi::Object general = Napi::Object::New(env);
	Napi::Array generalObjects = Napi::Array::New(env);
	uint32_t indexGeneral = 0;

	const char* processPriorityCurrentValue =
	    config_get_string(global_config, "General", "ProcessPriority");
	if (processPriorityCurrentValue == NULL) {
		processPriorityCurrentValue = "Normal";
		config_set_string(global_config, "General", "ProcessPriority", processPriorityCurrentValue);
	}
	generalObjects.Set(indexGeneral++, buildJSObject(
		"ProcessPriority", "OBS_PROPERTY_LIST",
		"Process Priority", "OBS_COMBO_FORMAT_STRING",
		data::value(processPriorityCurrentValue), {
			std::make_pair("High", data::value("High")),
			std::make_pair("Above Normal", data::value("Above Normal")),
			std::make_pair("Normal", data::value("Normal")),
			std::make_pair("Below Normal", data::value("Below Normal")),
			std::make_pair("Idle", data::value("Idle")),
		}, env
	));

	general.Set("nameSubCategory", Napi::String::New(env, "General"));
	general.Set("parameters", generalObjects);
	subCategories.Set(indexSubCategories++, general);
#endif

	// Video
	Napi::Object video = Napi::Object::New(env);
	Napi::Array videoObjects = Napi::Array::New(env);
	uint32_t indexVideo = 0;

	videoObjects.Set(indexVideo++, buildJSObject(
		"ColorFormat", "OBS_PROPERTY_LIST",
		"Color Format", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(basic_config, "Video", "ColorFormat"))), {
			std::make_pair("NV12", data::value("NV12")),
			std::make_pair("I420", data::value("I420")),
			std::make_pair("I444", data::value("I444")),
			std::make_pair("RGB", data::value("RGB"))
		}, env
	));
	videoObjects.Set(indexVideo++, buildJSObject(
		"ColorSpace", "OBS_PROPERTY_LIST",
		"YUV Color Space", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(basic_config, "Video", "ColorSpace"))), {
			std::make_pair("601", data::value("601")),
			std::make_pair("709", data::value("709"))
		}, env
	));
	videoObjects.Set(indexVideo++, buildJSObject(
		"ColorRange", "OBS_PROPERTY_LIST",
		"YUV Color Range", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(basic_config, "Video", "ColorRange"))), {
			std::make_pair("Partial", data::value("Partial")),
			std::make_pair("Full", data::value("Full"))
		}, env
	));
	videoObjects.Set(indexVideo++, buildJSObject(
		"ForceGPUAsRenderDevice", "OBS_PROPERTY_BOOL",
		"Force GPU as render device", "",
		data::value(config_get_bool(basic_config, "Video", "ForceGPUAsRenderDevice")), {
			std::make_pair("Partial", data::value("Partial")),
			std::make_pair("Full", data::value("Full"))
		}, env
	));

	video.Set("nameSubCategory", Napi::String::New(env, "Video"));
	video.Set("parameters", videoObjects);
	subCategories.Set(indexSubCategories++, video);

	// Audio
	Napi::Object audio = Napi::Object::New(env);
	Napi::Array audioObjects = Napi::Array::New(env);
	uint32_t indexAudio = 0;

	std::vector<settings_value> audioDevices;
	audioDevices.push_back(std::make_pair("Default", data::value("Default")));
	auto enum_devices = [](void* param, const char* name, const char* id) {
		std::vector<settings_value>* audioDevices =
		    (std::vector<settings_value>*)param;
		audioDevices->push_back(std::make_pair(name, data::value(name)));
		return true;
	};
	obs_enum_audio_monitoring_devices(enum_devices, &audioDevices);
	audioObjects.Set(indexAudio++, buildJSObject(
		"MonitoringDeviceName", "OBS_PROPERTY_LIST",
		"Audio Monitoring Device", "OBS_COMBO_FORMAT_STRING",
		data::value(getSafeOBSstr(config_get_string(basic_config, "Audio", "MonitoringDeviceName"))),
		audioDevices, env
	));

#if defined(_WIN32)
	audioObjects.Set(indexAudio++, buildJSObject(
		"DisableAudioDucking", "OBS_PROPERTY_BOOL",
		"Disable Windows audio ducking", "",
		data::value(config_get_bool(basic_config, "Audio", "DisableAudioDucking")),
		{}, env
	));
#endif

	audio.Set("nameSubCategory", Napi::String::New(env, "Audio"));
	audio.Set("parameters", videoObjects);
	subCategories.Set(indexSubCategories++, audio);

	// Recording
	Napi::Object recording = Napi::Object::New(env);
	Napi::Array recordingObjects = Napi::Array::New(env);
	uint32_t indexRecording = 0;

	recordingObjects.Set(indexRecording++, buildJSObject(
		"FilenameFormatting", "OBS_PROPERTY_EDIT_TEXT",
		"Filename Formatting", "",
		data::value(getSafeOBSstr(config_get_string(basic_config, "Output", "FilenameFormatting"))),
		{}, env
	));
	recordingObjects.Set(indexRecording++, buildJSObject(
		"OverwriteIfExists", "OBS_PROPERTY_BOOL",
		"Overwrite if file exists", "",
		data::value(config_get_bool(basic_config, "Output", "OverwriteIfExists")),
		{}, env
	));

	recording.Set("nameSubCategory", Napi::String::New(env, "Recording"));
	recording.Set("parameters", recordingObjects);
	subCategories.Set(indexSubCategories++, recording);

	// Replay Buffer
	Napi::Object replayBuffer = Napi::Object::New(env);
	Napi::Array replayBufferObjects = Napi::Array::New(env);
	uint32_t indexReplayBuffer = 0;

	replayBufferObjects.Set(indexReplayBuffer++, buildJSObject(
		"RecRBPrefix", "OBS_PROPERTY_EDIT_TEXT",
		"Replay Buffer Filename Prefix", "",
		data::value(getSafeOBSstr(config_get_string(basic_config, "SimpleOutput", "RecRBPrefix"))),
		{}, env
	));
	replayBufferObjects.Set(indexReplayBuffer++, buildJSObject(
		"RecRBSuffix", "OBS_PROPERTY_EDIT_TEXT",
		"Replay Buffer Filename Suffix", "",
		data::value(getSafeOBSstr(config_get_string(basic_config, "SimpleOutput", "RecRBSuffix"))),
		{}, env
	));

	replayBuffer.Set("nameSubCategory", Napi::String::New(env, "Replay Buffer"));
	replayBuffer.Set("parameters", replayBufferObjects);
	subCategories.Set(indexSubCategories++, replayBuffer);

	//Stream Delay
	Napi::Object streamDelay = Napi::Object::New(env);
	Napi::Array streamDelayObjects = Napi::Array::New(env);
	uint32_t indexStreamDelay = 0;

	streamDelayObjects.Set(indexStreamDelay++, buildJSObject(
		"DelayEnable", "OBS_PROPERTY_BOOL",
		"Enable", "", data::value(config_get_bool(basic_config, "Output", "DelayEnable")),
		{}, env
	));
	streamDelayObjects.Set(indexStreamDelay++, buildJSObject(
		"DelaySec", "OBS_PROPERTY_INT",
		"Duration (seconds)", "", data::value(config_get_int(basic_config, "Output", "DelaySec")),
		{}, env
	));
	streamDelayObjects.Set(indexStreamDelay++, buildJSObject(
		"DelayPreserve", "OBS_PROPERTY_BOOL",
		"Preserved cutoff point (increase delay) when reconnecting", "",
		data::value(config_get_bool(basic_config, "Output", "DelayPreserve")),
		{}, env
	));

	streamDelay.Set("nameSubCategory", Napi::String::New(env, "Stream Delay"));
	streamDelay.Set("parameters", streamDelayObjects);
	subCategories.Set(indexSubCategories++, streamDelay);

	// Automatically Reconnect
	Napi::Object autoReconnect = Napi::Object::New(env);
	Napi::Array autoReconnectObjects = Napi::Array::New(env);
	uint32_t indexAutoReconnect = 0;

	autoReconnectObjects.Set(indexAutoReconnect++, buildJSObject(
		"Reconnect", "OBS_PROPERTY_BOOL",
		"Enable", "",
		data::value(config_get_bool(basic_config, "Output", "Reconnect")),
		{}, env
	));
	autoReconnectObjects.Set(indexAutoReconnect++, buildJSObject(
		"RetryDelay", "OBS_PROPERTY_INT",
		"Retry Delay (seconds)", "",
		data::value(config_get_int(basic_config, "Output", "RetryDelay")),
		{}, env, 0, 30, 0
	));
	autoReconnectObjects.Set(indexAutoReconnect++, buildJSObject(
		"MaxRetries", "OBS_PROPERTY_INT",
		"Maximum Retries", "",
		data::value(config_get_int(basic_config, "Output", "MaxRetries")),
		{}, env, 0, 10000, 0
	));

	autoReconnect.Set("nameSubCategory", Napi::String::New(env, "Automatically Reconnect"));
	autoReconnect.Set("parameters", autoReconnectObjects);
	subCategories.Set(indexSubCategories++, autoReconnect);

	// Network
	Napi::Object network = Napi::Object::New(env);
	Napi::Array networkObjects = Napi::Array::New(env);
	uint32_t indexNetwork = 0;

	std::vector<settings_value> bindIPValues;
	obs_properties_t* ppts = obs_get_output_properties("rtmp_output");
	obs_property_t* p = obs_properties_get(ppts, "bind_ip");
	size_t count = obs_property_list_item_count(p);
	for (size_t i = 0; i < count; i++) {
		const char* name = obs_property_list_item_name(p, i);
		const char* val  = obs_property_list_item_string(p, i);

		bindIPValues.push_back(std::make_pair(name, data::value(val)));
	}

	networkObjects.Set(indexNetwork++, buildJSObject(
		"BindIP", "OBS_PROPERTY_LIST",
		"Bind to IP", "OBS_COMBO_FORMAT_STRING",
		data::value(config_get_bool(basic_config, "Output", "BindIP")),
		bindIPValues, env
	));
	networkObjects.Set(indexNetwork++, buildJSObject(
		"DynamicBitrate", "OBS_PROPERTY_BOOL",
		"Dynamically change bitrate when dropping frames while streaming", "",
		data::value(config_get_bool(basic_config, "Output", "DynamicBitrate")),
		{}, env
	));

#ifdef WIN32
	networkObjects.Set(indexNetwork++, buildJSObject(
		"NewSocketLoopEnable", "OBS_PROPERTY_BOOL",
		"Enable new networking code", "",
		data::value(config_get_bool(basic_config, "Output", "NewSocketLoopEnable")),
		{}, env
	));
	networkObjects.Set(indexNetwork++, buildJSObject(
		"LowLatencyEnable", "OBS_PROPERTY_BOOL",
		"Low latency mode", "",
		data::value(config_get_bool(basic_config, "Output", "LowLatencyEnable")),
		{}, env
	));
#endif

	network.Set("nameSubCategory", Napi::String::New(env, "Network"));
	network.Set("parameters", networkObjects);
	subCategories.Set(indexSubCategories++, network);

	// Sources
	Napi::Object sources = Napi::Object::New(env);
	Napi::Array sourcesObjects = Napi::Array::New(env);
	uint32_t indexSources = 0;

	networkObjects.Set(indexNetwork++, buildJSObject(
		"browserHWAccel", "OBS_PROPERTY_BOOL",
		"Enable Browser Source Hardware Acceleration (requires a restart)", "",
		data::value(config_get_bool(basic_config, "General", "browserHWAccel")),
		{}, env
	));

	sources.Set("nameSubCategory", Napi::String::New(env, "Sources"));
	sources.Set("parameters", sourcesObjects);
	subCategories.Set(indexSubCategories++, sources);

	//Media Files
	Napi::Object fileCaching = Napi::Object::New(env);
	Napi::Array fileCachingObjects = Napi::Array::New(env);
	uint32_t indexFileCaching = 0;

	fileCachingObjects.Set(indexFileCaching++, buildJSObject(
		"fileCaching", "OBS_PROPERTY_BOOL",
		"Enable media file caching", "",
		data::value(config_get_bool(basic_config, "General", "fileCaching")),
		{}, env
	));

	fileCaching.Set("nameSubCategory", Napi::String::New(env, "Media Files"));
	fileCaching.Set("parameters", fileCachingObjects);
	subCategories.Set(indexSubCategories++, fileCaching);

	settings.Set("data", subCategories);
	settings.Set("type", Napi::Number::New(env, NODEOBS_CATEGORY_LIST));
}

Napi::Value settings::OBS_settings_getSettings(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
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
	}
	else if (category.compare("Output") == 0) {
		getOutputSettings(settings, info.Env());
	} else if (category.compare("Audio") == 0) {
		getAudioSettings(settings, info.Env());
	} 
	else if (category.compare("Video") == 0) {
		getVideoSettings(settings, info.Env());
	}
	else if (category.compare("Advanced") == 0) {
		getAdvancedSettings(settings, info.Env());
	}

	return settings;
}

bool saveStreamSettings(const Napi::Array& streamSettings)
{
	obs_service_t* currentService = OBS_service::getService();
	if (!obs_service_is_ready_to_update(currentService))
		return false;

	obs_data_t* settings = nullptr;

	std::string currentStreamType = obs_service_get_type(currentService);
	std::string newserviceTypeValue;

	std::string currentServiceName =
		getSafeOBSstr(obs_data_get_string(obs_service_get_settings(currentService), "service"));
	std::string newServiceValue;

	bool serviceChanged = false;
	bool serviceTypeChanged = false;
	bool serviceSettingsInvalid = false;

	for (int i = 0; i < streamSettings.Length(); i++) {
		Napi::Object subCategoryObject = streamSettings.Get(i).ToObject();
		Napi::Array parameters = subCategoryObject.Get("parameters").As<Napi::Array>();

		for (int j = 0; j < parameters.Length(); j++) {
			Napi::Object parameterObject = parameters.Get(j).ToObject();
			auto name = parameterObject.Get("name").ToString().Utf8Value();
			auto type = parameterObject.Get("type").ToString().Utf8Value();

			if (type.compare("OBS_PROPERTY_LIST") == 0 || type.compare("OBS_PROPERTY_EDIT_TEXT") == 0) {
				std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();

				if (name.compare("streamType") == 0) {
					if (value.size() == 0) {
						serviceSettingsInvalid = true;
						break;
					}
					newserviceTypeValue = value;
					settings            = obs_service_defaults(newserviceTypeValue.c_str());
					if (currentStreamType.compare(newserviceTypeValue) != 0) {
						serviceTypeChanged = true;
					}
				}

				if (name.compare("service") == 0) {
					if (value.size() == 0) {
						serviceSettingsInvalid = true;
						break;
					}
					newServiceValue = value;
					if (currentServiceName.compare(newServiceValue) != 0) {
						serviceChanged = true;
					}
				}
				obs_data_set_string(settings, name.c_str(), value.c_str());
			} else if (type.compare("OBS_PROPERTY_INT") == 0 || type.compare("OBS_PROPERTY_UINT") == 0) {
				int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();
				obs_data_set_int(settings, name.c_str(), value);
			} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
				bool value = parameterObject.Get("currentValue").ToBoolean().Value();
				obs_data_set_bool(settings, name.c_str(), value);
			} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();
				obs_data_set_double(settings, name.c_str(), value);
			}
		}
	}

	if (serviceSettingsInvalid) {
		if (settings)
			obs_data_release(settings);
		return false;
	}

	if (serviceTypeChanged) {
		settings = obs_service_defaults(newserviceTypeValue.c_str());

		if (newserviceTypeValue.compare("rtmp_common") == 0) {
			obs_data_set_string(settings, "streamType", "rtmp_common");
			obs_data_set_string(settings, "service", "Twitch");
			obs_data_set_bool(settings, "show_all", 0);
			obs_data_set_string(settings, "server", "auto");
			obs_data_set_string(settings, "key", "");
		}
	}

	obs_data_t* hotkeyData = obs_hotkeys_save_service(currentService);

	obs_service_t* newService =
	    obs_service_create(newserviceTypeValue.c_str(), "default_service", settings, hotkeyData);

	if (serviceChanged) {
		std::string server = getSafeOBSstr(obs_data_get_string(settings, "server"));
		bool        serverFound = false;
		std::string defaultServer;

		// Check if server is valid
		obs_properties_t* properties = obs_service_properties(newService);
		obs_property_t*   property   = obs_properties_first(properties);

		while (property) {
			std::string name = obs_property_name(property);

			if (name.compare("server") == 0) {
				int count = (int)obs_property_list_item_count(property);
				int i     = 0;

				while (i < count && !serverFound) {
					std::string value = obs_property_list_item_string(property, i);

					if (i == 0)
						defaultServer = value;

					if (value.compare(server) == 0)
						serverFound = true;

					i++;
				}
			}
			obs_property_next(&property);
		}

		if (!serverFound && defaultServer.compare("") != 0) {
			// Server not found, we set the default server
			obs_data_set_string(settings, "server", defaultServer.c_str());
			obs_service_update(newService, settings);
		}
	}

	obs_data_release(hotkeyData);

	OBS_service::setService(newService);

	obs_data_t* data = obs_data_create();
	obs_data_set_string(data, "type", obs_service_get_type(newService));
	obs_data_set_obj(data, "settings", settings);

	if (!obs_data_save_json_safe(data, ConfigManager::getInstance().getService().c_str(), "tmp", "bak")) {
		blog(LOG_WARNING, "Failed to save service");
	}

	obs_data_release(hotkeyData);
	obs_data_release(data);
	return true;
}

void saveAdvancedOutputStreamingSettings(const Napi::Array& settings)
{
	int indexStreamingCategory = 1;

	std::string section = "AdvOut";

	obs_encoder_t* encoder         = OBS_service::getStreamingEncoder();
	obs_data_t*    encoderSettings = obs_encoder_get_settings(encoder);
	int indexEncoderSettings = 4;

	obs_data_t *service_settings = obs_service_get_settings(OBS_service::getService());
	std::string serviceName = getSafeOBSstr(obs_data_get_string(service_settings, "service"));
	obs_data_release(service_settings);

	if (serviceName.compare("Twitch") == 0)
		indexEncoderSettings++;

	bool newEncoderType = false;

	Napi::Object subCategoryObject = settings.Get(indexStreamingCategory).ToObject();
	Napi::Array parameters = subCategoryObject.Get("parameters").As<Napi::Array>();

	for (int i = 0; i < parameters.Length(); i++) {
		Napi::Object parameterObject = parameters.Get(i).ToObject();
		auto name = parameterObject.Get("name").ToString().Utf8Value();
		auto type = parameterObject.Get("type").ToString().Utf8Value();

		if (type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || type.compare("OBS_PROPERTY_PATH") == 0
		    || type.compare("OBS_PROPERTY_TEXT") == 0 || type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
			std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();
			if (i < indexEncoderSettings) {
				config_set_string(
				    ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value.c_str());
			} else {
				obs_data_set_string(encoderSettings, name.c_str(), value.c_str());
			}
		} else if (type.compare("OBS_PROPERTY_INT") == 0) {
			int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();
			if (i < indexEncoderSettings) {
				config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_UINT") == 0) {
			uint64_t value = uint64_t(parameterObject.Get("currentValue").ToNumber().Uint32Value());
			if (i < indexEncoderSettings) {
				config_set_uint(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
			bool value = parameterObject.Get("currentValue").ToBoolean().Value();
			if (i < indexEncoderSettings) {
				if (name.compare("Rescale") == 0 && value ||
					name.compare("VodTrackEnabled") == 0 && value) {
					indexEncoderSettings++;
				}
				config_set_bool(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_bool(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0 || type.compare("OBS_PROPERTY_FLOAT") == 0) {
			double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();
			if (i < indexEncoderSettings) {
				config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_double(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_LIST") == 0) {
			auto subType = parameterObject.Get("subType").ToString().Utf8Value();
			if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
				int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();
				if (i < indexEncoderSettings) {
					config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
				} else {
					obs_data_set_int(encoderSettings, name.c_str(), value);
				}
			} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
				double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();
				if (i < indexEncoderSettings) {
					config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
				} else {
					obs_data_set_double(encoderSettings, name.c_str(), value);
				}
			} else {
				std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();
				if (i < indexEncoderSettings) {
					if (name.compare("Encoder") == 0) {
						const char* currentEncoder =
						    config_get_string(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str());
						if (currentEncoder != NULL)
							newEncoderType = value.compare(currentEncoder) != 0;
					}
					config_set_string(
					    ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value.c_str());
				} else {
					obs_data_set_string(encoderSettings, name.c_str(), value.c_str());
				}
			}
		} else {
			std::cout << "type not found ! " << type.c_str() << std::endl;
		}
	}

#ifdef WIN32
	bool dynamicBitrate = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "DynamicBitrate");
	std::string encoderID = getSafeOBSstr(config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "Encoder"));

	if (dynamicBitrate && encoderID.compare(ENCODER_NEW_NVENC) == 0)
		obs_data_set_bool(encoderSettings, "lookahead", false);
#elif __APPLE__
	bool applyServiceSettings = config_get_bool(ConfigManager::getInstance().getBasic(), "Output", "ApplyServiceSettings");
	std::string encoderID = getSafeOBSstr(config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "Encoder"));

	if (!applyServiceSettings && encoderID.compare(APPLE_HARDWARE_VIDEO_ENCODER) == 0)
		config_set_bool(ConfigManager::getInstance().getBasic(), "AdvOut", "ApplyServiceSettings", true);
#endif

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);

	if (newEncoderType) {
		encoderSettings = obs_encoder_defaults(
		    config_get_string(ConfigManager::getInstance().getBasic(), section.c_str(), "Encoder"));
	}

	obs_encoder_update(encoder, encoderSettings);

	if (!obs_data_save_json_safe(encoderSettings, ConfigManager::getInstance().getStream().c_str(), "tmp", "bak")) {
		blog(LOG_WARNING, "Failed to save encoder %s", ConfigManager::getInstance().getStream().c_str());
	}
}

void saveAdvancedOutputRecordingSettings(const Napi::Array& settings)
{
	int indexRecordingCategory = 2;
	std::string section = "AdvOut";

	obs_encoder_t* encoder         = OBS_service::getRecordingEncoder();
	obs_data_t*    encoderSettings = obs_encoder_get_settings(encoder);

	size_t indexEncoderSettings = 8;

	bool newEncoderType = false;
	std::string currentFormat;

	Napi::Object subCategoryObject = settings.Get(indexRecordingCategory).ToObject();
	Napi::Array parameters = subCategoryObject.Get("parameters").As<Napi::Array>();

	for (int i = 0; i < parameters.Length(); i++) {
		Napi::Object parameterObject = parameters.Get(i).ToObject();
		auto name = parameterObject.Get("name").ToString().Utf8Value();
		auto type = parameterObject.Get("type").ToString().Utf8Value();

		if (i >= indexEncoderSettings) {
			name.erase(0, strlen("Rec"));
		}

		if (name.compare("RecType") == 0) {
			std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();
			if (value.compare("Custom Output (FFmpeg)") == 0) {
				indexEncoderSettings = parameters.Length();
			}
		}

		if (type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || type.compare("OBS_PROPERTY_PATH") == 0
		    || type.compare("OBS_PROPERTY_TEXT") == 0 || type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
			std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();
			if (i < indexEncoderSettings) {
				config_set_string(
				    ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value.c_str());
			} else {
				obs_data_set_string(encoderSettings, name.c_str(), value.c_str());
			}
		} else if (type.compare("OBS_PROPERTY_INT") == 0) {
			int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();
			if (i < indexEncoderSettings) {
				config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_UINT") == 0 || type.compare("OBS_PROPERTY_BITMASK") == 0) {
			uint64_t value = uint64_t(parameterObject.Get("currentValue").ToNumber().Uint32Value());

			// Use the first audio track if multitrack isnt supported
			if (name.compare("RecTracks") == 0 && !IsMultitrackAudioSupported(currentFormat.c_str())) {
				value = 1;
			}
			
			if (i < indexEncoderSettings) {
				config_set_uint(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
			bool value = parameterObject.Get("currentValue").ToBoolean().Value();
			if (i < indexEncoderSettings) {
				if (name.compare("RecRescale") == 0 && value) {
					indexEncoderSettings++;
				}
				config_set_bool(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_bool(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0 || type.compare("OBS_PROPERTY_FLOAT") == 0) {
			double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();
			if (i < indexEncoderSettings) {
				config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_double(encoderSettings, name.c_str(), value);
			}
		} else if (type.compare("OBS_PROPERTY_LIST") == 0) {
			auto subType = parameterObject.Get("subType").ToString().Utf8Value();
			if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
				int64_t value = parameterObject.Get("currentValue").ToNumber().Int64Value();
				if (i < indexEncoderSettings) {
					config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
				} else {
					obs_data_set_int(encoderSettings, name.c_str(), value);
				}
			} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
				double value = parameterObject.Get("currentValue").ToNumber().DoubleValue();
				if (i < indexEncoderSettings) {
					config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value);
				} else {
					obs_data_set_double(encoderSettings, name.c_str(), value);
				}
			} else {
				std::string value = parameterObject.Get("currentValue").ToString().Utf8Value();
				if (i < indexEncoderSettings) {
					if (name.compare("RecEncoder") == 0) {
						const char* currentEncoder =
						    config_get_string(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str());

						if (currentEncoder != NULL)
							newEncoderType = value.compare(currentEncoder) != 0;
					}
					if (name.compare("RecFormat") == 0) {
						currentFormat = value;
					}
					config_set_string(
					    ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value.c_str());
				} else {
					obs_data_set_string(encoderSettings, name.c_str(), value.c_str());
				}
			}
		} else {
			std::cout << "type not found ! " << type.c_str() << std::endl;
		}
	}

	int ret = config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);

	if (newEncoderType)
		encoderSettings = obs_encoder_defaults(
		    config_get_string(ConfigManager::getInstance().getBasic(), section.c_str(), "RecEncoder"));

	obs_encoder_update(encoder, encoderSettings);

	if (!obs_data_save_json_safe(encoderSettings, ConfigManager::getInstance().getRecord().c_str(), "tmp", "bak")) {
		blog(LOG_WARNING, "Failed to save encoder %s", ConfigManager::getInstance().getRecord().c_str());
	}
}

void saveAdvancedOutputSettings(const Napi::Array& settings, Napi::Env env)
{
	// Streaming
	if (!obs_output_active(OBS_service::getStreamingOutput()))
		saveAdvancedOutputStreamingSettings(settings);

	// Recording
	if (!obs_output_active(OBS_service::getRecordingOutput()))
		saveAdvancedOutputRecordingSettings(settings);

	// Audio
	if (settings.Length() > 3) {
		Napi::Array audioSettings = Napi::Array::New(env);;
		int indexTrack = 3;

		for (int i = 0; i < 6; i++) {
			audioSettings.Set(i, settings.Get(i + indexTrack));
		}

		// Update the current audio settings, limiting them if necessary
		currentAudioSettings = new Napi::Array(env, audioSettings);
		UpdateAudioSettings(false, env);
	}

	// Replay buffer
	Napi::Array replayBufferSettings = Napi::Array::New(env);
	replayBufferSettings.Set((uint32_t)0, settings.Get((uint32_t)9));
	saveGenericSettings(replayBufferSettings, "AdvOut", ConfigManager::getInstance().getBasic());
}

void saveOutputSettings(const Napi::Array& settings, Napi::Env env)
{
	// Get selected output mode
	Napi::Object outputObject = settings.Get((uint32_t)0).ToObject();
	Napi::Array parametersOutput = outputObject.Get("parameters").As<Napi::Array>();
	Napi::Object parameterOutputObject = parametersOutput.Get((uint32_t)0).ToObject();
	auto outputMode = parameterOutputObject.Get("currentValue").ToString().Utf8Value();

	std::string currentOutputMode = getSafeOBSstr(config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode"));

	if (outputMode.compare(currentOutputMode) != 0) {
		config_set_string(ConfigManager::getInstance().getBasic(), "Output", "Mode", outputMode.c_str());
		config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
		return;
	}

	if (outputMode.compare("Advanced") == 0)
		saveAdvancedOutputSettings(settings, env);
	else
		saveGenericSettings(settings, "SimpleOutput", ConfigManager::getInstance().getBasic());
}

struct BaseLexer
{
	lexer lex;

	public:
	inline BaseLexer()
	{
		lexer_init(&lex);
	}
	inline ~BaseLexer()
	{
		lexer_free(&lex);
	}
	operator lexer*()
	{
		return &lex;
	}
};

// parses "[width]x[height]", string, i.e. 1024x768

static bool ConvertResText(const char* res, uint32_t& cx, uint32_t& cy)
{
	BaseLexer  lex;
	base_token token;

	lexer_start(lex, res);

	// parse width
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	try {
		cx = std::stoul(token.text.array);
	} catch (const std::exception&) {
		return false;
	}

	// parse 'x'
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (strref_cmpi(&token.text, "x") != 0)
		return false;

	// parse height
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	try {
		cy = std::stoul(token.text.array);
	} catch (const std::exception&) {
		return false;
	}

	// shouldn't be any more tokens after this
	if (lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;

	return true;
}

void saveVideoSettings(const Napi::Array& settings)
{
	Napi::Object subCategoryObject = settings.Get((uint32_t)0).ToObject();
	Napi::Array parameters = subCategoryObject.Get("parameters").As<Napi::Array>();

	//Base resolution
	Napi::Object baseRes = parameters.Get((uint32_t)0).ToObject();
	std::string baseResString = baseRes.Get("currentValue").ToString().Utf8Value();
	uint32_t baseWidth = 0, baseHeight = 0;

	if (ConvertResText(baseResString.c_str(), baseWidth, baseHeight)) {
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCX", baseWidth);
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCY", baseHeight);
	}

	//Output resolution
	Napi::Object outputRes = parameters.Get((uint32_t)1).ToObject();
	std::string outputResString = outputRes.Get("currentValue").ToString().Utf8Value();
	uint32_t outputWidth = 0, outputHeight = 0;

	if (ConvertResText(outputResString.c_str(), outputWidth, outputHeight)) {
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCX", outputWidth);
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCY", outputHeight);
	}

	Napi::Object scaleParameter = parameters.Get((uint32_t)2).ToObject();
	std::string scaleString = scaleParameter.Get("currentValue").ToString().Utf8Value();
	config_set_string(ConfigManager::getInstance().getBasic(), "Video", "ScaleType", scaleString.c_str());

	Napi::Object fpsType = parameters.Get((uint32_t)3).ToObject();
	std::string fpsTypeString = fpsType.Get("currentValue").ToString().Utf8Value();

	if (fpsTypeString.compare("Common FPS Values") == 0) {
		if (config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType") != 0) {
			config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType", 0);
		} else {
			Napi::Object fpsCommon = parameters.Get((uint32_t)4).ToObject();
			std::string fpsCommonString = fpsCommon.Get("currentValue").ToString().Utf8Value();
			config_set_string(ConfigManager::getInstance().getBasic(), "Video", "FPSCommon", fpsCommonString.c_str());
		}
	} else if (fpsTypeString.compare("Integer FPS Value") == 0) {
		if (config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType") != 1) {
			config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType", 1);
		} else {
			Napi::Object fpsInt = parameters.Get((uint32_t)4).ToObject();
			uint64_t fpsIntValue = uint64_t(fpsInt.Get("currentValue").ToNumber().Uint32Value());
			if (fpsIntValue > 0 && fpsIntValue < 500) {
				config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSInt", fpsIntValue);
			}
		}
	} else if (fpsTypeString.compare("Fractional FPS Value") == 0) {
		if (config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType") != 2) {
			config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType", 2);
		} else {
			Napi::Object fpsNum = parameters.Get((uint32_t)4).ToObject();
			uint32_t fpsNumValue = fpsNum.Get("currentValue").ToNumber().Uint32Value();

			if (fpsNumValue > 0 && fpsNumValue < 500) {
				config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSNum", fpsNumValue);
			}

			if (parameters.Length() > 5) {
				Napi::Object fpsDen = parameters.Get((uint32_t)5).ToObject();
				uint32_t fpsDenValue = fpsDen.Get("currentValue").ToNumber().Uint32Value();
				if (fpsDenValue > 0)
					config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSDen", fpsDenValue);
			}
		}
	}

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
}

void saveAdvancedSettings(const Napi::Array& settings, Napi::Env env)
{
	uint32_t index = 0;
#ifdef WIN32
	//General
	Napi::Array generalAdvancedSettings = Napi::Array::New(env);
	generalAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(generalAdvancedSettings, "General", ConfigManager::getInstance().getGlobal());
#endif

	//Video
	Napi::Array videoAdvancedSettings = Napi::Array::New(env);
	videoAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(videoAdvancedSettings, "Video", ConfigManager::getInstance().getBasic());

	//Audio
	Napi::Array audioAdvancedSettings = Napi::Array::New(env);
	audioAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(audioAdvancedSettings, "Audio", ConfigManager::getInstance().getBasic());

	//Recording
	Napi::Array recordingAdvancedSettings = Napi::Array::New(env);
	recordingAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(recordingAdvancedSettings, "Output", ConfigManager::getInstance().getBasic());

	//Replay buffer
	Napi::Array replayBufferAdvancedSettings = Napi::Array::New(env);
	replayBufferAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(replayBufferAdvancedSettings, "SimpleOutput", ConfigManager::getInstance().getBasic());

	//Stream Delay
	Napi::Array stresmDelayAdvancedSettings = Napi::Array::New(env);
	stresmDelayAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(stresmDelayAdvancedSettings, "Output", ConfigManager::getInstance().getBasic());

	//Automatically Reconnect
	Napi::Array automaticallyReconnectAdvancedSettings = Napi::Array::New(env);
	automaticallyReconnectAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(automaticallyReconnectAdvancedSettings, "Output", ConfigManager::getInstance().getBasic());

	//Network
	Napi::Array networkAdvancedSettings = Napi::Array::New(env);
	networkAdvancedSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(networkAdvancedSettings, "Output", ConfigManager::getInstance().getBasic());

	//Sources
	Napi::Array sourcesSettings = Napi::Array::New(env);
	sourcesSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(sourcesSettings, "General", ConfigManager::getInstance().getGlobal());

	//Media Files
	Napi::Array mediaFilesSettings = Napi::Array::New(env);
	mediaFilesSettings.Set((uint32_t)0, settings.Get(index++));
	saveGenericSettings(mediaFilesSettings, "General", ConfigManager::getInstance().getGlobal());
	MemoryManager::GetInstance().updateSourcesCache();
}

void saveAudioSettings(const Napi::Array& settings)
{
	Napi::Object subCategoryObject = settings.Get((uint32_t)0).ToObject();
	Napi::Array parameters = subCategoryObject.Get("parameters").As<Napi::Array>();

	Napi::Object sampleRate = parameters.Get((uint32_t)0).ToObject();
	uint64_t valueSampleRate = uint64_t(sampleRate.Get("currentValue").ToNumber().Uint32Value());
	config_set_uint(ConfigManager::getInstance().getBasic(), "Audio", "SampleRate", valueSampleRate);

	Napi::Object channels = parameters.Get((uint32_t)1).ToObject();
	std::string valueChannels = channels.Get("currentValue").ToString().Utf8Value();
	config_set_string(ConfigManager::getInstance().getBasic(), "Audio", "ChannelSetup", valueChannels.c_str());

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
}

void settings::OBS_settings_saveSettings(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	std::string category = info[0].ToString().Utf8Value();
	Napi::Array settings = info[1].As<Napi::Array>();

	if (category.compare("General") == 0) {
		saveGenericSettings(settings, "BasicWindow", ConfigManager::getInstance().getGlobal());
	} else if (category.compare("Stream") == 0) {
		if (saveStreamSettings(settings)) {
			OBS_service::updateService();
		}
	} else if (category.compare("Output") == 0) {
		saveOutputSettings(settings, info.Env());
	} else if (category.compare("Audio") == 0) {
		saveAudioSettings(settings);
	} else if (category.compare("Video") == 0) {
		saveVideoSettings(settings);
		OBS_service::resetVideoContext();
	} else if (category.compare("Advanced") == 0) {
		saveAdvancedSettings(settings, info.Env());

		if (!OBS_service::isStreamingOutputActive())
			OBS_service::resetVideoContext();

		OBS_API::setAudioDeviceMonitoring();
	}
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
	PROFINY_SCOPE
	Napi::Array categories = Napi::Array::New(info.Env());
	std::vector<std::string> settings = getListCategories();

	size_t index = 0;
	for (auto &category: settings)
		categories.Set(index++, Napi::String::New(info.Env(), category));

	return categories;
}

Napi::Array devices_to_js(const Napi::CallbackInfo& info, const std::vector<settings::DeviceInfo> &data)
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

std::vector<settings::DeviceInfo> getDevices(const char* source_id, const char* property_name)
{
	std::vector<settings::DeviceInfo> devices;

	auto settings = obs_get_source_defaults(source_id);
	if (!settings)
		return devices;

	const char* dummy_device_name = "does_not_exist";
	obs_data_set_string(settings, property_name, dummy_device_name);
	if (strcmp(source_id, "dshow_input") == 0) {
		obs_data_set_string(settings, "video_device_id", dummy_device_name);
		obs_data_set_string(settings, "audio_device_id", dummy_device_name);
	}

	auto dummy_source = obs_source_create(source_id, dummy_device_name, settings, nullptr);
	if (!dummy_source)
		return devices;

	auto props = obs_source_properties(dummy_source);
	if (!props)
		return devices;

	auto prop = obs_properties_get(props, property_name);
	if (!prop)
		return devices;

	size_t size = obs_property_list_item_count(prop);
	for (size_t idx = 0; idx < size; idx++) {
		const char* description = obs_property_list_item_name(prop, idx);
		const char* device_id = obs_property_list_item_string(prop, idx);

		if (!description || !strcmp(description, "") ||
			!device_id || !strcmp(device_id, ""))
			continue;

		devices.push_back({
			description,
			device_id
		});
	}

	obs_properties_destroy(props);
	obs_data_release(settings);
	obs_source_release(dummy_source);

	return devices;
}

std::vector<settings::DeviceInfo> getInputAudioDevices()
{
#ifdef WIN32
	const char* source_id = "wasapi_input_capture";
#elif __APPLE__
	const char* source_id = "coreaudio_input_capture";
#endif

	return getDevices(source_id, "device_id");
}

std::vector<settings::DeviceInfo> getOutputAudioDevices()
{
#ifdef WIN32
	const char* source_id = "wasapi_output_capture";
#elif __APPLE__
	const char* source_id = "coreaudio_output_capture";
#endif

	return getDevices(source_id, "device_id");
}

std::vector<settings::DeviceInfo> getVideoDevices()
{
#ifdef WIN32
	const char* source_id = "dshow_input";
	const char* property_name = "video_device_id";
#elif __APPLE__
	const char* source_id = "av_capture_input";
	const char* property_name = "device";
#endif

	return getDevices(source_id, property_name);
}

Napi::Value settings::OBS_settings_getInputAudioDevices(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return devices_to_js(info, getInputAudioDevices());
}

Napi::Value settings::OBS_settings_getOutputAudioDevices(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return devices_to_js(info, getOutputAudioDevices());
}

Napi::Value settings::OBS_settings_getVideoDevices(const Napi::CallbackInfo& info)
{
	PROFINY_SCOPE
	return devices_to_js(info, getVideoDevices());
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