#include "error.hpp"
#include "nodeobs_api.h"
#include "nodeobs_settings.h"
#include "shared.hpp"

#include <windows.h>
vector<const char*> tabStreamTypes;
const char*         currentServiceName;

/* some nice default output resolution vals */
static const double vals[] = {1.0, 1.25, (1.0 / 0.75), 1.5, (1.0 / 0.6), 1.75, 2.0, 2.25, 2.5, 2.75, 3.0};

static const size_t numVals = sizeof(vals) / sizeof(double);

static string ResString(uint64_t cx, uint64_t cy)
{
	ostringstream res;
	res << cx << "x" << cy;
	return res.str();
}

OBS_settings::OBS_settings() {}
OBS_settings::~OBS_settings() {}

void OBS_settings::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Settings");

	cls->register_function(std::make_shared<ipc::function>(
	    "OBS_settings_getSettings", std::vector<ipc::type>{ipc::type::String}, OBS_settings_getSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "OBS_settings_saveSettings",
	    std::vector<ipc::type>{ipc::type::String, ipc::type::UInt32, ipc::type::UInt32, ipc::type::Binary},
	    OBS_settings_saveSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "OBS_settings_getListCategories", std::vector<ipc::type>{}, OBS_settings_getListCategories));

	srv.register_collection(cls);
}

void OBS_settings::OBS_settings_getListCategories(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::vector<std::string> listCategories = getListCategories();
	size_t                   size           = listCategories.size();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	for (int i = 0; i < size; i++) {
		rval.push_back(ipc::value(listCategories.at(i).c_str()));
	}
	AUTO_DEBUG;
}

void OBS_settings::OBS_settings_getSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::string              nameCategory = args[0].value_str;
	std::vector<SubCategory> settings     = getSettings(nameCategory);
	std::vector<char>        binaryValue;

	for (int i = 0; i < settings.size(); i++) {
		std::vector<char> serializedBuf = settings.at(i).serialize();

		binaryValue.insert(binaryValue.end(), serializedBuf.begin(), serializedBuf.end());
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(settings.size()));
	rval.push_back(ipc::value(binaryValue.size()));

	rval.push_back(ipc::value(binaryValue));
	AUTO_DEBUG;
}

std::vector<SubCategory> serializeCategory(uint32_t subCategoriesCount, uint32_t sizeStruct, std::vector<char> buffer)
{
	std::vector<SubCategory> category;

	size_t indexData = 0;
	for (uint32_t i = 0; i < subCategoriesCount; i++) {
		SubCategory sc;

		size_t* sizeMessage = reinterpret_cast<size_t*>(buffer.data() + indexData);
		indexData += sizeof(size_t);

		std::string name(buffer.data() + indexData, *sizeMessage);
		indexData += *sizeMessage;

		uint32_t* paramsCount = reinterpret_cast<uint32_t*>(buffer.data() + indexData);
		indexData += sizeof(uint32_t);

		Parameter param;
		for (uint32_t j = 0; j < *paramsCount; j++) {
			size_t* sizeName = reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string name(buffer.data() + indexData, *sizeName);
			indexData += *sizeName;

			size_t* sizeDescription = reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string description(buffer.data() + indexData, *sizeDescription);
			indexData += *sizeDescription;

			size_t* sizeType = reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string type(buffer.data() + indexData, *sizeType);
			indexData += *sizeType;

			size_t* sizeSubType = reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::string subType(buffer.data() + indexData, *sizeSubType);
			indexData += *sizeSubType;

			bool* enabled = reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			bool* masked = reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			bool* visible = reinterpret_cast<bool*>(buffer.data() + indexData);
			indexData += sizeof(bool);

			size_t* sizeOfCurrentValue = reinterpret_cast<std::size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::vector<char> currentValue;
			currentValue.resize(*sizeOfCurrentValue);
			memcpy(currentValue.data(), buffer.data() + indexData, *sizeOfCurrentValue);
			indexData += *sizeOfCurrentValue;

			size_t* sizeOfValues = reinterpret_cast<size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			size_t* countValues = reinterpret_cast<size_t*>(buffer.data() + indexData);
			indexData += sizeof(size_t);

			std::vector<char> values;
			values.resize(*sizeOfValues);
			memcpy(values.data(), buffer.data() + indexData, *sizeOfValues);
			indexData += *sizeOfValues;

			param.name         = name;
			param.description  = description;
			param.type         = type;
			param.subType      = subType;
			param.enabled      = enabled;
			param.masked       = masked;
			param.visible      = visible;
			param.currentValue = currentValue;
			param.values       = values;
			param.countValues  = *countValues;

			sc.params.push_back(param);
		}
		sc.name        = name;
		sc.paramsCount = *paramsCount;
		category.push_back(sc);
	}
	return category;
}

void OBS_settings::OBS_settings_saveSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::string nameCategory       = args[0].value_str;
	uint32_t    subCategoriesCount = args[1].value_union.ui32;
	uint32_t    sizeStruct         = args[2].value_union.ui32;

	std::vector<char> buffer;
	buffer.resize(sizeStruct);
	memcpy(buffer.data(), args[3].value_bin.data(), sizeStruct);

	std::vector<SubCategory> settings = serializeCategory(subCategoriesCount, sizeStruct, buffer);

	saveSettings(nameCategory, settings);
	AUTO_DEBUG;
}

