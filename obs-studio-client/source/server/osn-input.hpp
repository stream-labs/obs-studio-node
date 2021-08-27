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
		static std::tuple<uint64_t, std::string, uint32_t>
		            Create(
						std::string sourceId, std::string name,
						std::string settingsData, std::string hotkeyData);
		static uint64_t CreatePrivate(
						std::string sourceId, std::string name,
						std::string settingsData);
		static uint64_t Duplicate(uint64_t sourceId);
		static uint64_t FromName(std::string name);
		static std::vector<uint64_t> GetPublicSources();

		/// Status
		static bool GetActive(uint64_t uid);
		static bool GetShowing(uint64_t uid);

		/// Audio
		static float_t GetVolume(uint64_t uid);
		static float_t SetVolume(uint64_t uid, float_t volume);
		static int64_t GetSyncOffset(uint64_t uid);
		static int64_t SetSyncOffset(uint64_t uid, int64_t offset);
		static uint32_t GetAudioMixers(uint64_t uid);
		static uint32_t SetAudioMixers(uint64_t uid, uint32_t mixers);
		static int32_t GetMonitoringType(uint64_t uid);
		static int32_t SetMonitoringType(uint64_t uid, int32_t monitoringType);

		/// Video
		static uint32_t GetWidth(uint64_t uid);
		static uint32_t GetHeight(uint64_t uid);
		static int32_t GetDeInterlaceFieldOrder(uint64_t uid);
		static int32_t SetDeInterlaceFieldOrder(uint64_t uid, int32_t deinterlaceOrder);
		static int32_t GetDeInterlaceMode(uint64_t uid);
		static int32_t SetDeInterlaceMode(uint64_t uid, int32_t deinterlaceMode);
		/// Filters
		static void AddFilter(uint64_t sourceId, uint64_t filterId);
		static void RemoveFilter(uint64_t sourceId, uint64_t filterId);
		static void MoveFilter(uint64_t sourceId, uint64_t filterId, uint32_t move);
		static uint64_t FindFilter(uint64_t sourceId, std::string name);
		static std::vector<uint64_t> GetFilters(uint64_t uid);
		static void CopyFiltersTo(uint64_t inputIdFrom, uint64_t inputIdTo);
		static int64_t GetDuration(uint64_t uid);
		static int64_t GetTime(uint64_t uid);
		static int64_t SetTime(uint64_t uid, int64_t time);
		static void Play(uint64_t uid);
		static void Pause(uint64_t uid);
		static void Restart(uint64_t uid);
		static void Stop(uint64_t uid);
		static uint64_t GetMediaState(uint64_t uid);
	};
}