#include "nodeobs_settings.h"

#include <windows.h>
vector<const char*> tabStreamTypes;
const char* currentServiceName;

/* some nice default output resolution vals */
static const double vals[] =
{
	1.0,
	1.25,
	(1.0/0.75),
	1.5,
	(1.0/0.6),
	1.75,
	2.0,
	2.25,
	2.5,
	2.75,
	3.0
};

static const size_t numVals = sizeof(vals)/sizeof(double);

static string ResString(uint32_t cx, uint32_t cy)
{
	ostringstream res;
	res << cx << "x" << cy;
	return res.str();
}

OBS_settings::OBS_settings()
{

}
OBS_settings::~OBS_settings()
{

}

void OBS_settings::OBS_settings_getListCategories(const FunctionCallbackInfo<Value>& args)
{
	args.GetReturnValue().Set(getListCategories());
}

void OBS_settings::OBS_settings_getSettings(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string nameCategory = std::string(*param1);

	args.GetReturnValue().Set(getSettings(nameCategory));
}

void OBS_settings::OBS_settings_saveSettings(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string nameCategory = std::string(*param1);

	Local<Array> generalSettings = v8::Local<v8::Array>::Cast(args[1]);

	v8::String::Utf8Value param3(args[2]->ToString());
	std::string pathConfigDirectory = std::string(*param3);

	saveSettings(nameCategory, generalSettings);
}

static bool compareStringValue (std::pair<std::string, std::string> value) {
	if (value.first.compare("currentValue") == 0) {
		return true;
	} else {
		return false;
	}
}

Local<Object> OBS_settings::serializeSettingsData(std::string nameSubCategory,
												  std::vector<std::vector<std::pair<std::string, std::string>>> entries, 
												  config_t* config, std::string section, bool isVisible, bool isEnabled)
{
	Isolate *isolate = v8::Isolate::GetCurrent();

	Local<Array> subCategoryParameters = Array::New(isolate);
	Local<Object> subCategory = Object::New(isolate);

	for(int i=0;i<entries.size();i++) {
		Local<Object> parameter = Object::New(isolate);

		std::string name = entries.at(i).at(0).second.c_str();
		parameter->Set(String::NewFromUtf8(isolate, entries.at(i).at(0).first.c_str()),
		 				String::NewFromUtf8(isolate, entries.at(i).at(0).second.c_str()));

		std::string type = entries.at(i).at(1).second.c_str();
		parameter->Set(String::NewFromUtf8(isolate, entries.at(i).at(1).first.c_str()),
		 				String::NewFromUtf8(isolate, entries.at(i).at(1).second.c_str()));

		parameter->Set(String::NewFromUtf8(isolate, entries.at(i).at(2).first.c_str()),
		 				String::NewFromUtf8(isolate, entries.at(i).at(2).second.c_str()));

		std::string currentValue;
		if(entries.at(i).size() > 3) {
			currentValue = entries.at(i).at(3).first.c_str();
		}

	  	if(!currentValue.empty() && currentValue.compare("currentValue") == 0 ) {
	  		parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, entries.at(i).at(3).second.c_str()));
	  		entries.at(i).erase(entries.at(i).begin() + 3);
	  	} else {
			if(type.compare("OBS_PROPERTY_LIST") == 0 ||
				type.compare("OBS_PROPERTY_PATH") == 0 ||
				type.compare("OBS_PROPERTY_EDIT_PATH") == 0 ||
				type.compare("OBS_PROPERTY_EDIT_TEXT") == 0) {
				const char* currentValue = config_get_string(config, section.c_str(), name.c_str());

				if(currentValue != NULL) {
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, currentValue));
				} else {
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, ""));
				}
			} else if(type.compare("OBS_PROPERTY_INT") == 0) {
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Integer::New(isolate, config_get_int(config, section.c_str(), name.c_str())));		
			} else if(type.compare("OBS_PROPERTY_UINT") == 0) {
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Integer::New(isolate, config_get_uint(config, section.c_str(), name.c_str())));
			} else if(type.compare("OBS_PROPERTY_BOOL") == 0) {
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Boolean::New(isolate, config_get_bool(config, section.c_str(), name.c_str())));
			} else if(type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Integer::New(isolate, config_get_double(config, section.c_str(), name.c_str())));
			}
		}
		if(entries.at(i).size() > 3) {
			Local<Array> values = Array::New(isolate);
			for(int j=3;j<entries.at(i).size();j++) {
				Local<Object> value = Object::New(isolate);
				value->Set(String::NewFromUtf8(isolate, entries.at(i).at(j).first.c_str()), 
							String::NewFromUtf8(isolate, entries.at(i).at(j).second.c_str()));
				values->Set(j-3, value);
			}
			parameter->Set(String::NewFromUtf8(isolate, "values"), values); 
		}

		parameter->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, isVisible));
		parameter->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isEnabled));
		parameter->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));

		subCategoryParameters->Set(i, parameter);
	}
	subCategory->Set(String::NewFromUtf8(isolate, "nameSubCategory"), String::NewFromUtf8(isolate, nameSubCategory.c_str()));
	subCategory->Set(String::NewFromUtf8(isolate, "parameters"), subCategoryParameters);
	return subCategory;
}

Local<Array> OBS_settings::getGeneralSettings()
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> generalSettings = Array::New(isolate);

	std::string globalConfigFile = OBS_API::getGlobalConfigPath();

	config_t* config;
    int result = config_open(&config, globalConfigFile.c_str(), CONFIG_OPEN_EXISTING);

    if(result != CONFIG_SUCCESS) {
        config = config_create(globalConfigFile.c_str());

        config_set_bool(config, "BasicWindow", "SnappingEnabled", true);
        config_set_double(config, "BasicWindow", "SnapDistance", 10);
        config_set_bool(config, "BasicWindow", "ScreenSnapping", true);
        config_set_bool(config, "BasicWindow", "SourceSnapping", true);
        config_set_bool(config, "BasicWindow", "CenterSnapping", false);

        config_save_safe(config, "tmp", nullptr);
    }

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	// General
	generalSettings->Set(0, serializeSettingsData("General", entries, config, "BasicWindow", true, true));
	entries.clear();

	// Output
	std::vector<std::pair<std::string, std::string>> warnBeforeStartingStream;
	warnBeforeStartingStream.push_back(std::make_pair("name", "WarnBeforeStartingStream"));
	warnBeforeStartingStream.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	warnBeforeStartingStream.push_back(std::make_pair("description", "Show confirmation dialog when starting streams"));
	entries.push_back(warnBeforeStartingStream);

	std::vector<std::pair<std::string, std::string>> warnBeforeStoppingStream;
	warnBeforeStoppingStream.push_back(std::make_pair("name", "WarnBeforeStoppingStream"));
	warnBeforeStoppingStream.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	warnBeforeStoppingStream.push_back(std::make_pair("description", "Show confirmation dialog when stoping streams"));
	entries.push_back(warnBeforeStoppingStream);

	std::vector<std::pair<std::string, std::string>> recordWhenStreaming;
	recordWhenStreaming.push_back(std::make_pair("name", "RecordWhenStreaming"));
	recordWhenStreaming.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	recordWhenStreaming.push_back(std::make_pair("description", "Automatically record when streaming"));
	entries.push_back(recordWhenStreaming);

	std::vector<std::pair<std::string, std::string>> keepRecordingWhenStreamStops;
	keepRecordingWhenStreamStops.push_back(std::make_pair("name", "KeepRecordingWhenStreamStops"));
	keepRecordingWhenStreamStops.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	keepRecordingWhenStreamStops.push_back(std::make_pair("description", "Keep recording when stream stops"));
	entries.push_back(keepRecordingWhenStreamStops);

	std::vector<std::pair<std::string, std::string>> replayBufferWhileStreaming;
	replayBufferWhileStreaming.push_back(std::make_pair("name", "ReplayBufferWhileStreaming"));
	replayBufferWhileStreaming.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	replayBufferWhileStreaming.push_back(std::make_pair("description", "Automatically start replay buffer when streaming"));
	entries.push_back(replayBufferWhileStreaming);

	std::vector<std::pair<std::string, std::string>> keepReplayBufferStreamStops;
	keepReplayBufferStreamStops.push_back(std::make_pair("name", "KeepReplayBufferStreamStops"));
	keepReplayBufferStreamStops.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	keepReplayBufferStreamStops.push_back(std::make_pair("description", "Keep replay buffer active when stream stops"));
	entries.push_back(keepReplayBufferStreamStops);

	generalSettings->Set(1, serializeSettingsData("Output", entries, config, "BasicWindow", true, true));
	entries.clear();

	// Source Alignement Snapping
	std::vector<std::pair<std::string, std::string>> snappingEnabled;
	snappingEnabled.push_back(std::make_pair("name", "SnappingEnabled"));
	snappingEnabled.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	snappingEnabled.push_back(std::make_pair("description", "Enable"));
	entries.push_back(snappingEnabled);

	std::vector<std::pair<std::string, std::string>> snapDistance;
	snapDistance.push_back(std::make_pair("name", "SnapDistance"));
	snapDistance.push_back(std::make_pair("type", "OBS_PROPERTY_DOUBLE"));
	snapDistance.push_back(std::make_pair("description", "Snap Sensitivy"));
	entries.push_back(snapDistance);

	std::vector<std::pair<std::string, std::string>> screenSnapping;
	screenSnapping.push_back(std::make_pair("name", "ScreenSnapping"));
	screenSnapping.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	screenSnapping.push_back(std::make_pair("description", "Snap Sources to edge of screen"));
	entries.push_back(screenSnapping);

	std::vector<std::pair<std::string, std::string>> sourceSnapping;
	sourceSnapping.push_back(std::make_pair("name", "SourceSnapping"));
	sourceSnapping.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sourceSnapping.push_back(std::make_pair("description", "Snap Sources to other sources"));
	entries.push_back(sourceSnapping);

	std::vector<std::pair<std::string, std::string>> centerSnapping;
	centerSnapping.push_back(std::make_pair("name", "CenterSnapping"));
	centerSnapping.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	centerSnapping.push_back(std::make_pair("description", "Snap Sources to horizontal and vertical center"));
	entries.push_back(centerSnapping);

	generalSettings->Set(2, serializeSettingsData("Source Alignement Snapping", entries, config, "BasicWindow", true, true));
	entries.clear();

	// Projectors
	std::vector<std::pair<std::string, std::string>> hideProjectorCursor;
	hideProjectorCursor.push_back(std::make_pair("name", "HideProjectorCursor"));
	hideProjectorCursor.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	hideProjectorCursor.push_back(std::make_pair("description", "Hide cursor over projectors"));
	entries.push_back(hideProjectorCursor);

	std::vector<std::pair<std::string, std::string>> projectorAlwaysOnTop;
	projectorAlwaysOnTop.push_back(std::make_pair("name", "ProjectorAlwaysOnTop"));
	projectorAlwaysOnTop.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	projectorAlwaysOnTop.push_back(std::make_pair("description", "Make projectors always on top"));
	entries.push_back(projectorAlwaysOnTop);

	std::vector<std::pair<std::string, std::string>> saveProjectors;
	saveProjectors.push_back(std::make_pair("name", "SaveProjectors"));
	saveProjectors.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	saveProjectors.push_back(std::make_pair("description", "Save projectors on exit"));
	entries.push_back(saveProjectors);

	generalSettings->Set(3, serializeSettingsData("Projectors", entries, config, "BasicWindow", true, true));
	entries.clear();

	// System Tray
	std::vector<std::pair<std::string, std::string>> sysTrayEnabled;
	sysTrayEnabled.push_back(std::make_pair("name", "SysTrayEnabled"));
	sysTrayEnabled.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sysTrayEnabled.push_back(std::make_pair("description", "Enable"));
	entries.push_back(sysTrayEnabled);

	std::vector<std::pair<std::string, std::string>> sysTrayWhenStarted;
	sysTrayWhenStarted.push_back(std::make_pair("name", "SysTrayWhenStarted"));
	sysTrayWhenStarted.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sysTrayWhenStarted.push_back(std::make_pair("description", "Minimize to system tray when started"));
	entries.push_back(sysTrayWhenStarted);

	std::vector<std::pair<std::string, std::string>> sysTrayMinimizeToTray;
	sysTrayMinimizeToTray.push_back(std::make_pair("name", "SysTrayMinimizeToTray"));
	sysTrayMinimizeToTray.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sysTrayMinimizeToTray.push_back(std::make_pair("description", "Always minimize to system try instead of task bar"));
	entries.push_back(sysTrayMinimizeToTray);

	generalSettings->Set(4, serializeSettingsData("System Tray", entries, config, "BasicWindow", true, true));
	entries.clear();

	return generalSettings;
}

