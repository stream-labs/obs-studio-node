#include "nodeobs_configManager.hpp"

void ConfigManager::setAppdataPath(std::string path) {
	appdata = path;
}

config_t *ConfigManager::getConfig(std::string name) {
	config_t* config;
	std::string file = appdata + name;

	int result = config_open(&config, file.c_str(), CONFIG_OPEN_EXISTING);

	if (result != CONFIG_SUCCESS) {
		config = config_create(file.c_str());
		config_open(&config, file.c_str(), CONFIG_OPEN_EXISTING);
	}

	return config;
};

config_t *ConfigManager::getGlobal() {
	if (!global)
		global = getConfig("\\global.ini");

	return global;
};
config_t *ConfigManager::getBasic() {
	if (!basic)
		basic = getConfig("\\basic.ini");

	return basic;
};
std::string ConfigManager::getService() {
	return appdata + "\\service.json";
};
std::string ConfigManager::getStream() {
	return appdata + "\\streamEncoder.json";
};
std::string ConfigManager::getRecord() {
	return appdata + "\\recordEncoder.json";
};