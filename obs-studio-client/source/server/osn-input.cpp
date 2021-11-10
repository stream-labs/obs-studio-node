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

#include "osn-Input.hpp"
#include <iostream>
#include <memory>
#include <obs.h>
#include "error.hpp"
#include "osn-source.hpp"
#include "shared-server.hpp"

std::vector<std::string> obs::Input::Types()
{
	std::vector<std::string> types;

	const char* typeId = nullptr;
	for (size_t idx = 0; obs_enum_input_types(idx, &typeId); idx++) {
		if (typeId)
			types.push_back(std::string(typeId));
	}

	return types;
}

obs_source* obs::Input::Create(
	std::string sourceId, std::string name,
	std::string settingsData, std::string hotkeyData)
{
	obs_data_t *settings = nullptr, *hotkeys = nullptr;

	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());

	if (!hotkeyData.empty())
		hotkeys = obs_data_create_from_json(hotkeyData.c_str());

	obs_source_t* source = obs_source_create(sourceId.c_str(), name.c_str(), settings, hotkeys);
	if (!source) {
		blog(LOG_ERROR, "Failed to create input.");
		return nullptr;
	}

	obs_data_release(hotkeys);
	obs_data_release(settings);

	obs::Source::attach_source_signals(source);

	return source;
}

obs_source* obs::Input::CreatePrivate(
    std::string sourceId, std::string name,
	std::string settingsData)
{
	obs_data_t* settings = nullptr;
	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());

	obs_source_t* source = obs_source_create_private(sourceId.c_str(), name.c_str(), settings);
	if (!source) {
		blog(LOG_ERROR, "Failed to create input.");
		return nullptr;
	}

	obs_data_release(settings);

	obs::Source::attach_source_signals(source);

	return source;
}

obs_source* obs::Input::Duplicate(obs_source* sourceOld)
{
	if (!sourceOld) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return nullptr;
	}

	bool        isPrivate    = false;
	const char* nameOverride = nullptr;

	obs_source_t* source = nullptr;
	source               = obs_source_duplicate(sourceOld, nameOverride, isPrivate);

	if (!source) {
		blog(LOG_ERROR, "Failed to duplicate input.");
		return nullptr;
	}

	return source;
}

obs_source* obs::Input::FromName(std::string name)
{
	obs_source_t* source = obs_get_source_by_name(name.c_str());
	if (!source) {
		blog(LOG_ERROR, "Named input could not be found.");
		return nullptr;
	}

	return source;
}

std::vector<obs_source_t*> obs::Input::GetPublicSources()
{
	std::vector<obs_source_t*> inputs;

	// !FIXME! Optimize for zero-copy operation, can directly write to rval.
	auto enum_cb = [](void* data, obs_source_t* source) {
		if (source)
			static_cast<std::vector<obs_source_t*>*>(data)->push_back(source);
		return true;
	};

	obs_enum_sources(enum_cb, &inputs);

	return inputs;
}

bool obs::Input::GetActive(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return false;
	}

	return obs_source_active(input);
}

bool obs::Input::GetShowing(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return false;
	}

	return obs_source_showing(input);
}

float_t obs::Input::GetVolume(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return 0.0;
	}

	return obs_source_get_volume(input);
}

float_t obs::Input::SetVolume(obs_source* input, float_t volume)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return 0.0;
	}

	obs_source_set_volume(input, volume);

	return obs_source_get_volume(input);
}

int64_t obs::Input::GetSyncOffset(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	return obs_source_get_sync_offset(input);
}

int64_t obs::Input::SetSyncOffset(obs_source* input, int64_t offset)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	obs_source_set_sync_offset(input, offset);

	return obs_source_get_sync_offset(input);
}

uint32_t obs::Input::GetAudioMixers(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	return obs_source_get_audio_mixers(input);
}

uint32_t obs::Input::SetAudioMixers(obs_source* input, uint32_t mixers)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	obs_source_set_audio_mixers(input, (obs_monitoring_type)mixers);

	return obs_source_get_audio_mixers(input);
}

