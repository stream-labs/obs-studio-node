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

#include "callback-manager.h"
#include "osn-source.hpp"
#include <windows.h>
#include "error.hpp"
#include "shared.hpp"

std::mutex                             mtx;
std::map<std::string, SourceSizeInfo*> sources;

void CallbackManager::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("CallbackManager");

	cls->register_function(std::make_shared<ipc::function>("QuerySourceSize", std::vector<ipc::type>{}, QuerySourceSize));

	srv.register_collection(cls);
}

void CallbackManager::QuerySourceSize(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::unique_lock<std::mutex> ulock(mtx);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	if (sources.empty()) {
		return;
	}

	uint32_t size = 0;

	for (auto item : sources) {
		SourceSizeInfo* si = item.second;
		// See if width or height changed here
		uint32_t newWidth  = obs_source_get_width(si->source);
		uint32_t newHeight = obs_source_get_height(si->source);
		uint32_t newFlags  = obs_source_get_output_flags(si->source);

		if (si->width != newWidth || si->height != newHeight || si->flags != newFlags) {
			si->width = newWidth;
			si->height = newHeight;
			si->flags  = newFlags;

			rval.push_back(ipc::value(obs_source_get_name(si->source)));
			rval.push_back(ipc::value(si->width));
			rval.push_back(ipc::value(si->height));
			rval.push_back(ipc::value(si->flags));

			size++;
		}
	}

	rval.insert(rval.begin() + 1, ipc::value(size));

	AUTO_DEBUG;
}

void CallbackManager::addSource(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);

	if (!source || obs_source_get_type(source) == OBS_SOURCE_TYPE_FILTER)
		return;

	SourceSizeInfo* si               = new SourceSizeInfo;
	si->source                       = source;
	si->width                        = obs_source_get_width(source);
	si->height                       = obs_source_get_height(source);

	sources.emplace(std::make_pair(std::string(obs_source_get_name(source)), si));
}
void CallbackManager::removeSource(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(mtx);
	
	if (!source)
		return;

	const char* name = obs_source_get_name(source);
	
	if (name)
		sources.erase(name);
}