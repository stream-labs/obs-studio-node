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

#include "osn-Properties.hpp"
#include "error.hpp"
#include "osn-source.hpp"
#include "shared-server.hpp"

bool obs::Properties::Modified(obs_source_t* source, std::string name, std::string value)
{
	bool res = false;
	if (!source) {
		blog(LOG_ERROR, "Invalid reference.");
		return res;
	}
	obs_data_t* settings = obs_data_create_from_json(value.c_str());

	obs_properties_t* props = obs_source_properties(source);
	obs_property_t*   prop  = obs_properties_get(props, name.c_str());
	if (!prop) {
		obs_properties_destroy(props);
		obs_data_release(settings);
		blog(LOG_ERROR, "Failed to find property in source.");
		return res;
	} else {
		res = obs_property_modified(prop, settings);
	}
	obs_properties_destroy(props);
	obs_data_release(settings);
	return res;
}

bool obs::Properties::Clicked(obs_source_t* source, std::string name)
{
	bool res = false;
	if (!source) {
		blog(LOG_ERROR, "Invalid reference.");
		return res;
	}

	obs_properties_t* props = obs_source_properties(source);
	obs_property_t*   prop  = obs_properties_get(props, name.c_str());
	if (!prop) {
		obs_properties_destroy(props);
		blog(LOG_ERROR, "Failed to find property in source.");
		return res;
	} else {
		res = obs_property_button_clicked(prop, source);
	}
	obs_properties_destroy(props);
	return res;
}
