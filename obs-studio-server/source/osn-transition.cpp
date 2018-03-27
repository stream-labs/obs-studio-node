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

#include "osn-transition.hpp"
#include "osn-source.hpp"
#include "error.hpp"
#include <ipc-server.hpp>
#include <obs.h>
#include <memory>

void osn::Transition::Register(IPC::Server& srv) {
	std::shared_ptr<IPC::Class> cls = std::make_shared<IPC::Class>("Transition");
	cls->RegisterFunction(std::make_shared<IPC::Function>("Types", std::vector<IPC::Type>{}, Types));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Create", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String}, Create));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Create", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String, IPC::Type::String}, Create));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Create", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String, IPC::Type::String, IPC::Type::String}, Create));
	cls->RegisterFunction(std::make_shared<IPC::Function>("CreatePrivate", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String}, CreatePrivate));
	cls->RegisterFunction(std::make_shared<IPC::Function>("CreatePrivate", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String, IPC::Type::String}, CreatePrivate));
	cls->RegisterFunction(std::make_shared<IPC::Function>("FromName", std::vector<IPC::Type>{IPC::Type::UInt64}, FromName));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetActiveSource", std::vector<IPC::Type>{IPC::Type::UInt64}, GetActiveSource));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Clear", std::vector<IPC::Type>{IPC::Type::UInt64}, Clear));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Set", std::vector<IPC::Type>{IPC::Type::UInt64}, Set));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Start", std::vector<IPC::Type>{IPC::Type::UInt64, IPC::Type::UInt32, IPC::Type::UInt64}, Start));
	srv.RegisterClass(cls);
}

void osn::Transition::Types(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	const char* typeId = nullptr;
	for (size_t idx = 0; obs_enum_transition_types(idx, &typeId); idx++) {
		rval.push_back(IPC::Value(typeId ? typeId : ""));
	}
	return;
}

void osn::Transition::Create(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	std::string sourceId, name;
	obs_data_t *settings = nullptr, *hotkeys = nullptr;

	switch (args.size()) {
		case 4:
			hotkeys = obs_data_create_from_json(args[3].value_str.c_str());
		case 3:
			settings = obs_data_create_from_json(args[2].value_str.c_str());
		case 2:
			name = args[1].value_str;
			sourceId = args[0].value_str;
			break;
	}

	obs_source_t* source = obs_source_create(sourceId.c_str(), name.c_str(), settings, hotkeys);
	if (!source) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::Error));
		rval.push_back(IPC::Value("Failed to create transition."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		// No further Ids left, leak somewhere.
		rval.push_back(IPC::Value((uint64_t)ErrorCode::OutOfIndexes));
		rval.push_back(IPC::Value("Index list is full."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(uid));
	return;
}

void osn::Transition::CreatePrivate(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	std::string sourceId, name;
	obs_data_t *settings = nullptr;

	switch (args.size()) {
		case 3:
			settings = obs_data_create_from_json(args[2].value_str.c_str());
		case 2:
			name = args[1].value_str;
			sourceId = args[0].value_str;
			break;
	}

	obs_source_t* source = obs_source_create_private(sourceId.c_str(), name.c_str(), settings);
	if (!source) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::Error));
		rval.push_back(IPC::Value("Failed to create transition."));
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		// No further Ids left, leak somewhere.
		rval.push_back(IPC::Value((uint64_t)ErrorCode::OutOfIndexes));
		rval.push_back(IPC::Value("Index list is full."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(uid));
	return;
}

void osn::Transition::FromName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* source = obs_get_source_by_name(args[0].value_str.c_str());
	if (!source) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::NotFound));
		rval.push_back(IPC::Value("Named transition could not be found."));
		return;
	}
	
	uint64_t uid = osn::Source::GetInstance()->Get(source);
	if (uid == UINT64_MAX) {
		// This is an impossible case, but we handle it in case it happens.
		obs_source_release(source);
	#ifdef DEBUG // Debug should throw an error for debuggers to catch.
		throw std::runtime_error("Source found but not indexed.");
	#endif
		rval.push_back(IPC::Value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(IPC::Value("Source found but not indexed."));
		return;
	}

	obs_source_release(source);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(uid));
	return;
}

void osn::Transition::GetActiveSource(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	uint64_t uid = -1;

	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!transition) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Transition reference is not valid."));
		return;
	}
	
	obs_source_t *source = obs_transition_get_active_source(transition);
	if (source) {
		uid = osn::Source::GetInstance()->Get(source);
		obs_source_release(source);
	}
	
	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(uid));
	return;
}

void osn::Transition::Clear(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!transition) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Transition reference is not valid."));
		return;
	}

	obs_transition_clear(transition);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Transition::Set(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!transition) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Transition reference is not valid."));
		return;
	}

	obs_source_t* source = osn::Source::GetInstance()->Get(args[1].value.ui64);
	if (!source) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_transition_set(transition, source);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Transition::Start(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!transition) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Transition reference is not valid."));
		return;
	}

	obs_source_t* source = osn::Source::GetInstance()->Get(args[2].value.ui64);
	if (!source) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	uint32_t ms = args[1].value.ui32;

	bool result = obs_transition_start(transition, OBS_TRANSITION_MODE_AUTO, ms, source);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(result));
	return;
}
