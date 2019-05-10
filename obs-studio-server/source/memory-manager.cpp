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
	uint64_t size = width * height * 1.5 * nb_frames / 1000000;

	sources.emplace(source, size);
}

void MemoryManager::unregisterSource(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);
	
	if (strcmp(obs_source_get_id(source), "ffmpeg_source") != 0)
		return;

	sources.erase(source);
}

void MemoryManager::updateCacheState(bool caching)
{
	std::unique_lock<std::mutex> ulock(mtx);

	for (auto data : sources) {
		obs_data_t* settings = obs_source_get_settings(data.first);
		obs_data_set_bool(settings, "caching", caching);
		obs_source_update(data.first, settings);
	}
}