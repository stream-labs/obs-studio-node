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
#include <map>
#include <obs.h>

namespace osn {
	class SourceManager {
		utility::unique_id sourceIdGenerator;
		std::map<uint64_t, obs_source_t*> sourceMap;

	#pragma region Singleton
		private:
		SourceManager() {};
		~SourceManager() {};

		public:
		SourceManager(SourceManager&) = delete;
		SourceManager(SourceManager const&) = delete;
		SourceManager operator=(SourceManager&) = delete;
		SourceManager operator=(SourceManager const&) = delete;

		static bool Initialize();
		static SourceManager* GetInstance();
		static bool Finalize();
	#pragma endregion Singleton

		public:
		uint64_t Allocate(obs_source_t*);
		obs_source_t* Free(uint64_t);

		obs_source_t* Get(uint64_t);
		uint64_t Get(obs_source_t*);
	};
	
	class Source {
	};
}
