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

#include "osn-Input.hpp"
#include "osn-source.hpp"
#include "error.hpp"
#include <ipc-server.hpp>
#include <obs.h>
#include <memory>
#include "shared.hpp"
#include <iostream>

void osn::Input::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Input");
	cls->register_function(std::make_shared<ipc::function>("Types", std::vector<ipc::type>{}, Types));
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String, ipc::type::String}, Create));
	cls->register_function(std::make_shared<ipc::function>("CreatePrivate", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, CreatePrivate));
	cls->register_function(std::make_shared<ipc::function>("CreatePrivate", std::vector<ipc::type>{ipc::type::String, ipc::type::String, ipc::type::String}, CreatePrivate));
	cls->register_function(std::make_shared<ipc::function>("FromName", std::vector<ipc::type>{ipc::type::String}, FromName));
	cls->register_function(std::make_shared<ipc::function>("GetPublicSources", std::vector<ipc::type>{}, GetPublicSources));

	cls->register_function(std::make_shared<ipc::function>("Duplicate", std::vector<ipc::type>{ipc::type::UInt64}, Duplicate));
	cls->register_function(std::make_shared<ipc::function>("Duplicate", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String}, Duplicate));
	cls->register_function(std::make_shared<ipc::function>("Duplicate", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String, ipc::type::Int32}, Duplicate));
	cls->register_function(std::make_shared<ipc::function>("GetActive", std::vector<ipc::type>{ipc::type::UInt64}, GetActive));
	cls->register_function(std::make_shared<ipc::function>("GetShowing", std::vector<ipc::type>{ipc::type::UInt64}, GetShowing));
	cls->register_function(std::make_shared<ipc::function>("GetWidth", std::vector<ipc::type>{ipc::type::UInt64}, GetWidth));
	cls->register_function(std::make_shared<ipc::function>("GetHeight", std::vector<ipc::type>{ipc::type::UInt64}, GetHeight));
	cls->register_function(std::make_shared<ipc::function>("GetVolume", std::vector<ipc::type>{ipc::type::UInt64}, GetVolume));
	cls->register_function(std::make_shared<ipc::function>("SetVolume", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Float}, SetVolume));
	cls->register_function(std::make_shared<ipc::function>("GetSyncOffset", std::vector<ipc::type>{ipc::type::UInt64}, GetSyncOffset));
	cls->register_function(std::make_shared<ipc::function>("SetSyncOffset", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int64}, SetSyncOffset));
	cls->register_function(std::make_shared<ipc::function>("GetAudioMixers", std::vector<ipc::type>{ipc::type::UInt64}, GetAudioMixers));
	cls->register_function(std::make_shared<ipc::function>("SetAudioMixers", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, SetAudioMixers));
	cls->register_function(std::make_shared<ipc::function>("GetMonitoringType", std::vector<ipc::type>{ipc::type::UInt64}, GetMonitoringType));
	cls->register_function(std::make_shared<ipc::function>("SetMonitoringType", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetMonitoringType));
	cls->register_function(std::make_shared<ipc::function>("GetDeInterlaceFieldOrder", std::vector<ipc::type>{ipc::type::UInt64}, GetDeInterlaceFieldOrder));
	cls->register_function(std::make_shared<ipc::function>("SetDeInterlaceFieldOrder", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, SetDeInterlaceFieldOrder));
	cls->register_function(std::make_shared<ipc::function>("GetDeInterlaceMode", std::vector<ipc::type>{ipc::type::UInt64}, GetDeInterlaceMode));
	cls->register_function(std::make_shared<ipc::function>("SetDeInterlaceMode", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::Int32}, GetDeInterlaceMode));

	cls->register_function(std::make_shared<ipc::function>("GetFilters", std::vector<ipc::type>{ipc::type::UInt64}, GetFilters));
	cls->register_function(std::make_shared<ipc::function>("AddFilter", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, AddFilter));
	cls->register_function(std::make_shared<ipc::function>("RemoveFilter", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, RemoveFilter));
	cls->register_function(std::make_shared<ipc::function>("MoveFilter", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64, ipc::type::UInt32}, MoveFilter));
	cls->register_function(std::make_shared<ipc::function>("FindFilter", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String}, FindFilter));
	cls->register_function(std::make_shared<ipc::function>("CopyFiltersTo", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, CopyFiltersTo));

	srv.register_collection(cls);
}

void osn::Input::Types(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	const char* typeId = nullptr;
	for (size_t idx = 0; obs_enum_input_types(idx, &typeId); idx++) {
		rval.push_back(ipc::value(typeId ? typeId : ""));
	}
	AUTO_DEBUG;
}

void osn::Input::Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	std::string sourceId, name;
	obs_data_t *settings = nullptr, *hotkeys = nullptr;

	switch (args.size()) {
		case 4:
			// hotkeys = obs_data_create_from_json(args[3].value_str.c_str());
		case 3:
			// settings = obs_data_create_from_json(args[2].value_str.c_str());/**/
		case 2:
			name = args[1].value_str;
			sourceId = args[0].value_str;
			break;
	}

	obs_source_t* source = obs_source_create(sourceId.c_str(), name.c_str(), nullptr, nullptr);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create input."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::Manager::GetInstance().allocate(source);
	if (uid == UINT64_MAX) {
		// No further Ids left, leak somewhere.
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Input::CreatePrivate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
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
		rval.push_back(ipc::value("Failed to create input."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::Manager::GetInstance().allocate(source);
	if (uid == UINT64_MAX) {
		// No further Ids left, leak somewhere.
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Index list is full."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Input::Duplicate(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* filter = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!filter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	bool isPrivate = false;
	const char* nameOverride = nullptr;

	obs_source_t* source = nullptr;
	source = obs_source_duplicate(filter, nameOverride, isPrivate);

	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to duplicate input."));
		AUTO_DEBUG;
		return;
	}

	if (source != filter) {
		uint64_t uid = osn::Source::Manager::GetInstance().allocate(source);
		if (uid == UINT64_MAX) {
			// No further Ids left, leak somewhere.
			rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
			rval.push_back(ipc::value("Index list is full."));
			AUTO_DEBUG;
			return;
		}

		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		rval.push_back(ipc::value(uid));
	} else {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		rval.push_back(ipc::value(args[0].value_union.ui64));
	}
	AUTO_DEBUG;
}

void osn::Input::FromName(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* source = obs_get_source_by_name(args[0].value_str.c_str());
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::NotFound));
		rval.push_back(ipc::value("Named input could not be found."));
		AUTO_DEBUG;
		return;
	}

	uint64_t uid = osn::Source::Manager::GetInstance().find(source);
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

