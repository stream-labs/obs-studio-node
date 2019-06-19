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

	if (GlobalMemoryStatusEx(&statex)) {
		available_memory    = statex.ullTotalPhys;
		allowed_cached_size = std::min((uint64_t)LIMIT, (uint64_t)available_memory / 2);
	} else {
		available_memory    = 0;
		allowed_cached_size = LIMIT;
	}
}

void MemoryManager::calculateRawSize(source_info* si)
{
	calldata_t      cd = {0};
	proc_handler_t* ph = obs_source_get_proc_handler(si->source);
	proc_handler_call(ph, "get_nb_frames", &cd);

	uint64_t nb_frames = calldata_int(&cd, "num_frames");
	uint64_t width     = calldata_int(&cd, "width");
	uint64_t height    = calldata_int(&cd, "height");
	uint32_t pix_fmt   = calldata_int(&cd, "pix_format");
	float    bpp       = 0;

	switch (pix_fmt) {
		case VIDEO_FORMAT_I420:
		case VIDEO_FORMAT_NV12:
			bpp = 1.5;
			break;
		case VIDEO_FORMAT_YVYU:
		case VIDEO_FORMAT_YUY2:
		case VIDEO_FORMAT_UYVY:
			bpp = 2;
			break;
		case VIDEO_FORMAT_RGBA:
		case VIDEO_FORMAT_BGRA:
		case VIDEO_FORMAT_BGRX:
		case VIDEO_FORMAT_Y800:
			bpp = 4;
			break;
		case VIDEO_FORMAT_I444:
			bpp = 4;
			break;
	}

	uint64_t size = width * height * bpp * nb_frames;
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
	bool showing        = obs_source_showing(si->source);

	if (!showing && !obs_data_get_bool(settings, "close_when_inactive"))
		showing = true;

	obs_data_release(settings);

	return looping &&
           local_file &&
           enable_caching &&
           is_small && showing;
}

void updateSource(obs_source_t* source, bool caching)
{
	obs_data_t* settings = obs_source_get_settings(source);
	if (obs_data_get_bool(settings, "caching") != caching) {
		obs_data_set_bool(settings, "caching", caching);
		obs_source_update(source, settings);
	}
	obs_data_release(settings);
}

