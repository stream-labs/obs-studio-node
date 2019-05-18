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
	si->running     = false;
	sources.emplace(obs_source_get_name(source), si);

	mtx.unlock();
	updateCacheSettings(source, true);
}

void MemoryManager::calculateRawSize(source_info* si)
{
	uint64_t width  = obs_source_get_width(si->source);
	uint64_t height = obs_source_get_height(si->source);

	calldata_t      cd = {0};
	proc_handler_t* ph = obs_source_get_proc_handler(si->source);
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
	bool is_small       = si->size < allowed_cached_size;
	blog(
	    LOG_INFO,
	    "current cached size: %dMB, source size: %dMB, allowed cached size %dMB",
	    current_cached_size / 1000000,
	    si->size / 1000000,
	    allowed_cached_size / 1000000);
	obs_data_release(settings);

	return looping && local_file && enable_caching && is_small;
}

void MemoryManager::addCachedMemory(source_info* si)
{
	blog(LOG_INFO, "adding %dMB", si->size / 1000000);

	current_cached_size += si->size;
	si->cached          = true;
}

void MemoryManager::removeCacheMemory(source_info* si)
{
	blog(LOG_INFO, "removing %dMB", si->size / 1000000);

	current_cached_size     -= si->size;
	si->cached              = false;
	const char* current_key = obs_source_get_name(si->source);

	if (current_cached_size < allowed_cached_size) {
		blog(LOG_INFO, "size sources: %d", sources.size());
		for (auto data : sources) {
			if (strcmp(current_key, data.first) != 0) {
				bool should_cache = shouldCacheSource(si);

				if (!data.second->cached && should_cache) {
					blog(LOG_INFO, "checking if we should add sources in the cache");
					addCachedMemory(data.second);
					data.second->cached = true;
					obs_data_t* settings = obs_source_get_settings(data.second->source);
					obs_data_set_bool(settings, "caching", true);
					blog(LOG_INFO, "updating caching in source settings");
					obs_source_update(data.second->source, settings);
					obs_data_release(settings);
				}
			}
		}
	}
}

void MemoryManager::worker(obs_source_t* source, bool updateSize)
{
	std::unique_lock<std::mutex> ulock(mtx);

	auto it = sources.find(obs_source_get_name(source));
	if (it == sources.end())
		return;

	it->second->running = true;

	bool cache_source = shouldCacheSource(it->second);
	if (updateSize) {
		uint32_t retry = 10;
		while (it->second->size == 0 && retry > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			calculateRawSize(it->second);
			retry--;
		}

		if (it->second->size != 0 && cache_source && !it->second->cached) {
			cache_source = current_cached_size + it->second->size < allowed_cached_size;
			if (cache_source)
				addCachedMemory(it->second);
			else
				blog(LOG_INFO, "Too big, not caching %dMB", it->second->size / 1000000);
		} else if (it->second->cached && !cache_source)
			removeCacheMemory(it->second);

		blog(LOG_INFO, "current cached size: %dMB", current_cached_size / 1000000);
	}

	obs_data_t* settings = obs_source_get_settings(source);
	if (obs_data_get_bool(settings, "caching") != cache_source) {
		obs_data_set_bool(settings, "caching", cache_source);
		obs_source_update(source, settings);
	}
	obs_data_release(settings);
	it->second->running = false;
}

void MemoryManager::updateCacheSettings(obs_source_t* source, bool updateSize)
{
	auto it = sources.find(obs_source_get_name(source));

	if (it == sources.end())
		return;

	if (it->second->running)
		return;

	if (it->second->worker.joinable())
		it->second->worker.join();

	it->second->worker = std::thread(&MemoryManager::worker, this, source, updateSize);
	it->second->worker.detach();
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

	blog(LOG_INFO, "current cached size: %dMB", current_cached_size / 1000000);
	free(it->second);
	sources.erase(obs_source_get_name(source));
}

void MemoryManager::updateCacheState()
{
	std::unique_lock<std::mutex> ulock(mtx);

	for (auto data : sources)
		updateCacheSettings(data.second->source, true);
}