void OBS_settings::saveGeneralSettings(Local<Array> generalSettings, std::string pathConfigDirectory)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	config_t* config;
	pathConfigDirectory += "global.ini";

	int result = config_open(&config, pathConfigDirectory.c_str(), CONFIG_OPEN_EXISTING);

    if(result != CONFIG_SUCCESS) {
        config = config_create(pathConfigDirectory.c_str());
    }

	for(int i=0;i<generalSettings->Length();i++) {
		Local<Object> generalSettingsObject = v8::Local<v8::Object>::Cast(generalSettings->Get(i));

		v8::String::Utf8Value param0(generalSettingsObject->Get(String::NewFromUtf8(isolate, "nameSubCategory")));
		std::string nameSubcategory = std::string(*param0);

		Local<Array> parameters = v8::Local<v8::Array>::Cast(generalSettingsObject->Get(String::NewFromUtf8(isolate, "parameters")));

		for(int j=0;j<parameters->Length();j++){
			Local<Object> parameter = v8::Local<v8::Object>::Cast(parameters->Get(j));

			std::string name, type, value;

			v8::String::Utf8Value param0(parameter->Get(String::NewFromUtf8(isolate, "name")));
			name = std::string(*param0);

			v8::String::Utf8Value param1(parameter->Get(String::NewFromUtf8(isolate, "type")));
			type = std::string(*param1);

			if(type.compare("OBS_PROPERTY_LIST") == 0) {
				v8::String::Utf8Value param2(parameter->Get(String::NewFromUtf8(isolate, "currentValue")));
				value = std::string(*param2);
				config_set_string(config, "BasicWindow", name.c_str(), value.c_str());
			} else if(type.compare("OBS_PROPERTY_INT") == 0) {
				int64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_int(config, "BasicWindow", name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_UINT") == 0) {
				uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_uint(config, "BasicWindow", name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_BOOL") == 0) {
				uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_bool(config, "BasicWindow", name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_double(config, "BasicWindow", name.c_str(), value);
			}
		}
	}
	config_save_safe(config, "tmp", nullptr);
}

Local<Array> OBS_settings::getStreamSettings()
{
	Isolate *isolate = v8::Isolate::GetCurrent();

	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive();

	obs_service_t* currentService = OBS_service::getService();
	obs_data_t* settings = obs_service_get_settings(currentService);

	Local<Array> streamSettings = Array::New(isolate);
	Local<Object> object = Object::New(isolate);

	Local<Array> subCategoryParametersStreamType = Array::New(isolate);

	Local<Object> streamType = Object::New(isolate);
	Local<Object> parameter = Object::New(isolate);

	streamType->Set(String::NewFromUtf8(isolate, "nameSubCategory"), String::NewFromUtf8(isolate, "Untitled"));
	streamType->Set(String::NewFromUtf8(isolate, "parameters"), subCategoryParametersStreamType);

	parameter->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "streamType"));

	parameter->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));

	int index = 0;
	const char* type;
	Local<Array> values = Array::New(isolate);
	while (obs_enum_service_types(index++, &type)) {
		Local<Object> parameterValue = Object::New(isolate);

		parameterValue->Set(String::NewFromUtf8(isolate, obs_service_get_display_name(type)), 
							String::NewFromUtf8(isolate, type));

		values->Set(index-1, parameterValue);
	}

	parameter->Set(String::NewFromUtf8(isolate, "values"), values);

	parameter->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Stream Type"));

	const char* servType = obs_service_get_type(currentService);

	parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, obs_service_get_type(currentService)));

	parameter->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));

	parameter->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));

	parameter->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));

	streamSettings->Set(0, streamType);

	subCategoryParametersStreamType->Set(0, parameter);

	Local<Object> serviceConfiguration = Object::New(isolate);

	Local<Array> subCategoryParametersServiceConfiguration = Array::New(isolate);

	obs_properties_t* properties = obs_service_properties(currentService);
	obs_property_t* property = obs_properties_first(properties);
	obs_combo_format format;
	string formatString;

	index = 0;

	while(property) {
		Local<Object> parameter = Object::New(isolate);

		parameter->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, obs_property_name(property)));

		Local<Array> values = Array::New(isolate);
		int count = (int)obs_property_list_item_count(property);

		for(int i=0;i<count;i++) {
			Local<Object> parameterValue = Object::New(isolate);

			//Value
			format = obs_property_list_format(property);

			if (format == OBS_COMBO_FORMAT_INT)
			{
				long long value = obs_property_list_item_int(property, i);
				parameterValue->Set(String::NewFromUtf8(isolate, obs_property_list_item_name(property, i)),
									Integer::New(isolate, value));
				formatString = "OBS_PROPERTY_INT";
			}
			else if (format == OBS_COMBO_FORMAT_FLOAT)
			{
				double value = obs_property_list_item_float(property, i);
				parameterValue->Set(String::NewFromUtf8(isolate, obs_property_list_item_name(property, i)),
									Integer::New(isolate, value));
				formatString = "OBS_PROPERTY_DOUBLE";
			}
			else if (format == OBS_COMBO_FORMAT_STRING)
			{
				const char* name = obs_property_list_item_name(property, i);
				const char* value = obs_property_list_item_string(property, i);

        parameterValue->Set(String::NewFromUtf8(isolate, name),
									String::NewFromUtf8(isolate, value));

				formatString = "OBS_PROPERTY_LIST";
			} else {
				cout << "INVALID FORMAT" << endl;
			}
			values->Set(i, parameterValue);
		}

		if(count == 0) {
			if(strcmp(obs_property_name(property), "key") == 0) {
				const char* stream_key = obs_service_get_key(currentService);
				formatString = "OBS_PROPERTY_EDIT_TEXT";

				if(stream_key != NULL){
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, stream_key));
				} else {
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, ""));
				}
			}
			if(strcmp(obs_property_name(property), "show_all") == 0) {
				bool show_all = obs_data_get_bool(settings, "show_all");
				formatString = "OBS_PROPERTY_BOOL";
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Integer::New(isolate, show_all));
			}
			if(strcmp(obs_property_name(property), "server") == 0) {
				const char* server = obs_service_get_url(currentService);
				if(strcmp(obs_service_get_type(currentService), "rtmp_common") == 0) {
					formatString = "OBS_PROPERTY_LIST";
				} else {
					formatString = "OBS_PROPERTY_EDIT_TEXT";
				}

				if(server != NULL){
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, server));
				} else {
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, ""));
				}
			}
			if(strcmp(obs_property_name(property), "username") == 0) {
				const char* username = obs_service_get_username(currentService);
				formatString = "OBS_PROPERTY_EDIT_TEXT";

				if(username != NULL){
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, username));
				} else {
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, ""));
				}
			}
			if(strcmp(obs_property_name(property), "password") == 0) {
				const char* password = obs_service_get_password(currentService);
				formatString = "OBS_PROPERTY_EDIT_TEXT";

				if(password != NULL){
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, password));
				} else {
					parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, ""));
				}
			}
			if(strcmp(obs_property_name(property), "use_auth") == 0) {
				bool use_auth = obs_data_get_bool(settings, "use_auth");
				formatString = "OBS_PROPERTY_BOOL";
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Integer::New(isolate, use_auth));
			}
		} else {
			if (format == OBS_COMBO_FORMAT_INT) 
			{
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Integer::New(isolate, obs_data_get_double(settings, obs_property_name(property))));
			} 
			else if (format == OBS_COMBO_FORMAT_FLOAT) 
			{
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), Number::New(isolate, obs_data_get_double(settings, obs_property_name(property))));
			} 
			else if (format == OBS_COMBO_FORMAT_STRING) 
			{
				currentServiceName = obs_data_get_string(settings, obs_property_name(property));
				parameter->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, currentServiceName));
			}
			parameter->Set(String::NewFromUtf8(isolate, "values"), values);
		}
		v8::String::Utf8Value param1(parameter->Get(String::NewFromUtf8(isolate, "currentValue")));
		std::string current = std::string(*param1);

		parameter->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, formatString.c_str()));

		parameter->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, obs_property_description(property)));

		parameter->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, obs_property_visible(property)));

		parameter->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));

		if(formatString.compare("OBS_PROPERTY_EDIT_TEXT") == 0 && obs_proprety_text_type(property) == OBS_TEXT_PASSWORD) {
			parameter->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, true));
		} else {
			parameter->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
		}

		subCategoryParametersServiceConfiguration->Set(index, parameter);

		index++;
		obs_property_next(&property);
	}

	serviceConfiguration->Set(String::NewFromUtf8(isolate, "nameSubCategory"), String::NewFromUtf8(isolate, "Untitled"));
	serviceConfiguration->Set(String::NewFromUtf8(isolate, "parameters"), subCategoryParametersServiceConfiguration);

	streamSettings->Set(1, serviceConfiguration);

	return streamSettings;
}

void OBS_settings::saveStreamSettings(Local<Array> streamSettings)
{
	Isolate *isolate = v8::Isolate::GetCurrent();

	obs_service_t* currentService = OBS_service::getService();

	obs_data_t* settings;

	std::string currentStreamType = obs_service_get_type(currentService);
	std::string newserviceTypeValue;

	std::string currentServiceName = obs_data_get_string(obs_service_get_settings(currentService), "service");

	for(int i=0;i<streamSettings->Length();i++) {
		Local<Object> streamSettingsCategory = v8::Local<v8::Object>::Cast(streamSettings->Get(i));

		v8::String::Utf8Value param0(streamSettingsCategory->Get(String::NewFromUtf8(isolate, "nameSubCategory")));
		std::string nameSubcategory = std::string(*param0);

		Local<Array> parameters = v8::Local<v8::Array>::Cast(streamSettingsCategory->Get(String::NewFromUtf8(isolate, "parameters")));

		bool serviceChanged = false;

		for(int j=0;j<parameters->Length();j++)
		{
			Local<Array> parameter = v8::Local<v8::Array>::Cast(parameters->Get(j));
			Local<Object> object;

			std::string name, type, value;

			object = v8::Local<v8::Object>::Cast(parameters->Get(j));
			v8::String::Utf8Value param0(object->Get(String::NewFromUtf8(isolate, "name")));

			name = std::string(*param0);
			object->Delete(String::NewFromUtf8(isolate, "name"));

			v8::String::Utf8Value param1(object->Get(String::NewFromUtf8(isolate, "type")));

			type = std::string(*param1);
			object->Delete(String::NewFromUtf8(isolate, "type"));
			const char *servName;
			if(type.compare("OBS_PROPERTY_LIST") == 0 ||
				type.compare("OBS_PROPERTY_EDIT_TEXT") == 0) {
				v8::String::Utf8Value param2(object->Get(String::NewFromUtf8(isolate, "currentValue")));
				value = std::string(*param2);

				if(name.compare("streamType") == 0) {
					newserviceTypeValue = value;
					settings = obs_service_defaults(newserviceTypeValue.c_str());
				} else if (name.compare("service") == 0 && value.compare(currentServiceName) != 0) {
					serviceChanged = true;
				}

				object = v8::Local<v8::Object>::Cast(parameters->Get(0));
				v8::String::Utf8Value param(object->Get(String::NewFromUtf8(isolate, "value")));

				std::string serviceTypeName = std::string(*param);

				if (name.compare("server") == 0 && serviceChanged) {
					obs_service_t *newService = obs_service_create(newserviceTypeValue.c_str(),
									"service", settings, NULL);

					obs_properties_t* properties = obs_service_properties(newService);
					obs_property_t* property = obs_properties_first(properties);

					while(property) {
						int count = (int)obs_property_list_item_count(property);
						const char *nameProperty = obs_property_name(property);
						if(strcmp(nameProperty, "server") == 0) {
							value = obs_property_list_item_string(property, 0);
							break;
						}

						obs_property_next(&property);
					}
				}

				obs_data_set_string(settings, name.c_str(), value.c_str());
				object->Delete(String::NewFromUtf8(isolate, "currentValue"));
			} else if(type.compare("OBS_PROPERTY_INT") == 0 ||
					  type.compare("OBS_PROPERTY_UINT") == 0) {
				int64_t value = object->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				obs_data_set_int(settings, name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_BOOL") == 0) {
				uint64_t value = object->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				obs_data_set_bool(settings, name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double value = object->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				obs_data_set_double(settings, name.c_str(), value);
			} 
		}
	}
	if(currentStreamType.compare(newserviceTypeValue) != 0) {
		//Stream type value has changed
		settings = obs_service_defaults(newserviceTypeValue.c_str());
	}

	obs_data_t *data = obs_data_create();

	obs_data_t *hotkeyData = obs_hotkeys_save_service(currentService);

	obs_service_t *newService = obs_service_create(newserviceTypeValue.c_str(),
			"default_service", settings, hotkeyData);

	obs_data_release(hotkeyData);

	OBS_service::setService(newService);

	obs_data_set_string(data, "type", obs_service_get_type(newService));
	obs_data_set_obj(data, "settings", settings);

	if (!obs_data_save_json_safe(data, OBS_API::getServiceConfigPath().c_str(), "tmp", "bak")) {
		blog(LOG_WARNING, "Failed to save service");
	}
}

static bool EncoderAvailable(const char *encoder)
{
	const char *val;
	int i = 0;

	while (obs_enum_encoder_types(i++, &val)){
		if (strcmp(val, encoder) == 0)
			return true;
	}

	return false;
}

void OBS_settings::getAvailableEncoders(std::vector<std::pair<std::string, std::string>>* streamEncoder) 
{
	streamEncoder->push_back(std::make_pair("Software (x264)", "obs_x264"));	
	if (EncoderAvailable("obs_qsv11"))
	{
		streamEncoder->push_back(std::make_pair("QSV", "obs_qsv11"));
	}
	
	if (EncoderAvailable("ffmpeg_nvenc"))
	{
		streamEncoder->push_back(std::make_pair("NVENC", "ffmpeg_nvenc"));
	}
	if (EncoderAvailable("amd_amf_h264"))
	{
		streamEncoder->push_back(std::make_pair("AMD", "amd_amf_h264"));
	}
}

void OBS_settings::getSimpleOutputSettings(Local<Array> outputSettings, config_t* config, bool isCategoryEnabled)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	//Streaming

	//Video Bitrate
	std::vector<std::pair<std::string, std::string>> vBitrate;
	vBitrate.push_back(std::make_pair("name", "VBitrate"));
	vBitrate.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	vBitrate.push_back(std::make_pair("description", "Video Bitrate"));
	entries.push_back(vBitrate);

	//Encoder
	std::vector<std::pair<std::string, std::string>> streamEncoder;
	streamEncoder.push_back(std::make_pair("name", "StreamEncoder"));
	streamEncoder.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	streamEncoder.push_back(std::make_pair("description", "Encoder"));

	getAvailableEncoders(&streamEncoder);

	entries.push_back(streamEncoder);

	//Audio Bitrate
	std::vector<std::pair<std::string, std::string>> aBitrate;
	aBitrate.push_back(std::make_pair("name", "ABitrate"));
	aBitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	aBitrate.push_back(std::make_pair("description", "Audio Bitrate"));


	auto &bitrateMap = GetAACEncoderBitrateMap();
	for (auto &entry : bitrateMap)
		aBitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(aBitrate);

	//Enable Advanced Encoder Settings
	std::vector<std::pair<std::string, std::string>> useAdvanced;
	useAdvanced.push_back(std::make_pair("name", "UseAdvanced"));
	useAdvanced.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	useAdvanced.push_back(std::make_pair("description", "Enable Advanced Encoder Settings"));
	entries.push_back(useAdvanced);

	if(config_get_bool(config, "SimpleOutput","UseAdvanced")) {
		//Enforce streaming service bitrate limits
		std::vector<std::pair<std::string, std::string>> enforceBitrate;
		enforceBitrate.push_back(std::make_pair("name", "EnforceBitrate"));
		enforceBitrate.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
		enforceBitrate.push_back(std::make_pair("description", "Enforce streaming service bitrate limits"));
		entries.push_back(enforceBitrate);

		//Encoder Preset
		const char* defaultPreset;
		const char* encoder = config_get_string(config, "SimpleOutput", "StreamEncoder");

		std::vector<std::pair<std::string, std::string>> preset;

		if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0) {
			preset.push_back(std::make_pair("name", "QSVPreset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));

			preset.push_back(std::make_pair("Speed", "speed"));
			preset.push_back(std::make_pair("Balanced", "balanced"));
			preset.push_back(std::make_pair("Quality", "quality"));

			defaultPreset = "balanced";
			// preset = curQSVPreset;

		} else if (strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0) {
			preset.push_back(std::make_pair("name", "NVENCPreset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));

			obs_properties_t *props =
				obs_get_encoder_properties("ffmpeg_nvenc");

			obs_property_t *p = obs_properties_get(props, "preset");
			size_t num = obs_property_list_item_count(p);
			for (size_t i = 0; i < num; i++) {
				const char *name = obs_property_list_item_name(p, i);
				const char *val  = obs_property_list_item_string(p, i);

				/* bluray is for ideal bluray disc recording settings,
					* not streaming */
				if (strcmp(val, "bd") == 0)
					continue;
				/* lossless should of course not be used to stream */
				if (astrcmp_n(val, "lossless", 8) == 0)
					continue;

				preset.push_back(std::make_pair(name, val));
			}

			obs_properties_destroy(props);

			defaultPreset = "default";
			// preset = curNVENCPreset;

		} else if (strcmp(encoder, SIMPLE_ENCODER_AMD) == 0) {
			preset.push_back(std::make_pair("name", "AMDPreset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));

			preset.push_back(std::make_pair("Speed",  "speed"));
			preset.push_back(std::make_pair("Balanced",  "balanced"));
			preset.push_back(std::make_pair("Quality",  "quality"));

			defaultPreset = "balanced";
			// preset = curAMDPreset;
		} else {
			preset.push_back(std::make_pair("name", "Preset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));

			preset.push_back(std::make_pair("ultrafast", "ultrafast"));
			preset.push_back(std::make_pair("superfast", "superfast"));
			preset.push_back(std::make_pair("veryfast", "veryfast"));
			preset.push_back(std::make_pair("faster", "faster"));
			preset.push_back(std::make_pair("fast", "fast"));
			preset.push_back(std::make_pair("medium", "medium"));
			preset.push_back(std::make_pair("slow", "slow"));
			preset.push_back(std::make_pair("slower", "slower"));

			defaultPreset = "veryfast";
			// preset = curPreset;
		}

		entries.push_back(preset);

		//Custom Encoder Settings
		std::vector<std::pair<std::string, std::string>> x264opts;
		x264opts.push_back(std::make_pair("name", "x264Settings"));
		x264opts.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
		x264opts.push_back(std::make_pair("description", "Custom Encoder Settings"));
		entries.push_back(x264opts);
	}

	outputSettings->Set(1, serializeSettingsData("Streaming", entries, config, "SimpleOutput", true, isCategoryEnabled));
	entries.clear();

	//Recording

	//Recording Path
	std::vector<std::pair<std::string, std::string>> filePath;
	filePath.push_back(std::make_pair("name", "FilePath"));
	filePath.push_back(std::make_pair("type", "OBS_PROPERTY_PATH"));
	filePath.push_back(std::make_pair("description", "Recording Path"));
	entries.push_back(filePath);

	//Generate File Name without Space
	std::vector<std::pair<std::string, std::string>> fileNameWithoutSpace;
	fileNameWithoutSpace.push_back(std::make_pair("name", "FileNameWithoutSpace"));
	fileNameWithoutSpace.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	fileNameWithoutSpace.push_back(std::make_pair("description", "Generate File Name without Space"));
	entries.push_back(fileNameWithoutSpace);

	//Recording Quality
	std::vector<std::pair<std::string, std::string>> recQuality;
	recQuality.push_back(std::make_pair("name", "RecQuality"));
	recQuality.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	recQuality.push_back(std::make_pair("description", "Recording Quality"));
	recQuality.push_back(std::make_pair("Same as stream", "Stream"));
	recQuality.push_back(std::make_pair("High Quality, Medium File Size", "Small"));
	recQuality.push_back(std::make_pair("Indistinguishable Quality, Large File Size", "HQ"));
	recQuality.push_back(std::make_pair("Lossless Quality, Tremendously Large File Size", "Lossless"));
	entries.push_back(recQuality);

	//Recording Format
	std::vector<std::pair<std::string, std::string>> recFormat;
	recFormat.push_back(std::make_pair("name", "RecFormat"));
	recFormat.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	recFormat.push_back(std::make_pair("description", "Recording Format"));
	recFormat.push_back(std::make_pair("flv", "flv"));
	recFormat.push_back(std::make_pair("mp4", "mp4"));
	recFormat.push_back(std::make_pair("mov", "mov"));
	recFormat.push_back(std::make_pair("mkv", "mkv"));
	recFormat.push_back(std::make_pair("ts", "ts"));
	recFormat.push_back(std::make_pair("m3u8", "m3u8"));
	entries.push_back(recFormat);

	//Custom Muxer Settings
	std::vector<std::pair<std::string, std::string>> muxerCustom;
	muxerCustom.push_back(std::make_pair("name", "MuxerCustom"));
	muxerCustom.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	muxerCustom.push_back(std::make_pair("description", "Custom Muxer Settings"));
	entries.push_back(muxerCustom);

	//Enable Replay Buffer
	std::vector<std::pair<std::string, std::string>> recRB;
	recRB.push_back(std::make_pair("name", "RecRB"));
	recRB.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	recRB.push_back(std::make_pair("description", "Enable Replay Buffer"));
	entries.push_back(recRB);

	outputSettings->Set(2, serializeSettingsData("Recording", entries, config, "SimpleOutput", true, isCategoryEnabled));
}

void OBS_settings::getEncoderSettings(Isolate *isolate, const obs_encoder_t *encoder, obs_data_t *settings,
										Local<Array>* subCategoryParameters, int index, bool isCategoryEnabled)
{
	obs_properties_t* encoderProperties = obs_encoder_properties(encoder);
	obs_property_t* property = obs_properties_first(encoderProperties);

	while(property) {
		Local<Object> entryObject = Object::New(isolate);

		std::string name = obs_property_name(property);
		obs_property_type typeProperty = obs_property_get_type(property);

		entryObject->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, name.c_str()));

		switch(typeProperty) {
			case OBS_PROPERTY_BOOL:
			{
				entryObject->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
				entryObject->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, obs_property_description(property)));
				entryObject->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, obs_data_get_bool(settings, name.c_str())));

				break;
			}
			case OBS_PROPERTY_INT:
			{
				entryObject->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_INT"));
				entryObject->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, obs_property_description(property)));
				entryObject->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, obs_data_get_int(settings, name.c_str())));

				break;
			}
			case OBS_PROPERTY_FLOAT:
			{
				entryObject->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_FLOAT"));
				entryObject->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, obs_property_description(property)));
				entryObject->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, obs_data_get_double(settings, name.c_str())));
				break;
			}
			case OBS_PROPERTY_TEXT:
			{
				entryObject->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_TEXT"));
				entryObject->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, obs_property_description(property)));

				const char* currentValue = obs_data_get_string(settings, name.c_str());

				if(currentValue == NULL) {
					currentValue = "";
				}
				entryObject->Set(String::NewFromUtf8(isolate, "currentValue"),
									String::NewFromUtf8(isolate, currentValue));

				break;
			}
			case OBS_PROPERTY_PATH:
			{
				entryObject->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_PATH"));
				entryObject->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, obs_property_description(property)));

				const char* currentValue = obs_data_get_string(settings, name.c_str());

				if(currentValue == NULL) {
					currentValue = "";
				}
				entryObject->Set(String::NewFromUtf8(isolate, "currentValue"),
									String::NewFromUtf8(isolate, currentValue));
				break;
			}
			case OBS_PROPERTY_LIST:
			{
				entryObject->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
				entryObject->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, obs_property_description(property)));

				const char* currentValue = obs_data_get_string(settings, name.c_str());

				if(currentValue == NULL) {
					currentValue = "";
				}
				entryObject->Set(String::NewFromUtf8(isolate, "currentValue"),
									String::NewFromUtf8(isolate, currentValue));

				Local<Array> listValues = Array::New(isolate);
				int count = (int)obs_property_list_item_count(property);
				for(int i=0;i<count;i++) {
					// Name
					std::string itemName = obs_property_list_item_name(property, i);

					//Value
					obs_combo_format format = obs_property_list_format(property);

					std::string valueString;

					if (format == OBS_COMBO_FORMAT_INT)
					{
						long long val = obs_property_list_item_int(property, i);
						valueString = std::to_string(val);
					}
					else if (format == OBS_COMBO_FORMAT_FLOAT)
					{
						double val = obs_property_list_item_float(property, i);
						valueString = to_string(val).c_str();
					}
					else if (format == OBS_COMBO_FORMAT_STRING)
					{
						valueString = obs_property_list_item_string(property, i);
					}
					Local<Object> object = Object::New(isolate);
					object->Set(String::NewFromUtf8(isolate, itemName.c_str()),
									String::NewFromUtf8(isolate, valueString.c_str()));
					listValues->Set(i, object);
				}
				entryObject->Set(String::NewFromUtf8(isolate, "values"), listValues);
				break;
			}
			case OBS_PROPERTY_EDITABLE_LIST:
			{
				entryObject->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_EDITABLE_LIST"));
				entryObject->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, obs_property_description(property)));

				const char* currentValue = obs_data_get_string(settings, name.c_str());

				if(currentValue == NULL) {
					currentValue = "";
				}
				entryObject->Set(String::NewFromUtf8(isolate, "currentValue"),
									String::NewFromUtf8(isolate, currentValue));

				break;
			}
		}
		entryObject->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, obs_property_visible(property)));

		bool isEnabled = obs_property_enabled(property);
		if(!isCategoryEnabled)
			isEnabled = isCategoryEnabled;

		entryObject->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isEnabled));
		entryObject->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
		(*subCategoryParameters)->Set(index++, entryObject);

		obs_property_next(&property);
	}
}

