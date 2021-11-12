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

#include "osn-transition.hpp"
#include <memory>
#include <obs.h>
#include "osn-source.hpp"
#include "shared-server.hpp"

std::vector<std::string> obs::Transition::Types()
{
	std::vector<std::string> types;
	const char* typeId = nullptr;
	for (size_t idx = 0; obs_enum_transition_types(idx, &typeId); idx++)
		if(typeId)
			types.push_back(std::string(typeId));

	return types;
}

obs_source_t* obs::Transition::Create(
    std::string sourceId, std::string name,
	std::string settingsData)
{
	obs_data_t *settings = nullptr;
	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());

	obs_source_t* source = obs_source_create(sourceId.c_str(), name.c_str(), settings, nullptr);
	if (!source) {
		blog(LOG_ERROR, "Failed to create transition.");
		return nullptr;
	}

	obs_data_release(settings);

	obs::Source::attach_source_signals(source);

	return source;
}

obs_source_t* obs::Transition::CreatePrivate(
    std::string sourceId, std::string name,
	std::string settingsData)
{
	obs_data_t* settings = nullptr;
	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());


	obs_source_t* source = obs_source_create_private(sourceId.c_str(), name.c_str(), settings);
	if (!source) {
		blog(LOG_ERROR, "Failed to create transition.");
		return nullptr;
	}

	obs_data_release(settings);

	obs::Source::attach_source_signals(source);

	return source;
}

obs_source_t* obs::Transition::FromName(std::string name)
{
	obs_source_t* source = obs_get_source_by_name(name.c_str());
	if (!source) {
		blog(LOG_ERROR, "Named transition could not be found.");
		return nullptr;
	}

	obs_source_release(source);

	return source;
}

std::pair<obs_source_t*, uint32_t> obs::Transition::GetActiveSource(obs_source_t* transition)
{
	uint64_t uid = -1;

	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return std::make_pair(nullptr, UINT32_MAX);
	}

	obs_source_type type   = OBS_SOURCE_TYPE_INPUT;
	obs_source_t*   source = obs_transition_get_active_source(transition);
	if (source) {
		uid  = obs::Source::Manager::GetInstance().find(source);
		type = obs_source_get_type(source);
		obs_source_release(source);
	}

	return std::make_pair(source, type);
}

void obs::Transition::Clear(obs_source_t* transition)
{
	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return;
	}

	obs_transition_clear(transition);
}

void obs::Transition::Set(obs_source_t* transition, obs_source_t* source)
{
	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return;
	}

	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_transition_set(transition, source);
}

bool obs::Transition::Start(
	obs_source_t* transition, uint32_t ms, obs_source_t* source)
{
	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return false;
	}

	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return false;
	}

	return obs_transition_start(transition, OBS_TRANSITION_MODE_AUTO, ms, source);
}
