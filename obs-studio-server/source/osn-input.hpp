// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#pragma once
#include "osn-source.hpp"

namespace osn {
	class Input : Source {
		public:
		static void Register(IPC::Server&);

		// Function
		static void Types(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void Create(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CreatePrivate(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void Duplicate(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FromName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetPublicSources(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		// Methods
		/// Status
		static void GetActive(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetShowing(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		/// Audio
		static void GetVolume(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetVolume(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSyncOffset(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetSyncOffset(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetAudioMixers(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetAudioMixers(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetMonitoringType(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetMonitoringType(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		/// Video
		static void GetWidth(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetHeight(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetDeInterlaceFieldOrder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetDeInterlaceFieldOrder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetDeInterlaceMode(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetDeInterlaceMode(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		/// Filters
		static void AddFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void RemoveFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void FindFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetFilters(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void CopyFiltersTo(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

	};
}