Local<Object> OBS_settings::getAdvancedOutputStreamingSettings(config_t* config, bool isCategoryEnabled)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> subCategoryParameters = Array::New(isolate);
	Local<Object> subCategory = Object::New(isolate);

	int index = 0;

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	// Audio Track : list
	Local<Object> TrackIndex = Object::New(isolate);
	TrackIndex->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "TrackIndex"));
	TrackIndex->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	TrackIndex->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Audio Track"));
	Local<Array> TrackIndexValues = Array::New(isolate);

	Local<Object> track1 = Object::New(isolate);
	track1->Set(String::NewFromUtf8(isolate, "1"), String::NewFromUtf8(isolate, "1"));
	TrackIndexValues->Set(0, track1);

	Local<Object> track2 = Object::New(isolate);
	track2->Set(String::NewFromUtf8(isolate, "2"), String::NewFromUtf8(isolate, "2"));
	TrackIndexValues->Set(1, track2);

	Local<Object> track3 = Object::New(isolate);
	track3->Set(String::NewFromUtf8(isolate, "3"), String::NewFromUtf8(isolate, "3"));
	TrackIndexValues->Set(2, track3);

	Local<Object> track4 = Object::New(isolate);
	track4->Set(String::NewFromUtf8(isolate, "4"), String::NewFromUtf8(isolate, "4"));
	TrackIndexValues->Set(3, track4);

	Local<Object> track5 = Object::New(isolate);
	track5->Set(String::NewFromUtf8(isolate, "5"), String::NewFromUtf8(isolate, "5"));
	TrackIndexValues->Set(4, track5);

	Local<Object> track6 = Object::New(isolate);
	track6->Set(String::NewFromUtf8(isolate, "6"), String::NewFromUtf8(isolate, "6"));
	TrackIndexValues->Set(5, track6);

	TrackIndex->Set(String::NewFromUtf8(isolate, "values"), TrackIndexValues);

	const char* TrackIndexCurrentValue = config_get_string(config, "AdvOut", "TrackIndex");
	if(TrackIndexCurrentValue == NULL) {
		TrackIndexCurrentValue = "";
	}
	TrackIndex->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, TrackIndexCurrentValue));

	TrackIndex->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	TrackIndex->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	TrackIndex->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	subCategoryParameters->Set(index++, TrackIndex);

	// Encoder : list
	Local<Object> EncoderList = Object::New(isolate);
	EncoderList->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "Encoder"));
	EncoderList->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	EncoderList->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Encoder"));

	const char* EncoderListCurrentValue = config_get_string(config, "AdvOut", "Encoder");
	if(EncoderListCurrentValue == NULL) {
		EncoderListCurrentValue = "";
	}
	EncoderList->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, EncoderListCurrentValue));

	std::vector<std::pair<std::string, std::string>> Encoder;
	getAvailableEncoders(&Encoder);
	Local<Array> EncoderListValues = Array::New(isolate);
	for(int i=0;i<Encoder.size();i++) {
		Local<Object> object = Object::New(isolate);
		object->Set(String::NewFromUtf8(isolate, Encoder.at(i).first.c_str()),
						String::NewFromUtf8(isolate, Encoder.at(i).second.c_str()));
		EncoderListValues->Set(i, object);
	}
	EncoderList->Set(String::NewFromUtf8(isolate, "values"), EncoderListValues); 
	EncoderList->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	EncoderList->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	EncoderList->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	subCategoryParameters->Set(index++, EncoderList);
	
	// Enforce streaming service encoder settings : boolean
	Local<Object> ApplyServiceSettings = Object::New(isolate);
	ApplyServiceSettings->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "ApplyServiceSettings"));
	ApplyServiceSettings->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
	ApplyServiceSettings->Set(String::NewFromUtf8(isolate, "description"),
							String::NewFromUtf8(isolate, "Enforce streaming service encoder settings"));
	ApplyServiceSettings->Set(String::NewFromUtf8(isolate, "currentValue"),
						Integer::New(isolate, config_get_bool(config, "AdvOut", "ApplyServiceSettings")));

	ApplyServiceSettings->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	ApplyServiceSettings->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	ApplyServiceSettings->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	subCategoryParameters->Set(index++, ApplyServiceSettings);
	
	// Rescale Output : boolean
	Local<Object> Rescale = Object::New(isolate);
	Rescale->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "Rescale"));
	Rescale->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
	Rescale->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Rescale Output"));
	bool doRescale = config_get_bool(config, "AdvOut", "Rescale");
	Rescale->Set(String::NewFromUtf8(isolate, "currentValue"), Integer::New(isolate, doRescale));
	Rescale->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	Rescale->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	Rescale->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	subCategoryParameters->Set(index++, Rescale);
	

	if(doRescale) {
		// Output Resolution : list
		Local<Object> RescaleRes = Object::New(isolate);
		RescaleRes->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RescaleRes"));
		RescaleRes->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
		RescaleRes->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Output Resolution"));

		uint32_t base_cx = config_get_uint(config, "Video", "BaseCX");
		uint32_t base_cy = config_get_uint(config, "Video", "BaseCY");

		const char* outputResString = config_get_string(config, "AdvOut", "RescaleRes");

		if (outputResString == NULL) {
			outputResString = "1280x720";
			config_set_string(config, "AdvOut", "RescaleRes", outputResString);
			config_save_safe(config, "tmp", nullptr);
		}

		RescaleRes->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, outputResString));

		std::vector<pair<int, int>> outputResolutions = getOutputResolutions(base_cx, base_cy);

		Local<Array> RescaleResValues = Array::New(isolate);
		for(int i = 0;i<outputResolutions.size();i++) {
			string outRes = ResString(outputResolutions.at(i).first, outputResolutions.at(i).second);
			Local<Object> object = Object::New(isolate);
			object->Set(String::NewFromUtf8(isolate, outRes.c_str()),
							String::NewFromUtf8(isolate, outRes.c_str()));
			RescaleResValues->Set(i, object);
		}
		RescaleRes->Set(String::NewFromUtf8(isolate, "values"), RescaleResValues);
		RescaleRes->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
		RescaleRes->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
		RescaleRes->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
		subCategoryParameters->Set(index++, RescaleRes);
	}

	// Encoder settings
	const char *encoderID = config_get_string(config, "AdvOut", "Encoder");
	if(encoderID == NULL) {
		encoderID = "obs_x264";
		config_set_string(config, "AdvOut", "Encoder", encoderID);
	}

	std::string path = OBS_API::getStreamingEncoderConfigPath();
	struct stat buffer;

	bool fileExist = (stat(path.c_str(), &buffer) == 0);

	obs_data_t *settings = obs_encoder_defaults(encoderID);
	obs_encoder_t* streamingEncoder;
	if(!fileExist) {
		streamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", nullptr, nullptr);
		OBS_service::setStreamingEncoder(streamingEncoder);

		if (!obs_data_save_json_safe(settings, path.c_str(), "tmp", "bak")) {
			blog(LOG_WARNING, "Failed to save encoder %s", path.c_str());
		}
	} else {
		obs_data_t *data = obs_data_create_from_json_file_safe(path.c_str(), "bak");
		obs_data_apply(settings, data);
		streamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", settings, nullptr);
		OBS_service::setStreamingEncoder(streamingEncoder);
	}

	getEncoderSettings(isolate, streamingEncoder, settings, &subCategoryParameters, index, isCategoryEnabled);

	subCategory->Set(String::NewFromUtf8(isolate, "nameSubCategory"), String::NewFromUtf8(isolate, "Streaming"));
	subCategory->Set(String::NewFromUtf8(isolate, "parameters"), subCategoryParameters);

	return subCategory;
}

