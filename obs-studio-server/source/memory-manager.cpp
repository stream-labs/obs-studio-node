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
	bool is_small       = si->size < allowed_cached_size;
	bool showing        = obs_source_showing(si->source);

	if (!showing && !obs_data_get_bool(settings, "close_when_inactive"))
		showing = true;

	obs_data_release(settings);
	return looping && local_file && enable_caching && is_small && showing;
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
	bool allow_caching = current_cached_size + si->size < allowed_cached_size;
	if (allow_caching) {
		current_cached_size += si->size;
		si->cached = true;
		updateSource(si->source, true);
		blog(LOG_INFO, "adding %dMB, source: %s", si->size / 1000000, obs_source_get_name(si->source));
	} else {
		updateSource(si->source, false);
		blog(LOG_INFO, "Too big, not caching %dMB, source: %s", si->size / 1000000, obs_source_get_name(si->source));
	}
}

void MemoryManager::removeCacheMemory(source_info* si)
{
	blog(LOG_INFO, "removing %dMB, source: %s", si->size / 1000000, obs_source_get_name(si->source));

	current_cached_size -= si->size;
	si->cached              = false;
	const char* current_key = obs_source_get_name(si->source);

	if (current_cached_size < allowed_cached_size) {
		for (auto data : sources) {
			if (strcmp(current_key, data.first) != 0) {
				bool should_cache = shouldCacheSource(data.second);

				if (!data.second->cached && should_cache) {
					addCachedMemory(data.second);
				}
			}
		}
	}
	updateSource(si->source, false);
}

void MemoryManager::worker(source_info* si)
{
	if (si->size == 0) {
		uint32_t retry = 10;
		while (si->size == 0 && retry > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			si->mtx.lock();
			calculateRawSize(si);
			si->mtx.unlock();
			retry--;
		}
	}

	if (si->size == 0)
		return;

	bool cache_source = shouldCacheSource(si);

	if (cache_source && !si->cached) {
		addCachedMemory(si);
	} else if (!cache_source && si->cached) {
		removeCacheMemory(si);
	}
}

void MemoryManager::updateCacheSettings(obs_source_t * source)
{
	const char* name = obs_source_get_name(source);
	auto it = sources.find(obs_source_get_name(source));

	if (it == sources.end())
		return;

	if (it->second->worker.joinable())
		it->second->worker.join();

	it->second->worker = std::thread(&MemoryManager::worker, this, it->second);
	it->second->worker.detach();
}

void MemoryManager::unregisterSource(obs_source_t * source)
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

	free(it->second);
	sources.erase(obs_source_get_name(source));
}

void MemoryManager::updateCacheState()
{
	std::unique_lock<std::mutex> ulock(mtx);

	for (auto data : sources)
		updateCacheSettings(data.second->source);
}