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
#include "utility.hpp"
#include <ipc-server.hpp>
#include <obs.h>

namespace osn {
	class Source {
	#pragma region Singleton
		public:
		class SingletonObjectManager {
			friend class osn::Source;
			friend class std::shared_ptr<SingletonObjectManager>;

			utility::unique_id idGenerator;
			std::map<uint64_t, obs_source_t*> objectMap;

			protected:
			SingletonObjectManager();
			~SingletonObjectManager();

			public:
			SingletonObjectManager(SingletonObjectManager const&) = delete;
			SingletonObjectManager operator=(SingletonObjectManager const&) = delete;

			uint64_t Allocate(obs_source_t* obj) {
				uint64_t id = idGenerator.allocate();
				objectMap.insert_or_assign(id, obj);
				return id;
			}
			obs_source_t* Free(uint64_t id) {
				obs_source_t* obj = nullptr;
				auto iter = objectMap.find(id);
				if (iter != objectMap.end()) {
					obj = iter->second;
				}
				objectMap.erase(iter);
				idGenerator.free(id);
				return obj;
			}
			obs_source_t* Get(uint64_t id) {
				obs_source_t* obj = nullptr;
				auto iter = objectMap.find(id);
				if (iter != objectMap.end()) {
					obj = iter->second;
				}
				return obj;
			}
			uint64_t Get(obs_source_t* obj) {
				for (auto kv : objectMap) {
					if (kv.second == obj)
						return kv.first;
				}
				return UINT64_MAX;
			}
		};

		public:
		static bool Initialize();
		static bool Finalize();
		static SingletonObjectManager* GetInstance();
	#pragma endregion Singleton

		public:
		static void Register(IPC::Server&);

		// References
		static void Remove(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void Release(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		// Settings & Properties
		static void IsConfigurable(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void Update(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void Load(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void Save(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		static void GetType(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetOutputFlags(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetFlags(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetFlags(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetStatus(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetId(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);

		// Flags
		static void GetMuted(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetMuted(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void GetEnabled(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
		static void SetEnabled(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval);
	};
}