void OBS_settings::getStandardRecordingSettings(Local<Array>* subCategoryParameters, config_t* config, bool isCategoryEnabled)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	int index = 1;

	// Recording Path : file
	Local<Object> RecFilePath = Object::New(isolate);
	RecFilePath->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecFilePath"));
	RecFilePath->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_PATH"));
	RecFilePath->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Recording Path"));

	const char* RecFilePathCurrentValue = config_get_string(config, "AdvOut", "RecFilePath");
	std::string RecFilePathText;
	if(RecFilePathCurrentValue == NULL) { 
		RecFilePathText = OBS_service::GetDefaultVideoSavePath().c_str();
		RecFilePath->Set(String::NewFromUtf8(isolate, "currentValue"),
			String::NewFromUtf8(isolate, RecFilePathText.c_str()));
	}
	else {
		RecFilePath->Set(String::NewFromUtf8(isolate, "currentValue"),
			String::NewFromUtf8(isolate, RecFilePathCurrentValue));
	}

	RecFilePath->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecFilePath->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecFilePath->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, RecFilePath);

	// Generate File Name without Space : boolean
	Local<Object> RecFileNameWithoutSpace = Object::New(isolate);
	RecFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecFileNameWithoutSpace"));
	RecFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
	RecFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Generate File Name without Space"));

	RecFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, config_get_bool(config, "AdvOut", "RecFileNameWithoutSpace")));

	RecFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, RecFileNameWithoutSpace);

	// Recording Format : list
	Local<Object> RecFormat = Object::New(isolate);
	RecFormat->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecFormat"));
	RecFormat->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	RecFormat->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Recording Format"));

	const char* RecFormatCurrentValue = config_get_string(config, "AdvOut", "RecFormat");
	if(RecFormatCurrentValue == NULL) {
		RecFormatCurrentValue = "";
	}
	RecFormat->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, RecFormatCurrentValue));

	Local<Array> RecFormatValues = Array::New(isolate);

	Local<Object> flv = Object::New(isolate);
	flv->Set(String::NewFromUtf8(isolate, "flv"), String::NewFromUtf8(isolate, "flv"));
	RecFormatValues->Set(0, flv);

	Local<Object> mp4 = Object::New(isolate);
	mp4->Set(String::NewFromUtf8(isolate, "mp4"), String::NewFromUtf8(isolate, "mp4"));
	RecFormatValues->Set(1, mp4);

	Local<Object> mov = Object::New(isolate);
	mov->Set(String::NewFromUtf8(isolate, "mov"), String::NewFromUtf8(isolate, "mov"));
	RecFormatValues->Set(2, mov);

	Local<Object> mkv = Object::New(isolate);
	mkv->Set(String::NewFromUtf8(isolate, "mkv"), String::NewFromUtf8(isolate, "mkv"));
	RecFormatValues->Set(3, mkv);

	Local<Object> ts = Object::New(isolate);
	ts->Set(String::NewFromUtf8(isolate, "ts"), String::NewFromUtf8(isolate, "ts"));
	RecFormatValues->Set(4, ts);

	Local<Object> m3u8 = Object::New(isolate);
	m3u8->Set(String::NewFromUtf8(isolate, "m3u8"), String::NewFromUtf8(isolate, "m3u8"));
	RecFormatValues->Set(5, m3u8);

	RecFormat->Set(String::NewFromUtf8(isolate, "values"), RecFormatValues);

	RecFormat->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecFormat->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecFormat->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, RecFormat);

	// Audio Track : list
	Local<Object> RecTracks = Object::New(isolate);
	RecTracks->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecTracks"));
	RecTracks->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	RecTracks->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Audio Track"));

	const char* RecTracksCurrentValue = config_get_string(config, "AdvOut", "RecTracks");
	if(RecTracksCurrentValue == NULL) {
		RecTracksCurrentValue = "";
	}
	RecTracks->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, RecTracksCurrentValue));

	Local<Array> RecTracksValues = Array::New(isolate);

	Local<Object> track1 = Object::New(isolate);
	track1->Set(String::NewFromUtf8(isolate, "1"), String::NewFromUtf8(isolate, "1"));
	RecTracksValues->Set(0, track1);

	Local<Object> track2 = Object::New(isolate);
	track2->Set(String::NewFromUtf8(isolate, "2"), String::NewFromUtf8(isolate, "2"));
	RecTracksValues->Set(1, track2);

	Local<Object> track3 = Object::New(isolate);
	track3->Set(String::NewFromUtf8(isolate, "3"), String::NewFromUtf8(isolate, "3"));
	RecTracksValues->Set(2, track3);

	Local<Object> track4 = Object::New(isolate);
	track4->Set(String::NewFromUtf8(isolate, "4"), String::NewFromUtf8(isolate, "4"));
	RecTracksValues->Set(3, track4);

	Local<Object> track5 = Object::New(isolate);
	track5->Set(String::NewFromUtf8(isolate, "5"), String::NewFromUtf8(isolate, "5"));
	RecTracksValues->Set(4, track5);

	Local<Object> track6 = Object::New(isolate);
	track6->Set(String::NewFromUtf8(isolate, "6"), String::NewFromUtf8(isolate, "6"));
	RecTracksValues->Set(5, track6);

	RecTracks->Set(String::NewFromUtf8(isolate, "values"), RecTracksValues);

	RecTracks->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecTracks->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecTracks->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, RecTracks);

	// Encoder : list
	Local<Object> RecEncoder = Object::New(isolate);
	RecEncoder->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecEncoder"));
	RecEncoder->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	RecEncoder->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Recording"));

	const char* RecEncoderCurrentValue = config_get_string(config, "AdvOut", "RecEncoder");
	if(RecEncoderCurrentValue == NULL || strcmp(RecEncoderCurrentValue, "none") == 0) {
		RecEncoderCurrentValue = "obs_x264";
	}
	RecEncoder->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, RecEncoderCurrentValue));

	std::vector<std::pair<std::string, std::string>> Encoder;
	getAvailableEncoders(&Encoder);
	Local<Array> EncoderListValues = Array::New(isolate);
	for(int i=0;i<Encoder.size();i++) {
		Local<Object> object = Object::New(isolate);
		object->Set(String::NewFromUtf8(isolate, Encoder.at(i).first.c_str()),
						String::NewFromUtf8(isolate, Encoder.at(i).second.c_str()));
		EncoderListValues->Set(i, object);
	}
	RecEncoder->Set(String::NewFromUtf8(isolate, "values"), EncoderListValues);

	RecEncoder->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecEncoder->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecEncoder->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, RecEncoder);

	// Rescale Output : boolean
	Local<Object> RecRescale = Object::New(isolate);
	RecRescale->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecRescale"));
	RecRescale->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
	RecRescale->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Rescale Output"));

	bool doRescale = config_get_bool(config, "AdvOut", "RecRescale");

	RecRescale->Set(String::NewFromUtf8(isolate, "currentValue"),
						Integer::New(isolate, doRescale));

	RecRescale->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecRescale->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecRescale->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, RecRescale);

	// Output Resolution : list
	if(doRescale) {
		// Output Resolution : list
		Local<Object> RecRescaleRes = Object::New(isolate);
		RecRescaleRes->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecRescaleRes"));
		RecRescaleRes->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
		RecRescaleRes->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Output Resolution"));

		uint32_t base_cx = config_get_uint(config, "Video", "BaseCX");
		uint32_t base_cy = config_get_uint(config, "Video", "BaseCY");

		const char* outputResString = config_get_string(config, "AdvOut", "RecRescaleRes");

		if (outputResString == NULL) {
			outputResString = "1280x720";
			config_set_string(config, "AdvOut", "RecRescaleRes", outputResString);
			config_save_safe(config, "tmp", nullptr);
		}

		RecRescaleRes->Set(String::NewFromUtf8(isolate, "currentValue"), String::NewFromUtf8(isolate, outputResString));

		std::vector<pair<int, int>> outputResolutions = getOutputResolutions(base_cx, base_cy);

		Local<Array> RecRescaleResValues = Array::New(isolate);
		for(int i = 0;i<outputResolutions.size();i++) {
			string outRes = ResString(outputResolutions.at(i).first, outputResolutions.at(i).second);
			Local<Object> object = Object::New(isolate);
			object->Set(String::NewFromUtf8(isolate, outRes.c_str()),
							String::NewFromUtf8(isolate, outRes.c_str()));
							RecRescaleResValues->Set(i, object);
		}
		RecRescaleRes->Set(String::NewFromUtf8(isolate, "values"), RecRescaleResValues);
		RecRescaleRes->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
		RecRescaleRes->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
		RecRescaleRes->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
		(*subCategoryParameters)->Set(index++, RecRescaleRes);
	}

	// Custom Muxer Settings : edit_text
	Local<Object> RecMuxerCustom = Object::New(isolate);
	RecMuxerCustom->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecMuxerCustom"));
	RecMuxerCustom->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_EDIT_TEXT"));
	RecMuxerCustom->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Custom Muxer Settings"));

	const char* RecMuxerCustomCurrentValue = config_get_string(config, "AdvOut", "RecMuxerCustom");
	if(RecMuxerCustomCurrentValue == NULL) {
		RecMuxerCustomCurrentValue = "";
	}
	RecMuxerCustom->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, RecMuxerCustomCurrentValue));

	RecMuxerCustom->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecMuxerCustom->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecMuxerCustom->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, RecMuxerCustom);

	// Encoder settings
	std::string path = OBS_API::getRecordingEncoderConfigPath();
	struct stat buffer;

	bool fileExist = (stat(path.c_str(), &buffer) == 0);

	obs_data_t *settings = obs_encoder_defaults(RecEncoderCurrentValue);
	obs_encoder_t* recordingEncoder;
	if(!fileExist) {
		recordingEncoder =
			obs_video_encoder_create(RecEncoderCurrentValue, "streaming_h264", nullptr, nullptr);
		OBS_service::setRecordingEncoder(recordingEncoder);

		if (!obs_data_save_json_safe(settings, path.c_str(), "tmp", "bak")) {
			blog(LOG_WARNING, "Failed to save encoder %s", path.c_str());
		}
	} else {
		obs_data_t *data = obs_data_create_from_json_file_safe(path.c_str(), "bak");
		obs_data_apply(settings, data);
		recordingEncoder =
			obs_video_encoder_create(RecEncoderCurrentValue, "streaming_h264", settings, nullptr);
		OBS_service::setRecordingEncoder(recordingEncoder);
	}

	getEncoderSettings(isolate, recordingEncoder, settings, subCategoryParameters, index, isCategoryEnabled);
}

// class OBSFFDeleter
// {
// public:
// 	void operator()(const ff_format_desc *format)
// 	{
// 		ff_format_desc_free(format);
// 	}
// 	void operator()(const ff_codec_desc *codec)
// 	{
// 		ff_codec_desc_free(codec);
// 	}
// };

// using OBSFFFormatDesc = std::unique_ptr<const ff_format_desc, OBSFFDeleter>;
// OBSFFFormatDesc formats;

// using OBSFFCodecDesc = std::unique_ptr<const ff_codec_desc, OBSFFDeleter>;

