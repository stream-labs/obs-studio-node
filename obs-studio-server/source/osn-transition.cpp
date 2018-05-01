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
#include "shared.hpp"

void osn::Transition::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Transition");
	cls->register_function(std::make_shared<ipc::function>("Types", std::vector<ipc::type>{}, Types));
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("CreatePrivate", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, CreatePrivate));
	cls->register_function(std::make_shared<ipc::function>("CreatePrivate", std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String}, CreatePrivate));
	cls->register_function(std::make_shared<ipc::function>("FromName", std::vector<ipc::type>{ipc::type::UInt64}, FromName));
	cls->register_function(std::make_shared<ipc::function>("GetActiveSource", std::vector<ipc::type>{ipc::type::UInt64}, GetActiveSource));
	cls->register_function(std::make_shared<ipc::function>("Clear", std::vector<ipc::type>{ipc::type::UInt64}, Clear));
	cls->register_function(std::make_shared<ipc::function>("Set", std::vector<ipc::type>{ipc::type::UInt64}, Set));
	cls->register_function(std::make_shared<ipc::function>("Start", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32, ipc::type::UInt64}, Start));
	srv.register_collection(cls);
}

void osn::Transition::Types(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	const char* typeId = nullptr;
	for (size_t idx = 0; obs_enum_transition_types(idx, &typeId); idx++) {
		rval.push_back(ipc::value(typeId ? typeId : ""));
	}
	AUTO_DEBUG;
}

void osn::Transition::Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
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
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create transition."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		// No further Ids left, leak somewhere.
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfIndexes));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Transition::CreatePrivate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
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
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create transition."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::GetInstance()->Allocate(source);
	if (uid == UINT64_MAX) {
		// No further Ids left, leak somewhere.
		rval.push_back(ipc::value((uint64_t)ErrorCode::OutOfIndexes));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Transition::FromName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = obs_get_source_by_name(args[0].value_str.c_str());
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::NotFound));
		rval.push_back(ipc::value("Named transition could not be found."));
		AUTO_DEBUG;
		return;
	}
	
	uint64_t uid = osn::Source::GetInstance()->Get(source);
	if (uid == UINT64_MAX) {
		// This is an impossible case, but we handle it in case it happens.
		obs_source_release(source);
	#ifdef DEBUG // Debug should throw an error for debuggers to catch.
		throw std::runtime_error("Source found but not indexed.");
	#endif
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Source found but not indexed."));
		AUTO_DEBUG;
		return;
	}

	obs_source_release(source);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Transition::GetActiveSource(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	uint64_t uid = -1;

	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!transition) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Transition reference is not valid."));
		AUTO_DEBUG;
		return;
	}
	
	obs_source_type type = OBS_SOURCE_TYPE_INPUT;
	obs_source_t *source = obs_transition_get_active_source(transition);
	if (source) {
		uid = osn::Source::GetInstance()->Get(source);
		type = obs_source_get_type(source);
		obs_source_release(source);
	}

	if (uid == UINT64_MAX) {
	#ifdef DEBUG // Debug should throw an error for debuggers to catch.
		throw std::runtime_error("Source found but not indexed.");
	#endif
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Source found but not indexed."));
		AUTO_DEBUG;
		return;
	}
	
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	rval.push_back(ipc::value(type));
	AUTO_DEBUG;
}

void osn::Transition::Clear(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!transition) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Transition reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_transition_clear(transition);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Transition::Set(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!transition) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Transition reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* source = osn::Source::GetInstance()->Get(args[1].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_transition_set(transition, source);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Transition::Start(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* transition = osn::Source::GetInstance()->Get(args[0].value_union.ui64);
	if (!transition) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Transition reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* source = osn::Source::GetInstance()->Get(args[2].value_union.ui64);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Source reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	uint32_t ms = args[1].value_union.ui32;

	bool result = obs_transition_start(transition, OBS_TRANSITION_MODE_AUTO, ms, source);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(result));
	AUTO_DEBUG;
}