SubCategory OBS_settings::serializeSettingsData(
    std::string                                                   nameSubCategory,
    std::vector<std::vector<std::pair<std::string, std::string>>> entries,
    config_t*                                                     config,
    std::string                                                   section,
    bool                                                          isVisible,
    bool                                                          isEnabled)
{
	SubCategory sc;

	for (int i = 0; i < entries.size(); i++) {
		Parameter param;

		param.name        = entries.at(i).at(0).second;
		param.type        = entries.at(i).at(1).second;
		param.description = entries.at(i).at(2).second;
		param.subType     = entries.at(i).at(3).second;

		std::string currentValue;
		if (entries.at(i).size() > 4) {
			currentValue = entries.at(i).at(4).first.c_str();
		}

		// Current value
		if (!currentValue.empty() && currentValue.compare("currentValue") == 0) {
			const char* currentValue = entries.at(i).at(4).second.c_str();
			param.currentValue.resize(strlen(currentValue));
			std::memcpy(param.currentValue.data(), currentValue, strlen(currentValue));
			param.sizeOfCurrentValue = strlen(currentValue);
			entries.at(i).erase(entries.at(i).begin() + 4);
		} else {
			if (param.type.compare("OBS_PROPERTY_LIST") == 0 || param.type.compare("OBS_PROPERTY_PATH") == 0
			    || param.type.compare("OBS_PROPERTY_EDIT_PATH") == 0
			    || param.type.compare("OBS_PROPERTY_EDIT_TEXT") == 0) {
				const char* currentValue = config_get_string(config, section.c_str(), param.name.c_str());

				if (currentValue != NULL) {
					param.currentValue.resize(strlen(currentValue));
					std::memcpy(param.currentValue.data(), currentValue, strlen(currentValue));
					param.sizeOfCurrentValue = strlen(currentValue);

				} else {
					param.sizeOfCurrentValue = 0;
				}
			} else if (param.type.compare("OBS_PROPERTY_INT") == 0) {
				int64_t val = config_get_int(config, section.c_str(), param.name.c_str());

				param.currentValue.resize(sizeof(val));
				memcpy(param.currentValue.data(), &val, sizeof(val));
				param.sizeOfCurrentValue = sizeof(val);
			} else if (param.type.compare("OBS_PROPERTY_UINT") == 0) {
				uint64_t val = config_get_uint(config, section.c_str(), param.name.c_str());

				param.currentValue.resize(sizeof(val));
				memcpy(param.currentValue.data(), &val, sizeof(val));
				param.sizeOfCurrentValue = sizeof(val);
			} else if (param.type.compare("OBS_PROPERTY_BOOL") == 0) {
				bool val = config_get_bool(config, section.c_str(), param.name.c_str());

				param.currentValue.resize(sizeof(val));
				memcpy(param.currentValue.data(), &val, sizeof(val));
				param.sizeOfCurrentValue = sizeof(val);
			} else if (param.type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double val = config_get_double(config, section.c_str(), param.name.c_str());

				param.currentValue.resize(sizeof(val));
				memcpy(param.currentValue.data(), &val, sizeof(val));
				param.sizeOfCurrentValue = sizeof(val);
			}
		}

		// Values
		if (entries.at(i).size() > 4) {
			for (int j = 4; j < entries.at(i).size(); j++) {
				std::string name = entries.at(i).at(j).first;

				uint64_t          sizeName = name.length();
				std::vector<char> sizeNameBuffer;
				sizeNameBuffer.resize(sizeof(sizeName));
				memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

				param.values.insert(param.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
				param.values.insert(param.values.end(), name.begin(), name.end());

				std::string value = entries.at(i).at(j).second;

				uint64_t          sizeValue = value.length();
				std::vector<char> sizeValueBuffer;
				sizeValueBuffer.resize(sizeof(sizeValue));
				memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

				param.values.insert(param.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
				param.values.insert(param.values.end(), value.begin(), value.end());
			}

			param.sizeOfValues = param.values.size();
			param.countValues  = entries.at(i).size() - 4;
		}

		param.visible = isVisible;
		param.enabled = isEnabled;
		param.masked  = false;

		sc.params.push_back(param);
	}

	sc.paramsCount = sc.params.size();
	sc.name        = nameSubCategory;
	return sc;
}

std::vector<SubCategory> OBS_settings::getGeneralSettings()
{
	std::vector<SubCategory> generalSettings;

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	// General
	// generalSettings.push_back(serializeSettingsData("General", entries, config, "BasicWindow", true, true));
	// entries.clear();

	// Output
	std::vector<std::pair<std::string, std::string>> warnBeforeStartingStream;
	warnBeforeStartingStream.push_back(std::make_pair("name", "WarnBeforeStartingStream"));
	warnBeforeStartingStream.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	warnBeforeStartingStream.push_back(std::make_pair("description", "Show confirmation dialog when starting streams"));
	warnBeforeStartingStream.push_back(std::make_pair("subType", ""));
	entries.push_back(warnBeforeStartingStream);

	std::vector<std::pair<std::string, std::string>> warnBeforeStoppingStream;
	warnBeforeStoppingStream.push_back(std::make_pair("name", "WarnBeforeStoppingStream"));
	warnBeforeStoppingStream.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	warnBeforeStoppingStream.push_back(std::make_pair("description", "Show confirmation dialog when stopping streams"));
	warnBeforeStoppingStream.push_back(std::make_pair("subType", ""));
	entries.push_back(warnBeforeStoppingStream);

	std::vector<std::pair<std::string, std::string>> recordWhenStreaming;
	recordWhenStreaming.push_back(std::make_pair("name", "RecordWhenStreaming"));
	recordWhenStreaming.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	recordWhenStreaming.push_back(std::make_pair("description", "Automatically record when streaming"));
	recordWhenStreaming.push_back(std::make_pair("subType", ""));
	entries.push_back(recordWhenStreaming);

	std::vector<std::pair<std::string, std::string>> keepRecordingWhenStreamStops;
	keepRecordingWhenStreamStops.push_back(std::make_pair("name", "KeepRecordingWhenStreamStops"));
	keepRecordingWhenStreamStops.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	keepRecordingWhenStreamStops.push_back(std::make_pair("description", "Keep recording when stream stops"));
	keepRecordingWhenStreamStops.push_back(std::make_pair("subType", ""));
	entries.push_back(keepRecordingWhenStreamStops);

	std::vector<std::pair<std::string, std::string>> replayBufferWhileStreaming;
	replayBufferWhileStreaming.push_back(std::make_pair("name", "ReplayBufferWhileStreaming"));
	replayBufferWhileStreaming.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	replayBufferWhileStreaming.push_back(
	    std::make_pair("description", "Automatically start replay buffer when streaming"));
	replayBufferWhileStreaming.push_back(std::make_pair("subType", ""));
	entries.push_back(replayBufferWhileStreaming);

	std::vector<std::pair<std::string, std::string>> keepReplayBufferStreamStops;
	keepReplayBufferStreamStops.push_back(std::make_pair("name", "KeepReplayBufferStreamStops"));
	keepReplayBufferStreamStops.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	keepReplayBufferStreamStops.push_back(std::make_pair("description", "Keep replay buffer active when stream stops"));
	keepReplayBufferStreamStops.push_back(std::make_pair("subType", ""));
	entries.push_back(keepReplayBufferStreamStops);

	generalSettings.push_back(
	    serializeSettingsData("Output", entries, ConfigManager::getInstance().getGlobal(), "BasicWindow", true, true));
	entries.clear();

	// Source Alignement Snapping
	std::vector<std::pair<std::string, std::string>> snappingEnabled;
	snappingEnabled.push_back(std::make_pair("name", "SnappingEnabled"));
	snappingEnabled.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	snappingEnabled.push_back(std::make_pair("description", "Enable"));
	snappingEnabled.push_back(std::make_pair("subType", ""));
	entries.push_back(snappingEnabled);

	std::vector<std::pair<std::string, std::string>> snapDistance;
	snapDistance.push_back(std::make_pair("name", "SnapDistance"));
	snapDistance.push_back(std::make_pair("type", "OBS_PROPERTY_DOUBLE"));
	snapDistance.push_back(std::make_pair("description", "Snap Sensitivy"));
	snapDistance.push_back(std::make_pair("subType", ""));
	entries.push_back(snapDistance);

	std::vector<std::pair<std::string, std::string>> screenSnapping;
	screenSnapping.push_back(std::make_pair("name", "ScreenSnapping"));
	screenSnapping.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	screenSnapping.push_back(std::make_pair("description", "Snap Sources to edge of screen"));
	screenSnapping.push_back(std::make_pair("subType", ""));
	entries.push_back(screenSnapping);

	std::vector<std::pair<std::string, std::string>> sourceSnapping;
	sourceSnapping.push_back(std::make_pair("name", "SourceSnapping"));
	sourceSnapping.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sourceSnapping.push_back(std::make_pair("description", "Snap Sources to other sources"));
	sourceSnapping.push_back(std::make_pair("subType", ""));
	entries.push_back(sourceSnapping);

	std::vector<std::pair<std::string, std::string>> centerSnapping;
	centerSnapping.push_back(std::make_pair("name", "CenterSnapping"));
	centerSnapping.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	centerSnapping.push_back(std::make_pair("description", "Snap Sources to horizontal and vertical center"));
	centerSnapping.push_back(std::make_pair("subType", ""));
	entries.push_back(centerSnapping);

	generalSettings.push_back(serializeSettingsData(
	    "Source Alignement Snapping", entries, ConfigManager::getInstance().getGlobal(), "BasicWindow", true, true));
	entries.clear();

	// Projectors
	std::vector<std::pair<std::string, std::string>> hideProjectorCursor;
	hideProjectorCursor.push_back(std::make_pair("name", "HideProjectorCursor"));
	hideProjectorCursor.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	hideProjectorCursor.push_back(std::make_pair("description", "Hide cursor over projectors"));
	hideProjectorCursor.push_back(std::make_pair("subType", ""));
	entries.push_back(hideProjectorCursor);

	std::vector<std::pair<std::string, std::string>> projectorAlwaysOnTop;
	projectorAlwaysOnTop.push_back(std::make_pair("name", "ProjectorAlwaysOnTop"));
	projectorAlwaysOnTop.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	projectorAlwaysOnTop.push_back(std::make_pair("description", "Make projectors always on top"));
	projectorAlwaysOnTop.push_back(std::make_pair("subType", ""));
	entries.push_back(projectorAlwaysOnTop);

	std::vector<std::pair<std::string, std::string>> saveProjectors;
	saveProjectors.push_back(std::make_pair("name", "SaveProjectors"));
	saveProjectors.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	saveProjectors.push_back(std::make_pair("description", "Save projectors on exit"));
	saveProjectors.push_back(std::make_pair("subType", ""));
	entries.push_back(saveProjectors);

	generalSettings.push_back(serializeSettingsData(
	    "Projectors", entries, ConfigManager::getInstance().getGlobal(), "BasicWindow", true, true));
	entries.clear();

	// System Tray
	std::vector<std::pair<std::string, std::string>> sysTrayEnabled;
	sysTrayEnabled.push_back(std::make_pair("name", "SysTrayEnabled"));
	sysTrayEnabled.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sysTrayEnabled.push_back(std::make_pair("description", "Enable"));
	sysTrayEnabled.push_back(std::make_pair("subType", ""));
	entries.push_back(sysTrayEnabled);

	std::vector<std::pair<std::string, std::string>> sysTrayWhenStarted;
	sysTrayWhenStarted.push_back(std::make_pair("name", "SysTrayWhenStarted"));
	sysTrayWhenStarted.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sysTrayWhenStarted.push_back(std::make_pair("description", "Minimize to system tray when started"));
	sysTrayWhenStarted.push_back(std::make_pair("subType", ""));
	entries.push_back(sysTrayWhenStarted);

	std::vector<std::pair<std::string, std::string>> sysTrayMinimizeToTray;
	sysTrayMinimizeToTray.push_back(std::make_pair("name", "SysTrayMinimizeToTray"));
	sysTrayMinimizeToTray.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	sysTrayMinimizeToTray.push_back(
	    std::make_pair("description", "Always minimize to system tray instead of task bar"));
	sysTrayMinimizeToTray.push_back(std::make_pair("subType", ""));
	entries.push_back(sysTrayMinimizeToTray);

	generalSettings.push_back(serializeSettingsData(
	    "System Tray", entries, ConfigManager::getInstance().getGlobal(), "BasicWindow", true, true));
	entries.clear();

	return generalSettings;
}

void OBS_settings::saveGeneralSettings(std::vector<SubCategory> generalSettings, std::string pathConfigDirectory)
{
	config_t* config;
	pathConfigDirectory += "global.ini";

	int result = config_open(&config, pathConfigDirectory.c_str(), CONFIG_OPEN_EXISTING);

	if (result != CONFIG_SUCCESS) {
		config = config_create(pathConfigDirectory.c_str());
	}

	SubCategory sc;

	for (int i = 0; i < generalSettings.size(); i++) {
		sc = generalSettings.at(i);

		std::string nameSubcategory = sc.name;

		Parameter param;
		for (int j = 0; j < sc.params.size(); j++) {
			param = sc.params.at(i);

			std::string name, type;

			name = param.name;
			type = param.type;

			if (type.compare("OBS_PROPERTY_LIST") == 0) {
				config_set_string(config, "BasicWindow", name.c_str(), param.currentValue.data());
			} else if (type.compare("OBS_PROPERTY_INT") == 0) {
				int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
				config_set_int(config, "BasicWindow", name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_UINT") == 0) {
				uint64_t* value = reinterpret_cast<uint64_t*>(param.currentValue.data());
				config_set_uint(config, "BasicWindow", name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
				bool* value = reinterpret_cast<bool*>(param.currentValue.data());
				config_set_bool(config, "BasicWindow", name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double* value = reinterpret_cast<double*>(param.currentValue.data());
				config_set_double(config, "BasicWindow", name.c_str(), *value);
			}
		}
	}
	config_save_safe(config, "tmp", nullptr);
}

std::vector<SubCategory> OBS_settings::getStreamSettings()
{
	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive();

	obs_service_t* currentService = OBS_service::getService();
	obs_data_t*    settings       = obs_service_get_settings(currentService);

	std::vector<SubCategory> streamSettings;
	SubCategory              service;

	service.name = "Untitled";

	Parameter streamType;
	streamType.name    = "streamType";
	streamType.type    = "OBS_PROPERTY_LIST";
	streamType.subType = "OBS_COMBO_FORMAT_STRING";

	int         index = 0;
	const char* type;

	uint32_t indexData = 0;
	while (obs_enum_service_types(index++, &type)) {
		std::string name = obs_service_get_display_name(type);

		uint64_t          sizeName = name.length();
		std::vector<char> sizeNameBuffer;
		sizeNameBuffer.resize(sizeof(sizeName));
		memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

		streamType.values.insert(streamType.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
		streamType.values.insert(streamType.values.end(), name.begin(), name.end());

		std::string value = type;

		uint64_t          sizeValue = value.length();
		std::vector<char> sizeValueBuffer;
		sizeValueBuffer.resize(sizeof(sizeValue));
		memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

		streamType.values.insert(streamType.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
		streamType.values.insert(streamType.values.end(), value.begin(), value.end());
	}

	streamType.sizeOfValues = streamType.values.size();
	streamType.countValues  = index - 1;

	streamType.description = "Stream Type";

	const char* servType = obs_service_get_type(currentService);
	streamType.currentValue.resize(strlen(servType));
	memcpy(streamType.currentValue.data(), servType, strlen(servType));
	streamType.sizeOfCurrentValue = strlen(servType);

	streamType.visible = true;
	streamType.enabled = isCategoryEnabled;
	streamType.masked  = false;

	service.params.push_back(streamType);
	service.paramsCount = service.params.size();

	streamSettings.push_back(service);

	SubCategory serviceConfiguration;

	obs_properties_t* properties = obs_service_properties(currentService);
	obs_property_t*   property   = obs_properties_first(properties);
	obs_combo_format  format;
	string            formatString;

	index                                  = 0;
	uint32_t indexDataServiceConfiguration = 0;

	while (property) {
		Parameter param;

		param.name = obs_property_name(property);

		std::vector<std::pair<std::string, void*>> values;

		int count = (int)obs_property_list_item_count(property);

		for (int i = 0; i < count; i++) {
			//Value
			format = obs_property_list_format(property);

			if (format == OBS_COMBO_FORMAT_INT) {
				std::string name = obs_property_list_item_name(property, i);

				uint64_t          sizeName = name.length();
				std::vector<char> sizeNameBuffer;
				sizeNameBuffer.resize(sizeof(sizeName));
				memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

				param.values.insert(param.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
				param.values.insert(param.values.end(), name.begin(), name.end());

				int64_t value = obs_property_list_item_int(property, i);

				std::vector<char> valueBuffer;
				valueBuffer.resize(sizeof(value));
				memcpy(valueBuffer.data(), &value, sizeof(value));

				param.values.insert(param.values.end(), valueBuffer.begin(), valueBuffer.end());

				formatString  = "OBS_PROPERTY_INT";
				param.subType = "OBS_COMBO_FORMAT_INT";
			} else if (format == OBS_COMBO_FORMAT_FLOAT) {
				std::string name = obs_property_list_item_name(property, i);

				uint64_t          sizeName = name.length();
				std::vector<char> sizeNameBuffer;
				sizeNameBuffer.resize(sizeof(sizeName));
				memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

				param.values.insert(param.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
				param.values.insert(param.values.end(), name.begin(), name.end());

				double value = obs_property_list_item_float(property, i);

				std::vector<char> valueBuffer;
				valueBuffer.resize(sizeof(value));
				memcpy(valueBuffer.data(), &value, sizeof(value));

				param.values.insert(param.values.end(), valueBuffer.begin(), valueBuffer.end());

				formatString  = "OBS_PROPERTY_DOUBLE";
				param.subType = "OBS_COMBO_FORMAT_FLOAT";
			} else if (format == OBS_COMBO_FORMAT_STRING) {
				std::string name = obs_property_list_item_name(property, i);

				uint64_t          sizeName = name.length();
				std::vector<char> sizeNameBuffer;
				sizeNameBuffer.resize(sizeof(sizeName));
				memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

				param.values.insert(param.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
				param.values.insert(param.values.end(), name.begin(), name.end());

				std::string value = obs_property_list_item_string(property, i);

				uint64_t          sizeValue = value.length();
				std::vector<char> sizeValueBuffer;
				sizeValueBuffer.resize(sizeof(sizeValue));
				memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

				param.values.insert(param.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
				param.values.insert(param.values.end(), value.begin(), value.end());

				formatString  = "OBS_PROPERTY_LIST";
				param.subType = "OBS_COMBO_FORMAT_STRING";
			} else {
				cout << "INVALID FORMAT" << endl;
			}
		}

		param.sizeOfValues = param.values.size();
		param.countValues  = count;

		if (count == 0) {
			if (strcmp(obs_property_name(property), "key") == 0) {
				const char* stream_key = obs_service_get_key(currentService);
				formatString           = "OBS_PROPERTY_EDIT_TEXT";

				if (stream_key == NULL)
					stream_key = "";

				param.currentValue.resize(strlen(stream_key));
				memcpy(param.currentValue.data(), stream_key, strlen(stream_key));
				param.sizeOfCurrentValue = strlen(stream_key);
			}
			if (strcmp(obs_property_name(property), "show_all") == 0) {
				bool show_all = obs_data_get_bool(settings, "show_all");
				formatString  = "OBS_PROPERTY_BOOL";

				param.currentValue.resize(sizeof(show_all));
				memcpy(param.currentValue.data(), &show_all, sizeof(show_all));
				param.sizeOfCurrentValue = sizeof(show_all);
			}
			if (strcmp(obs_property_name(property), "server") == 0) {
				const char* server = obs_service_get_url(currentService);
				if (strcmp(obs_service_get_type(currentService), "rtmp_common") == 0) {
					formatString = "OBS_PROPERTY_LIST";
				} else {
					formatString = "OBS_PROPERTY_EDIT_TEXT";
				}

				if (server == NULL)
					server = "";

				param.currentValue.resize(strlen(server));
				memcpy(param.currentValue.data(), server, strlen(server));
				param.sizeOfCurrentValue = strlen(server);
			}
			if (strcmp(obs_property_name(property), "username") == 0) {
				const char* username = obs_service_get_username(currentService);
				formatString         = "OBS_PROPERTY_EDIT_TEXT";

				if (username == NULL)
					username = "";

				param.currentValue.resize(strlen(username));
				memcpy(param.currentValue.data(), username, strlen(username));
				param.sizeOfCurrentValue = strlen(username);
			}
			if (strcmp(obs_property_name(property), "password") == 0) {
				const char* password = obs_service_get_password(currentService);
				formatString         = "OBS_PROPERTY_EDIT_TEXT";

				if (password == NULL)
					password = "";

				param.currentValue.resize(strlen(password));
				memcpy(param.currentValue.data(), password, strlen(password));
				param.sizeOfCurrentValue = strlen(password);
			}
			if (strcmp(obs_property_name(property), "use_auth") == 0) {
				bool use_auth = obs_data_get_bool(settings, "use_auth");
				formatString  = "OBS_PROPERTY_BOOL";

				param.currentValue.resize(sizeof(use_auth));
				memcpy(param.currentValue.data(), &use_auth, sizeof(use_auth));
				param.sizeOfCurrentValue = sizeof(use_auth);
			}
		} else {
			if (format == OBS_COMBO_FORMAT_INT) {
				int64_t value = obs_data_get_int(settings, obs_property_name(property));

				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
				param.sizeOfCurrentValue = sizeof(value);
			} else if (format == OBS_COMBO_FORMAT_FLOAT) {
				double value = obs_data_get_double(settings, obs_property_name(property));

				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
				param.sizeOfCurrentValue = sizeof(value);
			} else if (format == OBS_COMBO_FORMAT_STRING) {
				currentServiceName = obs_data_get_string(settings, obs_property_name(property));

				param.currentValue.resize(strlen(currentServiceName));
				memcpy(param.currentValue.data(), currentServiceName, strlen(currentServiceName));
				param.sizeOfCurrentValue = strlen(currentServiceName);
			}
		}

		param.type        = formatString;
		param.description = obs_property_description(property);
		param.visible     = obs_property_visible(property);
		param.enabled     = isCategoryEnabled;

		param.masked = formatString.compare("OBS_PROPERTY_EDIT_TEXT") == 0
		               && obs_proprety_text_type(property) == OBS_TEXT_PASSWORD;

		serviceConfiguration.params.push_back(param);

		index++;
		obs_property_next(&property);
	}

	serviceConfiguration.name        = "Untitled";
	serviceConfiguration.paramsCount = serviceConfiguration.params.size();
	streamSettings.push_back(serviceConfiguration);

	return streamSettings;
}

void OBS_settings::saveStreamSettings(std::vector<SubCategory> streamSettings)
{
	obs_service_t* currentService = OBS_service::getService();

	obs_data_t* settings;

	std::string currentStreamType = obs_service_get_type(currentService);
	const char* newserviceTypeValue;

	std::string currentServiceName = obs_data_get_string(obs_service_get_settings(currentService), "service");
	std::string newServiceValue;

	SubCategory sc;
	bool        serviceChanged     = false;
	bool        serviceTypeChanged = false;

	for (int i = 0; i < streamSettings.size(); i++) {
		sc = streamSettings.at(i);

		std::string nameSubcategory = sc.name;
		Parameter   param;
		for (int j = 0; j < sc.params.size(); j++) {
			param = sc.params.at(j);

			std::string name = param.name;
			std::string type = param.type;

			std::string* value;
			if (type.compare("OBS_PROPERTY_LIST") == 0 || type.compare("OBS_PROPERTY_EDIT_TEXT") == 0) {
				value = new std::string(param.currentValue.data(), param.currentValue.size());

				if (name.compare("streamType") == 0) {
					newserviceTypeValue = value->c_str();
					settings            = obs_service_defaults(newserviceTypeValue);
					if (currentStreamType.compare(newserviceTypeValue) != 0) {
						serviceTypeChanged = true;
					}
				}

				if (name.compare("service") == 0) {
					newServiceValue = value->c_str();
					if (currentServiceName.compare(newServiceValue) != 0) {
						serviceChanged = true;
					}
				}
				obs_data_set_string(settings, name.c_str(), value->c_str());
			} else if (type.compare("OBS_PROPERTY_INT") == 0 || type.compare("OBS_PROPERTY_UINT") == 0) {
				int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
				obs_data_set_int(settings, name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
				bool* value = reinterpret_cast<bool*>(param.currentValue.data());
				obs_data_set_bool(settings, name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double* value = reinterpret_cast<double*>(param.currentValue.data());
				obs_data_set_double(settings, name.c_str(), *value);
			}
		}
	}

	if (serviceTypeChanged) {
		settings = obs_service_defaults(newserviceTypeValue);

		if (strcmp(newserviceTypeValue, "rtmp_common") == 0) {
			obs_data_set_string(settings, "streamType", "rtmp_common");
			obs_data_set_string(settings, "service", "Twitch");
			obs_data_set_bool(settings, "show_all", 0);
			obs_data_set_string(settings, "server", "auto");
			obs_data_set_string(settings, "key", "");
		}
	}

	obs_data_t* hotkeyData = obs_hotkeys_save_service(currentService);

	obs_service_t* newService = obs_service_create(newserviceTypeValue, "default_service", settings, hotkeyData);

	if (serviceChanged) {
		std::string server      = obs_data_get_string(settings, "server");
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
}

static bool EncoderAvailable(const char* encoder)
{
	const char* val;
	int         i = 0;

	while (obs_enum_encoder_types(i++, &val)) {
		if (strcmp(val, encoder) == 0)
			return true;
	}

	return false;
}

void OBS_settings::getSimpleAvailableEncoders(std::vector<std::pair<std::string, std::string>>* streamEncoder)
{
	streamEncoder->push_back(std::make_pair("Software (x264)", SIMPLE_ENCODER_X264));

	if (EncoderAvailable("obs_qsv11"))
		streamEncoder->push_back(std::make_pair("QSV", SIMPLE_ENCODER_QSV));

	if (EncoderAvailable("ffmpeg_nvenc"))
		streamEncoder->push_back(std::make_pair("NVENC", SIMPLE_ENCODER_NVENC));

	if (EncoderAvailable("amd_amf_h264"))
		streamEncoder->push_back(std::make_pair("AMD", SIMPLE_ENCODER_AMD));
}

void OBS_settings::getAdvancedAvailableEncoders(std::vector<std::pair<std::string, std::string>>* streamEncoder)
{
	streamEncoder->push_back(std::make_pair("Software (x264)", ADVANCED_ENCODER_X264));

	if (EncoderAvailable("obs_qsv11"))
		streamEncoder->push_back(std::make_pair("QSV", ADVANCED_ENCODER_QSV));

	if (EncoderAvailable("ffmpeg_nvenc"))
		streamEncoder->push_back(std::make_pair("NVENC", ADVANCED_ENCODER_NVENC));

	if (EncoderAvailable("amd_amf_h264"))
		streamEncoder->push_back(std::make_pair("AMD", ADVANCED_ENCODER_AMD));
}

void OBS_settings::getSimpleOutputSettings(
    std::vector<SubCategory>* outputSettings,
    config_t*                 config,
    bool                      isCategoryEnabled)
{
	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	//Streaming

	//Video Bitrate
	std::vector<std::pair<std::string, std::string>> vBitrate;
	vBitrate.push_back(std::make_pair("name", "VBitrate"));
	vBitrate.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	vBitrate.push_back(std::make_pair("description", "Video Bitrate"));
	vBitrate.push_back(std::make_pair("subType", ""));
	entries.push_back(vBitrate);

	//Encoder
	std::vector<std::pair<std::string, std::string>> streamEncoder;
	streamEncoder.push_back(std::make_pair("name", "StreamEncoder"));
	streamEncoder.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	streamEncoder.push_back(std::make_pair("description", "Encoder"));
	streamEncoder.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	getSimpleAvailableEncoders(&streamEncoder);

	entries.push_back(streamEncoder);

	//Audio Bitrate
	std::vector<std::pair<std::string, std::string>> aBitrate;
	aBitrate.push_back(std::make_pair("name", "ABitrate"));
	aBitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	aBitrate.push_back(std::make_pair("description", "Audio Bitrate"));
	aBitrate.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	auto& bitrateMap = GetAACEncoderBitrateMap();
	for (auto& entry : bitrateMap)
		aBitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));
	entries.push_back(aBitrate);

	//Enable Advanced Encoder Settings
	std::vector<std::pair<std::string, std::string>> useAdvanced;
	useAdvanced.push_back(std::make_pair("name", "UseAdvanced"));
	useAdvanced.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	useAdvanced.push_back(std::make_pair("description", "Enable Advanced Encoder Settings"));
	useAdvanced.push_back(std::make_pair("subType", ""));
	entries.push_back(useAdvanced);

	if (config_get_bool(config, "SimpleOutput", "UseAdvanced")) {
		//Enforce streaming service bitrate limits
		std::vector<std::pair<std::string, std::string>> enforceBitrate;
		enforceBitrate.push_back(std::make_pair("name", "EnforceBitrate"));
		enforceBitrate.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
		enforceBitrate.push_back(std::make_pair("description", "Enforce streaming service bitrate limits"));
		enforceBitrate.push_back(std::make_pair("subType", ""));
		entries.push_back(enforceBitrate);

		//Encoder Preset
		const char* defaultPreset;
		const char* encoder = config_get_string(config, "SimpleOutput", "StreamEncoder");

		std::vector<std::pair<std::string, std::string>> preset;

		if (strcmp(encoder, SIMPLE_ENCODER_QSV) == 0 || strcmp(encoder, ADVANCED_ENCODER_QSV) == 0) {
			preset.push_back(std::make_pair("name", "QSVPreset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));
			preset.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

			preset.push_back(std::make_pair("Speed", "speed"));
			preset.push_back(std::make_pair("Balanced", "balanced"));
			preset.push_back(std::make_pair("Quality", "quality"));

			defaultPreset = "balanced";
			// preset = curQSVPreset;

		} else if (strcmp(encoder, SIMPLE_ENCODER_NVENC) == 0 || strcmp(encoder, ADVANCED_ENCODER_NVENC) == 0) {
			preset.push_back(std::make_pair("name", "NVENCPreset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));
			preset.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

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

				preset.push_back(std::make_pair(name, val));
			}

			obs_properties_destroy(props);

			defaultPreset = "default";
			// preset = curNVENCPreset;

		} else if (strcmp(encoder, SIMPLE_ENCODER_AMD) == 0 || strcmp(encoder, ADVANCED_ENCODER_AMD) == 0) {
			preset.push_back(std::make_pair("name", "AMDPreset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));
			preset.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

			preset.push_back(std::make_pair("Speed", "speed"));
			preset.push_back(std::make_pair("Balanced", "balanced"));
			preset.push_back(std::make_pair("Quality", "quality"));

			defaultPreset = "balanced";
			// preset = curAMDPreset;
		} else {
			preset.push_back(std::make_pair("name", "Preset"));
			preset.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
			preset.push_back(std::make_pair("description", "Encoder Preset (higher = less CPU)"));
			preset.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

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
		x264opts.push_back(std::make_pair("subType", ""));
		entries.push_back(x264opts);
	}

	outputSettings->push_back(
	    serializeSettingsData("Streaming", entries, config, "SimpleOutput", true, isCategoryEnabled));
	entries.clear();

	//Recording

	//Recording Path
	std::vector<std::pair<std::string, std::string>> filePath;
	filePath.push_back(std::make_pair("name", "FilePath"));
	filePath.push_back(std::make_pair("type", "OBS_PROPERTY_PATH"));
	filePath.push_back(std::make_pair("description", "Recording Path"));
	filePath.push_back(std::make_pair("subType", ""));
	entries.push_back(filePath);

	//Generate File Name without Space
	std::vector<std::pair<std::string, std::string>> fileNameWithoutSpace;
	fileNameWithoutSpace.push_back(std::make_pair("name", "FileNameWithoutSpace"));
	fileNameWithoutSpace.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	fileNameWithoutSpace.push_back(std::make_pair("description", "Generate File Name without Space"));
	fileNameWithoutSpace.push_back(std::make_pair("subType", ""));
	entries.push_back(fileNameWithoutSpace);

	//Recording Quality
	std::vector<std::pair<std::string, std::string>> recQuality;
	recQuality.push_back(std::make_pair("name", "RecQuality"));
	recQuality.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	recQuality.push_back(std::make_pair("description", "Recording Quality"));
	recQuality.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
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
	recFormat.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
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
	muxerCustom.push_back(std::make_pair("subType", ""));
	entries.push_back(muxerCustom);

	//Enable Replay Buffer
	std::vector<std::pair<std::string, std::string>> recRB;
	recRB.push_back(std::make_pair("name", "RecRB"));
	recRB.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	recRB.push_back(std::make_pair("description", "Enable Replay Buffer"));
	recRB.push_back(std::make_pair("subType", ""));
	entries.push_back(recRB);

	outputSettings->push_back(
	    serializeSettingsData("Recording", entries, config, "SimpleOutput", true, isCategoryEnabled));
}

void OBS_settings::getEncoderSettings(
    const obs_encoder_t*    encoder,
    obs_data_t*             settings,
    std::vector<Parameter>* subCategoryParameters,
    int                     index,
    bool                    isCategoryEnabled,
    bool                    recordEncoder)
{
	obs_properties_t* encoderProperties = obs_encoder_properties(encoder);
	obs_property_t*   property          = obs_properties_first(encoderProperties);

	Parameter param;
	while (property) {
		param.name                     = obs_property_name(property);
		obs_property_type typeProperty = obs_property_get_type(property);

		switch (typeProperty) {
		case OBS_PROPERTY_BOOL: {
			param.type        = "OBS_PROPERTY_BOOL";
			param.description = obs_property_description(property);

			bool value = obs_data_get_bool(settings, param.name.c_str());

			param.currentValue.resize(sizeof(value));
			memcpy(param.currentValue.data(), &value, sizeof(value));
			param.sizeOfCurrentValue = sizeof(value);

			break;
		}
		case OBS_PROPERTY_INT: {
			param.type        = "OBS_PROPERTY_INT";
			param.description = obs_property_description(property);

			int64_t value = obs_data_get_int(settings, param.name.c_str());

			param.currentValue.resize(sizeof(value));
			memcpy(param.currentValue.data(), &value, sizeof(value));
			param.sizeOfCurrentValue = sizeof(value);

			break;
		}
		case OBS_PROPERTY_FLOAT: {
			param.type        = "OBS_PROPERTY_DOUBLE";
			param.description = obs_property_description(property);

			double value = obs_data_get_double(settings, param.name.c_str());

			param.currentValue.resize(sizeof(value));
			memcpy(param.currentValue.data(), &value, sizeof(value));
			param.sizeOfCurrentValue = sizeof(value);

			break;
		}
		case OBS_PROPERTY_TEXT: {
			param.type        = "OBS_PROPERTY_TEXT";
			param.description = obs_property_description(property);

			const char* currentValue = obs_data_get_string(settings, param.name.c_str());

			if (currentValue == NULL) {
				currentValue = "";
			}

			param.currentValue.resize(strlen(currentValue));
			memcpy(param.currentValue.data(), currentValue, strlen(currentValue));
			param.sizeOfCurrentValue = strlen(currentValue);

			break;
		}
		case OBS_PROPERTY_PATH: {
			param.type        = "OBS_PROPERTY_PATH";
			param.description = obs_property_description(property);

			const char* currentValue = obs_data_get_string(settings, param.name.c_str());

			if (currentValue == NULL) {
				currentValue = "";
			}

			param.currentValue.resize(strlen(currentValue));
			memcpy(param.currentValue.data(), currentValue, strlen(currentValue));
			param.sizeOfCurrentValue = strlen(currentValue);

			break;
		}
		case OBS_PROPERTY_LIST: {
			param.type        = "OBS_PROPERTY_LIST";
			param.description = obs_property_description(property);

			obs_combo_format format = obs_property_list_format(property);

			if (format == OBS_COMBO_FORMAT_INT) {
				int64_t value = obs_data_get_int(settings, param.name.c_str());
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
				param.sizeOfCurrentValue = sizeof(value);
			} else if (format == OBS_COMBO_FORMAT_FLOAT) {
				double value = obs_data_get_double(settings, param.name.c_str());
				param.currentValue.resize(sizeof(value));
				memcpy(param.currentValue.data(), &value, sizeof(value));
				param.sizeOfCurrentValue = sizeof(value);
			} else if (format == OBS_COMBO_FORMAT_STRING) {
				const char* currentValue = obs_data_get_string(settings, param.name.c_str());

				if (currentValue == NULL) {
					currentValue = "";
				}

				param.currentValue.resize(strlen(currentValue));
				memcpy(param.currentValue.data(), currentValue, strlen(currentValue));
				param.sizeOfCurrentValue = strlen(currentValue);
			}

			int count = (int)obs_property_list_item_count(property);

			param.values.clear();

			for (int i = 0; i < count; i++) {
				// Name
				std::string itemName = obs_property_list_item_name(property, i);

				if (format == OBS_COMBO_FORMAT_INT) {
					param.subType = "OBS_COMBO_FORMAT_INT";

					uint64_t          sizeName = itemName.length();
					std::vector<char> sizeNameBuffer;
					sizeNameBuffer.resize(sizeof(sizeName));
					memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

					param.values.insert(param.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
					param.values.insert(param.values.end(), itemName.begin(), itemName.end());

					int64_t value = obs_property_list_item_int(property, i);

					std::vector<char> valueBuffer;
					valueBuffer.resize(sizeof(uint64_t));
					memcpy(valueBuffer.data(), &value, sizeof(value));

					param.values.insert(param.values.end(), valueBuffer.begin(), valueBuffer.end());
				} else if (format == OBS_COMBO_FORMAT_FLOAT) {
					param.subType = "OBS_COMBO_FORMAT_FLOAT";

					uint64_t          sizeName = itemName.length();
					std::vector<char> sizeNameBuffer;
					sizeNameBuffer.resize(sizeof(sizeName));
					memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

					param.values.insert(param.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
					param.values.insert(param.values.end(), itemName.begin(), itemName.end());

					double value = obs_property_list_item_float(property, i);

					std::vector<char> valueBuffer;
					valueBuffer.resize(sizeof(value));
					memcpy(valueBuffer.data(), &value, sizeof(value));

					param.values.insert(param.values.end(), valueBuffer.begin(), valueBuffer.end());
				} else if (format == OBS_COMBO_FORMAT_STRING) {
					param.subType = "OBS_COMBO_FORMAT_STRING";

					uint64_t          sizeName = itemName.length();
					std::vector<char> sizeNameBuffer;
					sizeNameBuffer.resize(sizeof(sizeName));
					memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

					param.values.insert(param.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
					param.values.insert(param.values.end(), itemName.begin(), itemName.end());

					std::string value = obs_property_list_item_string(property, i);

					uint64_t          sizeValue = value.length();
					std::vector<char> sizeValueBuffer;
					sizeValueBuffer.resize(sizeof(sizeValue));
					memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

					param.values.insert(param.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
					param.values.insert(param.values.end(), value.begin(), value.end());
				}
			}

			param.sizeOfValues = param.values.size();
			param.countValues  = count;

			break;
		}
		case OBS_PROPERTY_EDITABLE_LIST: {
			param.type        = "OBS_PROPERTY_EDITABLE_LIST";
			param.description = obs_property_description(property);

			const char* currentValue = obs_data_get_string(settings, param.name.c_str());

			if (currentValue == NULL)
				currentValue = "";

			param.currentValue.resize(strlen(currentValue));
			memcpy(param.currentValue.data(), currentValue, strlen(currentValue));
			param.sizeOfCurrentValue = strlen(currentValue);

			break;
		}
		}
		param.visible = obs_property_visible(property);

		bool isEnabled = obs_property_enabled(property);
		if (!isCategoryEnabled)
			isEnabled = isCategoryEnabled;

		param.enabled = isEnabled;
		param.masked  = false;

		if (recordEncoder) {
			param.name.insert(0, "Rec");
		}

		subCategoryParameters->push_back(param);

		obs_property_next(&property);
	}
}

SubCategory OBS_settings::getAdvancedOutputStreamingSettings(config_t* config, bool isCategoryEnabled)
{
	int index = 0;

	SubCategory                                                   streamingSettings;
	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	streamingSettings.name = "Streaming";

	// Audio Track : list
	Parameter trackIndex;
	trackIndex.name        = "TrackIndex";
	trackIndex.type        = "OBS_PROPERTY_LIST";
	trackIndex.subType     = "OBS_COMBO_FORMAT_STRING";
	trackIndex.description = "Audio Track";

	std::vector<std::pair<std::string, std::string>> trackIndexValues;
	trackIndexValues.push_back(std::make_pair("1", "1"));
	trackIndexValues.push_back(std::make_pair("2", "2"));
	trackIndexValues.push_back(std::make_pair("3", "3"));
	trackIndexValues.push_back(std::make_pair("4", "4"));
	trackIndexValues.push_back(std::make_pair("5", "5"));
	trackIndexValues.push_back(std::make_pair("6", "6"));

	for (int i = 0; i < trackIndexValues.size(); i++) {
		std::string name = trackIndexValues.at(i).first;

		uint64_t          sizeName = name.length();
		std::vector<char> sizeNameBuffer;
		sizeNameBuffer.resize(sizeof(sizeName));
		memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

		trackIndex.values.insert(trackIndex.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
		trackIndex.values.insert(trackIndex.values.end(), name.begin(), name.end());

		std::string value = trackIndexValues.at(i).second;

		uint64_t          sizeValue = value.length();
		std::vector<char> sizeValueBuffer;
		sizeValueBuffer.resize(sizeof(sizeValue));
		memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

		trackIndex.values.insert(trackIndex.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
		trackIndex.values.insert(trackIndex.values.end(), value.begin(), value.end());
	}

	trackIndex.sizeOfValues = trackIndex.values.size();
	trackIndex.countValues  = trackIndexValues.size();

	const char* trackIndexCurrentValue = config_get_string(config, "AdvOut", "TrackIndex");
	if (trackIndexCurrentValue == NULL)
		trackIndexCurrentValue = "";

	trackIndex.currentValue.resize(strlen(trackIndexCurrentValue));
	memcpy(trackIndex.currentValue.data(), trackIndexCurrentValue, strlen(trackIndexCurrentValue));
	trackIndex.sizeOfCurrentValue = strlen(trackIndexCurrentValue);

	trackIndex.visible = true;
	trackIndex.enabled = isCategoryEnabled;
	trackIndex.masked  = false;

	streamingSettings.params.push_back(trackIndex);

	// Encoder : list
	Parameter videoEncoders;
	videoEncoders.name        = "Encoder";
	videoEncoders.type        = "OBS_PROPERTY_LIST";
	videoEncoders.description = "Encoder";
	videoEncoders.subType     = "OBS_COMBO_FORMAT_STRING";

	const char* encoderCurrentValue = config_get_string(config, "AdvOut", "Encoder");
	if (encoderCurrentValue == NULL) {
		encoderCurrentValue = "";
	}

	videoEncoders.currentValue.resize(strlen(encoderCurrentValue));
	memcpy(videoEncoders.currentValue.data(), encoderCurrentValue, strlen(encoderCurrentValue));
	videoEncoders.sizeOfCurrentValue = strlen(encoderCurrentValue);

	std::vector<std::pair<std::string, std::string>> encoderValues;
	getAdvancedAvailableEncoders(&encoderValues);

	for (int i = 0; i < encoderValues.size(); i++) {
		std::string name = encoderValues.at(i).first;

		uint64_t          sizeName = name.length();
		std::vector<char> sizeNameBuffer;
		sizeNameBuffer.resize(sizeof(sizeName));
		memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

		videoEncoders.values.insert(videoEncoders.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
		videoEncoders.values.insert(videoEncoders.values.end(), name.begin(), name.end());

		std::string value = encoderValues.at(i).second;

		uint64_t          sizeValue = value.length();
		std::vector<char> sizeValueBuffer;
		sizeValueBuffer.resize(sizeof(sizeValue));
		memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

		videoEncoders.values.insert(videoEncoders.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
		videoEncoders.values.insert(videoEncoders.values.end(), value.begin(), value.end());
	}

	videoEncoders.sizeOfValues = videoEncoders.values.size();
	videoEncoders.countValues  = encoderValues.size();

	videoEncoders.visible = true;
	videoEncoders.enabled = isCategoryEnabled;
	videoEncoders.masked  = false;

	streamingSettings.params.push_back(videoEncoders);

	// Enforce streaming service encoder settings : boolean
	Parameter applyServiceSettings;
	applyServiceSettings.name        = "ApplyServiceSettings";
	applyServiceSettings.type        = "OBS_PROPERTY_BOOL";
	applyServiceSettings.description = "Enforce streaming service encoder settings";

	bool applyServiceSettingsValue = config_get_bool(config, "AdvOut", "ApplyServiceSettings");

	applyServiceSettings.currentValue.resize(sizeof(applyServiceSettingsValue));
	memcpy(applyServiceSettings.currentValue.data(), &applyServiceSettingsValue, sizeof(applyServiceSettingsValue));
	applyServiceSettings.sizeOfCurrentValue = sizeof(applyServiceSettingsValue);

	applyServiceSettings.visible = true;
	applyServiceSettings.enabled = isCategoryEnabled;
	applyServiceSettings.masked  = false;

	streamingSettings.params.push_back(applyServiceSettings);

	// Rescale Output : boolean
	Parameter rescale;
	rescale.name        = "Rescale";
	rescale.type        = "OBS_PROPERTY_BOOL";
	rescale.description = "Rescale Output";

	bool doRescale = config_get_bool(config, "AdvOut", "Rescale");

	rescale.currentValue.resize(sizeof(doRescale));
	memcpy(rescale.currentValue.data(), &doRescale, sizeof(doRescale));
	rescale.sizeOfCurrentValue = sizeof(doRescale);

	rescale.visible = true;
	rescale.enabled = isCategoryEnabled;
	rescale.masked  = false;

	streamingSettings.params.push_back(rescale);

	if (doRescale) {
		// Output Resolution : list
		Parameter rescaleRes;
		rescaleRes.name        = "RescaleRes";
		rescaleRes.type        = "OBS_PROPERTY_LIST";
		rescaleRes.description = "Output Resolution";
		rescaleRes.subType     = "OBS_COMBO_FORMAT_STRING";

		uint64_t base_cx = config_get_uint(config, "Video", "BaseCX");
		uint64_t base_cy = config_get_uint(config, "Video", "BaseCY");

		const char* outputResString = config_get_string(config, "AdvOut", "RescaleRes");

		if (outputResString == NULL) {
			outputResString = "1280x720";
			config_set_string(config, "AdvOut", "RescaleRes", outputResString);
			config_save_safe(config, "tmp", nullptr);
		}

		rescaleRes.currentValue.resize(strlen(outputResString));
		memcpy(rescaleRes.currentValue.data(), outputResString, strlen(outputResString));
		rescaleRes.sizeOfCurrentValue = strlen(outputResString);

		std::vector<pair<uint64_t, uint64_t>> outputResolutions = getOutputResolutions(base_cx, base_cy);

		uint32_t indexDataRescaleRes = 0;

		for (int i = 0; i < outputResolutions.size(); i++) {
			std::string outRes = ResString(outputResolutions.at(i).first, outputResolutions.at(i).second);

			for (int j = 0; j < 2; j++) {
				uint64_t          sizeRes = outRes.length();
				std::vector<char> sizeNameBuffer;
				sizeNameBuffer.resize(sizeof(sizeRes));
				memcpy(sizeNameBuffer.data(), &sizeRes, sizeof(sizeRes));

				rescaleRes.values.insert(rescaleRes.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
				rescaleRes.values.insert(rescaleRes.values.end(), outRes.begin(), outRes.end());
			}
		}

		rescaleRes.sizeOfValues = rescaleRes.values.size();
		rescaleRes.countValues  = outputResolutions.size();

		rescaleRes.visible = true;
		rescaleRes.enabled = isCategoryEnabled;
		rescaleRes.masked  = false;

		streamingSettings.params.push_back(rescaleRes);
	}

	// Encoder settings
	const char* encoderID = config_get_string(config, "AdvOut", "Encoder");
	if (encoderID == NULL) {
		encoderID = "obs_x264";
		config_set_string(config, "AdvOut", "Encoder", encoderID);
		config_save_safe(config, "tmp", nullptr);
	}

	struct stat buffer;

	bool fileExist = (os_stat(ConfigManager::getInstance().getStream().c_str(), &buffer) == 0);

	obs_data_t*    settings = obs_encoder_defaults(encoderID);
	obs_encoder_t* streamingEncoder;
	obs_output_t*  streamOutput = OBS_service::getStreamingOutput();

	if (streamOutput == NULL)
		return streamingSettings;

	if (!obs_output_active(streamOutput)) {
		if (!fileExist) {
			streamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", nullptr, nullptr);
			OBS_service::setStreamingEncoder(streamingEncoder);

			if (!obs_data_save_json_safe(settings, ConfigManager::getInstance().getStream().c_str(), "tmp", "bak")) {
				blog(LOG_WARNING, "Failed to save encoder %s", ConfigManager::getInstance().getStream().c_str());
			}
		} else {
			obs_data_t* data =
			    obs_data_create_from_json_file_safe(ConfigManager::getInstance().getStream().c_str(), "bak");
			obs_data_apply(settings, data);
			streamingEncoder = obs_video_encoder_create(encoderID, "streaming_h264", settings, nullptr);
			OBS_service::setStreamingEncoder(streamingEncoder);
		}
	} else {
		streamingEncoder = OBS_service::getStreamingEncoder();
		settings         = obs_encoder_get_settings(streamingEncoder);
	}

	getEncoderSettings(streamingEncoder, settings, &(streamingSettings.params), index, isCategoryEnabled, false);
	streamingSettings.paramsCount = streamingSettings.params.size();
	return streamingSettings;
}

void OBS_settings::getStandardRecordingSettings(
    SubCategory* subCategoryParameters,
    config_t*    config,
    bool         isCategoryEnabled)
{
	int index = 1;

	// Recording Path : file
	Parameter recFilePath;
	recFilePath.name        = "RecFilePath";
	recFilePath.type        = "OBS_PROPERTY_PATH";
	recFilePath.description = "Recording Path";

	const char* RecFilePathCurrentValue = config_get_string(config, "AdvOut", "RecFilePath");

	if (RecFilePathCurrentValue == NULL) {
		const char* RecFilePathText = OBS_service::GetDefaultVideoSavePath().c_str();

		recFilePath.currentValue.resize(strlen(RecFilePathText));
		memcpy(recFilePath.currentValue.data(), RecFilePathText, strlen(RecFilePathText));
		recFilePath.sizeOfCurrentValue = strlen(RecFilePathText);
	} else {
		recFilePath.currentValue.resize(strlen(RecFilePathCurrentValue));
		memcpy(recFilePath.currentValue.data(), RecFilePathCurrentValue, strlen(RecFilePathCurrentValue));
		recFilePath.sizeOfCurrentValue = strlen(RecFilePathCurrentValue);
	}

	recFilePath.visible = true;
	recFilePath.enabled = isCategoryEnabled;
	recFilePath.masked  = false;

	subCategoryParameters->params.push_back(recFilePath);

	// Generate File Name without Space : boolean
	Parameter recFileNameWithoutSpace;
	recFileNameWithoutSpace.name        = "RecFileNameWithoutSpace";
	recFileNameWithoutSpace.type        = "OBS_PROPERTY_BOOL";
	recFileNameWithoutSpace.description = "Generate File Name without Space";

	const char* currentValue = "RecFileNameWithoutSpace";
	recFileNameWithoutSpace.currentValue.resize(strlen(currentValue));
	memcpy(recFileNameWithoutSpace.currentValue.data(), currentValue, strlen(currentValue));
	recFileNameWithoutSpace.sizeOfCurrentValue = strlen(currentValue);

	recFileNameWithoutSpace.visible = true;
	recFileNameWithoutSpace.enabled = isCategoryEnabled;
	recFileNameWithoutSpace.masked  = false;

	subCategoryParameters->params.push_back(recFileNameWithoutSpace);

	// Recording Format : list
	Parameter recFormat;
	recFormat.name        = "RecFormat";
	recFormat.type        = "OBS_PROPERTY_LIST";
	recFormat.description = "Recording Format";
	recFormat.subType     = "OBS_COMBO_FORMAT_STRING";

	const char* recFormatCurrentValue = config_get_string(config, "AdvOut", "RecFormat");
	if (recFormatCurrentValue == NULL)
		recFormatCurrentValue = "";

	recFormat.currentValue.resize(strlen(recFormatCurrentValue));
	memcpy(recFormat.currentValue.data(), recFormatCurrentValue, strlen(recFormatCurrentValue));
	recFormat.sizeOfCurrentValue = strlen(recFormatCurrentValue);

	std::vector<std::pair<std::string, std::string>> recFormatValues;
	recFormatValues.push_back(std::make_pair("flv", "flv"));
	recFormatValues.push_back(std::make_pair("mp4", "mp4"));
	recFormatValues.push_back(std::make_pair("mov", "mov"));
	recFormatValues.push_back(std::make_pair("mkv", "mkv"));
	recFormatValues.push_back(std::make_pair("ts", "ts"));
	recFormatValues.push_back(std::make_pair("m3u8", "m3u8"));

	uint32_t indexDataRecFormat = 0;

	for (int i = 0; i < recFormatValues.size(); i++) {
		std::string name = recFormatValues.at(i).first;

		uint64_t          sizeName = name.length();
		std::vector<char> sizeNameBuffer;
		sizeNameBuffer.resize(sizeof(sizeName));
		memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

		recFormat.values.insert(recFormat.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
		recFormat.values.insert(recFormat.values.end(), name.begin(), name.end());

		std::string value = recFormatValues.at(i).second;

		uint64_t          sizeValue = value.length();
		std::vector<char> sizeValueBuffer;
		sizeValueBuffer.resize(sizeof(sizeValue));
		memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

		recFormat.values.insert(recFormat.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
		recFormat.values.insert(recFormat.values.end(), value.begin(), value.end());
	}

	recFormat.sizeOfValues = recFormat.values.size();
	recFormat.countValues  = recFormatValues.size();

	recFormat.visible = true;
	recFormat.enabled = isCategoryEnabled;
	recFormat.masked  = false;

	subCategoryParameters->params.push_back(recFormat);

	// Audio Track : list
	Parameter recTracks;
	recTracks.name        = "RecTracks";
	recTracks.type        = "OBS_PROPERTY_LIST";
	recTracks.description = "Audio Track";
	recTracks.subType     = "OBS_COMBO_FORMAT_STRING";

	const char* recTracksCurrentValue = config_get_string(config, "AdvOut", "RecTracks");
	if (recTracksCurrentValue == NULL)
		recTracksCurrentValue = "";

	recTracks.currentValue.resize(strlen(recTracksCurrentValue));
	memcpy(recTracks.currentValue.data(), recTracksCurrentValue, strlen(recTracksCurrentValue));
	recTracks.sizeOfCurrentValue = strlen(recTracksCurrentValue);

	std::vector<std::pair<std::string, std::string>> recTracksValues;
	recTracksValues.push_back(std::make_pair("1", "1"));
	recTracksValues.push_back(std::make_pair("2", "2"));
	recTracksValues.push_back(std::make_pair("3", "3"));
	recTracksValues.push_back(std::make_pair("4", "4"));
	recTracksValues.push_back(std::make_pair("5", "5"));
	recTracksValues.push_back(std::make_pair("6", "6"));

	uint32_t indexRecTracksCurrentValue = 0;

	for (int i = 0; i < recTracksValues.size(); i++) {
		std::string name = recTracksValues.at(i).first;

		uint64_t          sizeName = name.length();
		std::vector<char> sizeNameBuffer;
		sizeNameBuffer.resize(sizeof(sizeName));
		memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

		recTracks.values.insert(recTracks.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
		recTracks.values.insert(recTracks.values.end(), name.begin(), name.end());

		std::string value = recTracksValues.at(i).second;

		uint64_t          sizeValue = value.length();
		std::vector<char> sizeValueBuffer;
		sizeValueBuffer.resize(sizeof(sizeValue));
		memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

		recTracks.values.insert(recTracks.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
		recTracks.values.insert(recTracks.values.end(), value.begin(), value.end());
	}

	recTracks.sizeOfValues = recTracks.values.size();
	recTracks.countValues  = recTracksValues.size();

	recTracks.visible = true;
	recTracks.enabled = isCategoryEnabled;
	recTracks.masked  = false;

	subCategoryParameters->params.push_back(recTracks);

	// Encoder : list
	Parameter recEncoder;
	recEncoder.name        = "RecEncoder";
	recEncoder.type        = "OBS_PROPERTY_LIST";
	recEncoder.description = "Recording";
	recEncoder.subType     = "OBS_COMBO_FORMAT_STRING";

	const char* recEncoderCurrentValue = config_get_string(config, "AdvOut", "RecEncoder");
	if (!recEncoderCurrentValue)
		recEncoderCurrentValue = "none";

	recEncoder.currentValue.resize(strlen(recEncoderCurrentValue));
	memcpy(recEncoder.currentValue.data(), recEncoderCurrentValue, strlen(recEncoderCurrentValue));
	recEncoder.sizeOfCurrentValue = strlen(recEncoderCurrentValue);

	std::vector<std::pair<std::string, std::string>> Encoder;
	Encoder.push_back(std::make_pair("Use stream encoder", "none"));
	getAdvancedAvailableEncoders(&Encoder);

	uint32_t indexDataRecEncoder = 0;

	for (int i = 0; i < Encoder.size(); i++) {
		std::string name = Encoder.at(i).first;

		uint64_t          sizeName = name.length();
		std::vector<char> sizeNameBuffer;
		sizeNameBuffer.resize(sizeof(sizeName));
		memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

		recEncoder.values.insert(recEncoder.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
		recEncoder.values.insert(recEncoder.values.end(), name.begin(), name.end());

		std::string value = Encoder.at(i).second;

		uint64_t          sizeValue = value.length();
		std::vector<char> sizeValueBuffer;
		sizeValueBuffer.resize(sizeof(sizeValue));
		memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

		recEncoder.values.insert(recEncoder.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
		recEncoder.values.insert(recEncoder.values.end(), value.begin(), value.end());
	}

	recEncoder.sizeOfValues = recEncoder.values.size();
	recEncoder.countValues  = Encoder.size();

	recEncoder.visible = true;
	recEncoder.enabled = isCategoryEnabled;
	recEncoder.masked  = false;

	subCategoryParameters->params.push_back(recEncoder);

	// Rescale Output : boolean
	Parameter recRescale;
	recRescale.name        = "RecRescale";
	recRescale.type        = "OBS_PROPERTY_BOOL";
	recRescale.description = "Rescale Output";

	bool doRescale = config_get_bool(config, "AdvOut", "RecRescale");

	recRescale.currentValue.resize(sizeof(doRescale));
	memcpy(recRescale.currentValue.data(), &doRescale, sizeof(doRescale));
	recRescale.sizeOfCurrentValue = sizeof(doRescale);

	recRescale.visible = true;
	recRescale.enabled = isCategoryEnabled;
	recRescale.masked  = false;

	subCategoryParameters->params.push_back(recRescale);

	// Output Resolution : list
	if (doRescale) {
		// Output Resolution : list
		Parameter recRescaleRes;
		recRescaleRes.name        = "RecRescaleRes";
		recRescaleRes.type        = "OBS_PROPERTY_LIST";
		recRescaleRes.description = "Output Resolution";
		recRescaleRes.subType     = "OBS_COMBO_FORMAT_STRING";

		uint64_t base_cx = config_get_uint(config, "Video", "BaseCX");
		uint64_t base_cy = config_get_uint(config, "Video", "BaseCY");

		const char* outputResString = config_get_string(config, "AdvOut", "RecRescaleRes");

		if (outputResString == NULL) {
			outputResString = "1280x720";
			config_set_string(config, "AdvOut", "RecRescaleRes", outputResString);
			config_save_safe(config, "tmp", nullptr);
		}

		recRescaleRes.currentValue.resize(strlen(outputResString));
		memcpy(recRescaleRes.currentValue.data(), outputResString, strlen(outputResString));
		recRescaleRes.sizeOfCurrentValue = strlen(outputResString);

		std::vector<std::pair<uint64_t, uint64_t>> outputResolutions = getOutputResolutions(base_cx, base_cy);

		uint32_t indexDataRecRescaleRes = 0;

		for (int i = 0; i < outputResolutions.size(); i++) {
			std::string outRes = ResString(outputResolutions.at(i).first, outputResolutions.at(i).second);

			for (int j = 0; j < 2; j++) {
				uint64_t          sizeRes = outRes.length();
				std::vector<char> sizeNameBuffer;
				sizeNameBuffer.resize(sizeof(sizeRes));
				memcpy(sizeNameBuffer.data(), &sizeRes, sizeof(sizeRes));

				recRescaleRes.values.insert(recRescaleRes.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
				recRescaleRes.values.insert(recRescaleRes.values.end(), outRes.begin(), outRes.end());
			}
		}

		recRescaleRes.sizeOfValues = recRescaleRes.values.size();
		recRescaleRes.countValues  = outputResolutions.size();

		recRescaleRes.visible = true;
		recRescaleRes.enabled = isCategoryEnabled;
		recRescaleRes.masked  = false;

		subCategoryParameters->params.push_back(recRescaleRes);
	}

	// Custom Muxer Settings : edit_text
	Parameter recMuxerCustom;
	recMuxerCustom.name        = "RecMuxerCustom";
	recMuxerCustom.type        = "OBS_PROPERTY_EDIT_TEXT";
	recMuxerCustom.description = "Custom Muxer Settings";

	const char* RecMuxerCustomCurrentValue = config_get_string(config, "AdvOut", "RecMuxerCustom");
	if (RecMuxerCustomCurrentValue == NULL)
		RecMuxerCustomCurrentValue = "";

	recMuxerCustom.currentValue.resize(strlen(RecMuxerCustomCurrentValue));
	memcpy(recMuxerCustom.currentValue.data(), RecMuxerCustomCurrentValue, strlen(RecMuxerCustomCurrentValue));
	recMuxerCustom.sizeOfCurrentValue = strlen(RecMuxerCustomCurrentValue);

	recMuxerCustom.visible = true;
	recMuxerCustom.enabled = isCategoryEnabled;
	recMuxerCustom.masked  = false;

	subCategoryParameters->params.push_back(recMuxerCustom);

	// Encoder settings
	struct stat buffer;

	bool fileExist = (os_stat(ConfigManager::getInstance().getRecord().c_str(), &buffer) == 0);

	obs_data_t*    settings = obs_encoder_defaults(recEncoderCurrentValue);
	obs_encoder_t* recordingEncoder;

	obs_output_t* recordOutput = OBS_service::getRecordingOutput();

	if (recordOutput == NULL)
		return;

	if (!obs_output_active(recordOutput)) {
		if (!fileExist) {
			recordingEncoder = obs_video_encoder_create(recEncoderCurrentValue, "recording_h264", nullptr, nullptr);
			OBS_service::setRecordingEncoder(recordingEncoder);

			if (!obs_data_save_json_safe(settings, ConfigManager::getInstance().getRecord().c_str(), "tmp", "bak")) {
				blog(LOG_WARNING, "Failed to save encoder %s", ConfigManager::getInstance().getRecord().c_str());
			}
		} else {
			obs_data_t* data =
			    obs_data_create_from_json_file_safe(ConfigManager::getInstance().getRecord().c_str(), "bak");
			obs_data_apply(settings, data);
			recordingEncoder = obs_video_encoder_create(recEncoderCurrentValue, "recording_h264", settings, nullptr);
			OBS_service::setRecordingEncoder(recordingEncoder);
		}
	} else {
		recordingEncoder = OBS_service::getRecordingEncoder();
		settings         = obs_encoder_get_settings(recordingEncoder);
	}

	if (strcmp(recEncoderCurrentValue, "none")) {
		getEncoderSettings(
		    recordingEncoder, settings, &(subCategoryParameters->params), index, isCategoryEnabled, true);
	}

	subCategoryParameters->paramsCount = subCategoryParameters->params.size();
}

SubCategory OBS_settings::getAdvancedOutputRecordingSettings(config_t* config, bool isCategoryEnabled)
{
	SubCategory recordingSettings;

	int index = 0;

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	// Type : list
	Parameter recType;
	recType.name        = "RecType";
	recType.type        = "OBS_PROPERTY_LIST";
	recType.description = "Type";

	std::vector<std::pair<std::string, std::string>> recTypeValues;
	recTypeValues.push_back(std::make_pair("Standard", "Standard"));

	uint32_t indexDataRecType = 0;

	for (int i = 0; i < recTypeValues.size(); i++) {
		std::string name = recTypeValues.at(i).first;

		uint64_t          sizeName = name.length();
		std::vector<char> sizeNameBuffer;
		sizeNameBuffer.resize(sizeof(sizeName));
		memcpy(sizeNameBuffer.data(), &sizeName, sizeof(sizeName));

		recType.values.insert(recType.values.end(), sizeNameBuffer.begin(), sizeNameBuffer.end());
		recType.values.insert(recType.values.end(), name.begin(), name.end());

		std::string value = recTypeValues.at(i).second;

		uint64_t          sizeValue = value.length();
		std::vector<char> sizeValueBuffer;
		sizeValueBuffer.resize(sizeof(sizeValue));
		memcpy(sizeValueBuffer.data(), &sizeValue, sizeof(sizeValue));

		recType.values.insert(recType.values.end(), sizeValueBuffer.begin(), sizeValueBuffer.end());
		recType.values.insert(recType.values.end(), value.begin(), value.end());
	}

	recType.sizeOfValues = recType.values.size();
	recType.countValues  = recTypeValues.size();

	const char* RecTypeCurrentValue = config_get_string(config, "AdvOut", "RecType");
	if (RecTypeCurrentValue == NULL)
		RecTypeCurrentValue = "";

	recType.currentValue.resize(strlen(RecTypeCurrentValue));
	memcpy(recType.currentValue.data(), RecTypeCurrentValue, strlen(RecTypeCurrentValue));
	recType.sizeOfCurrentValue = strlen(RecTypeCurrentValue);

	recType.visible = true;
	recType.enabled = isCategoryEnabled;
	recType.masked  = false;

	recordingSettings.params.push_back(recType);

	const char* currentRecType = config_get_string(config, "AdvOut", "RecType");

	if (currentRecType == NULL) {
		currentRecType = "Standard";
		config_set_string(config, "AdvOut", "RecType", currentRecType);
	}

	getStandardRecordingSettings(&recordingSettings, config, isCategoryEnabled);

	recordingSettings.name        = "Recording";
	recordingSettings.paramsCount = recordingSettings.params.size();

	return recordingSettings;
}

void OBS_settings::getAdvancedOutputAudioSettings(
    std::vector<SubCategory>* outputSettings,
    config_t*                 config,
    bool                      isCategoryEnabled)
{
	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	auto& bitrateMap = GetAACEncoderBitrateMap();

	// Track 1
	std::vector<std::pair<std::string, std::string>> Track1Bitrate;
	Track1Bitrate.push_back(std::make_pair("name", "Track1Bitrate"));
	Track1Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track1Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));
	Track1Bitrate.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	for (auto& entry : bitrateMap)
		Track1Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track1Bitrate);

	std::vector<std::pair<std::string, std::string>> Track1Name;
	Track1Name.push_back(std::make_pair("name", "Track1Name"));
	Track1Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track1Name.push_back(std::make_pair("description", "Name"));
	Track1Name.push_back(std::make_pair("subType", ""));
	entries.push_back(Track1Name);

	outputSettings->push_back(
	    serializeSettingsData("Audio - Track 1", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 2
	std::vector<std::pair<std::string, std::string>> Track2Bitrate;
	Track2Bitrate.push_back(std::make_pair("name", "Track2Bitrate"));
	Track2Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track2Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));
	Track2Bitrate.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	for (auto& entry : bitrateMap)
		Track2Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track2Bitrate);

	std::vector<std::pair<std::string, std::string>> Track2Name;
	Track2Name.push_back(std::make_pair("name", "Track2Name"));
	Track2Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track2Name.push_back(std::make_pair("description", "Name"));
	Track2Name.push_back(std::make_pair("subType", ""));
	entries.push_back(Track2Name);

	outputSettings->push_back(
	    serializeSettingsData("Audio - Track 2", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 3
	std::vector<std::pair<std::string, std::string>> Track3Bitrate;
	Track3Bitrate.push_back(std::make_pair("name", "Track3Bitrate"));
	Track3Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track3Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));
	Track3Bitrate.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	for (auto& entry : bitrateMap)
		Track3Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track3Bitrate);

	std::vector<std::pair<std::string, std::string>> Track3Name;
	Track3Name.push_back(std::make_pair("name", "nameTrack3"));
	Track3Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track3Name.push_back(std::make_pair("description", "Name"));
	Track3Name.push_back(std::make_pair("subType", ""));
	entries.push_back(Track3Name);

	outputSettings->push_back(
	    serializeSettingsData("Audio - Track 3", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 4
	std::vector<std::pair<std::string, std::string>> Track4Bitrate;
	Track4Bitrate.push_back(std::make_pair("name", "Track4Bitrate"));
	Track4Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track4Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));
	Track4Bitrate.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	for (auto& entry : bitrateMap)
		Track4Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track4Bitrate);

	std::vector<std::pair<std::string, std::string>> Track4Name;
	Track4Name.push_back(std::make_pair("name", "nameTrack4"));
	Track4Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track4Name.push_back(std::make_pair("description", "Name"));
	Track4Name.push_back(std::make_pair("subType", ""));
	entries.push_back(Track4Name);

	outputSettings->push_back(
	    serializeSettingsData("Audio - Track 4", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 5
	std::vector<std::pair<std::string, std::string>> Track5Bitrate;
	Track5Bitrate.push_back(std::make_pair("name", "Track5Bitrate"));
	Track5Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track5Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));
	Track4Bitrate.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	for (auto& entry : bitrateMap)
		Track5Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track5Bitrate);

	std::vector<std::pair<std::string, std::string>> Track5Name;
	Track5Name.push_back(std::make_pair("name", "nameTrack5"));
	Track5Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track5Name.push_back(std::make_pair("description", "Name"));
	Track5Name.push_back(std::make_pair("subType", ""));
	entries.push_back(Track5Name);

	outputSettings->push_back(
	    serializeSettingsData("Audio - Track 5", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();

	// Track 6
	std::vector<std::pair<std::string, std::string>> Track6Bitrate;
	Track6Bitrate.push_back(std::make_pair("name", "Track6Bitrate"));
	Track6Bitrate.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	Track6Bitrate.push_back(std::make_pair("description", "Audio Bitrate"));
	Track6Bitrate.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	for (auto& entry : bitrateMap)
		Track6Bitrate.push_back(std::make_pair(std::to_string(entry.first), std::to_string(entry.first)));

	entries.push_back(Track6Bitrate);

	std::vector<std::pair<std::string, std::string>> Track6Name;
	Track6Name.push_back(std::make_pair("name", "Track6Name"));
	Track6Name.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	Track6Name.push_back(std::make_pair("description", "Name"));
	Track6Name.push_back(std::make_pair("subType", ""));
	entries.push_back(Track6Name);

	outputSettings->push_back(
	    serializeSettingsData("Audio - Track 6", entries, config, "AdvOut", true, isCategoryEnabled));
	entries.clear();
}

void OBS_settings::getAdvancedOutputSettings(
    std::vector<SubCategory>* outputSettings,
    config_t*                 config,
    bool                      isCategoryEnabled)
{
	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	// Streaming
	outputSettings->push_back(getAdvancedOutputStreamingSettings(config, isCategoryEnabled));

	// Recording
	outputSettings->push_back(getAdvancedOutputRecordingSettings(config, isCategoryEnabled));

	// Audio
	getAdvancedOutputAudioSettings(outputSettings, config, isCategoryEnabled);
}

std::vector<SubCategory> OBS_settings::getOutputSettings()
{
	std::vector<SubCategory> outputSettings;

	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive();

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	//Output mode
	std::vector<std::pair<std::string, std::string>> outputMode;

	outputMode.push_back(std::make_pair("name", "Mode"));
	outputMode.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	outputMode.push_back(std::make_pair("description", "Output Mode"));
	outputMode.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
	outputMode.push_back(std::make_pair("Simple", "Simple"));
	outputMode.push_back(std::make_pair("Advanced", "Advanced"));
	entries.push_back(outputMode);

	outputSettings.push_back(serializeSettingsData(
	    "Untitled", entries, ConfigManager::getInstance().getBasic(), "Output", true, isCategoryEnabled));
	entries.clear();

	const char* currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");

	if (currentOutputMode == NULL) {
		currentOutputMode = "Simple";
	}

	if (strcmp(currentOutputMode, "Advanced") == 0) {
		getAdvancedOutputSettings(&outputSettings, ConfigManager::getInstance().getBasic(), isCategoryEnabled);
	} else {
		getSimpleOutputSettings(&outputSettings, ConfigManager::getInstance().getBasic(), isCategoryEnabled);
	}

	return outputSettings;
}

void OBS_settings::saveSimpleOutputSettings(std::vector<SubCategory> settings)
{
	saveGenericSettings(settings, "SimpleOutput", ConfigManager::getInstance().getBasic());
}

void OBS_settings::saveAdvancedOutputStreamingSettings(std::vector<SubCategory> settings)
{
	int indexStreamingCategory = 1;

	std::string section = "AdvOut";

	obs_encoder_t* encoder         = OBS_service::getStreamingEncoder();
	obs_data_t*    encoderSettings = obs_encoder_get_settings(encoder);

	int indexEncoderSettings = 4;

	bool newEncoderType = false;

	Parameter param;

	for (int i = 0; i < settings.at(indexStreamingCategory).params.size(); i++) {
		param = settings.at(indexStreamingCategory).params.at(i);

		std::string name = param.name;
		std::string type = param.type;

		if (type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || type.compare("OBS_PROPERTY_PATH") == 0
		    || type.compare("OBS_PROPERTY_TEXT") == 0) {
			std::string value(param.currentValue.data(), param.currentValue.size());

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
		} else if (type.compare("OBS_PROPERTY_INT") == 0) {
			int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_UINT") == 0) {
			uint64_t* value = reinterpret_cast<uint64_t*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				config_set_uint(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
			uint32_t* value = reinterpret_cast<uint32_t*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				if (name.compare("Rescale") == 0 && *value) {
					indexEncoderSettings++;
				}
				config_set_bool(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_bool(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0 || type.compare("OBS_PROPERTY_FLOAT") == 0) {
			double* value = reinterpret_cast<double*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_double(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_LIST") == 0) {
			std::string subType(param.subType.data(), param.subType.size());

			if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
				int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
				if (i < indexEncoderSettings) {
					config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
				} else {
					obs_data_set_int(encoderSettings, name.c_str(), *value);
				}
			} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
				double* value = reinterpret_cast<double*>(param.currentValue.data());
				if (i < indexEncoderSettings) {
					config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
				} else {
					obs_data_set_double(encoderSettings, name.c_str(), *value);
				}
			} else {
				std::string value(param.currentValue.data(), param.currentValue.size());
				if (i < indexEncoderSettings) {
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

void OBS_settings::saveAdvancedOutputRecordingSettings(std::vector<SubCategory> settings)
{
	int         indexRecordingCategory = 2;
	std::string section                = "AdvOut";

	obs_encoder_t* encoder         = OBS_service::getRecordingEncoder();
	obs_data_t*    encoderSettings = obs_encoder_get_settings(encoder);

	size_t indexEncoderSettings = 8;

	bool newEncoderType = false;

	Parameter param;

	for (int i = 0; i < settings.at(indexRecordingCategory).params.size(); i++) {
		param = settings.at(indexRecordingCategory).params.at(i);

		std::string name = param.name;
		std::string type = param.type;

		if (i >= indexEncoderSettings) {
			name.erase(0, strlen("Rec"));
		}

		if (name.compare("RecType") == 0) {
			std::string value(param.currentValue.data(), param.currentValue.size());
			if (value.compare("Custom Output (FFmpeg)") == 0) {
				indexEncoderSettings = settings.at(indexRecordingCategory).params.size();
			}
		}

		if (type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || type.compare("OBS_PROPERTY_PATH") == 0
		    || type.compare("OBS_PROPERTY_TEXT") == 0) {
			if (i < indexEncoderSettings) {
				std::string value(param.currentValue.data(), param.currentValue.size());

				if (name.compare("RecEncoder") == 0) {
					const char* currentEncoder =
					    config_get_string(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str());

					if (currentEncoder != NULL)
						newEncoderType = value.compare(currentEncoder) != 0;
				}

				config_set_string(
				    ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), value.c_str());
			} else {
				std::string value(param.currentValue.data(), param.currentValue.size());
				obs_data_set_string(encoderSettings, name.c_str(), value.c_str());
			}
		} else if (type.compare("OBS_PROPERTY_INT") == 0) {
			int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_UINT") == 0) {
			uint64_t* value = reinterpret_cast<uint64_t*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				config_set_uint(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_int(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
			uint32_t* value = reinterpret_cast<uint32_t*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				if (name.compare("RecRescale") == 0 && *value) {
					indexEncoderSettings++;
				}
				config_set_bool(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_bool(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0 || type.compare("OBS_PROPERTY_FLOAT") == 0) {
			double* value = reinterpret_cast<double*>(param.currentValue.data());
			if (i < indexEncoderSettings) {
				config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
			} else {
				obs_data_set_double(encoderSettings, name.c_str(), *value);
			}
		} else if (type.compare("OBS_PROPERTY_LIST") == 0) {
			std::string subType(param.subType.data(), param.subType.size());

			if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
				int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
				if (i < indexEncoderSettings) {
					config_set_int(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
				} else {
					obs_data_set_int(encoderSettings, name.c_str(), *value);
				}
			} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
				double* value = reinterpret_cast<double*>(param.currentValue.data());
				if (i < indexEncoderSettings) {
					config_set_double(ConfigManager::getInstance().getBasic(), section.c_str(), name.c_str(), *value);
				} else {
					obs_data_set_double(encoderSettings, name.c_str(), *value);
				}
			} else {
				std::string value(param.currentValue.data(), param.currentValue.size());
				if (i < indexEncoderSettings) {
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

void OBS_settings::saveAdvancedOutputSettings(std::vector<SubCategory> settings)
{
	// Streaming
	if (!obs_output_active(OBS_service::getStreamingOutput()))
		saveAdvancedOutputStreamingSettings(settings);

	// Recording
	if (!obs_output_active(OBS_service::getRecordingOutput()))
		saveAdvancedOutputRecordingSettings(settings);

	// Audio
	if (settings.size() > 3) {
		std::vector<SubCategory> audioSettings;
		int                      indexTrack = 3;
		audioSettings.push_back(settings.at(indexTrack));

		for (int i = 0; i < 6; i++) {
			audioSettings.push_back(settings.at(i + 2));
		}
		saveGenericSettings(audioSettings, "AdvOut", ConfigManager::getInstance().getBasic());
	}
}

bool useAdvancedOutput;

void OBS_settings::saveOutputSettings(std::vector<SubCategory> settings)
{
	// Get selected output mode
	Parameter   outputMode = settings.at(0).params.at(0);
	std::string currentOutputMode(outputMode.currentValue.data(), outputMode.currentValue.size());

	config_set_string(ConfigManager::getInstance().getBasic(), "Output", "Mode", currentOutputMode.c_str());
	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);

	if (currentOutputMode.compare("Advanced") == 0) {
		if (useAdvancedOutput) {
			saveAdvancedOutputSettings(settings);
		}
		useAdvancedOutput = true;
	} else {
		if (!useAdvancedOutput) {
			saveSimpleOutputSettings(settings);
		}
		useAdvancedOutput = false;
	}
}

std::vector<SubCategory> OBS_settings::getAudioSettings()
{
	std::vector<SubCategory> audioSettings;

	return audioSettings;
}

void OBS_settings::saveAudioSettings(std::vector<SubCategory> audioSettings) {}

std::vector<pair<uint64_t, uint64_t>> OBS_settings::getOutputResolutions(uint64_t base_cx, uint64_t base_cy)
{
	std::vector<pair<uint64_t, uint64_t>> outputResolutions;
	for (size_t idx = 0; idx < numVals; idx++) {
		uint64_t outDownscaleCX = uint64_t(double(base_cx) / vals[idx]);
		uint64_t outDownscaleCY = uint64_t(double(base_cy) / vals[idx]);

		outDownscaleCX &= 0xFFFFFFFE;
		outDownscaleCY &= 0xFFFFFFFE;

		outputResolutions.push_back(std::make_pair(outDownscaleCX, outDownscaleCY));
	}
	return outputResolutions;
}

std::vector<SubCategory> OBS_settings::getVideoSettings()
{
	std::vector<SubCategory> videoSettings;

	bool isCategoryEnabled = !OBS_service::isStreamingOutputActive();

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

	//Base (Canvas) Resolution
	std::vector<std::pair<std::string, std::string>> baseResolution;
	baseResolution.push_back(std::make_pair("name", "Base"));
	baseResolution.push_back(std::make_pair("type", "OBS_INPUT_RESOLUTION_LIST"));
	baseResolution.push_back(std::make_pair("description", "Base (Canvas) Resolution"));
	baseResolution.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	uint64_t base_cx = config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCX");
	uint64_t base_cy = config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCY");

	std::string baseResolutionString = ResString(base_cx, base_cy);

	baseResolution.push_back(std::make_pair("1920x1080", "1920x1080"));
	baseResolution.push_back(std::make_pair("1280x720", "1280x720"));

	std::vector<Screen> resolutions = OBS_API::availableResolutions();

	// Fill available display resolutions
	for (int i = 0; i < resolutions.size(); i++) {
		std::string baseResolutionString;
		baseResolutionString = to_string(resolutions.at(i).width);
		baseResolutionString += "x";
		baseResolutionString += to_string(resolutions.at(i).height);

		pair<std::string, std::string> newBaseResolution =
		    std::make_pair(baseResolutionString.c_str(), baseResolutionString.c_str());

		std::vector<pair<std::string, std::string>>::iterator it = std::find_if(
		    baseResolution.begin(),
		    baseResolution.end(),
		    [&baseResolutionString](const pair<std::string, std::string> value) {
			    return (value.second.compare(baseResolutionString) == 0);
		    });

		if (baseResolution.size() == 4 || it == baseResolution.end()) {
			baseResolution.push_back(newBaseResolution);
		}
	}

	// Set the current base resolution selected by the user
	pair<std::string, std::string> newBaseResolution = std::make_pair("currentValue", baseResolutionString);

	//Check if the current resolution is in the available ones
	std::vector<pair<std::string, std::string>>::iterator it = std::find_if(
	    baseResolution.begin(),
	    baseResolution.end(),
	    [&baseResolutionString](const pair<std::string, std::string> value) {
		    return (value.second.compare(baseResolutionString) == 0);
	    });

	if (it == baseResolution.end()) {
		baseResolution.push_back(std::make_pair(baseResolutionString.c_str(), baseResolutionString.c_str()));
	}

	int indexFirstValue = 4;
	baseResolution.insert(baseResolution.begin() + indexFirstValue, newBaseResolution);

	entries.push_back(baseResolution);

	std::vector<std::pair<std::string, std::string>> outputResolution;
	outputResolution.push_back(std::make_pair("name", "Output"));
	outputResolution.push_back(std::make_pair("type", "OBS_INPUT_RESOLUTION_LIST"));
	outputResolution.push_back(std::make_pair("description", "Output (Scaled) Resolution"));
	outputResolution.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	uint64_t out_cx = config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCX");
	uint64_t out_cy = config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCY");

	std::string outputResString = ResString(out_cx, out_cy);

	outputResolution.push_back(std::make_pair("currentValue", outputResString));

	std::vector<pair<uint64_t, uint64_t>> outputResolutions = getOutputResolutions(base_cx, base_cy);

	for (int i = 0; i < outputResolutions.size(); i++) {
		string outRes = ResString(outputResolutions.at(i).first, outputResolutions.at(i).second);
		outputResolution.push_back(std::make_pair(outRes, outRes));
	}

	entries.push_back(outputResolution);

	//Downscale Filter
	std::vector<std::pair<std::string, std::string>> scaleType;
	scaleType.push_back(std::make_pair("name", "ScaleType"));
	scaleType.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	scaleType.push_back(std::make_pair("description", "Downscale Filter"));
	scaleType.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
	scaleType.push_back(std::make_pair("Bilinear (Fastest, but blurry if scaling)", "bilinear"));
	scaleType.push_back(std::make_pair("Bicubic (Sharpened scaling, 16 samples)", "bicubic"));
	scaleType.push_back(std::make_pair("Lanczos (Sharpened scaling, 32 samples)", "lanczos"));
	entries.push_back(scaleType);

	//FPS Type
	std::vector<std::pair<std::string, std::string>> fpsType;
	fpsType.push_back(std::make_pair("name", "FPSType"));
	fpsType.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	fpsType.push_back(std::make_pair("description", "FPS Type"));
	fpsType.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	uint64_t fpsTypeValue = config_get_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType");

	if (fpsTypeValue == 0) {
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
		fpsCommon.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
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
		fpsInt.push_back(std::make_pair("subType", ""));
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
		fpsNum.push_back(std::make_pair("subType", ""));
		entries.push_back(fpsNum);

		std::vector<std::pair<std::string, std::string>> fpsDen;
		fpsDen.push_back(std::make_pair("name", "FPSDen"));
		fpsDen.push_back(std::make_pair("type", "OBS_PROPERTY_UINT"));
		fpsDen.push_back(std::make_pair("description", "FPSDen"));
		fpsDen.push_back(std::make_pair("subType", ""));
		entries.push_back(fpsDen);
	}

	videoSettings.push_back(serializeSettingsData(
	    "Untitled", entries, ConfigManager::getInstance().getBasic(), "Video", true, isCategoryEnabled));
	entries.clear();

	return videoSettings;
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

	cx = std::stoul(token.text.array);

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

	cy = std::stoul(token.text.array);

	// shouldn't be any more tokens after this
	if (lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;

	return true;
}

void OBS_settings::saveVideoSettings(std::vector<SubCategory> videoSettings)
{
	SubCategory sc = videoSettings.at(0);

	//Base resolution
	Parameter baseRes = sc.params.at(0);

	std::string baseResString(baseRes.currentValue.data(), baseRes.currentValue.size());

	uint32_t baseWidth, baseHeight;

	ConvertResText(baseResString.c_str(), baseWidth, baseHeight);
	config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCX", baseWidth);
	config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "BaseCY", baseHeight);

	//Output resolution
	Parameter outputRes = sc.params.at(1);

	std::string outputResString(outputRes.currentValue.data(), outputRes.currentValue.size());

	uint32_t outputWidth, outputHeight;

	ConvertResText(outputResString.c_str(), outputWidth, outputHeight);
	config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCX", outputWidth);
	config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "OutputCY", outputHeight);

	Parameter scaleParameter = sc.params.at(2);

	std::string scaleString(scaleParameter.currentValue.data(), scaleParameter.currentValue.size());

	config_set_string(ConfigManager::getInstance().getBasic(), "Video", "ScaleType", scaleString.c_str());

	Parameter fpsType = sc.params.at(3);

	std::string fpsTypeString(fpsType.currentValue.data(), fpsType.currentValue.size());

	if (fpsTypeString.compare("Common FPS Values") == 0) {
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType", 0);

		Parameter   fpsCommon = sc.params.at(4);
		std::string fpsCommonString(fpsCommon.currentValue.data(), fpsCommon.currentValue.size());
		config_set_string(ConfigManager::getInstance().getBasic(), "Video", "FPSCommon", fpsCommonString.c_str());

	} else if (fpsTypeString.compare("Integer FPS Value") == 0) {
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType", 1);

		Parameter fpsInt = sc.params.at(4);

		uint64_t* fpsIntValue = reinterpret_cast<uint64_t*>(fpsInt.currentValue.data());

		if (*fpsIntValue < 500) {
			config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSInt", *fpsIntValue);
		}

	} else if (fpsTypeString.compare("Fractional FPS Value") == 0) {
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType", 2);

		Parameter fpsNum      = sc.params.at(4);
		uint32_t* fpsNumValue = reinterpret_cast<uint32_t*>(fpsNum.currentValue.data());

		if (*fpsNumValue < 500) {
			config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSNum", *fpsNumValue);
		}

		if (sc.params.size() > 5) {
			Parameter fpsDen      = sc.params.at(5);
			uint32_t* fpsDenValue = reinterpret_cast<uint32_t*>(fpsDen.currentValue.data());
			config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSDen", *fpsDenValue);
		}
	}

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
}

std::vector<SubCategory> OBS_settings::getAdvancedSettings()
{
	std::vector<SubCategory> advancedSettings;

	std::vector<std::vector<std::pair<std::string, std::string>>> entries;

#if _WIN32
	//General

	//Process Priority
	std::vector<std::pair<std::string, std::string>> processPriority;
	processPriority.push_back(std::make_pair("name", "ProcessPriority"));
	processPriority.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	processPriority.push_back(std::make_pair("description", "Process Priority"));
	processPriority.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
	processPriority.push_back(std::make_pair("High", "High"));
	processPriority.push_back(std::make_pair("Above Normal", "AboveNormal"));
	processPriority.push_back(std::make_pair("Normal", "Normal"));
	processPriority.push_back(std::make_pair("Below Normal", "BelowNormal"));
	processPriority.push_back(std::make_pair("Idle", "Idle"));

	const char* processPriorityCurrentValue =
	    config_get_string(ConfigManager::getInstance().getGlobal(), "General", "ProcessPriority");

	if (processPriorityCurrentValue == NULL) {
		processPriorityCurrentValue = "Normal";
		config_set_string(
		    ConfigManager::getInstance().getGlobal(), "General", "ProcessPriority", processPriorityCurrentValue);
	}

	OBS_API::SetProcessPriority(processPriorityCurrentValue);

	entries.push_back(processPriority);

	advancedSettings.push_back(
	    serializeSettingsData("General", entries, ConfigManager::getInstance().getGlobal(), "General", true, true));
	entries.clear();

	// bool enableNewSocketLoop = config_get_bool(config, "Output", "NewSocketLoopEnable");
	// bool enableLowLatencyMode = config_get_bool(config, "Output", "LowLatencyEnable");
#endif
	//Video
	const char* videoColorFormat = config_get_string(ConfigManager::getInstance().getBasic(), "Video", "ColorFormat");
	const char* videoColorSpace  = config_get_string(ConfigManager::getInstance().getBasic(), "Video", "ColorSpace");
	const char* videoColorRange  = config_get_string(ConfigManager::getInstance().getBasic(), "Video", "ColorRange");

	//Color Format
	std::vector<std::pair<std::string, std::string>> colorFormat;
	colorFormat.push_back(std::make_pair("name", "ColorFormat"));
	colorFormat.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	colorFormat.push_back(std::make_pair("description", "Color Format"));
	colorFormat.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
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
	colorSpace.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
	colorSpace.push_back(std::make_pair("601", "601"));
	colorSpace.push_back(std::make_pair("709", "709"));
	entries.push_back(colorSpace);

	//YUV Color Range
	std::vector<std::pair<std::string, std::string>> colorRange;
	colorRange.push_back(std::make_pair("name", "ColorRange"));
	colorRange.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	colorRange.push_back(std::make_pair("description", "YUV Color Range"));
	colorRange.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
	colorRange.push_back(std::make_pair("Partial", "Partial"));
	colorRange.push_back(std::make_pair("Full", "Full"));
	entries.push_back(colorRange);

	advancedSettings.push_back(
	    serializeSettingsData("Video", entries, ConfigManager::getInstance().getBasic(), "Video", true, true));
	entries.clear();

#if defined(_WIN32) || defined(__APPLE__)
	//Audio
	const char* monDevName =
	    config_get_string(ConfigManager::getInstance().getBasic(), "Audio", "MonitoringDeviceName");
	const char* monDevId = config_get_string(ConfigManager::getInstance().getBasic(), "Audio", "MonitoringDeviceId");

	//Audio Monitoring Device
	std::vector<std::pair<std::string, std::string>>* monitoringDevice =
	    new std::vector<std::pair<std::string, std::string>>();

	monitoringDevice->push_back(std::make_pair("name", "MonitoringDeviceName"));
	monitoringDevice->push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	monitoringDevice->push_back(std::make_pair("description", "Audio Monitoring Device"));
	monitoringDevice->push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));
	monitoringDevice->push_back(std::make_pair("currentValue", monDevName));
	monitoringDevice->push_back(std::make_pair("Default", "Default"));

	auto enum_devices = [](void* param, const char* name, const char* id) {
		std::vector<std::pair<std::string, std::string>>* monitoringDevice =
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
	disableAudioDucking.push_back(std::make_pair("subType", ""));
	entries.push_back(disableAudioDucking);

	advancedSettings.push_back(
	    serializeSettingsData("Audio", entries, ConfigManager::getInstance().getBasic(), "Audio", true, true));
	entries.clear();
#endif

	//Recording

	//Filename Formatting
	std::vector<std::pair<std::string, std::string>> filenameFormatting;
	filenameFormatting.push_back(std::make_pair("name", "FilenameFormatting"));
	filenameFormatting.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	filenameFormatting.push_back(std::make_pair("description", "Filename Formatting"));
	filenameFormatting.push_back(std::make_pair("subType", ""));
	entries.push_back(filenameFormatting);

	//Overwrite if file exists
	std::vector<std::pair<std::string, std::string>> overwriteIfExists;
	overwriteIfExists.push_back(std::make_pair("name", "OverwriteIfExists"));
	overwriteIfExists.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	overwriteIfExists.push_back(std::make_pair("description", "Overwrite if file exists"));
	overwriteIfExists.push_back(std::make_pair("subType", ""));
	entries.push_back(overwriteIfExists);

	//Replay Buffer Filename Prefix
	std::vector<std::pair<std::string, std::string>> recRBPrefix;
	recRBPrefix.push_back(std::make_pair("name", "RecRBPrefix"));
	recRBPrefix.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	recRBPrefix.push_back(std::make_pair("description", "Replay Buffer Filename Prefix"));
	recRBPrefix.push_back(std::make_pair("subType", ""));
	entries.push_back(recRBPrefix);

	//Replay Buffer Filename Suffix
	std::vector<std::pair<std::string, std::string>> recRBSuffix;
	recRBSuffix.push_back(std::make_pair("name", "RecRBSuffix"));
	recRBSuffix.push_back(std::make_pair("type", "OBS_PROPERTY_EDIT_TEXT"));
	recRBSuffix.push_back(std::make_pair("description", "Replay Buffer Filename Suffix"));
	recRBSuffix.push_back(std::make_pair("subType", ""));
	entries.push_back(recRBSuffix);

	advancedSettings.push_back(serializeSettingsData(
	    "Recording", entries, ConfigManager::getInstance().getBasic(), "SimpleOutput", true, true));
	entries.clear();

	//Stream Delay

	//Enable
	std::vector<std::pair<std::string, std::string>> delayEnable;
	delayEnable.push_back(std::make_pair("name", "DelayEnable"));
	delayEnable.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	delayEnable.push_back(std::make_pair("description", "Enable"));
	delayEnable.push_back(std::make_pair("subType", ""));
	entries.push_back(delayEnable);

	//Duration (seconds)
	std::vector<std::pair<std::string, std::string>> delaySec;
	delaySec.push_back(std::make_pair("name", "DelaySec"));
	delaySec.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	delaySec.push_back(std::make_pair("description", "Duration (seconds)"));
	delaySec.push_back(std::make_pair("subType", ""));
	entries.push_back(delaySec);

	//Preserved cutoff point (increase delay) when reconnecting
	std::vector<std::pair<std::string, std::string>> delayPreserve;
	delayPreserve.push_back(std::make_pair("name", "DelayPreserve"));
	delayPreserve.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	delayPreserve.push_back(std::make_pair("description", "Preserved cutoff point (increase delay) when reconnecting"));
	delayPreserve.push_back(std::make_pair("subType", ""));
	entries.push_back(delayPreserve);

	advancedSettings.push_back(
	    serializeSettingsData("Stream Delay", entries, ConfigManager::getInstance().getBasic(), "Output", true, true));
	entries.clear();

	//Automatically Reconnect

	//Enable
	std::vector<std::pair<std::string, std::string>> reconnect;
	reconnect.push_back(std::make_pair("name", "Reconnect"));
	reconnect.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	reconnect.push_back(std::make_pair("description", "Enable"));
	reconnect.push_back(std::make_pair("subType", ""));
	entries.push_back(reconnect);

	//Retry Delay (seconds)
	std::vector<std::pair<std::string, std::string>> retryDelay;
	retryDelay.push_back(std::make_pair("name", "RetryDelay"));
	retryDelay.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	retryDelay.push_back(std::make_pair("description", "Retry Delay (seconds)"));
	retryDelay.push_back(std::make_pair("subType", ""));
	entries.push_back(retryDelay);

	//Maximum Retries
	std::vector<std::pair<std::string, std::string>> maxRetries;
	maxRetries.push_back(std::make_pair("name", "MaxRetries"));
	maxRetries.push_back(std::make_pair("type", "OBS_PROPERTY_INT"));
	maxRetries.push_back(std::make_pair("description", "Maximum Retries"));
	maxRetries.push_back(std::make_pair("subType", ""));
	entries.push_back(maxRetries);

	advancedSettings.push_back(serializeSettingsData(
	    "Automatically Reconnect", entries, ConfigManager::getInstance().getBasic(), "Output", true, true));
	entries.clear();

	//Network

	//Bind to IP
	std::vector<std::pair<std::string, std::string>> bindIP;
	bindIP.push_back(std::make_pair("name", "BindIP"));
	bindIP.push_back(std::make_pair("type", "OBS_PROPERTY_LIST"));
	bindIP.push_back(std::make_pair("description", "Bind to IP"));
	bindIP.push_back(std::make_pair("subType", "OBS_COMBO_FORMAT_STRING"));

	obs_properties_t* ppts = obs_get_output_properties("rtmp_output");
	obs_property_t*   p    = obs_properties_get(ppts, "bind_ip");

	size_t count = obs_property_list_item_count(p);
	for (size_t i = 0; i < count; i++) {
		const char* name = obs_property_list_item_name(p, i);
		const char* val  = obs_property_list_item_string(p, i);

		bindIP.push_back(std::make_pair(name, name));
	}

	entries.push_back(bindIP);

	//Enable new networking code
	std::vector<std::pair<std::string, std::string>> newSocketLoopEnable;
	newSocketLoopEnable.push_back(std::make_pair("name", "NewSocketLoopEnable"));
	newSocketLoopEnable.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	newSocketLoopEnable.push_back(std::make_pair("description", "Enable new networking code"));
	newSocketLoopEnable.push_back(std::make_pair("subType", ""));
	entries.push_back(newSocketLoopEnable);

	//Low latency mode
	std::vector<std::pair<std::string, std::string>> lowLatencyEnable;
	lowLatencyEnable.push_back(std::make_pair("name", "LowLatencyEnable"));
	lowLatencyEnable.push_back(std::make_pair("type", "OBS_PROPERTY_BOOL"));
	lowLatencyEnable.push_back(std::make_pair("description", "Low latency mode"));
	lowLatencyEnable.push_back(std::make_pair("subType", ""));
	entries.push_back(lowLatencyEnable);

	advancedSettings.push_back(
	    serializeSettingsData("Network", entries, ConfigManager::getInstance().getBasic(), "Output", true, true));
	entries.clear();

	return advancedSettings;
}

void OBS_settings::saveAdvancedSettings(std::vector<SubCategory> advancedSettings)
{
	//General
	std::vector<SubCategory> generalAdvancedSettings;

	generalAdvancedSettings.push_back(advancedSettings.at(0));
	saveGenericSettings(generalAdvancedSettings, "General", ConfigManager::getInstance().getGlobal());

	//Video
	std::vector<SubCategory> videoAdvancedSettings;

	videoAdvancedSettings.push_back(advancedSettings.at(1));
	saveGenericSettings(videoAdvancedSettings, "Video", ConfigManager::getInstance().getBasic());

	//Audio
	std::vector<SubCategory> audioAdvancedSettings;

	audioAdvancedSettings.push_back(advancedSettings.at(2));
	saveGenericSettings(audioAdvancedSettings, "Audio", ConfigManager::getInstance().getBasic());

	//Recording
	std::vector<SubCategory> recordingAdvancedSettings;

	recordingAdvancedSettings.push_back(advancedSettings.at(3));
	saveGenericSettings(recordingAdvancedSettings, "SimpleOutput", ConfigManager::getInstance().getBasic());

	//Stream Delay
	std::vector<SubCategory> stresmDelayAdvancedSettings;

	stresmDelayAdvancedSettings.push_back(advancedSettings.at(4));
	saveGenericSettings(stresmDelayAdvancedSettings, "Output", ConfigManager::getInstance().getBasic());

	//Automatically Reconnect
	std::vector<SubCategory> automaticallyReconnectAdvancedSettings;

	automaticallyReconnectAdvancedSettings.push_back(advancedSettings.at(5));
	saveGenericSettings(automaticallyReconnectAdvancedSettings, "Output", ConfigManager::getInstance().getBasic());

	//Network
	std::vector<SubCategory> networkAdvancedSettings;

	networkAdvancedSettings.push_back(advancedSettings.at(6));
	saveGenericSettings(networkAdvancedSettings, "Output", ConfigManager::getInstance().getBasic());
}

std::vector<std::string> OBS_settings::getListCategories(void)
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

std::vector<SubCategory> OBS_settings::getSettings(std::string nameCategory)
{
	std::vector<SubCategory> settings;

	if (nameCategory.compare("General") == 0) {
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

void OBS_settings::saveSettings(std::string nameCategory, std::vector<SubCategory> settings)
{
	if (nameCategory.compare("General") == 0) {
		saveGenericSettings(settings, "BasicWindow", ConfigManager::getInstance().getGlobal());
	} else if (nameCategory.compare("Stream") == 0) {
		saveStreamSettings(settings);
		OBS_service::updateService();
	} else if (nameCategory.compare("Output") == 0) {
		saveOutputSettings(settings);
	} else if (nameCategory.compare("Audio") == 0) {
		saveAudioSettings(settings);
	} else if (nameCategory.compare("Video") == 0) {
		saveVideoSettings(settings);
		OBS_service::resetVideoContext();
	} else if (nameCategory.compare("Advanced") == 0) {
		saveAdvancedSettings(settings);
		OBS_API::setAudioDeviceMonitoring();
	}
}

void OBS_settings::saveGenericSettings(std::vector<SubCategory> genericSettings, std::string section, config_t* config)
{
	SubCategory sc;

	for (int i = 0; i < genericSettings.size(); i++) {
		sc = genericSettings.at(i);

		std::string nameSubcategory = sc.name;

		Parameter param;

		for (int j = 0; j < sc.params.size(); j++) {
			param = sc.params.at(j);

			std::string name    = param.name;
			std::string type    = param.type;
			std::string subType = param.subType;

			if (type.compare("OBS_PROPERTY_EDIT_TEXT") == 0 || type.compare("OBS_PROPERTY_PATH") == 0
			    || type.compare("OBS_PROPERTY_TEXT") == 0 || type.compare("OBS_INPUT_RESOLUTION_LIST") == 0) {
				std::string value(param.currentValue.data(), param.currentValue.size());
				config_set_string(config, section.c_str(), name.c_str(), value.c_str());
			} else if (type.compare("OBS_PROPERTY_INT") == 0) {
				int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
				config_set_int(config, section.c_str(), name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_UINT") == 0) {
				uint64_t* value = reinterpret_cast<uint64_t*>(param.currentValue.data());
				config_set_uint(config, section.c_str(), name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_BOOL") == 0) {
				uint32_t* value = reinterpret_cast<uint32_t*>(param.currentValue.data());
				config_set_bool(config, section.c_str(), name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_DOUBLE") == 0) {
				double* value = reinterpret_cast<double*>(param.currentValue.data());
				config_set_double(config, section.c_str(), name.c_str(), *value);
			} else if (type.compare("OBS_PROPERTY_LIST") == 0) {
				if (subType.compare("OBS_COMBO_FORMAT_INT") == 0) {
					int64_t* value = reinterpret_cast<int64_t*>(param.currentValue.data());
					config_set_int(config, section.c_str(), name.c_str(), *value);
				} else if (subType.compare("OBS_COMBO_FORMAT_FLOAT") == 0) {
					double* value = reinterpret_cast<double*>(param.currentValue.data());
					config_set_double(config, section.c_str(), name.c_str(), *value);
				} else if (subType.compare("OBS_COMBO_FORMAT_STRING") == 0) {
					std::string value(param.currentValue.data(), param.currentValue.size());

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

							std::vector<pair<std::string, std::string>>::iterator it = std::find_if(
							    monitoringDevice.begin(),
							    monitoringDevice.end(),
							    [&value](const pair<std::string, std::string> device) {
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