void OBS_settings::getFFmpegOutputRecordingSettings(Local<Array>* subCategoryParameters, config_t* config, bool isCategoryEnabled)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	int index = 1;

	// FFmpeg Output Type : list											FFOutputToFile
	Local<Object> FFOutputToFile = Object::New(isolate);
	FFOutputToFile->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFOutputToFile"));
	FFOutputToFile->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	FFOutputToFile->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "FFmpeg Output Type"));
	bool FFOutputToFileCurrentValue = config_get_bool(config, "AdvOut", "FFOutputToFile");
	FFOutputToFile->Set(String::NewFromUtf8(isolate, "currentValue"),
						Integer::New(isolate, FFOutputToFileCurrentValue));

	Local<Array> FFOutputToFileValues = Array::New(isolate);

	Local<Object> file = Object::New(isolate);
	file->Set(String::NewFromUtf8(isolate, "Output to File"), Integer::New(isolate, true));
	FFOutputToFileValues->Set(0, file);

	Local<Object> url = Object::New(isolate);
	url->Set(String::NewFromUtf8(isolate, "Output to URL"), Integer::New(isolate, false));
	FFOutputToFileValues->Set(1, url);

	FFOutputToFile->Set(String::NewFromUtf8(isolate, "values"), FFOutputToFileValues);

	FFOutputToFile->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFOutputToFile->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFOutputToFile->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFOutputToFile);

	// File path or URL : path												FFURL
	if(FFOutputToFileCurrentValue) {
		Local<Object> FFfile = Object::New(isolate);
		FFfile->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFFilePath"));
		FFfile->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_PATH"));
		FFfile->Set(String::NewFromUtf8(isolate, "description"),
										String::NewFromUtf8(isolate, "File path"));
		const char* FFfileCurrentValue = config_get_string(config, "AdvOut", "FFFilePath");
		if(FFfileCurrentValue == NULL) {
			FFfileCurrentValue = "";
		}
		FFfile->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, FFfileCurrentValue));
		FFfile->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
		FFfile->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
		FFfile->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
		(*subCategoryParameters)->Set(index++, FFfile);
	} else {
		Local<Object> FFURL = Object::New(isolate);
		FFURL->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFURL"));
		FFURL->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_EDIT_TEXT"));
		FFURL->Set(String::NewFromUtf8(isolate, "description"),
										String::NewFromUtf8(isolate, "URL"));
		const char* FFURLCurrentValue = config_get_string(config, "AdvOut", "FFURL");
		if(FFURLCurrentValue == NULL) {
			FFURLCurrentValue = "";
		}
		FFURL->Set(String::NewFromUtf8(isolate, "currentValue"),
						String::NewFromUtf8(isolate, FFURLCurrentValue));
		FFURL->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
		FFURL->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
		FFURL->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
		(*subCategoryParameters)->Set(index++, FFURL);
	}

	// Generate File Name without Space : boolean 							FFFileNameWithoutSpace
	Local<Object> FFFileNameWithoutSpace = Object::New(isolate);
	FFFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFFileNameWithoutSpace"));
	FFFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
	FFFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Generate File Name without Space"));
	FFFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, config_get_bool(config, "AdvOut", "FFFileNameWithoutSpace")));
	FFFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFFileNameWithoutSpace->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFFileNameWithoutSpace);

	// Container Format : list 												FFFormat
	Local<Object> FFFormat = Object::New(isolate);
	FFFormat->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFFormat"));
	FFFormat->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	FFFormat->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Container Format"));

	const char* FFFormatCurrentValue = config_get_string(config, "AdvOut", "FFFormat");
	if(FFFormatCurrentValue == NULL) {
		FFFormatCurrentValue = "";
	}
	FFFormat->Set(String::NewFromUtf8(isolate, "currentValue"),
					String::NewFromUtf8(isolate, FFFormatCurrentValue));

	// formats.reset(ff_format_supported());
	// const ff_format_desc *format = formats.get();

	// Local<Array> FFFormattValues = Array::New(isolate);
	// int indexFormat = 0;
	// const ff_format_desc *formatDesc = NULL;
	// while(format != nullptr) {
	// 	bool audio = ff_format_desc_has_audio(format);
	// 	bool video = ff_format_desc_has_video(format);
	// 	const char* formatName = ff_format_desc_name(format);

	// 	std::string encoderDisplayName;
	// 	if (audio || video) {
	// 		if(strcmp(formatName, FFFormatCurrentValue) == 0) {
	// 			formatDesc = format;
	// 		}
	// 		Local<Object> object = Object::New(isolate);
	// 		encoderDisplayName = formatName;
	// 		if (audio ^ video)
	// 		encoderDisplayName += audio ? "(Audio)" : "(Video)";
	// 		object->Set(String::NewFromUtf8(isolate, encoderDisplayName.c_str()),
	// 						String::NewFromUtf8(isolate, formatName));
	// 		FFFormattValues->Set(indexFormat++, object);
	// 	}

	// 	format = ff_format_desc_next(format);
	// }

	// FFFormat->Set(String::NewFromUtf8(isolate, "values"), FFFormattValues);

	FFFormat->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFFormat->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFFormat->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFFormat);

	// Container Format Description : text									nothing to save

	// Muxer Settings (if any) : edit text 									FFMCustom
	Local<Object> FFMCustom = Object::New(isolate);
	FFMCustom->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFMCustom"));
	FFMCustom->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_EDIT_TEXT"));
	FFMCustom->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Muxer Settings (if any)"));
	const char* FFMCustomCurrentValue = config_get_string(config, "AdvOut", "FFMCustom");
	if(FFMCustomCurrentValue == NULL) {
		FFMCustomCurrentValue = "";
	}
	FFMCustom->Set(String::NewFromUtf8(isolate, "currentValue"),
					String::NewFromUtf8(isolate, FFMCustomCurrentValue));
	FFMCustom->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFMCustom->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFMCustom->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFMCustom);

	// Video Bitrate : int 													FFVBitrate
	Local<Object> FFVBitrate = Object::New(isolate);
	FFVBitrate->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFVBitrate"));
	FFVBitrate->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_INT"));
	FFVBitrate->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Video Bitrate"));
	FFVBitrate->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, config_get_int(config, "AdvOut", "FFVBitrate")));
	FFVBitrate->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFVBitrate->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFVBitrate->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFVBitrate);

	// Keyframe interval (frames) : int 									FFVGOPSize
	Local<Object> FFVGOPSize = Object::New(isolate);
	FFVGOPSize->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFVGOPSize"));
	FFVGOPSize->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_INT"));
	FFVGOPSize->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Keyframe interval (frames)"));
	FFVGOPSize->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, config_get_int(config, "AdvOut", "FFVGOPSize")));
	FFVGOPSize->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFVGOPSize->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFVGOPSize->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFVGOPSize);

	// Rescale Output : boolean												FFRescale
	Local<Object> FFRescale = Object::New(isolate);
	FFRescale->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFRescale"));
	FFRescale->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
	FFRescale->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Rescale Output"));
	bool FFRescaleCurrentValue = config_get_bool(config, "AdvOut", "FFRescale");
	FFRescale->Set(String::NewFromUtf8(isolate, "currentValue"),
									Integer::New(isolate, FFRescaleCurrentValue));
	FFRescale->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFRescale->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFRescale->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFRescale);

	if(FFRescaleCurrentValue) {
		// Output resolutions : list											FFRescaleRes
		Local<Object> FFRescaleRes = Object::New(isolate);
		FFRescaleRes->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFRescaleRes"));
		FFRescaleRes->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
		FFRescaleRes->Set(String::NewFromUtf8(isolate, "description"),
										String::NewFromUtf8(isolate, "Output resolutions"));
		FFRescaleRes->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
		FFRescaleRes->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
		FFRescaleRes->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
		(*subCategoryParameters)->Set(index++, FFRescaleRes);
	}

	// Show all codes (even if potentially incompatible) : boolean			FFIgnoreCompat
	Local<Object> FFIgnoreCompat = Object::New(isolate);
	FFIgnoreCompat->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFIgnoreCompat"));
	FFIgnoreCompat->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_BOOL"));
	FFIgnoreCompat->Set(String::NewFromUtf8(isolate, "description"),
							String::NewFromUtf8(isolate, "Show all codes (even if potentially incompatible)"));
	bool ignore_compatability = config_get_bool(config, "AdvOut", "FFIgnoreCompat");
	FFIgnoreCompat->Set(String::NewFromUtf8(isolate, "currentValue"),
							Integer::New(isolate, ignore_compatability));
	FFIgnoreCompat->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFIgnoreCompat->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFIgnoreCompat->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	(*subCategoryParameters)->Set(index++, FFIgnoreCompat);

	// Video Encoder : list 												FFVEncoder / FFVEncoderId
	Local<Object> FFVEncoder = Object::New(isolate);
	FFVEncoder->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFVEncoder"));
	FFVEncoder->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	FFVEncoder->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Video Encoder"));

	const char* FFVEncoderCurrentValue = config_get_string(config, "AdvOut", "FFVEncoder");
	if(FFVEncoderCurrentValue == NULL) {
		FFVEncoderCurrentValue = "";
	}
	FFVEncoder->Set(String::NewFromUtf8(isolate, "currentValue"),
					String::NewFromUtf8(isolate, FFVEncoderCurrentValue));
	Local<Array> FFVEncoderValues = Array::New(isolate);

	// Audio Encoder : list 												FFAEncoder /FFAEncoderId
	Local<Object> FFAEncoder = Object::New(isolate);
	FFAEncoder->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFAEncoder"));
	FFAEncoder->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	FFAEncoder->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Audio Encoder"));

	const char* FFAEncoderCurrentValue = config_get_string(config, "AdvOut", "FFAEncoder");
	if(FFAEncoderCurrentValue == NULL) {
		FFAEncoderCurrentValue = "";
	}
	FFAEncoder->Set(String::NewFromUtf8(isolate, "currentValue"),
					String::NewFromUtf8(isolate, FFAEncoderCurrentValue));
	Local<Array> FFAEncoderValues = Array::New(isolate);

	// int indexAudioLoop = 0;
	// int indexVideoLoop = 0;
	// bool codec_has_audio = false;
	// bool codec_has_video = false;
	// if(formatDesc != NULL) {
	// 	OBSFFCodecDesc codecDescs(ff_codec_supported(formatDesc,
	// 		ignore_compatability));

	// 	const ff_codec_desc *codec = codecDescs.get();

	// 	while(codec != nullptr) {
	// 		switch (ff_codec_desc_type(codec)) {
	// 			case FF_CODEC_AUDIO:
	// 			{
	// 				Local<Object> object = Object::New(isolate);
	// 				object->Set(String::NewFromUtf8(isolate, ff_codec_desc_long_name(codec)),
	// 								String::NewFromUtf8(isolate, ff_codec_desc_name(codec)));
	// 				if(strcmp(ff_codec_desc_name(codec), FFAEncoderCurrentValue) == 0) {
	// 					config_set_int(config, "AdvOut", "FFAEncoderId", ff_codec_desc_id(codec));
	// 				}
	// 				FFAEncoderValues->Set(indexAudioLoop++, object);
	// 				codec_has_audio = true;
	// 				break;
	// 			}
	// 			case FF_CODEC_VIDEO:
	// 			{
	// 				Local<Object> object = Object::New(isolate);
	// 				object->Set(String::NewFromUtf8(isolate, ff_codec_desc_long_name(codec)),
	// 								String::NewFromUtf8(isolate, ff_codec_desc_name(codec)));
	// 				if(strcmp(ff_codec_desc_name(codec), FFVEncoderCurrentValue) == 0) {
	// 					config_set_int(config, "AdvOut", "FFVEncoderId", ff_codec_desc_id(codec));
	// 				}
	// 				FFVEncoderValues->Set(indexVideoLoop++, object);
	// 				codec_has_video = true;
	// 				break;
	// 			}
	// 			default:
	// 				break;
	// 		}
	// 		codec = ff_codec_desc_next(codec);
	// 	}
	// }

	// FFVEncoder->Set(String::NewFromUtf8(isolate, "values"), FFVEncoderValues);
	// FFAEncoder->Set(String::NewFromUtf8(isolate, "values"), FFAEncoderValues);

	FFVEncoder->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFVEncoder->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFVEncoder->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));
	
	FFAEncoder->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	// FFAEncoder->Set(String::NewFromUtf8(isolate, "enabled"), Integer::New(isolate, codec_has_audio));
	FFAEncoder->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFAEncoder->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));

	// Video Encoder Settings (if any) : edit text 							FFVCustom
	Local<Object> FFVCustom = Object::New(isolate);
	FFVCustom->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFVCustom"));
	FFVCustom->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_EDIT_TEXT"));
	FFVCustom->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Video Encoder Settings (if any)"));
	const char* FFVCustomCurrentValue = config_get_string(config, "AdvOut", "FFVCustom");
	if(FFVCustomCurrentValue == NULL) {
		FFVCustomCurrentValue = "";
	}
	FFVCustom->Set(String::NewFromUtf8(isolate, "currentValue"),
							String::NewFromUtf8(isolate, FFVCustomCurrentValue));
	FFVCustom->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	FFVCustom->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFVCustom->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));

	// Audio Bitrate : list 												FFABitrate
	Local<Object> FFABitrate = Object::New(isolate);
	FFABitrate->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFABitrate"));
	FFABitrate->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	FFABitrate->Set(String::NewFromUtf8(isolate, "description"),
								String::NewFromUtf8(isolate, "Audio Bitrate"));
	FFABitrate->Set(String::NewFromUtf8(isolate, "currentValue"),
								Integer::New(isolate, config_get_int(config, "AdvOut", "FFABitrate")));
	FFABitrate->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	// FFABitrate->Set(String::NewFromUtf8(isolate, "enabled"), Integer::New(isolate, codec_has_audio));
	FFABitrate->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFABitrate->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));

	// Audio track : list 													FFAudioTrack
	Local<Object> FFAudioTrack = Object::New(isolate);
	FFAudioTrack->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFAudioTrack"));
	FFAudioTrack->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	FFAudioTrack->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Audio track"));
	Local<Array> TrackIndexValues = Array::New(isolate);

	Local<Object> track1 = Object::New(isolate);
	track1->Set(String::NewFromUtf8(isolate, "1"), String::NewFromUtf8(isolate, "1"));
	TrackIndexValues->Set(0, track1);

	Local<Object> track2 = Object::New(isolate);
	track2->Set(String::NewFromUtf8(isolate, "2"), String::NewFromUtf8(isolate, "2"));
	TrackIndexValues->Set(1, track2);

	Local<Object> track3 = Object::New(isolate);
	track3->Set(String::NewFromUtf8(isolate, "3"), String::NewFromUtf8(isolate, "3"));
	TrackIndexValues->Set(2, track3);

	Local<Object> track4 = Object::New(isolate);
	track4->Set(String::NewFromUtf8(isolate, "4"), String::NewFromUtf8(isolate, "4"));
	TrackIndexValues->Set(3, track4);

	Local<Object> track5 = Object::New(isolate);
	track5->Set(String::NewFromUtf8(isolate, "5"), String::NewFromUtf8(isolate, "5"));
	TrackIndexValues->Set(4, track5);

	Local<Object> track6 = Object::New(isolate);
	track6->Set(String::NewFromUtf8(isolate, "6"), String::NewFromUtf8(isolate, "6"));
	TrackIndexValues->Set(5, track6);

	FFAudioTrack->Set(String::NewFromUtf8(isolate, "values"), TrackIndexValues);
	FFAudioTrack->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	// FFAudioTrack->Set(String::NewFromUtf8(isolate, "enabled"), Integer::New(isolate, codec_has_audio));
	FFAudioTrack->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFAudioTrack->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));

	// Audio Encoder Settings (if any) : edit text 							FFACustom
	Local<Object> FFACustom = Object::New(isolate);
	FFACustom->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "FFACustom"));
	FFACustom->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_EDIT_TEXT"));
	FFACustom->Set(String::NewFromUtf8(isolate, "description"),
									String::NewFromUtf8(isolate, "Audio Encoder Settings (if any)"));

	const char* FFACustomCurrentValue = config_get_string(config, "AdvOut", "FFACustom");
	if(FFACustomCurrentValue == NULL) {
		FFACustomCurrentValue = "";
	}
	FFACustom->Set(String::NewFromUtf8(isolate, "currentValue"),
					String::NewFromUtf8(isolate, FFACustomCurrentValue));
	FFACustom->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	// FFACustom->Set(String::NewFromUtf8(isolate, "enabled"), Integer::New(isolate, codec_has_audio));
	FFACustom->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	FFACustom->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));


	(*subCategoryParameters)->Set(index++, FFVEncoder);
	(*subCategoryParameters)->Set(index++, FFVCustom);
	(*subCategoryParameters)->Set(index++, FFABitrate);
	(*subCategoryParameters)->Set(index++, FFAudioTrack);
	(*subCategoryParameters)->Set(index++, FFAEncoder);
	(*subCategoryParameters)->Set(index++, FFACustom);
}

Local<Object> OBS_settings::getAdvancedOutputRecordingSettings(config_t* config, bool isCategoryEnabled)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> subCategoryParameters = Array::New(isolate);
	Local<Object> subCategory = Object::New(isolate);

	int index = 0;

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	// Type : list
	Local<Object> RecType = Object::New(isolate);
	RecType->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, "RecType"));
	RecType->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, "OBS_PROPERTY_LIST"));
	RecType->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, "Type"));
	Local<Array> RecTypeValues = Array::New(isolate);

	Local<Object> Standard = Object::New(isolate);
	Standard->Set(String::NewFromUtf8(isolate, "Standard"), String::NewFromUtf8(isolate, "Standard"));
	RecTypeValues->Set(0, Standard);

	Local<Object> Custom = Object::New(isolate);
	Custom->Set(String::NewFromUtf8(isolate, "Custom Output (FFmpeg)"), String::NewFromUtf8(isolate, "Custom Output (FFmpeg)"));
	RecTypeValues->Set(1, Custom);

	RecType->Set(String::NewFromUtf8(isolate, "values"), RecTypeValues);

	const char* RecTypeCurrentValue = config_get_string(config, "AdvOut", "RecType");
	if(RecTypeCurrentValue == NULL) {
		RecTypeCurrentValue = "";
	}
	RecType->Set(String::NewFromUtf8(isolate, "currentValue"),
					String::NewFromUtf8(isolate, RecTypeCurrentValue));

	RecType->Set(String::NewFromUtf8(isolate, "visible"), Boolean::New(isolate, true));
	RecType->Set(String::NewFromUtf8(isolate, "enabled"), Boolean::New(isolate, isCategoryEnabled));
	RecType->Set(String::NewFromUtf8(isolate, "masked"), Boolean::New(isolate, false));

	subCategoryParameters->Set(index++, RecType);

	const char* currentRecType = config_get_string(config, "AdvOut", "RecType");

	if(currentRecType == NULL) {
		currentRecType = "Standard";
		config_set_string(config, "AdvOut", "RecType", currentRecType);
	}

	if(strcmp(currentRecType, "Standard") == 0) {
		getStandardRecordingSettings(&subCategoryParameters, config, isCategoryEnabled);
	} else if (strcmp(currentRecType, "Custom Output (FFmpeg)") == 0) {
		getFFmpegOutputRecordingSettings(&subCategoryParameters, config, isCategoryEnabled);
	} else {
		getStandardRecordingSettings(&subCategoryParameters, config, isCategoryEnabled);
	}

	subCategory->Set(String::NewFromUtf8(isolate, "nameSubCategory"), String::NewFromUtf8(isolate, "Recording"));
	subCategory->Set(String::NewFromUtf8(isolate, "parameters"), subCategoryParameters);

	return subCategory;
}

void OBS_settings::getAdvancedOutputAudioSettings(Local<Array> outputSettings, config_t* config, bool isCategoryEnabled)
{
	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	auto &bitrateMap = GetAACEncoderBitrateMap();

	// Track 1
	std::vector<std::pair<std::string, std::string>> Track1Bitrate;
	Track1Bitrate.push_back(std::make_pair("name", "Track1Bitrate"));
	Track1Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track1Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));

	for (auto &entry : bitrateMap)
		Track1Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track1Bitrate);

	std::vector<std::pair<std::string, std::string>> Track1Name;
	Track1Name.push_back(std::make_pair("name", "Track1Name"));
	Track1Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track1Name.push_back(std::make_pair("description", "Name"));
	entries.push_back(Track1Name);

	outputSettings->Set(3, serializeSettingsData("Audio - Track 1", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 2
	std::vector<std::pair<std::string, std::string>> Track2Bitrate;
	Track2Bitrate.push_back(std::make_pair("name", "Track2Bitrate"));
	Track2Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track2Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));

	for (auto &entry : bitrateMap)
		Track2Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track2Bitrate);

	std::vector<std::pair<std::string, std::string>> Track2Name;
	Track2Name.push_back(std::make_pair("name", "Track2Name"));
	Track2Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track2Name.push_back(std::make_pair("description", "Name"));
	entries.push_back(Track2Name);

	outputSettings->Set(4, serializeSettingsData("Audio - Track 2", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 3
	std::vector<std::pair<std::string, std::string>> Track3Bitrate;
	Track3Bitrate.push_back(std::make_pair("name", "Track3Bitrate"));
	Track3Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track3Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));

	for (auto &entry : bitrateMap)
		Track3Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track3Bitrate);

	std::vector<std::pair<std::string, std::string>> Track3Name;
	Track3Name.push_back(std::make_pair("name", "nameTrack3"));
	Track3Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track3Name.push_back(std::make_pair("description", "Name"));
	entries.push_back(Track3Name);
	outputSettings->Set(5, serializeSettingsData("Audio - Track 3", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();


	// Track 4
	std::vector<std::pair<std::string, std::string>> Track4Bitrate;
	Track4Bitrate.push_back(std::make_pair("name", "Track4Bitrate"));
	Track4Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track4Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));

	for (auto &entry : bitrateMap)
		Track4Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track4Bitrate);

	std::vector<std::pair<std::string, std::string>> Track4Name;
	Track4Name.push_back(std::make_pair("name", "nameTrack4"));
	Track4Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track4Name.push_back(std::make_pair("description", "Name"));
	entries.push_back(Track4Name);

	outputSettings->Set(6, serializeSettingsData("Audio - Track 4", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 5
	std::vector<std::pair<std::string, std::string>> Track5Bitrate;
	Track5Bitrate.push_back(std::make_pair("name", "Track5Bitrate"));
	Track5Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track5Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));

	for (auto &entry : bitrateMap)
		Track5Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track5Bitrate);

	std::vector<std::pair<std::string, std::string>> Track5Name;
	Track5Name.push_back(std::make_pair("name", "nameTrack5"));
	Track5Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track5Name.push_back(std::make_pair("description", "Name"));
	entries.push_back(Track5Name);

	outputSettings->Set(7, serializeSettingsData("Audio - Track 5", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 6
	std::vector<std::pair<std::string, std::string>> Track6Bitrate;
	Track6Bitrate.push_back(std::make_pair("name", "Track6Bitrate"));
	Track6Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track6Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));

	for (auto &entry : bitrateMap)
		Track6Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track6Bitrate);

	std::vector<std::pair<std::string, std::string>> Track6Name;
	Track6Name.push_back(std::make_pair("name", "Track6Name"));
	Track6Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track6Name.push_back(std::make_pair("description", "Name"));
	entries.push_back(Track6Name);

	outputSettings->Set(8, serializeSettingsData("Audio - Track 6", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();
}

void OBS_settings::getAdvancedOutputSettings(Local<Array> outputSettings, config_t* config, bool isCategoryEnabled)
{
	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	// Streaming
	outputSettings->Set(1, getAdvancedOutputStreamingSettings(config, isCategoryEnabled));

	// Recording
	outputSettings->Set(2, getAdvancedOutputRecordingSettings(config, isCategoryEnabled));

	// Audio
	getAdvancedOutputAudioSettings(outputSettings, config, isCategoryEnabled);
}

Local<Array> OBS_settings::getOutputSettings()
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> outputSettings = Array::New(isolate);

	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t* config = OBS_API::openConfigFile(basicConfigFile);

	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive();

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	//Output mode
	Local<Array> subCategoryParametersUntitled = Array::New(isolate);

	std::vector<std::pair<std::string, std::string>> outputMode;

	outputMode.push_back(std::make_pair("name", "Mode"));
	outputMode.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	outputMode.push_back(std::make_pair("description", "Output Mode"));
	outputMode.push_back(std::make_pair("Simple", "Simple"));
	outputMode.push_back(std::make_pair("Advanced", "Advanced"));
	entries.push_back(outputMode);

	outputSettings->Set(0, serializeSettingsData("Untitled", entries, config, "Output", true, isCategoryEnabled));
	entries.clear();

	const char* currentOutputMode = config_get_string(config, "Output", "Mode");

	if (currentOutputMode == NULL) {
		currentOutputMode = "Simple";
	}

	if(strcmp(currentOutputMode, "Simple") == 0) {
		getSimpleOutputSettings(outputSettings, config, isCategoryEnabled);
	} else if (strcmp(currentOutputMode, "Advanced") == 0) {
		getAdvancedOutputSettings(outputSettings, config, isCategoryEnabled);
	} else {
		// We dont't have a great way right now to tell the frontend that an error occured
		// The default output mode is the simple one
		getSimpleOutputSettings(outputSettings, config, isCategoryEnabled);
	}


	return outputSettings;
}

