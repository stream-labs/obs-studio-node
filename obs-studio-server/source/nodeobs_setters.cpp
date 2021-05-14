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

#include "nodeobs_settings.h"

template<typename T>
void config_set(config_t* config, const char* section, const char* name, T value){};

template<>
void config_set<uint64_t>(config_t* config, const char* section, const char* name, uint64_t value)
{
	config_set_uint64_t(config, section, name, value);
}

template<>
void config_set<int>(config_t* config, const char* section, const char* name, int value)
{
	config_set_int(config, section, name, value);
}

template<>
void config_set<bool>(config_t* config, const char* section, const char* name, bool value)
{
	config_set_bool(config, section, name, value);
}

template<>
void config_set<std::string>(config_t* config, const char* section, const char* name, std::string value)
{
	config_set_string(config, section, name, value.c_str());
}

template<>
void config_set<double>(config_t* config, const char* section, const char* name, double value)
{
	config_set_double(config, section, name, value);
}

template<typename T>
void obs_data_set(obs_data_t* config, const char* name, T value){};

template<>
void obs_data_set<std::string>(obs_data_t* config, const char* name, std::string value)
{
	obs_data_set_string(config, name, value.c_str());
}

template<>
void obs_data_set<int>(obs_data_t* config, const char* name, int value)
{
	obs_data_set_int(config, name, value);
}

template<>
void obs_data_set<bool>(obs_data_t* config, const char* name, bool value)
{
	obs_data_set_bool(config, name, value);
}

template<>
void obs_data_set<double>(obs_data_t* config, const char* name, double value)
{
	obs_data_set_double(config, name, value);
}