int32_t obs::Input::GetMonitoringType(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	return obs_source_get_monitoring_type(input);
}

int32_t obs::Input::SetMonitoringType(obs_source* input, int32_t monitoringType)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	obs_source_set_monitoring_type(input, (obs_monitoring_type)monitoringType);

	return obs_source_get_monitoring_type(input);
}

uint32_t obs::Input::GetWidth(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	return obs_source_get_width(input);
}

uint32_t obs::Input::GetHeight(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	return obs_source_get_height(input);
}

int32_t obs::Input::GetDeInterlaceFieldOrder(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	return obs_source_get_deinterlace_field_order(input);
}

int32_t obs::Input::SetDeInterlaceFieldOrder(obs_source* input, int32_t deinterlaceOrder)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	obs_source_set_deinterlace_field_order(input, (obs_deinterlace_field_order)deinterlaceOrder);

	return obs_source_get_deinterlace_field_order(input);
}

int32_t obs::Input::GetDeInterlaceMode(obs_source* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	return obs_source_get_deinterlace_mode(input);
}

int32_t obs::Input::SetDeInterlaceMode(obs_source* input, int32_t deinterlaceMode)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	obs_source_set_deinterlace_mode(input, (obs_deinterlace_mode)deinterlaceMode);

	return obs_source_get_deinterlace_mode(input);
}

void obs::Input::AddFilter(obs_source* input, obs_source* filter)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return;
	}

	obs_source_filter_add(input, filter);
}

void obs::Input::RemoveFilter(obs_source* input, obs_source* filter)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return;
	}

	obs_source_filter_remove(input, filter);
}

void obs::Input::MoveFilter(obs_source* input, obs_source* filter, uint32_t move)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return;
	}

	obs_order_movement movement = (obs_order_movement)move;

	obs_source_filter_set_order(input, filter, movement);
}

obs_source_t* obs::Input::FindFilter(obs_source* input, std::string name)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return nullptr;
	}

	obs_source_t* filter = obs_source_get_filter_by_name(input, name.c_str());
	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return nullptr;
	}

	return filter;
}

std::vector<obs_source_t*> obs::Input::GetFilters(obs_source_t* input)
{
	std::vector<obs_source_t*> filters;

	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return filters;
	}

	auto enum_cb = [](obs_source_t* parent, obs_source_t* filter, void* data) {
		std::vector<obs_source_t*>* filters = reinterpret_cast<std::vector<obs_source_t*>*>(data);
		if (filter)
			filters->push_back(filter);
	};

	obs_source_enum_filters(input, enum_cb, &filters);
	return filters;
}

void obs::Input::CopyFiltersTo(obs_source_t* inputFrom, obs_source_t* inputTo)
{
	if (!inputFrom) {
		blog(LOG_ERROR, "1st Input reference is not valid.");
		return;
	}

	if (!inputTo) {
		blog(LOG_ERROR, "2nd Input reference is not valid.");
		return;
	}

	obs_source_copy_filters(inputTo, inputFrom);
}

int64_t obs::Input::GetDuration(obs_source_t* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	return obs_source_media_get_duration(input);
}

int64_t obs::Input::GetTime(obs_source_t* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	return obs_source_media_get_time(input);
}

int64_t obs::Input::SetTime(obs_source_t* input, int64_t time)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	obs_source_media_set_time(input, time);

	return obs_source_media_get_time(input);
}

void obs::Input::Play(obs_source_t* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_play_pause(input, false);
}

void obs::Input::Pause(obs_source_t* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_play_pause(input, true);
}

void obs::Input::Restart(obs_source_t* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_restart(input);
}

void obs::Input::Stop(obs_source_t* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_stop(input);
}

uint64_t obs::Input::GetMediaState(obs_source_t* input)
{
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT64_MAX;
	}

	return obs_source_media_get_state(input);
}