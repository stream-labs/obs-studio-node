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

void osn::Input::Register(IPC::Server& srv) {
	std::shared_ptr<IPC::Class> cls = std::make_shared<IPC::Class>("Filter");
	cls->RegisterFunction(std::make_shared<IPC::Function>("Types", std::vector<IPC::Type>{}, Types));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Create", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String}, Create));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Create", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String, IPC::Type::String}, Create));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Create", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String, IPC::Type::String, IPC::Type::String}, Create));
	cls->RegisterFunction(std::make_shared<IPC::Function>("CreatePrivate", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String}, CreatePrivate));
	cls->RegisterFunction(std::make_shared<IPC::Function>("CreatePrivate", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String, IPC::Type::String}, CreatePrivate));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Duplicate", std::vector<IPC::Type>{IPC::Type::UInt64}, Duplicate));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Duplicate", std::vector<IPC::Type>{IPC::Type::UInt64, IPC::Type::String}, Duplicate));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Duplicate", std::vector<IPC::Type>{IPC::Type::UInt64, IPC::Type::String, IPC::Type::Int32}, Duplicate));
	cls->RegisterFunction(std::make_shared<IPC::Function>("FromName", std::vector<IPC::Type>{IPC::Type::String}, FromName));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetPublicSources", std::vector<IPC::Type>{}, GetPublicSources));
	srv.RegisterClass(cls);
}

void osn::Input::Types(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	const char* typeId = nullptr;
	for (size_t idx = 0; obs_enum_input_types(idx, &typeId); idx++) {
		rval.push_back(IPC::Value(typeId ? typeId : ""));
	}
	return;
}

void osn::Input::Create(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
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
		rval.push_back(IPC::Value("Failed to create input."));
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

void osn::Input::CreatePrivate(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
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
		rval.push_back(IPC::Value("Failed to create input."));
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

void osn::Input::Duplicate(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* filter = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!filter) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	bool isPrivate = false;
	const char* nameOverride = nullptr;

	obs_source_t* source = nullptr;
	if (isPrivate) {
		source = obs_source_duplicate(filter, nameOverride, true);
	} else {
		source = obs_source_duplicate(filter, nameOverride, false);
	}

	if (!source) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::Error));
		rval.push_back(IPC::Value("Failed to duplicate input."));
		return;
	}

	if (source != filter) {
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
	} else {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
		rval.push_back(IPC::Value(args[0].value.ui64));
		return;
	}
}

void osn::Input::FromName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* source = obs_get_source_by_name(args[0].value_str.c_str());
	if (!source) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::NotFound));
		rval.push_back(IPC::Value("Named input could not be found."));
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

void osn::Input::GetPublicSources(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	std::list<uint64_t> inputs;

	auto enum_cb = [](void *data, obs_source_t *source) {
		uint64_t uid = osn::Source::GetInstance()->Get(source);
		if (uid != UINT64_MAX) {
			static_cast<std::list<uint64_t>*>(data)->push_back(uid);
		}
		return true;
	};

	obs_enum_sources(enum_cb, &inputs);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	for (uint64_t uid : inputs) {
		rval.push_back(uid);
	}
	return;
}

void osn::Input::GetActive(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_active(input)));
	return;
}

void osn::Input::GetShowing(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_showing(input)));
	return;
}

void osn::Input::GetVolume(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_volume(input)));
	return;
}

void osn::Input::SetVolume(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	obs_source_set_volume(input, (float_t)args[1].value.fp64);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_volume(input)));
	return;
}

void osn::Input::GetSyncOffset(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_sync_offset(input)));
	return;
}

void osn::Input::SetSyncOffset(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	obs_source_set_sync_offset(input, args[1].value.i64);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_sync_offset(input)));
	return;
}

void osn::Input::GetAudioMixers(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_audio_mixers(input)));
	return;
}

void osn::Input::SetAudioMixers(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	obs_source_set_audio_mixers(input, (obs_monitoring_type)args[1].value.i64);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_audio_mixers(input)));
	return;
}

void osn::Input::GetMonitoringType(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_monitoring_type(input)));
	return;
}

void osn::Input::SetMonitoringType(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	obs_source_set_monitoring_type(input, (obs_monitoring_type)args[1].value.i64);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_monitoring_type(input)));
	return;
}

void osn::Input::GetWidth(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_width(input)));
	return;
}

void osn::Input::GetHeight(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_height(input)));
	return;
}

void osn::Input::GetDeInterlaceFieldOrder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_deinterlace_field_order(input)));
	return;
}

void osn::Input::SetDeInterlaceFieldOrder(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	obs_source_set_deinterlace_field_order(input, (obs_deinterlace_field_order)args[1].value.i64);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_deinterlace_field_order(input)));
	return;
}

void osn::Input::GetDeInterlaceMode(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_deinterlace_mode(input)));
	return;
}

void osn::Input::SetDeInterlaceMode(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	obs_source_t* input = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (!input) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Input reference is not valid."));
		return;
	}

	obs_source_set_deinterlace_mode(input, (obs_deinterlace_mode)args[1].value.i64);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_deinterlace_mode(input)));
	return;
}

void osn::Input::AddFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}

void osn::Input::RemoveFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}

void osn::Input::FindFilter(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}

void osn::Input::GetFilters(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}

void osn::Input::CopyFiltersTo(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {

}
