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

#include "osn-source.hpp"
#include <mutex>

#pragma region Singleton
static osn::SourceManager* sourceManagerInst;
static std::mutex sourceManagerInstMutex;

bool osn::SourceManager::Initialize() {
	std::unique_lock<std::mutex> ulock(sourceManagerInstMutex);
	if (!sourceManagerInst) {
		sourceManagerInst = new osn::SourceManager();
	}
}

osn::SourceManager* osn::SourceManager::GetInstance() {
	if (sourceManagerInstMutex.try_lock()) {
		sourceManagerInstMutex.unlock();
		return sourceManagerInst;
	}
	return nullptr;
}

bool osn::SourceManager::Finalize() {
	std::unique_lock<std::mutex> ulock(sourceManagerInstMutex);
	delete sourceManagerInst;
	sourceManagerInst = nullptr;
}
#pragma endregion Singleton

uint64_t osn::SourceManager::Allocate(obs_source_t* ptr) {
	uint64_t idx = sourceIdGenerator.allocate();
	if (idx == UINT64_MAX) {
		throw std::runtime_error("Failed to allocate unique id.");
	}

	sourceMap.insert_or_assign(idx, ptr);
	return idx;
}

obs_source_t* osn::SourceManager::Free(uint64_t v) {
	auto iter = sourceMap.find(v);
	if (iter != sourceMap.end()) {
		return nullptr;
	}
	
	obs_source_t* ptr = iter->second;
	sourceIdGenerator.free(iter->first);
	sourceMap.erase(iter);

	return ptr;
}

obs_source_t* osn::SourceManager::Get(uint64_t v) {
	auto iter = sourceMap.find(v);
	if (iter != sourceMap.end()) {
		return nullptr;
	}

	return iter->second;
}

uint64_t osn::SourceManager::Get(obs_source_t* ptr) {
	//sourceMap.find(ptr);
	return UINT64_MAX;
}
