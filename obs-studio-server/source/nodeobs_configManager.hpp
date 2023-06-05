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

#pragma once
#include <obs.h>
#include <string>
#include <util/config-file.h>

class ConfigManager {
public:
	static ConfigManager &getInstance()
	{
		static ConfigManager instance;
		return instance;
	}

private:
	ConfigManager(){};

public:
	ConfigManager(ConfigManager const &) = delete;
	void operator=(ConfigManager const &) = delete;

private:
	config_t *global = NULL;
	config_t *basic = NULL;
	std::string service = "";
	std::string stream = "";
	std::string record = "";
	std::string appdata = "";

	config_t *getConfig(const std::string &name);

public:
	void setAppdataPath(const std::string &path);
	config_t *getGlobal();
	config_t *getBasic();
	std::string getService(size_t index);
	std::string getStream();
	std::string getRecord();
	void reloadConfig(void);
};