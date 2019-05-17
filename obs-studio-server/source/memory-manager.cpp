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
	int ret         = GlobalMemoryStatusEx(&statex);

	if (ret) {
		available_memory    = statex.ullTotalPhys;
		allowed_cached_size = std::min((uint64_t)LIMIT, (uint64_t)available_memory / 2);
	} else {
		available_memory    = 0;
		allowed_cached_size = LIMIT;
	}
}

void MemoryManager::registerSource(obs_source_t* source)
{
	mtx.lock();
	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0) {
		mtx.unlock();
		return;
	}

	source_info* si = new source_info;
	si->cached      = false;
	si->size        = 0;
	si->source      = source;
	sources.emplace(obs_source_get_name(source), si);

	mtx.unlock();
	updateCacheSettings(source, false);
}

void MemoryManager::calculateRawSize(source_info* si)
{
	uint64_t width  = obs_source_get_width(si->source);
	uint64_t height = obs_source_get_height(si->source);

	calldata_t      cd = {0};
	proc_handler_t* ph = obs_source_get_proc_handler(si->source);
	proc_handler_call(ph, "restart", &cd);
	proc_handler_call(ph, "get_nb_frames", &cd);
	uint64_t nb_frames = calldata_int(&cd, "num_frames");

	uint64_t size = width * height * 1.5 * nb_frames;
	si->size = size < 0 ? 0 : size;
}

bool MemoryManager::shouldCacheSource(source_info* si)
{
	bool should_cache = false;

	obs_data_t* settings = obs_source_get_settings(si->source);

	bool looping        = obs_data_get_bool(settings, "looping");
	bool local_file     = obs_data_get_bool(settings, "is_local_file");
	bool enable_caching = config_get_bool(ConfigManager::getInstance().getGlobal(), "General", "fileCaching");
	bool is_small       = current_cached_size + si->size < allowed_cached_size;

	obs_data_release(settings);

	return looping && local_file && enable_caching && is_small;
}

void MemoryManager::addCachedMemory(source_info* si)
{
	blog(LOG_INFO, "adding %d", si->size);

	current_cached_size += si->size;
	si->cached          = true;
}

void MemoryManager::removeCacheMemory(source_info* si)
{
	blog(LOG_INFO, "removing %d", si->size);

	current_cached_size -= si->size;
	si->cached          = false;

	if (current_cached_size < allowed_cached_size) {
		for (auto data : sources) {
			obs_data_t* settings = obs_source_get_settings(si->source);
			bool        should_cache = obs_data_get_bool(settings, "caching");

			if (!data.second->cached && should_cache)
				addCachedMemory(data.second);

			obs_data_release(settings);
		}
	}
}

void MemoryManager::updateCacheSettings(obs_source_t* source, bool updateSize)
{
	std::unique_lock<std::mutex> ulock(mtx);

	auto it = sources.find(obs_source_get_name(source));
	if (it == sources.end())
		return;

	obs_data_t* settings = obs_source_get_settings(source);

	bool looping        = obs_data_get_bool(settings, "looping");
	bool local_file     = obs_data_get_bool(settings, "is_local_file");
	bool enable_caching = config_get_bool(
		ConfigManager::getInstance().getGlobal(), "General", "fileCaching");

	bool cache_source = enable_caching && looping && local_file;

	if (updateSize) {
		if (it->second->size == 0)
			calculateRawSize(it->second);

		if (it->second->size != 0 && cache_source && !it->second->cached) {
			cache_source = current_cached_size + it->second->size < allowed_cached_size;
			if (cache_source)
				addCachedMemory(it->second);
		} else if (it->second->cached && !cache_source)
			removeCacheMemory(it->second);

		blog(LOG_INFO, "current cached size: %d", current_cached_size / 1000000);
	}

	if (obs_data_get_bool(settings, "caching") != cache_source) {
		obs_data_set_bool(settings, "caching", cache_source);
		obs_source_update(source, settings);
	}
	obs_data_release(settings);
}

void MemoryManager::unregisterSource(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);

	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;
	
	auto it = sources.find(obs_source_get_name(source));

	if (it == sources.end())
		return;

	obs_data_t* settings = obs_source_get_settings(source);

	if (obs_data_get_bool(settings, "caching") && it->second->cached)
		removeCacheMemory(it->second);

	blog(LOG_INFO, "current cached size: %d", current_cached_size / 1000000);
	free(it->second);
	sources.erase(obs_source_get_name(source));
}

void MemoryManager::updateCacheState()
{
	std::unique_lock<std::mutex> ulock(mtx);

	for (auto data : sources)
		updateCacheSettings(data.second->source, true);
}