void OBS_settings::saveSimpleOutputSettings(Local<Array> settings, std::string basicConfigFile)
{
	saveGenericSettings(settings, "SimpleOutput", basicConfigFile);
}

void OBS_settings::saveAdvancedOutputStreamingSettings(Local<Array> settings, std::string basicConfigFile)
{
	config_t* config = OBS_API::openConfigFile(basicConfigFile);
	Isolate *isolate = v8::Isolate::GetCurrent();
	int indexStreamingCategory = 1;
	Local<Array> streamSettings = v8::Local<v8::Array>::Cast(settings->Get(indexStreamingCategory));
	std::string section = "AdvOut";

	Local<Array> parameters = v8::Local<v8::Array>::Cast(streamSettings->Get(String::NewFromUtf8(isolate, "parameters")));

	obs_encoder_t* encoder = OBS_service::getStreamingEncoder();
	obs_data_t* encoderSettings = obs_encoder_get_settings(encoder);

	int indexEncoderSettings = 4;
	
	for(int i=0;i<parameters->Length();i++) {
		Local<Object> parameter = v8::Local<v8::Object>::Cast(parameters->Get(i));

		std::string name, type, value;

		v8::String::Utf8Value param0(parameter->Get(String::NewFromUtf8(isolate, "name")));
		name = std::string(*param0);

		v8::String::Utf8Value param1(parameter->Get(String::NewFromUtf8(isolate, "type")));
		type = std::string(*param1);

		if(type.compare("OBS_PROPERTY_LIST") == 0 ||
			type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 ||
			type.compare("OBS_PROPERTY_PATH") == 0 ||
			type.compare("OBS_PROPERTY_TEXT") == 0) {
			v8::String::Utf8Value param2(parameter->Get(String::NewFromUtf8(isolate, "currentValue")));
			value = std::string(*param2);
			if(i < indexEncoderSettings) {
				config_set_string(config, section.c_str(), name.c_str(), value.c_str());
			} else {
				obs_data_set_string(encoderSettings, name.c_str(), value.c_str());
			}
		} else if(type.compare("OBS_PROPERTY_INT") == 0) {
			int64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				config_set_int(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if(type.compare("OBS_PROPERTY_UINT") == 0) {
			uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				config_set_uint(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if(type.compare("OBS_PROPERTY_BOOL") == 0) {
			uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				if(name.compare("Rescale") == 0 && value) {
					indexEncoderSettings ++;
				}
				config_set_bool(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_bool(encoderSettings, name.c_str(), value);
			}
		} else if(type.compare("OBS_PROPERTY_DOUBLE") == 0) {
			double value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				config_set_double(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_double(encoderSettings, name.c_str(), value);
			}
		} else {
			std::cout << "type not found ! " << type.c_str() << std::endl;
		}
	}

	int ret = config_save_safe(config, "tmp", nullptr);
	if (ret != 0) {
		blog(LOG_WARNING, "Failed to config file %s", basicConfigFile.c_str());
	}

	obs_encoder_update(encoder, encoderSettings);

	std::string path = OBS_API::getStreamingEncoderConfigPath();
	if (!obs_data_save_json_safe(encoderSettings, path.c_str(), "tmp", "bak")) {
		blog(LOG_WARNING, "Failed to save encoder %s", path.c_str());
	}
}

void OBS_settings::saveAdvancedOutputRecordingSettings(Local<Array> settings, std::string basicConfigFile)
{
	config_t* config = OBS_API::openConfigFile(basicConfigFile);
	Isolate *isolate = v8::Isolate::GetCurrent();
	int indexStreamingCategory = 2;
	Local<Array> recordSettings = v8::Local<v8::Array>::Cast(settings->Get(indexStreamingCategory));
	std::string section = "AdvOut";

	Local<Array> parameters =
		v8::Local<v8::Array>::Cast(recordSettings->Get(String::NewFromUtf8(isolate, "parameters")));

	obs_encoder_t* encoder = OBS_service::getRecordingEncoder();
	obs_data_t* encoderSettings = obs_encoder_get_settings(encoder);

	int indexEncoderSettings = 9;
	
	for(int i=0;i<parameters->Length();i++) {
		Local<Object> parameter = v8::Local<v8::Object>::Cast(parameters->Get(i));

		std::string name, type, value;

		v8::String::Utf8Value param0(parameter->Get(String::NewFromUtf8(isolate, "name")));
		name = std::string(*param0);

		if(name.compare("RecType") == 0) {
			v8::String::Utf8Value param2(parameter->Get(String::NewFromUtf8(isolate, "currentValue")));
			value = std::string(*param2);
			if(value.compare("Custom Output (FFmpeg)") == 0) {
				indexEncoderSettings = parameters->Length();
			}
		}

		v8::String::Utf8Value param1(parameter->Get(String::NewFromUtf8(isolate, "type")));
		type = std::string(*param1);

		if(type.compare("OBS_PROPERTY_LIST") == 0 ||
			type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 ||
			type.compare("OBS_PROPERTY_PATH") == 0) {
			v8::String::Utf8Value param2(parameter->Get(String::NewFromUtf8(isolate, "currentValue")));
			value = std::string(*param2);
			if(i < indexEncoderSettings) {
				config_set_string(config, section.c_str(), name.c_str(), value.c_str());
			} else {
				obs_data_set_string(encoderSettings, name.c_str(), value.c_str());
			}
		} else if(type.compare("OBS_PROPERTY_INT") == 0) {
			int64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				config_set_int(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if(type.compare("OBS_PROPERTY_UINT") == 0) {
			uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				config_set_uint(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), value);
			}
		} else if(type.compare("OBS_PROPERTY_BOOL") == 0) {
			uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				if(name.compare("RecRescale") == 0 && value) {
					indexEncoderSettings ++;
				}
				config_set_bool(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_bool(encoderSettings, name.c_str(), value);
			}
		} else if(type.compare("OBS_PROPERTY_DOUBLE") == 0) {
			double value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
			if(i < indexEncoderSettings) {
				config_set_double(config, section.c_str(), name.c_str(), value);
			} else {
				obs_data_set_double(encoderSettings, name.c_str(), value);
			}
		} else {
			std::cout << "type not found ! " << type.c_str() << std::endl;
		}
	}

	int ret = config_save_safe(config, "tmp", nullptr);
	if (ret != 0) {
		blog(LOG_WARNING, "Failed to config file %s", basicConfigFile.c_str());
	}
	obs_encoder_update(encoder, encoderSettings);

	std::string path = OBS_API::getRecordingEncoderConfigPath();
	if (!obs_data_save_json_safe(encoderSettings, path.c_str(), "tmp", "bak")) {
		blog(LOG_WARNING, "Failed to save encoder %s", path.c_str());
	}
}

void OBS_settings::saveAdvancedOutputSettings(Local<Array> settings, std::string basicConfigFile)
{
	// Streaming
	saveAdvancedOutputStreamingSettings(settings, basicConfigFile);

	// Recording
	saveAdvancedOutputRecordingSettings(settings, basicConfigFile);

	// Audio
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> audioSettings = Array::New(isolate);
	int indextrack = 3;

	for(int i=0; i < 6;i++) {
		audioSettings->Set(0, v8::Local<v8::Array>::Cast(settings->Get(indextrack++)));
		saveGenericSettings(audioSettings, "AdvOut", basicConfigFile);
	}
}

bool useAdvancedOutput;

void OBS_settings::saveOutputSettings(Local<Array> settings)
{
	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t* config = OBS_API::openConfigFile(basicConfigFile);

	// Get selected output mode
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Object> outputModeObject = v8::Local<v8::Object>::Cast(settings->Get(0));
	Local<Array> parameters = v8::Local<v8::Array>::Cast(outputModeObject->Get(String::NewFromUtf8(isolate, "parameters")));
	Local<Object> parameter = v8::Local<v8::Object>::Cast(parameters->Get(0));

	v8::String::Utf8Value param2(parameter->Get(String::NewFromUtf8(isolate, "currentValue")));
	std::string currentOutputMode = std::string(*param2);

	config_set_string(config, "Output", "Mode", currentOutputMode.c_str());
	config_save_safe(config, "tmp", nullptr);

	if (currentOutputMode.compare("Advanced") == 0) {
		if(useAdvancedOutput) {
			saveAdvancedOutputSettings(settings, basicConfigFile);
		}
		useAdvancedOutput = true;
	} else {
		if(!useAdvancedOutput) {
		saveSimpleOutputSettings(settings, basicConfigFile);
	}
		useAdvancedOutput = false;
	}
}

static void LoadListValues(obs_property_t *prop, int index, std::vector<std::pair<std::string, std::string>>* listValues)
{
	size_t count = obs_property_list_item_count(prop);

	obs_source_t *source = obs_get_output_source(index);
	const char *deviceId = nullptr;
	obs_data_t *settings = nullptr;

	if (source) {
		settings = obs_source_get_settings(source);
		if (settings)
			deviceId = obs_data_get_string(settings, "device_id");
	}

	for (size_t i = 0; i < count; i++) {
		std::string name = obs_property_list_item_name(prop, i);
		std::string val  = obs_property_list_item_string(prop, i);
		listValues->push_back(std::make_pair(name, val));
	}	

	if (settings)
		obs_data_release(settings);
	if (source)
		obs_source_release(source);	
}

static void LoadAudioDevices(std::vector<std::pair<std::string, std::string>>* desktopAudioDevice, 
					 		 std::vector<std::pair<std::string, std::string>>* desktopAudioDevice2,
					 		 std::vector<std::pair<std::string, std::string>>* auxAudioDevice,
					 		 std::vector<std::pair<std::string, std::string>>* auxAudioDevice2,
					 		 std::vector<std::pair<std::string, std::string>>* auxAudioDevice3) 
{
	const char *input_id  = "wasapi_input_capture";
	const char *output_id = "wasapi_output_capture";

	obs_properties_t *input_props = obs_get_source_properties(input_id);
	obs_properties_t *output_props = obs_get_source_properties(output_id);

	if (output_props) {
		obs_property_t *outputs = obs_properties_get(output_props,
				"device_id");
		LoadListValues(outputs, 1, desktopAudioDevice);
		LoadListValues(outputs, 2, desktopAudioDevice2);
		obs_properties_destroy(output_props);
	}

	if (input_props) {
		obs_property_t *inputs  = obs_properties_get(input_props,
				"device_id");
		LoadListValues(inputs, 3, auxAudioDevice);
		LoadListValues(inputs, 4, auxAudioDevice2);
		LoadListValues(inputs, 5, auxAudioDevice3);
		obs_properties_destroy(input_props);
	}
}

Local<Array> OBS_settings::getAudioSettings()
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> audioSettings = Array::New(isolate);

	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t* config = OBS_API::openConfigFile(basicConfigFile);

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	//Untitled

	//Sample Rate
	std::vector<std::pair<std::string, std::string>> sampleRate;
	sampleRate.push_back(std::make_pair("name", "SampleRate"));
	sampleRate.push_back(std::make_pair("type", "OBS_PROPERTY_UINT"));
	sampleRate.push_back(std::make_pair("description", "Sample Rate"));
	entries.push_back(sampleRate);

	//Channels
	std::vector<std::pair<std::string, std::string>> channelSetup;
	channelSetup.push_back(std::make_pair("name", "ChannelSetup"));
	channelSetup.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	channelSetup.push_back(std::make_pair("description", "Channels"));
	channelSetup.push_back(std::make_pair("Mono", "Mono"));
	channelSetup.push_back(std::make_pair("Stereo", "Stereo"));
	entries.push_back(channelSetup);

	//Desktop Audio Device
	std::vector<std::pair<std::string, std::string>> desktopAudioDevice;
	desktopAudioDevice.push_back(std::make_pair("name", "Desktop Audio"));
	desktopAudioDevice.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	desktopAudioDevice.push_back(std::make_pair("description", "Desktop Audio Device"));

	//Desktop Audio Device 2
	std::vector<std::pair<std::string, std::string>> desktopAudioDevice2;
	desktopAudioDevice2.push_back(std::make_pair("name", "Desktop Audio 2"));
	desktopAudioDevice2.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	desktopAudioDevice2.push_back(std::make_pair("description", "Desktop Audio Device 2"));

	//Mic/Auxiliary Audio Device
	std::vector<std::pair<std::string, std::string>> auxAudioDevice;
	auxAudioDevice.push_back(std::make_pair("name", "Mic/Aux"));
	auxAudioDevice.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	auxAudioDevice.push_back(std::make_pair("description", "Mic/Auxiliary Audio Device"));

	//Mic/Auxiliary Audio Device 2
	std::vector<std::pair<std::string, std::string>> auxAudioDevice2;
	auxAudioDevice2.push_back(std::make_pair("name", "Mic/Aux 2"));
	auxAudioDevice2.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	auxAudioDevice2.push_back(std::make_pair("description", "Mic/Auxiliary Audio Device 2"));

	//Mic/Auxiliary Audio Device 3
	std::vector<std::pair<std::string, std::string>> auxAudioDevice3;
	auxAudioDevice3.push_back(std::make_pair("name", "Mic/Aux 3"));
	auxAudioDevice3.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	auxAudioDevice3.push_back(std::make_pair("description", "Mic/Auxiliary Audio Device 3"));

	LoadAudioDevices(&desktopAudioDevice, &desktopAudioDevice2,
					 &auxAudioDevice, &auxAudioDevice2, &auxAudioDevice3);

	entries.push_back(desktopAudioDevice);
	entries.push_back(desktopAudioDevice2);
	entries.push_back(auxAudioDevice);
	entries.push_back(auxAudioDevice2);
	entries.push_back(auxAudioDevice3);

	audioSettings->Set(0, serializeSettingsData("Untitled", entries, config, "Audio", true, true));
	entries.clear();

	return audioSettings;
}

void resetAudioDevice(const char *sourceId, const char *deviceId,
		const char *deviceDesc, int channel)
{
	obs_source_t *source;
	obs_data_t *settings;
	bool same = false;

	source = obs_get_output_source(channel);
	if (source) {
		settings = obs_source_get_settings(source);
		const char *curId = obs_data_get_string(settings, "device_id");

		same = (strcmp(curId, deviceId) == 0);

		obs_data_release(settings);
		obs_source_release(source);
	}

	if (!same)
		obs_set_output_source(channel, nullptr);

	if (!same && strcmp(deviceId, "disabled") != 0) {
		obs_data_t *settings = obs_data_create();
		obs_data_set_string(settings, "device_id", deviceId);
		source = obs_source_create(sourceId, deviceDesc, settings,
				nullptr);
		obs_data_release(settings);

		obs_set_output_source(channel, source);
		obs_source_release(source);
	}
}

void OBS_settings::saveAudioSettings(Local<Array> audioSettings)
{
	Isolate *isolate = v8::Isolate::GetCurrent();

	Local<Object> audioSettingsObject = v8::Local<v8::Object>::Cast(audioSettings->Get(0));
    Local<Array> parameters = v8::Local<v8::Array>::Cast(audioSettingsObject->Get(String::NewFromUtf8(isolate, "parameters")));

	std::string audioType;

	for(int i=2;i<parameters->Length();i++) {
		Local<Object> audioDevice = v8::Local<v8::Object>::Cast(parameters->Get(i));

		v8::String::Utf8Value param0(audioDevice->Get(String::NewFromUtf8(isolate, "name")));
		v8::String::Utf8Value param1(audioDevice->Get(String::NewFromUtf8(isolate, "currentValue")));

		std::string name = std::string(*param0);
		std::string currentValue = std::string(*param1);

		if(i < 4) 
			audioType = "wasapi_output_capture";
		else 
			audioType = "wasapi_input_capture";

		resetAudioDevice(audioType.c_str(), currentValue.c_str(), name.c_str(), i);
	}
}

static inline void LoadFPSCommon(config_t* config)
{
	const char *val = config_get_string(config, "Video", "FPSCommon");
}

static inline void LoadFPSInteger(config_t* config)
{
	uint32_t val = config_get_uint(config, "Video", "FPSInt");
}

static inline void LoadFPSFraction(config_t* config)
{
	uint32_t num = config_get_uint(config, "Video", "FPSNum");
	uint32_t den = config_get_uint(config, "Video", "FPSDen");
}

std::vector<pair<int, int>> OBS_settings::getOutputResolutions(int base_cx, int base_cy)
{
	std::vector<pair<int, int>> outputResolutions;
	for (size_t idx = 0; idx < numVals; idx++) {
		uint32_t outDownscaleCX = uint32_t(double(base_cx) / vals[idx]);
		uint32_t outDownscaleCY = uint32_t(double(base_cy) / vals[idx]);

		outputResolutions.push_back(std::make_pair(outDownscaleCX, outDownscaleCY));
	}
	return outputResolutions;
}

Local<Array>  OBS_settings::getVideoSettings()
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> videoSettings = Array::New(isolate);

	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t* config = OBS_API::openConfigFile(basicConfigFile);

	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive();

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	//Base (Canvas) Resolution
	std::vector<std::pair<std::string, std::string>> baseResolution;
	baseResolution.push_back(std::make_pair("name", "Base"));
	baseResolution.push_back(std::make_pair("type", "OBS_INPUT_RESOLUTION_LIST"));
	baseResolution.push_back(std::make_pair("description", "Base (Canvas) Resolution"));

	uint32_t base_cx = config_get_uint(config, "Video", "BaseCX");
	uint32_t base_cy = config_get_uint(config, "Video", "BaseCY");

	std::string baseResolutionString = ResString(base_cx, base_cy);

	baseResolution.push_back(std::make_pair("1920x1080", "1920x1080")); 
	baseResolution.push_back(std::make_pair("1280x720", "1280x720")); 

	std::vector<Screen> resolutions = OBS_API::availableResolutions();

	// Fill available display resolutions
	for(int i=0;i<resolutions.size();i++) {
		std::string baseResolutionString;
	    baseResolutionString = to_string(resolutions.at(i).width);
	    baseResolutionString += "x";
	    baseResolutionString += to_string(resolutions.at(i).height);

	    pair<std::string, std::string> newBaseResolution = std::make_pair(baseResolutionString.c_str(), baseResolutionString.c_str()); 

	    std::vector<pair<std::string, std::string>>::iterator it = 
	    	std::find_if(baseResolution.begin(), baseResolution.end(), 
	        	[&baseResolutionString] (const pair<std::string, std::string> value)
	        	{
	        		return (value.second.compare(baseResolutionString) == 0);
	        	});
	        
	    if(baseResolution.size() == 3 || it == baseResolution.end()) {
	    	baseResolution.push_back(newBaseResolution);  
	    }	
	}
    
	// Set the current base resolution selected by the user
    pair<std::string, std::string> newBaseResolution = std::make_pair("currentValue", baseResolutionString);

    //Check if the current resolution is in the available ones
    std::vector<pair<std::string, std::string>>::iterator it = 
        std::find_if(baseResolution.begin(), baseResolution.end(), 
        	[&baseResolutionString] (const pair<std::string, std::string> value)
        	{
        		return (value.second.compare(baseResolutionString) == 0);
        	});

	if (it == baseResolution.end()) {
		baseResolution.push_back(std::make_pair(baseResolutionString.c_str(), baseResolutionString.c_str()));
	}

	int indexFirstValue = 3;
	baseResolution.insert(baseResolution.begin() + indexFirstValue, newBaseResolution);

	entries.push_back(baseResolution);

	std::vector<std::pair<std::string, std::string>> outputResolution;
	outputResolution.push_back(std::make_pair("name", "Output"));
	outputResolution.push_back(std::make_pair("type", "OBS_INPUT_RESOLUTION_LIST"));
	outputResolution.push_back(std::make_pair("description", "Output (Scaled) Resolution"));

	uint32_t out_cx = config_get_uint(config, "Video", "OutputCX");
	uint32_t out_cy = config_get_uint(config, "Video", "OutputCY");

	std::string outputResString = ResString(out_cx, out_cy);

	outputResolution.push_back(std::make_pair("currentValue", outputResString));

	std::vector<pair<int, int>> outputResolutions = getOutputResolutions(base_cx, base_cy);

	for(int i = 0;i<outputResolutions.size();i++) {
		string outRes = ResString(outputResolutions.at(i).first, outputResolutions.at(i).second);
		outputResolution.push_back(std::make_pair(outRes, outRes));
	}

	entries.push_back(outputResolution);

	//Downscale Filter
	std::vector<std::pair<std::string, std::string>> scaleType;
	scaleType.push_back(std::make_pair("name", "ScaleType"));
	scaleType.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	scaleType.push_back(std::make_pair("description", "Downscale Filter"));
	scaleType.push_back(std::make_pair("Bilinear (Fastest, but blurry if scaling)", "bilinear"));
	scaleType.push_back(std::make_pair("Bicubic (Sharpened scaling, 16 samples", "bicubic"));
	scaleType.push_back(std::make_pair("Lanczos (Sharpened scaling, 32 samples)", "lanczos"));
	entries.push_back(scaleType);

	//FPS Type
	std::vector<std::pair<std::string, std::string>> fpsType;
	fpsType.push_back(std::make_pair("name", "FPSType"));
	fpsType.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	fpsType.push_back(std::make_pair("description", "FPS Type"));

	uint32_t fpsTypeValue = config_get_uint(config, "Video", "FPSType");

	if(fpsTypeValue == 0) {
		fpsType.push_back(std::make_pair("currentValue", "Common FPS Values"));
		fpsType.push_back(std::make_pair("Common FPS Values", "Common FPS Values"));
		fpsType.push_back(std::make_pair("Integer FPS Value", "Integer FPS Value"));
		fpsType.push_back(std::make_pair("Fractional FPS Value", "Fractional FPS Value"));
		entries.push_back(fpsType);

		//Common FPS Values
		std::vector<std::pair<std::string, std::string>> fpsCommon;
		fpsCommon.push_back(std::make_pair("name", "FPSCommon"));
		fpsCommon.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
		fpsCommon.push_back(std::make_pair("description", "Common FPS Values"));
		fpsCommon.push_back(std::make_pair("10", "10"));
		fpsCommon.push_back(std::make_pair("20", "20"));
		fpsCommon.push_back(std::make_pair("24 NTSC", "24 NTSC"));
		fpsCommon.push_back(std::make_pair("29.97", "29.97"));
		fpsCommon.push_back(std::make_pair("30", "30"));
		fpsCommon.push_back(std::make_pair("48", "48"));
		fpsCommon.push_back(std::make_pair("59.94", "59.94"));
		fpsCommon.push_back(std::make_pair("60", "60"));
		entries.push_back(fpsCommon);
	} else if (fpsTypeValue == 1) {
		fpsType.push_back(std::make_pair("currentValue", "Integer FPS Value"));
		fpsType.push_back(std::make_pair("Common FPS Values", "Common FPS Values"));
		fpsType.push_back(std::make_pair("Integer FPS Value", "Integer FPS Value"));
		fpsType.push_back(std::make_pair("Fractional FPS Value", "Fractional FPS Value"));
		entries.push_back(fpsType);

		std::vector<std::pair<std::string, std::string>> fpsInt;
		fpsInt.push_back(std::make_pair("name", "FPSInt"));
		fpsInt.push_back(std::make_pair("type", "OBS_PROPERTY_UINT"));
		fpsInt.push_back(std::make_pair("description", "Integer FPS Value"));
		entries.push_back(fpsInt);
	} else if (fpsTypeValue == 2) {
		fpsType.push_back(std::make_pair("currentValue", "Fractional FPS Value"));
		fpsType.push_back(std::make_pair("Common FPS Values", "Common FPS Values"));
		fpsType.push_back(std::make_pair("Integer FPS Value", "Integer FPS Value"));
		fpsType.push_back(std::make_pair("Fractional FPS Value", "Fractional FPS Value"));
		entries.push_back(fpsType);

		std::vector<std::pair<std::string, std::string>> fpsNum;
		fpsNum.push_back(std::make_pair("name", "FPSNum"));
		fpsNum.push_back(std::make_pair("type", "OBS_PROPERTY_UINT"));
		fpsNum.push_back(std::make_pair("description", "FPSNum"));
		entries.push_back(fpsNum);

		std::vector<std::pair<std::string, std::string>> fpsDen;
		fpsDen.push_back(std::make_pair("name", "FPSDen"));
		fpsDen.push_back(std::make_pair("type", "OBS_PROPERTY_UINT"));
		fpsDen.push_back(std::make_pair("description", "FPSDen"));
		entries.push_back(fpsDen);
	}

	videoSettings->Set(0, serializeSettingsData("Untitled", entries, config, "Video", true, isCategoryEnabled));
	entries.clear();

	return videoSettings;
}

struct BaseLexer {
	lexer lex;
public:
	inline BaseLexer() {lexer_init(&lex);}
	inline ~BaseLexer() {lexer_free(&lex);}
	operator lexer*() {return &lex;}
};

/* parses "[width]x[height]", string, i.e. 1024x768 */
static bool ConvertResText(const char *res, uint32_t &cx, uint32_t &cy)
{
	BaseLexer lex;
	base_token token;

	lexer_start(lex, res);

	/* parse width */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	cx = std::stoul(token.text.array);

	/* parse 'x' */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (strref_cmpi(&token.text, "x") != 0)
		return false;

	/* parse height */
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != BASETOKEN_DIGIT)
		return false;

	cy = std::stoul(token.text.array);

	/* shouldn't be any more tokens after this */
	if (lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;

	return true;
}

void OBS_settings::saveVideoSettings(Local<Array> videoSettings)
{
	Isolate *isolate = v8::Isolate::GetCurrent();

	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t* config = OBS_API::openConfigFile(basicConfigFile);

    Local<Object> videoSettingsObject = v8::Local<v8::Object>::Cast(videoSettings->Get(0));
    Local<Array> parameters = v8::Local<v8::Array>::Cast(videoSettingsObject->Get(String::NewFromUtf8(isolate, "parameters")));

    //Base resolution
	Local<Object> baseRes = v8::Local<v8::Object>::Cast(parameters->Get(0));

	v8::String::Utf8Value param0(baseRes->Get(String::NewFromUtf8(isolate, "currentValue")));
	std::string baseResString = std::string(*param0);
	
	uint32_t baseWidth, baseHeight;

	ConvertResText(baseResString.c_str(), baseWidth, baseHeight);
	config_set_uint(config, "Video", "BaseCX", baseWidth);
	config_set_uint(config, "Video", "BaseCY", baseHeight);

	//Output resolution
	Local<Object> outputRes = v8::Local<v8::Object>::Cast(parameters->Get(1));

	v8::String::Utf8Value param1(outputRes->Get(String::NewFromUtf8(isolate, "currentValue")));
	std::string outputResString = std::string(*param1);
	
	uint32_t outputWidth, outputHeight;

	ConvertResText(outputResString.c_str(), outputWidth, outputHeight);
	config_set_uint(config, "Video", "OutputCX", outputWidth);
	config_set_uint(config, "Video", "OutputCY", outputHeight);

	Local<Object> scaleParameter = v8::Local<v8::Object>::Cast(parameters->Get(2));

	v8::String::Utf8Value param2(scaleParameter->Get(String::NewFromUtf8(isolate, "currentValue")));
	std::string scaleString = std::string(*param2);

	config_set_string(config, "Video", "ScaleType", scaleString.c_str());

	Local<Object> fpsType = v8::Local<v8::Object>::Cast(parameters->Get(3));
	v8::String::Utf8Value param3(fpsType->Get(String::NewFromUtf8(isolate, "currentValue")));
	std::string fpsTypeString = std::string(*param3);

	if(fpsTypeString.compare("Common FPS Values") == 0) {
		config_set_uint(config, "Video", "FPSType", 0);

		Local<Object> fpsCommon = v8::Local<v8::Object>::Cast(parameters->Get(4));
		v8::String::Utf8Value param4(fpsCommon->Get(String::NewFromUtf8(isolate, "currentValue")));
		std::string fpsCommonString = std::string(*param4);
		config_set_string(config, "Video", "FPSCommon", fpsCommonString.c_str());

	} else if (fpsTypeString.compare("Integer FPS Value") == 0) {
		config_set_uint(config, "Video", "FPSType", 1);

		Local<Object> fpsInt = v8::Local<v8::Object>::Cast(parameters->Get(4));
		v8::Local<v8::Integer> fpsIntValue = v8::Local<v8::Integer>::Cast(fpsInt->Get(String::NewFromUtf8(isolate, "currentValue")));
		config_set_uint(config, "Video", "FPSInt", fpsIntValue->Uint32Value());

	} else if (fpsTypeString.compare("Fractional FPS Value") == 0) {
		config_set_uint(config, "Video", "FPSType", 2);

		Local<Object> fpsNum = v8::Local<v8::Object>::Cast(parameters->Get(4));
		v8::Local<v8::Integer> fpsNumValue = v8::Local<v8::Integer>::Cast(fpsNum->Get(String::NewFromUtf8(isolate, "currentValue")));
		config_set_uint(config, "Video", "FPSNum", fpsNumValue->Uint32Value());	
	
		if(parameters->Length() > 5)
		{
			Local<Object> fpsDen = v8::Local<v8::Object>::Cast(parameters->Get(5));
			v8::Local<v8::Integer> fpsDenValue = v8::Local<v8::Integer>::Cast(fpsDen->Get(String::NewFromUtf8(isolate, "currentValue")));
			config_set_uint(config, "Video", "FPSDen", fpsDenValue->Uint32Value());	
		}
	}

	config_save_safe(config, "tmp", nullptr);	
}

Local<Array>  OBS_settings::getAdvancedSettings()
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> advancedSettings = Array::New(isolate);

	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t* config = OBS_API::openConfigFile(basicConfigFile);

	std::string globalConfigFile = OBS_API::getGlobalConfigPath();
    config_t* globalConfig = OBS_API::openConfigFile(globalConfigFile);

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

#if _WIN32
	//General

	//Process Priority
	std::vector<std::pair<std::string, std::string>> processPriority;
	processPriority.push_back(std::make_pair("name", "ProcessPriority"));
	processPriority.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	processPriority.push_back(std::make_pair("description", "Process Priority"));
	processPriority.push_back(std::make_pair("High", "High"));
	processPriority.push_back(std::make_pair("Above Normal", "Above Normal"));
	processPriority.push_back(std::make_pair("Normal", "Normal"));
	processPriority.push_back(std::make_pair("Idle", "Idle"));

	const char* processPriorityCurrentValue = config_get_string(globalConfig, "General", "ProcessPriority");

	entries.push_back(processPriority);

	advancedSettings->Set(0, serializeSettingsData("General", entries, globalConfig, "General", true, true));
	entries.clear();

	// bool enableNewSocketLoop = config_get_bool(config, "Output", "NewSocketLoopEnable");
	// bool enableLowLatencyMode = config_get_bool(config, "Output", "LowLatencyEnable");
#endif
	//Video
	const char *videoColorFormat = config_get_string(config, "Video", "ColorFormat");
	const char *videoColorSpace = config_get_string(config, "Video", "ColorSpace");
	const char *videoColorRange = config_get_string(config, "Video", "ColorRange");

	//Renderer
	std::vector<std::pair<std::string, std::string>> renderer;
	renderer.push_back(std::make_pair("name", "Renderer"));
	renderer.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	renderer.push_back(std::make_pair("description", "Renderer"));

	const char* rendererValue = config_get_string(config, "Video", "Renderer");
	if(rendererValue == NULL) {
		rendererValue = "";
	} 

	renderer.push_back(std::make_pair("currentValue", rendererValue));
	renderer.push_back(std::make_pair("Direct3D 11", "Direct3D 11"));
	renderer.push_back(std::make_pair("OpenGL", "OpenGL"));
	entries.push_back(renderer);

	//Video Adapter

	//Color Format
	std::vector<std::pair<std::string, std::string>> colorFormat;
	colorFormat.push_back(std::make_pair("name", "ColorFormat"));
	colorFormat.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	colorFormat.push_back(std::make_pair("description", "Color Format"));
	colorFormat.push_back(std::make_pair("NV12", "NV12"));
	colorFormat.push_back(std::make_pair("I420", "I420"));
	colorFormat.push_back(std::make_pair("I444", "I444"));
	colorFormat.push_back(std::make_pair("RGB", "RGB"));
	entries.push_back(colorFormat);

	//YUV Color Space
	std::vector<std::pair<std::string, std::string>> colorSpace;
	colorSpace.push_back(std::make_pair("name", "ColorSpace"));
	colorSpace.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	colorSpace.push_back(std::make_pair("description", "YUV Color Space"));
	colorSpace.push_back(std::make_pair("601", "601"));
	colorSpace.push_back(std::make_pair("709", "709"));
	entries.push_back(colorSpace);

	//YUV Color Range
	std::vector<std::pair<std::string, std::string>> colorRange;
	colorRange.push_back(std::make_pair("name", "ColorRange"));
	colorRange.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	colorRange.push_back(std::make_pair("description", "YUV Color Range"));
	colorRange.push_back(std::make_pair("Partial", "Partial"));
	colorRange.push_back(std::make_pair("Full", "Full"));
	entries.push_back(colorRange);

	advancedSettings->Set(1, serializeSettingsData("Video", entries, config, "Video", true, true));
	entries.clear();

#if defined(_WIN32) || defined(__APPLE__)
	//Audio
	const char *monDevName = config_get_string(config, "Audio", "MonitoringDeviceName");
	const char *monDevId = config_get_string(config, "Audio", "MonitoringDeviceId");

	//Audio Monitoring Device
	std::vector<std::pair<std::string, std::string>>* monitoringDevice = new std::vector<std::pair<std::string, std::string>>();
	monitoringDevice->push_back(std::make_pair("name", "MonitoringDeviceName"));
	monitoringDevice->push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	monitoringDevice->push_back(std::make_pair("description", "Audio Monitoring Device"));
	monitoringDevice->push_back(std::make_pair("currentValue", monDevName));
	monitoringDevice->push_back(std::make_pair("Default", "Default"));

	auto enum_devices = [] (void *param, const char *name, const char *id)
	{
		std::vector<std::pair<std::string, std::string>> *monitoringDevice = 
								(std::vector<std::pair<std::string, std::string>>*)param;
		monitoringDevice->push_back(std::make_pair(name, name));
		return true;
	};
	obs_enum_audio_monitoring_devices(enum_devices, monitoringDevice);
	entries.push_back(*monitoringDevice);

	//Windows audio ducking
	std::vector<std::pair<std::string, std::string>> disableAudioDucking;
	disableAudioDucking.push_back(std::make_pair("name", "DisableAudioDucking"));
	disableAudioDucking.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	disableAudioDucking.push_back(std::make_pair("description", "Disable Windows audio ducking"));
	entries.push_back(disableAudioDucking);

	advancedSettings->Set(2, serializeSettingsData("Audio", entries, config, "Audio", true, true));
	entries.clear();
#endif

	//Recording

	//Filename Formatting
	std::vector<std::pair<std::string, std::string>> filenameFormatting;
	filenameFormatting.push_back(std::make_pair("name", "FilenameFormatting"));
	filenameFormatting.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	filenameFormatting.push_back(std::make_pair("description", "Filename Formatting"));
	// filenameFormatting.push_back(std::make_pair("currentValue", config_get_string(config, "Output", "FilenameFormatting")));
	entries.push_back(filenameFormatting);

	//Overwrite if file exists
	std::vector<std::pair<std::string, std::string>> overwriteIfExists;
	overwriteIfExists.push_back(std::make_pair("name", "OverwriteIfExists"));
	overwriteIfExists.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	overwriteIfExists.push_back(std::make_pair("description", "Overwrite if file exists"));
	// overwriteIfExists.push_back(std::make_pair("currentValue", to_string(config_get_bool(config, "Output", "OverwriteIfExists"))));
	entries.push_back(overwriteIfExists);

	//Replay Buffer Filename Prefix
	std::vector<std::pair<std::string, std::string>> recRBPrefix;
	recRBPrefix.push_back(std::make_pair("name", "RecRBPrefix"));
	recRBPrefix.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	recRBPrefix.push_back(std::make_pair("description", "Replay Buffer Filename Prefix"));
	entries.push_back(recRBPrefix);

	//Replay Buffer Filename Suffix
	std::vector<std::pair<std::string, std::string>> recRBSuffix;
	recRBSuffix.push_back(std::make_pair("name", "RecRBSuffix"));
	recRBSuffix.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	recRBSuffix.push_back(std::make_pair("description", "Replay Buffer Filename Suffix"));
	entries.push_back(recRBSuffix);

	advancedSettings->Set(3, serializeSettingsData("Recording", entries, config, "SimpleOutput", true, true));
	entries.clear();

	//Stream Delay

	//Enable
	std::vector<std::pair<std::string, std::string>> delayEnable;
	delayEnable.push_back(std::make_pair("name", "DelayEnable"));
	delayEnable.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	delayEnable.push_back(std::make_pair("description", "Enable"));
	entries.push_back(delayEnable);

	//Duration (seconds)
	std::vector<std::pair<std::string, std::string>> delaySec;
	delaySec.push_back(std::make_pair("name", "DelaySec"));
	delaySec.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	delaySec.push_back(std::make_pair("description", "Duration (seconds)"));
	entries.push_back(delaySec);


	//Preserved curtoff point (increase delay) when reconnecting
	std::vector<std::pair<std::string, std::string>> delayPreserve;
	delayPreserve.push_back(std::make_pair("name", "DelayPreserve"));
	delayPreserve.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	delayPreserve.push_back(std::make_pair("description", "Preserved curtoff point (increase delay) when reconnecting"));
	entries.push_back(delayPreserve);

	advancedSettings->Set(4, serializeSettingsData("Stream Delay", entries, config, "Output", true, true));
	entries.clear();

	//Automatically Reconnect

	//Enable
	std::vector<std::pair<std::string, std::string>> reconnect;
	reconnect.push_back(std::make_pair("name", "Reconnect"));
	reconnect.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	reconnect.push_back(std::make_pair("description", "Enable"));
	entries.push_back(reconnect);

	//Retry Delay (seconds)
	std::vector<std::pair<std::string, std::string>> retryDelay;
	retryDelay.push_back(std::make_pair("name", "RetryDelay"));
	retryDelay.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	retryDelay.push_back(std::make_pair("description", "Retry Delay (seconds)"));
	entries.push_back(retryDelay);

	//Maximum Retries
	std::vector<std::pair<std::string, std::string>> maxRetries;
	maxRetries.push_back(std::make_pair("name", "MaxRetries"));
	maxRetries.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	maxRetries.push_back(std::make_pair("description", "Maximum Retries"));
	entries.push_back(maxRetries);

	advancedSettings->Set(5, serializeSettingsData("Automatically Reconnect", entries, config, "Output", true, true));
	entries.clear();

	// const char *bindIP = config_get_string(config, "Output", "BindIP");
	// bool enableNewSocketLoop = config_get_bool(config, "Output", "NewSocketLoopEnable");
	// bool enableLowLatencyMode = config_get_bool(config, "Output", "LowLatencyEnable");

	//Network

	//Bind to IP
	std::vector<std::pair<std::string, std::string>> bindIP;
	bindIP.push_back(std::make_pair("name", "BindIP"));
	bindIP.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	bindIP.push_back(std::make_pair("description", "Bind to IP"));

	obs_properties_t *ppts = obs_get_output_properties("rtmp_output");
	obs_property_t *p = obs_properties_get(ppts, "bind_ip");

	size_t count = obs_property_list_item_count(p);
	for (size_t i = 0; i < count; i++) {
		const char *name = obs_property_list_item_name(p, i);
		const char *val = obs_property_list_item_string(p, i);

		bindIP.push_back(std::make_pair(name, name));
	}

	entries.push_back(bindIP);

	//Enable new networking code
	std::vector<std::pair<std::string, std::string>> newSocketLoopEnable;
	newSocketLoopEnable.push_back(std::make_pair("name", "NewSocketLoopEnable"));
	newSocketLoopEnable.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	newSocketLoopEnable.push_back(std::make_pair("description", "Enable new networking code"));
	entries.push_back(newSocketLoopEnable);

	//Low latency mode
	std::vector<std::pair<std::string, std::string>> lowLatencyEnable;
	lowLatencyEnable.push_back(std::make_pair("name", "LowLatencyEnable"));
	lowLatencyEnable.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	lowLatencyEnable.push_back(std::make_pair("description", "Low latency mode"));
	entries.push_back(lowLatencyEnable);

	advancedSettings->Set(6, serializeSettingsData("Network", entries, config, "Output", true, true));
	entries.clear();

	return advancedSettings;
}

void OBS_settings::saveAdvancedSettings(Local<Array> advancedSettings)
{
	Isolate *isolate = v8::Isolate::GetCurrent();

	std::string globalConfigFile = OBS_API::getGlobalConfigPath();
	std::string basicConfigFile = OBS_API::getBasicConfigPath();

	//General
	Local<Array> generalAdvancedSettings = Array::New(isolate);

	generalAdvancedSettings->Set(0, advancedSettings->Get(0));
	saveGenericSettings(generalAdvancedSettings, "General", globalConfigFile);

	//Video
	Local<Array> videoAdvancedSettings = Array::New(isolate);

	videoAdvancedSettings->Set(0, advancedSettings->Get(1));
	saveGenericSettings(videoAdvancedSettings, "Video", basicConfigFile);

	//Audio
	Local<Array> audioAdvancedSettings = Array::New(isolate);

	audioAdvancedSettings->Set(0, advancedSettings->Get(2));
	saveGenericSettings(audioAdvancedSettings, "Audio", basicConfigFile);

	//Recording
	Local<Array> recordingAdvancedSettings = Array::New(isolate);

	recordingAdvancedSettings->Set(0, advancedSettings->Get(3));
	saveGenericSettings(recordingAdvancedSettings, "SimpleOutput", basicConfigFile);

	//Stream Delay
	Local<Array> stresmDelayAdvancedSettings = Array::New(isolate);

	stresmDelayAdvancedSettings->Set(0, advancedSettings->Get(4));
	saveGenericSettings(stresmDelayAdvancedSettings, "Output", basicConfigFile);

	//Automatically Reconnect
	Local<Array> automaticallyReconnectAdvancedSettings = Array::New(isolate);

	automaticallyReconnectAdvancedSettings->Set(0, advancedSettings->Get(5));
	saveGenericSettings(automaticallyReconnectAdvancedSettings, "Output", basicConfigFile);

	//Network
	Local<Array> networkAdvancedSettings = Array::New(isolate);

	networkAdvancedSettings->Set(0, advancedSettings->Get(6));
	saveGenericSettings(networkAdvancedSettings, "Output", basicConfigFile);
}

Local<Array> OBS_settings::getListCategories(void)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> categories = Array::New(isolate);

	categories->Set(0, String::NewFromUtf8(isolate, "General"));
	categories->Set(1, String::NewFromUtf8(isolate, "Stream"));
	categories->Set(2, String::NewFromUtf8(isolate, "Output"));
	categories->Set(3, String::NewFromUtf8(isolate, "Audio"));
	categories->Set(4, String::NewFromUtf8(isolate, "Video"));
	categories->Set(5, String::NewFromUtf8(isolate, "Hotkeys"));
	categories->Set(6, String::NewFromUtf8(isolate, "Advanced"));

	return categories;
}