void osn::Input::GetPublicSources(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	std::list<uint64_t> inputs;

	// !FIXME! Optimize for zero-copy operation, can directly write to rval.
	auto enum_cb = [](void *data, obs_source_t *source) {
		uint64_t uid = osn::Source::Manager::GetInstance().find(source);
		if (uid != UINT64_MAX) {
			static_cast<std::list<uint64_t>*>(data)->push_back(uid);
		}
		return true;
	};

	obs_enum_sources(enum_cb, &inputs);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	for (uint64_t uid : inputs) {
		rval.push_back(uid);
	}
	AUTO_DEBUG;
}

void osn::Input::GetActive(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_active(input)));
	AUTO_DEBUG;
}

void osn::Input::GetShowing(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_showing(input)));
	AUTO_DEBUG;
}

void osn::Input::GetVolume(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_volume(input)));
	AUTO_DEBUG;
}

void osn::Input::SetVolume(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_set_volume(input, (float_t)args[1].value_union.fp64);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_volume(input)));
	AUTO_DEBUG;
}

void osn::Input::GetSyncOffset(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_sync_offset(input)));
	AUTO_DEBUG;
}

void osn::Input::SetSyncOffset(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_set_sync_offset(input, args[1].value_union.i64);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_sync_offset(input)));
	AUTO_DEBUG;
}

void osn::Input::GetAudioMixers(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_audio_mixers(input)));
	AUTO_DEBUG;
}

void osn::Input::SetAudioMixers(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_set_audio_mixers(input, (obs_monitoring_type)args[1].value_union.ui32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_audio_mixers(input)));
	AUTO_DEBUG;
}

void osn::Input::GetMonitoringType(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_monitoring_type(input)));
	AUTO_DEBUG;
}

void osn::Input::SetMonitoringType(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_set_monitoring_type(input, (obs_monitoring_type)args[1].value_union.ui32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_monitoring_type(input)));
	AUTO_DEBUG;
}

void osn::Input::GetWidth(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_width(input)));
	AUTO_DEBUG;
}

void osn::Input::GetHeight(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_height(input)));
	AUTO_DEBUG;
}

void osn::Input::GetDeInterlaceFieldOrder(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_deinterlace_field_order(input)));
	AUTO_DEBUG;
}

void osn::Input::SetDeInterlaceFieldOrder(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_set_deinterlace_field_order(input, (obs_deinterlace_field_order)args[1].value_union.ui32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_deinterlace_field_order(input)));
	AUTO_DEBUG;
}

void osn::Input::GetDeInterlaceMode(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_deinterlace_mode(input)));
	AUTO_DEBUG;
}

void osn::Input::SetDeInterlaceMode(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_set_deinterlace_mode(input, (obs_deinterlace_mode)args[1].value_union.ui32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_source_get_deinterlace_mode(input)));
	AUTO_DEBUG;
}

void osn::Input::AddFilter(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* filter = osn::Source::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!filter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Filter reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_filter_add(input, filter);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Input::RemoveFilter(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* filter = osn::Source::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!filter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Filter reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_filter_remove(input, filter);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Input::MoveFilter(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}
	
	obs_source_t* filter = osn::Source::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!filter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Filter reference is not valid."));
		AUTO_DEBUG;
		return;
	}
	
	obs_order_movement movement = (obs_order_movement)args[2].value_union.ui32;
	
	obs_source_filter_set_order(input, filter, movement);
	
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::Input::FindFilter(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* filter = obs_source_get_filter_by_name(input, args[1].value_str.c_str());
	if (!filter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		AUTO_DEBUG;
		return;
	}
	obs_source_release(filter);

	uint64_t uid = osn::Source::Manager::GetInstance().find(filter);
	if (uid == UINT64_MAX) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Filter found but not indexed."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::Input::GetFilters(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	auto enum_cb = [](obs_source_t *parent, obs_source_t *filter, void *data) {
		std::vector<ipc::value> *rval = reinterpret_cast<std::vector<ipc::value>*>(data);

		uint64_t id = osn::Source::Manager::GetInstance().find(filter);
		if (id != UINT64_MAX) {
			rval->push_back(id);
		}
	};

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	obs_source_enum_filters(input, enum_cb, &rval);
	AUTO_DEBUG;
}

void osn::Input::CopyFiltersTo(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_source_t* input_from = osn::Source::Manager::GetInstance().find(args[0].value_union.ui64);
	if (!input_from) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("1st Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_t* input_to = osn::Source::Manager::GetInstance().find(args[1].value_union.ui64);
	if (!input_to) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("2nd Input reference is not valid."));
		AUTO_DEBUG;
		return;
	}

	obs_source_copy_filters(input_to, input_from);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}
