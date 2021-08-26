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
#include <ipc-server.hpp>
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

std::tuple<uint64_t, std::string, uint32_t> obs::Input::Create(
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
		return std::make_tuple(UINT64_MAX, "", UINT32_MAX);
	}

	obs_data_release(hotkeys);
	obs_data_release(settings);

	uint64_t uid = osn::Source::Manager::GetInstance().find(source);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Index list is full.");
		return std::make_tuple(UINT64_MAX, "", UINT32_MAX);
	}
	obs_data_t* settingsSource = obs_source_get_settings(source);

	obs_data_release(settingsSource);
	return std::make_tuple(
		uid,
		obs_data_get_full_json(settingsSource),
		obs_source_get_audio_mixers(source)
	);
}

uint64_t obs::Input::CreatePrivate(
    std::string sourceId, std::string name,
	std::string settingsData)
{
	obs_data_t* settings = nullptr;
	if (!settingsData.empty())
		settings = obs_data_create_from_json(settingsData.c_str());

	obs_source_t* source = obs_source_create_private(sourceId.c_str(), name.c_str(), settings);
	if (!source) {
		blog(LOG_ERROR, "Failed to create input.");
		return UINT64_MAX;
	}

	obs_data_release(settings);

	uint64_t uid = osn::Source::Manager::GetInstance().allocate(source);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Index list is full.");
		return UINT64_MAX;
	}
	osn::Source::attach_source_signals(source);

	return uid;
}

uint64_t obs::Input::Duplicate(uint64_t sourceId)
{
	obs_source_t* sourceOld = osn::Source::Manager::GetInstance().find(sourceId);
	if (!sourceOld) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT64_MAX;
	}

	bool        isPrivate    = false;
	const char* nameOverride = nullptr;

	obs_source_t* source = nullptr;
	source               = obs_source_duplicate(sourceOld, nameOverride, isPrivate);

	if (!source) {
		blog(LOG_ERROR, "Failed to duplicate input.");
		return UINT64_MAX;
	}

	if (source != sourceOld) {
		uint64_t uid = osn::Source::Manager::GetInstance().allocate(source);
		if (uid == UINT64_MAX) {
			blog(LOG_ERROR, "Index list is full.");
			return UINT64_MAX;
		}
		return uid;
	} else {
		return sourceId;
	}
}

uint64_t obs::Input::FromName(std::string name)
{
	obs_source_t* source = obs_get_source_by_name(name.c_str());
	if (!source) {
		blog(LOG_ERROR, "Named input could not be found.");
		return UINT64_MAX;
	}

	uint64_t uid = osn::Source::Manager::GetInstance().find(source);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Source found but not indexed.");
		return UINT64_MAX;
	}

	obs_source_release(source);

	return uid;
}

std::vector<uint64_t> obs::Input::GetPublicSources()
{
	std::vector<uint64_t> inputs;

	// !FIXME! Optimize for zero-copy operation, can directly write to rval.
	auto enum_cb = [](void* data, obs_source_t* source) {
		uint64_t uid = osn::Source::Manager::GetInstance().find(source);
		if (uid != UINT64_MAX) {
			static_cast<std::list<uint64_t>*>(data)->push_back(uid);
		}
		return true;
	};

	obs_enum_sources(enum_cb, &inputs);

	return inputs;
}

bool obs::Input::GetActive(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return false;
	}

	return obs_source_active(input);
}

bool obs::Input::GetShowing(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return false;
	}

	return obs_source_showing(input);
}

float_t obs::Input::GetVolume(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return 0.0;
	}

	return obs_source_get_volume(input);
}

float_t obs::Input::SetVolume(uint64_t uid, float_t volume)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return 0.0;
	}

	obs_source_set_volume(input, volume);

	return obs_source_get_volume(input);
}

int64_t obs::Input::GetSyncOffset(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	return obs_source_get_sync_offset(input);
}

int64_t obs::Input::SetSyncOffset(uint64_t uid, int64_t offset)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	obs_source_set_sync_offset(input, offset);

	return obs_source_get_sync_offset(input);
}

uint32_t obs::Input::GetAudioMixers(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	return obs_source_get_audio_mixers(input);
}

uint32_t obs::Input::SetAudioMixers(uint64_t uid, uint32_t mixers)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	obs_source_set_audio_mixers(input, (obs_monitoring_type)mixers);

	return obs_source_get_audio_mixers(input);
}

int32_t obs::Input::GetMonitoringType(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	return obs_source_get_monitoring_type(input);
}

int32_t obs::Input::SetMonitoringType(uint64_t uid, int32_t monitoringType)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	obs_source_set_monitoring_type(input, (obs_monitoring_type)monitoringType);

	return obs_source_get_monitoring_type(input);
}