Local<Array> OBS_settings::getSettings(std::string nameCategory)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> settings = Array::New(isolate);

	if(nameCategory.compare("General") == 0) {
		settings = getGeneralSettings();
	} else if (nameCategory.compare("Stream") == 0) {
		settings = getStreamSettings();
	} else if (nameCategory.compare("Output") == 0) {
		settings = getOutputSettings();
	} else if (nameCategory.compare("Audio") == 0) {
		settings = getAudioSettings();
	} else if (nameCategory.compare("Video") == 0) {
		settings = getVideoSettings();
	} else if (nameCategory.compare("Advanced") == 0) {
		settings = getAdvancedSettings();
	}

	return settings;
}

void OBS_settings::saveSettings(std::string nameCategory, Local<Array> settings)
{
	std::string globalConfigFile = OBS_API::getGlobalConfigPath();
	std::string basicConfigFile = OBS_API::getBasicConfigPath();

	if(nameCategory.compare("General") == 0) {
		saveGenericSettings(settings, "BasicWindow", globalConfigFile);
	} else if (nameCategory.compare("Stream") == 0) {
		saveStreamSettings(settings);
		OBS_service::updateService();
	} else if (nameCategory.compare("Output") == 0) {
		saveOutputSettings(settings);
	} else if (nameCategory.compare("Audio") == 0) {
		saveAudioSettings(settings);
	} else if (nameCategory.compare("Video") == 0) {
		saveVideoSettings(settings);
		OBS_service::resetVideoContext(NULL);
	} else if (nameCategory.compare("Advanced") == 0) {
		saveAdvancedSettings(settings);
		OBS_API::setAudioDeviceMonitoring();
	}
}

