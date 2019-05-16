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
	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;

	source_info* info = new source_info;
	info->cached      = false;
	info->size        = 0;
	info->source      = source;
	sources.emplace(obs_source_get_name(source), info);

	mtx.unlock();
	updateCacheSettings(source, false);
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

	bool cache_source        = enable_caching && looping && local_file;

	if (updateSize) {
		uint64_t width  = obs_source_get_width(source);
		uint64_t height = obs_source_get_height(source);

		if (it->second->size == 0) {
			calldata_t      cd = {0};
			proc_handler_t* ph = obs_source_get_proc_handler(source);
			proc_handler_call(ph, "restart", &cd);
			proc_handler_call(ph, "get_nb_frames", &cd);
			uint64_t nb_frames = calldata_int(&cd, "num_frames");

			int64_t size     = width * height * 1.5 * nb_frames;
			it->second->size = size < 0 ? 0 : size;
		}

		if (it->second->size != 0 && cache_source && !it->second->cached) {
			cache_source = current_cached_size + it->second->size < allowed_cached_size;
			if (cache_source) {
				blog(LOG_INFO, "adding %d", it->second->size);
				current_cached_size += it->second->size;
				it->second->cached = true;
			}
		} else if (it->second->cached && !cache_source) {
			blog(LOG_INFO, "removing %d", it->second->size);
			current_cached_size -= it->second->size;
			it->second->cached = false;
		}

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
		current_cached_size -= it->second->size;

	blog(LOG_INFO, "current cached size: %d", current_cached_size / 1000000);
	free(it->second);
	sources.erase(obs_source_get_name(source));
}

void MemoryManager::updateCacheState()
{
	std::unique_lock<std::mutex> ulock(mtx);

	for (auto data : sources) {
		updateCacheSettings(data.second->source, true);
	}
}