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
#include "osn-source.hpp"
#include <tuple>

namespace obs
{
	class Input : public obs::Source
	{
		public:
		static std::vector<std::string> Types();
		static obs_source* Create(
						std::string sourceId, std::string name,
						std::string settingsData, std::string hotkeyData);
		static obs_source* CreatePrivate(
						std::string sourceId, std::string name,
						std::string settingsData);
		static obs_source* Duplicate(obs_source* sourceOld);
		static obs_source* FromName(std::string name);
		static std::vector<obs_source_t*> GetPublicSources();

		/// Status
		static bool GetActive(obs_source* input);
		static bool GetShowing(obs_source* input);

		/// Audio
		static float_t GetVolume(obs_source* input);
		static float_t SetVolume(obs_source* input, float_t volume);
		static int64_t GetSyncOffset(obs_source* input);
		static int64_t SetSyncOffset(obs_source* input, int64_t offset);
		static uint32_t GetAudioMixers(obs_source* input);
		static uint32_t SetAudioMixers(obs_source* input, uint32_t mixers);
		static int32_t GetMonitoringType(obs_source* input);
		static int32_t SetMonitoringType(obs_source* input, int32_t monitoringType);

		/// Video
		static uint32_t GetWidth(obs_source* input);
		static uint32_t GetHeight(obs_source* input);
		static int32_t GetDeInterlaceFieldOrder(obs_source* input);
		static int32_t SetDeInterlaceFieldOrder(obs_source* input, int32_t deinterlaceOrder);
		static int32_t GetDeInterlaceMode(obs_source* input);
		static int32_t SetDeInterlaceMode(obs_source* input, int32_t deinterlaceMode);
		/// Filters
		static void AddFilter(obs_source* input, obs_source* filter);
		static void RemoveFilter(obs_source* input, obs_source* filter);
		static void MoveFilter(obs_source* input, obs_source* filter, uint32_t move);
		static obs_source_t* FindFilter(obs_source* input, std::string name);
		static std::vector<obs_source_t*> GetFilters(obs_source_t* input);
		static void CopyFiltersTo(obs_source_t* inputFrom, obs_source_t* inputTo);
		static int64_t GetDuration(obs_source_t* input);
		static int64_t GetTime(obs_source_t* input);
		static int64_t SetTime(obs_source_t* input, int64_t time);
		static void Play(obs_source_t* input);
		static void Pause(obs_source_t* input);
		static void Restart(obs_source_t* input);
		static void Stop(obs_source_t* input);
		static uint64_t GetMediaState(obs_source_t* input);
	};
}