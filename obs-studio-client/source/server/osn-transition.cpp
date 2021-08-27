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
#include <ipc-server.hpp>
#include <memory>
#include <obs.h>
#include "error.hpp"
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

uint64_t obs::Transition::Create(
    std::string sourceId, std::string name,
	std::string settingsData)
{
	obs_data_t *settings = nullptr;
	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());

	obs_source_t* source = obs_source_create(sourceId.c_str(), name.c_str(), settings, nullptr);
	if (!source) {
		blog(LOG_ERROR, "Failed to create transition.");
		return UINT64_MAX;
	}

	obs_data_release(settings);

	uint64_t uid = obs::Source::Manager::GetInstance().find(source);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Index list is full.");
		return UINT64_MAX;
	}

	return uid;
}

uint64_t obs::Transition::CreatePrivate(
    std::string sourceId, std::string name,
	std::string settingsData)
{
	obs_data_t* settings = nullptr;
	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());


	obs_source_t* source = obs_source_create_private(sourceId.c_str(), name.c_str(), settings);
	if (!source) {
		blog(LOG_ERROR, "Failed to create transition.");
		return UINT64_MAX;
	}

	obs_data_release(settings);

	uint64_t uid = obs::Source::Manager::GetInstance().allocate(source);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Index list is full.");
		return UINT64_MAX;
	}
	obs::Source::attach_source_signals(source);

	return uid;
}

uint64_t obs::Transition::FromName(std::string name)
{
	obs_source_t* source = obs_get_source_by_name(name.c_str());
	if (!source) {
		blog(LOG_ERROR, "Named transition could not be found.");
		return UINT64_MAX;
	}

	uint64_t uid = obs::Source::Manager::GetInstance().find(source);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Source found but not indexed.");
		return UINT64_MAX;
	}

	obs_source_release(source);

	return uid;
}

std::pair<uint64_t, uint32_t> obs::Transition::GetActiveSource(uint64_t sourceId)
{
	uint64_t uid = -1;

	// Attempt to find the source asked to load.
	obs_source_t* transition = obs::Source::Manager::GetInstance().find(sourceId);
	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return std::make_pair(UINT64_MAX, UINT32_MAX);
	}

	obs_source_type type   = OBS_SOURCE_TYPE_INPUT;
	obs_source_t*   source = obs_transition_get_active_source(transition);
	if (source) {
		uid  = obs::Source::Manager::GetInstance().find(source);
		type = obs_source_get_type(source);
		obs_source_release(source);
	}

	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Source found but not indexed.");
		return std::make_pair(UINT64_MAX, UINT32_MAX);
	}

	return std::make_pair(uid, type);
}

void obs::Transition::Clear(uint64_t sourceId)
{
	// Attempt to find the source asked to load.
	obs_source_t* transition = obs::Source::Manager::GetInstance().find(sourceId);
	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return;
	}

	obs_transition_clear(transition);
}

void obs::Transition::Set(uint64_t transitionId, uint64_t sourceId)
{
	// Attempt to find the source asked to load.
	obs_source_t* transition = obs::Source::Manager::GetInstance().find(transitionId);
	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return;
	}

	obs_source_t* source = obs::Source::Manager::GetInstance().find(sourceId);
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_transition_set(transition, source);
}

bool obs::Transition::Start(
	uint64_t transitionId, uint32_t ms, uint64_t sourceId)
{
	// Attempt to find the source asked to load.
	obs_source_t* transition = obs::Source::Manager::GetInstance().find(transitionId);
	if (!transition) {
		blog(LOG_ERROR, "Transition reference is not valid.");
		return false;
	}

	obs_source_t* source = obs::Source::Manager::GetInstance().find(sourceId);
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return false;
	}

	return obs_transition_start(transition, OBS_TRANSITION_MODE_AUTO, ms, source);
}
