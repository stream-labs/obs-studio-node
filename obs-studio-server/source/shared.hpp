// Server program for the OBS Studio node module.
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
#include "osn-scene.hpp"
#include <list>
#include <memory>
#include <mutex>

namespace shared {
	template<class T>
	class SingletonObjectManager {
		utility::unique_id idGenerator;
		std::map<uint64_t, T> objectMap;

		private:
		SingletonObjectManager() {};
		~SingletonObjectManager() {};

		static std::shared_ptr<SingletonObjectManager> _inst = nullptr;

		public:
		SingletonObjectManager(SingletonObjectManager const&) = delete;
		SingletonObjectManager operator=(SingletonObjectManager const&) = delete;

		static bool Initialize() {
			if (_inst)
				return false;
			_inst = std::make_shared<SingletonObjectManager>();
			return true;
		}
		static std::shared_ptr<SingletonObjectManager> GetInstance() {
			return _inst;
		}
		static bool Finalize() {
			if (!_inst)
				return false;
			_inst = nullptr;
			return true;
		}

		public:
		uint64_t Allocate(T obj) {
			uint64_t id = idGenerator.allocate();
			objectMap.insert_or_assign(id, obj);
			return id;
		}
		T Free(uint64_t id) {
			T obj = nullptr;
			auto iter = objectMap.find(id);
			if (iter != objectMap.end()) {
				obj = iter->second;
			}
			objectMap.erase(iter);
			idGenerator.free(id);
			return obj;
		}
		T Get(uint64_t id) {
			T obj = nullptr;
			auto iter = objectMap.find(id);
			if (iter != objectMap.end()) {
				obj = iter->second;
			}
			return obj;
		}
		uint64_t Get(T obj) {
			for (auto kv : objectMap) {
				if (kv.second == obj)
					return kv.first;
			}
			return UINT64_MAX;
		}
	};
}