void OBS_settings::saveGenericSettings(Local<Array> genericSettings, std::string section, std::string pathFile)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	config_t* config = OBS_API::openConfigFile(pathFile);

	for(int i=0;i<genericSettings->Length();i++) {
		Local<Object> generalSettingsObject = v8::Local<v8::Object>::Cast(genericSettings->Get(i));

		v8::String::Utf8Value param0(generalSettingsObject->Get(String::NewFromUtf8(isolate, "nameSubCategory")));
		std::string nameSubcategory = std::string(*param0);

		Local<Array> parameters = v8::Local<v8::Array>::Cast(generalSettingsObject->Get(String::NewFromUtf8(isolate, "parameters")));

		for(int j=0;j<parameters->Length();j++){
			Local<Object> parameter = v8::Local<v8::Object>::Cast(parameters->Get(j));

			std::string name, type, value;

			v8::String::Utf8Value param0(parameter->Get(String::NewFromUtf8(isolate, "name")));
			name = std::string(*param0);

			v8::String::Utf8Value param1(parameter->Get(String::NewFromUtf8(isolate, "type")));
			type = std::string(*param1);

			if(type.compare("OBS_PROPERTY_LIST") == 0 ||
				type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 ||
				type.compare("OBS_PROPERTY_PATH") == 0) {
				v8::String::Utf8Value param2(parameter->Get(String::NewFromUtf8(isolate, "currentValue")));
				value = std::string(*param2);
				if(name.compare("MonitoringDeviceName") == 0) {
					std::string monDevName;
					std::string monDevId;
					if(value.compare("Default") != 0) {
						std::vector<std::pair<std::string, std::string>> monitoringDevice;

						auto enum_devices = [] (void *param, const char *name, const char *id)
						{
							std::vector<std::pair<std::string, std::string>> *monitoringDevice =
													(std::vector<std::pair<std::string, std::string>>*)param;
							monitoringDevice->push_back(std::make_pair(name, id));
							return true;
						};
						obs_enum_audio_monitoring_devices(enum_devices, &monitoringDevice);

						std::vector<pair<std::string, std::string>>::iterator it =
						std::find_if(monitoringDevice.begin(), monitoringDevice.end(),
							[&value] (const pair<std::string, std::string> device)
							{
								return (device.first.compare(value) == 0);
							});

						if (it != monitoringDevice.end()) {
							monDevName = it->first;
							monDevId = it->second;
						}
					} else {
						monDevName = value;
						monDevId = "default";
					}

					config_set_string(config, section.c_str(), "MonitoringDeviceName", monDevName.c_str());
					config_set_string(config, section.c_str(), "MonitoringDeviceId", monDevId.c_str());
				} else {
					config_set_string(config, section.c_str(), name.c_str(), value.c_str());
				}
			} else if(type.compare("OBS_PROPERTY_INT") == 0) {
				int64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_int(config, section.c_str(), name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_UINT") == 0) {
				uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_uint(config, section.c_str(), name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_BOOL") == 0) {
				uint64_t value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_bool(config, section.c_str(), name.c_str(), value);
			} else if(type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double value = parameter->Get(String::NewFromUtf8(isolate, "currentValue"))->NumberValue();
				config_set_double(config, section.c_str(), name.c_str(), value);
			} else {
				std::cout << "type not found ! " << type.c_str() << std::endl;
			}
		}
	}
	config_save_safe(config, "tmp", nullptr);	
}