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

#include "osn-global.hpp"
#include <error.hpp>
#include <obs.h>
#include "osn-source.hpp"
#include "shared-server.hpp"

std::pair<uint64_t, int32_t> obs::Global::GetOutputSource(uint32_t channel)
{
	obs_source_t* source = obs_get_output_source(channel);
	if (!source) {
		blog(LOG_ERROR, "Source not found.");
		return std::make_pair(UINT64_MAX, -1);
	}

	uint64_t uid = obs::Source::Manager::GetInstance().find(source);
	if (uid == UINT64_MAX) {
		blog(LOG_ERROR, "Source found but not indexed.");
		return std::make_pair(UINT64_MAX, -1);
	}

	obs_source_release(source);
	return std::make_pair(uid, obs_source_get_type(source));
}

void obs::Global::SetOutputSource(uint32_t channel, uint64_t sourceId)
{
	obs_source_t* source = nullptr;

	if (channel >= MAX_CHANNELS) {
		blog(LOG_ERROR, "Invalid output channel.");
		return;
	}

	if (sourceId != UINT64_MAX) {
		source = obs::Source::Manager::GetInstance().find(sourceId);
		if (!source) {
			blog(LOG_ERROR, "Source reference is not valid.");
			return;
		}
	}

	obs_set_output_source(channel, source);
	obs_source_t* newsource = obs_get_output_source(channel);
	if (newsource != source) {
		obs_source_release(newsource);
		blog(LOG_ERROR, "Failed to set output source.");
		return;
	}
	obs_source_release(newsource);
	return;
}

uint32_t obs::Global::GetOutputFlagsFromId(std::string id)
{
	return obs_get_source_output_flags(id.c_str());
}

uint32_t obs::Global::LaggedFrames()
{
	return obs_get_lagged_frames();
}

uint32_t obs::Global::TotalFrames()
{
	return obs_get_total_frames();
}

std::string obs::Global::GetLocale()
{
	return std::string(obs_get_locale());
}

void obs::Global::SetLocale(std::string locale)
{
	obs_set_locale(locale.c_str());
}

bool obs::Global::GetMultipleRendering()
{
	return obs_get_multiple_rendering();
}

void obs::Global::SetMultipleRendering(bool multipleRendering)
{
	obs_set_multiple_rendering(multipleRendering);
}
