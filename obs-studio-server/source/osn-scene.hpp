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
	class Scene : Source {
		public:
		static void Register(ipc::server&);

		static void Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void CreatePrivate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void FromName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

		static void AsSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void Duplicate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

		static void AddSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void FindItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void MoveItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void GetItem(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void GetItems(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void GetItemsInRange(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

		// Signals?
		static void Connect(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void Disconnect(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	};
}
