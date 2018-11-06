#pragma once
#include <obs.h>
#include <string>
#include <util/config-file.h>

class ConfigManager {
public:
	static ConfigManager& getInstance()
	{
		static ConfigManager instance;
		return instance;
	}
private:
	ConfigManager() {};
public:
	ConfigManager(ConfigManager const&) = delete;
	void operator=(ConfigManager const&) = delete;
private:
	config_t* global = NULL;
	config_t* basic = NULL;
	std::string service = "";
	std::string stream = "";
	std::string record = "";
	std::string appdata;

	config_t * getConfig(std::string name);
public:
	void setAppdataPath(std::string path);
	config_t* getGlobal();
	config_t* getBasic();
	std::string getService();
	std::string getStream();
	std::string getRecord();
};