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

#include "memory-manager.h"

MemoryManager::MemoryManager()
{
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	int ret = GlobalMemoryStatusEx(&statex);

	if (ret) {
		available_memory    = statex.ullTotalPhys;
		allowed_cached_size = std::min((uint64_t)LIMIT, (uint64_t)available_memory / 2);
	} else {
		available_memory    = 0;
		allowed_cached_size = LIMIT;
	}

	estimated_cached_size = 0;
	current_cached_size   = 0;
}

void MemoryManager::registerSource(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);

	const char *test = obs_source_get_id(source);

	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;

	uint64_t width  = obs_source_get_width(source);
	uint64_t height = obs_source_get_height(source);

	calldata_t cd = {0};
	proc_handler_t* ph = obs_source_get_proc_handler(source);
	proc_handler_call(ph, "restart", &cd);
	proc_handler_call(ph, "get_nb_frames", &cd);
	uint64_t nb_frames = calldata_int(&cd, "num_frames");

	// Size in MB
	uint64_t     size = width * height * 1.5 * nb_frames;
	source_info* info = new source_info;
	info->cached      = false;
	info->size        = size;
	info->source      = source;

	sources.emplace(obs_source_get_name(source), info);

	bool caching = config_get_bool(ConfigManager::getInstance().getGlobal(), "General", "fileCaching");
	updateSourceCache(info, caching);
}

void MemoryManager::unregisterSource(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);
	auto                         it     = sources.find(obs_source_get_name(source));

	if (it == sources.end())
		return;

	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;

	if (it->second->cached)
		current_cached_size -= it->second->size;

	estimated_cached_size -= it->second->size;
	sources.erase(obs_source_get_name(source));
}

void MemoryManager::refreshSourceState(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);
	auto                         it = sources.find(obs_source_get_name(source));

	if (it == sources.end())
		return;

	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;
}

void MemoryManager::updateCacheState(bool caching)
{
	std::unique_lock<std::mutex> ulock(mtx);

	for (auto data : sources)
		updateSourceCache(data.second, caching);
}

void MemoryManager::updateSourceCache(source_info* info, bool caching)
{
	obs_data_t* settings   = obs_source_get_settings(info->source);
	bool        looping    = obs_data_get_bool(settings, "looping");
	bool        local_file = obs_data_get_bool(settings, "is_local_file");

	info->cached =
		current_cached_size < allowed_cached_size &&
		looping && local_file;

	obs_data_set_bool(settings, "caching", info->cached);
	obs_source_update(info->source, settings);
	obs_data_release(settings);

	if (info->cached)
		current_cached_size += info->size;

	estimated_cached_size += info->size;
}