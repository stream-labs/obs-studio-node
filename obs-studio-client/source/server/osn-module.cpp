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

#include "osn-module.hpp"
#include "error.hpp"
#include "shared-server.hpp"

uint64_t obs::Module::Open(std::string bin_path, std::string data_path)
{
	obs_module_t*     module;
	int64_t result = obs_open_module(&module, bin_path.c_str(), data_path.c_str());

	if (result == MODULE_SUCCESS) {
		return obs::Module::Manager::GetInstance().allocate(module);
	} else {
		blog(LOG_ERROR, "Failed to create module.");
		return UINT64_MAX;
	}
}

std::vector<std::string> obs::Module::Modules()
{
	std::vector<std::string> modules;

	obs_enum_modules([](void* param, obs_module_t* module) {
		    std::vector<std::string>& modules = *static_cast<std::vector<std::string>*>(param);
		    const char*               name    = obs_get_module_file_name(module);
		    if (name)
				modules.push_back(name);
	},&modules);

	return modules;
}
bool obs::Module::Initialize(uint64_t uid)
{
	obs_module_t* module = obs::Module::Manager::GetInstance().find(uid);

	if (!module) {
		blog(LOG_ERROR, "Module reference is not valid.");
		return false;
	}

	return obs_init_module(module);
}

std::string obs::Module::GetName(uint64_t uid)
{
	obs_module_t* module = obs::Module::Manager::GetInstance().find(uid);

	if (!module) {
		blog(LOG_ERROR, "Module reference is not valid.");
		return "";
	}

	return std::string(obs_get_module_name(module));
}

std::string obs::Module::GetFileName(uint64_t uid)
{
	obs_module_t* module = obs::Module::Manager::GetInstance().find(uid);

	if (!module) {
		blog(LOG_ERROR, "Module reference is not valid.");
		return "";
	}

	return std::string(obs_get_module_file_name(module));
}

std::string obs::Module::GetAuthor(uint64_t uid)
{
	obs_module_t* module = obs::Module::Manager::GetInstance().find(uid);

	if (!module) {
		blog(LOG_ERROR, "Module reference is not valid.");
		return "";
	}

	return std::string(obs_get_module_author(module));
}

std::string obs::Module::GetDescription(uint64_t uid)
{
	obs_module_t* module = obs::Module::Manager::GetInstance().find(uid);

	if (!module) {
		blog(LOG_ERROR, "Module reference is not valid.");
		return "";
	}

	return std::string(obs_get_module_description(module));
}

std::string obs::Module::GetBinaryPath(uint64_t uid)
{
	obs_module_t* module = obs::Module::Manager::GetInstance().find(uid);

	if (!module) {
		blog(LOG_ERROR, "Module reference is not valid.");
		return "";
	}

	return std::string(obs_get_module_binary_path(module));
}

std::string obs::Module::GetDataPath(uint64_t uid)
{
	obs_module_t* module = obs::Module::Manager::GetInstance().find(uid);

	if (!module) {
		blog(LOG_ERROR, "Module reference is not valid.");
		return "";
	}

	return std::string(obs_get_module_data_path(module));
}

obs::Module::Manager& obs::Module::Manager::GetInstance()
{
	static obs::Module::Manager _inst;
	return _inst;
}