void MemoryManager::addCachedMemory(source_info* si)
{
	std::unique_lock<std::mutex> ulock(si->mtx);

	if (!si->size || si->cached || current_cached_size + si->size > allowed_cached_size)
		return;

	int32_t retry = MAX_POOLS;

	proc_handler_t* ph = obs_source_get_proc_handler(si->source);
	bool            playing = false;
	while (retry > 0) {
		calldata_t      cd = {0};
		proc_handler_call(ph, "get_playing", &cd);
		playing = calldata_bool(&cd, "playing");
		if (playing)
			break;

		retry--;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	if (!playing)
		return;

	blog(LOG_INFO, "adding %dMB, source: %s", si->size / 1000000, obs_source_get_name(si->source));
	current_cached_size += si->size;
	si->cached          =  true;

	updateSource(si->source, true);
}

void MemoryManager::removeCachedMemory(source_info* si, bool cacheNewFiles)
{
	std::unique_lock<std::mutex> ulock(si->mtx);
	if (!si->cached)
		return;

	blog(LOG_INFO, "removing %dMB, source: %s", si->size / 1000000, obs_source_get_name(si->source));
	current_cached_size -= si->size;
	si->cached          =  false;

	updateSource(si->source, false);

	if (!cacheNewFiles || current_cached_size >= allowed_cached_size)
		return;


	for (auto data : sources) {
		if (strcmp(obs_source_get_name(si->source), data.first) != 0 && shouldCacheSource(data.second))
			addCachedMemory(data.second);
	}
}

void MemoryManager::sourceManager(source_info* si)
{
	si->mtx.lock();
	obs_data_t*                  settings = obs_source_get_settings(si->source);

	bool looping    = obs_data_get_bool(settings, "looping");
	bool local_file = obs_data_get_bool(settings, "is_local_file");

	obs_data_release(settings);
	if (!looping || !local_file) {
		si->mtx.unlock();
		return;
	}
	si->mtx.unlock();

	if (si->size == 0) {
		uint32_t retry = MAX_POOLS;
		while (retry > 0) {
			mtx.lock();
			si->mtx.lock();
			calculateRawSize(si);

			if (si->size) {
				si->mtx.unlock();
				mtx.unlock();
				break;
			}
			si->mtx.unlock();
			mtx.unlock();

			retry--;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	std::unique_lock<std::mutex> ulock(mtx);
	si->mtx.lock();
	if (!si->size) {
		si->mtx.unlock();
		return;
	}
	bool should_cache = shouldCacheSource(si);
	si->mtx.unlock();

	if (should_cache)
		addCachedMemory(si);
	else
		removeCachedMemory(si, true);
}

void MemoryManager::updateSettings(obs_source_t * source)
{
	auto it = sources.find(obs_source_get_name(source));

	if (it == sources.end())
		return;

	it->second->workers.push_back(std::thread(&MemoryManager::sourceManager, this, it->second));
}

void MemoryManager::updateSourceCache(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);

	updateSettings(source);
}

void MemoryManager::updateSourcesCache(void)
{
	std::unique_lock<std::mutex> ulock(mtx);

	for (auto data : sources)
		updateSettings(data.second->source);
}

void MemoryManager::registerSource(obs_source_t* source)
{
	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;

	std::unique_lock<std::mutex> ulock(mtx);

	source_info* si = new source_info;
	si->cached      = false;
	si->size        = 0;
	si->source      = source;
	sources.emplace(obs_source_get_name(source), si);
	updateSource(source, false);
	if (!watcher.running) {
		watcher.running = true;
		watcher.worker  = std::thread(&MemoryManager::monitorMemory, this);
	}
}

void MemoryManager::unregisterSource(obs_source_t * source)
{
	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;

	mtx.lock();

	auto it = sources.find(obs_source_get_name(source));

	if (it == sources.end()) {
		mtx.unlock();
		return;
	}
	mtx.unlock();

	for (auto& worker : it->second->workers) {
		if (worker.joinable())
			worker.join();
	}

	mtx.lock();
	
	it->second->workers.clear();
	removeCachedMemory(it->second, true);

	free(it->second);
	sources.erase(obs_source_get_name(source));

	if (!sources.size() && watcher.running) {
		watcher.stop    = true;

		if (watcher.worker.joinable())
			watcher.worker.join();

		watcher.running = false;
	}

	mtx.unlock();
}

void MemoryManager::monitorMemory()
{
	while (!watcher.stop) {
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);

		if (GlobalMemoryStatusEx(&statex)) {
			std::unique_lock<std::mutex> ulock(mtx);

			uint64_t memory_in_use               = statex.ullTotalPhys - statex.ullAvailPhys;
			uint64_t memory_in_use_without_cache = memory_in_use - current_cached_size;

			float memory_load =
			    (float)(memory_in_use_without_cache + current_cached_size) / (float)statex.ullTotalPhys * 100;

			auto it = sources.begin();
			if (memory_load >= UPPER_LIMIT) {
				while (memory_load >= (UPPER_LIMIT - 10) && it != sources.end()) {
					removeCachedMemory(it->second, false);
					memory_load =
					    (float)(memory_in_use_without_cache + current_cached_size) / (float)statex.ullTotalPhys * 100;
					it++;
				}
			} else if (memory_load < LOWER_LIMIT) {
				while (memory_load < (LOWER_LIMIT + 10) && it != sources.end()) {
					if (shouldCacheSource(it->second))
						addCachedMemory(it->second);
					memory_load =
					    (float)(memory_in_use_without_cache + current_cached_size) / (float)statex.ullTotalPhys * 100;
					it++;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}