uint32_t obs::Input::GetWidth(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	return obs_source_get_width(input);
}

uint32_t obs::Input::GetHeight(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT32_MAX;
	}

	return obs_source_get_height(input);
}

int32_t obs::Input::GetDeInterlaceFieldOrder(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	return obs_source_get_deinterlace_field_order(input);
}

int32_t obs::Input::SetDeInterlaceFieldOrder(uint64_t uid, int32_t deinterlaceOrder)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	obs_source_set_deinterlace_field_order(input, (obs_deinterlace_field_order)deinterlaceOrder);

	return obs_source_get_deinterlace_field_order(input);
}

int32_t obs::Input::GetDeInterlaceMode(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	return obs_source_get_deinterlace_mode(input);
}

int32_t obs::Input::SetDeInterlaceMode(uint64_t uid, int32_t deinterlaceMode)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT32_MAX;
	}

	obs_source_set_deinterlace_mode(input, (obs_deinterlace_mode)deinterlaceMode);

	return obs_source_get_deinterlace_mode(input);
}

void obs::Input::AddFilter(uint64_t sourceId, uint64_t filterId)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(sourceId);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_t* filter = osn::Source::Manager::GetInstance().find(filterId);
	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return;
	}

	obs_source_filter_add(input, filter);
}

void obs::Input::RemoveFilter(uint64_t sourceId, uint64_t filterId)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(sourceId);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_t* filter = osn::Source::Manager::GetInstance().find(filterId);
	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return;
	}

	obs_source_filter_remove(input, filter);
}

void obs::Input::MoveFilter(uint64_t sourceId, uint64_t filterId, uint32_t move)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(sourceId);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_t* filter = osn::Source::Manager::GetInstance().find(filterId);
	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return;
	}

	obs_order_movement movement = (obs_order_movement)move;

	obs_source_filter_set_order(input, filter, movement);
}

uint64_t obs::Input::FindFilter(uint64_t sourceId, std::string name)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(sourceId);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT64_MAX;
	}

	obs_source_t* filter = obs_source_get_filter_by_name(input, name.c_str());
	if (!filter) {
		blog(LOG_ERROR, "Filter reference is not valid.");
		return UINT64_MAX;
	}
	obs_source_release(filter);

	uint64_t uid = osn::Source::Manager::GetInstance().find(filter);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Filter found but not indexed.");
		return UINT64_MAX;
	}

	return uid;
}

std::vector<uint64_t> obs::Input::GetFilters(uint64_t uid)
{
	std::vector<uint64_t> filters;

	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return filters;
	}

	auto enum_cb = [](obs_source_t* parent, obs_source_t* filter, void* data) {
		std::vector<uint64_t>* filters = reinterpret_cast<std::vector<uint64_t>*>(data);

		uint64_t id = osn::Source::Manager::GetInstance().find(filter);
		if (id != UINT64_MAX) {
			filters->push_back(id);
		}
	};

	obs_source_enum_filters(input, enum_cb, &filters);
	return filters;
}

void obs::Input::CopyFiltersTo(uint64_t inputIdFrom, uint64_t inputIdTo)
{
	obs_source_t* input_from = osn::Source::Manager::GetInstance().find(inputIdFrom);
	if (!input_from) {
		blog(LOG_ERROR, "1st Input reference is not valid.");
		return;
	}

	obs_source_t* input_to = osn::Source::Manager::GetInstance().find(inputIdTo);
	if (!input_to) {
		blog(LOG_ERROR, "2nd Input reference is not valid.");
		return;
	}

	obs_source_copy_filters(input_to, input_from);
}

int64_t obs::Input::GetDuration(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	return obs_source_media_get_duration(input);
}

int64_t obs::Input::GetTime(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	return obs_source_media_get_time(input);
}

int64_t obs::Input::SetTime(uint64_t uid, int64_t time)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return INT64_MAX;
	}

	obs_source_media_set_time(input, time);

	return obs_source_media_get_time(input);
}

void obs::Input::Play(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_play_pause(input, false);
}

void obs::Input::Pause(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_play_pause(input, true);
}

void obs::Input::Restart(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_restart(input);
}

void obs::Input::Stop(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return;
	}

	obs_source_media_stop(input);
}

uint64_t obs::Input::GetMediaState(uint64_t uid)
{
	obs_source_t* input = osn::Source::Manager::GetInstance().find(uid);
	if (!input) {
		blog(LOG_ERROR, "Input reference is not valid.");
		return UINT64_MAX;
	}

	return obs_source_media_get_state(input);
}