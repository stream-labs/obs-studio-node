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

#pragma once
#include "obs.h"
#include <utility>
#include <string>

namespace obs
{
	class Global
	{
		public:
		static std::pair<obs_source_t*, int32_t> GetOutputSource(uint32_t channel);
		static void SetOutputSource(uint32_t channel, obs_source_t* source);
		static uint32_t GetOutputFlagsFromId(std::string id);
		static uint32_t LaggedFrames();
		static uint32_t TotalFrames();
		static std::string GetLocale();
		static void SetLocale(std::string locale);
		static bool GetMultipleRendering();
		static void SetMultipleRendering(bool multipleRendering);
	};
}