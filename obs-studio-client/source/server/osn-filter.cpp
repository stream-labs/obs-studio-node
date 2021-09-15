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

#include "osn-Filter.hpp"
#include <ipc-server.hpp>
#include <memory>
#include <obs.h>
#include "error.hpp"
#include "osn-source.hpp"
#include "shared-server.hpp"

std::vector<std::string> obs::Filter::Types()
{
	std::vector<std::string> types;

	const char* typeId = nullptr;
	for (size_t idx = 0; obs_enum_filter_types(idx, &typeId); idx++) {
		if (typeId)
			types.push_back(std::string(typeId));
	}

	return types;
}

obs_source_t* obs::Filter::Create(std::string sourceId, std::string name, std::string settingsData)
{
	obs_data_t* settings = nullptr;
	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());

	obs_source_t* source = obs_source_create_private(sourceId.c_str(), name.c_str(), settings);
	if (!source) {
		blog(LOG_ERROR, "Failed to create filter.");
	}

	obs_data_release(settings);

	obs::Source::attach_source_signals(source);

	return source;
}
