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
#include "osn-common.hpp"
#include "error.hpp"
#include <ipc-server.hpp>
#include <ipc-class.hpp>
#include <ipc-function.hpp>
#include <ipc-value.hpp>
#include <map>
#include <memory>
#include <obs.h>

static osn::Source::SingletonObjectManager *somInstance;

bool osn::Source::Initialize() {
	if (somInstance)
		return false;
	somInstance = new SingletonObjectManager();
	return true;
}

bool osn::Source::Finalize() {
	if (!somInstance)
		return false;
	delete somInstance;
	somInstance = nullptr;
	return true;

}

osn::Source::SingletonObjectManager* osn::Source::GetInstance() {
	return somInstance;

}

void osn::Source::Register(IPC::Server& srv) {
	std::shared_ptr<IPC::Class> cls = std::make_shared<IPC::Class>("Source");
	cls->RegisterFunction(std::make_shared<IPC::Function>("Remove", std::vector<IPC::Type>{IPC::Type::UInt64}, Remove));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Release", std::vector<IPC::Type>{IPC::Type::UInt64}, Release));
	cls->RegisterFunction(std::make_shared<IPC::Function>("IsConfigurable", std::vector<IPC::Type>{IPC::Type::UInt64}, IsConfigurable));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetProperties", std::vector<IPC::Type>{IPC::Type::UInt64}, GetProperties));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetSettings", std::vector<IPC::Type>{IPC::Type::UInt64}, GetSettings));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Load", std::vector<IPC::Type>{IPC::Type::UInt64}, Load));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Save", std::vector<IPC::Type>{IPC::Type::UInt64}, Save));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Update", std::vector<IPC::Type>{IPC::Type::UInt64}, Update));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetType", std::vector<IPC::Type>{IPC::Type::UInt64}, GetType));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetName", std::vector<IPC::Type>{IPC::Type::UInt64}, GetName));
	cls->RegisterFunction(std::make_shared<IPC::Function>("SetName", std::vector<IPC::Type>{IPC::Type::UInt64}, SetName));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetOutputFlags", std::vector<IPC::Type>{IPC::Type::UInt64}, GetOutputFlags));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetFlags", std::vector<IPC::Type>{IPC::Type::UInt64}, GetFlags));
	cls->RegisterFunction(std::make_shared<IPC::Function>("SetFlags", std::vector<IPC::Type>{IPC::Type::UInt64}, SetFlags));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetStatus", std::vector<IPC::Type>{IPC::Type::UInt64}, GetStatus));
	cls->RegisterFunction(std::make_shared<IPC::Function>("GetId", std::vector<IPC::Type>{IPC::Type::UInt64}, GetId));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Muted", std::vector<IPC::Type>{IPC::Type::UInt64}, Muted));
	cls->RegisterFunction(std::make_shared<IPC::Function>("Enabled", std::vector<IPC::Type>{IPC::Type::UInt64}, Enabled));
	srv.RegisterClass(cls);
}

void osn::Source::Remove(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_source_remove(src);
	osn::Source::GetInstance()->Free(args[0].value.ui64);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::Release(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_source_release(src);
	if (obs_source_removed(src)) {
		osn::Source::GetInstance()->Free(args[0].value.ui64);
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::IsConfigurable(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_configurable(src)));
	return;
}

void osn::Source::GetProperties(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_properties_t* prp = obs_source_properties(src);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	for (obs_property_t* p = obs_properties_first(prp); (p != nullptr) && obs_property_next(&p);) {
		rval.push_back(IPC::Value(obs_property_name(p)));
		rval.push_back(IPC::Value(obs_property_description(p)));
		rval.push_back(IPC::Value(obs_property_long_description(p)));
		rval.push_back(IPC::Value(obs_property_enabled(p)));
		rval.push_back(IPC::Value(obs_property_visible(p)));

		obs_property_type pt = obs_property_get_type(p);
		rval.push_back(IPC::Value(pt));
		switch (pt) {
			case OBS_PROPERTY_FLOAT:
				rval.push_back(IPC::Value(obs_property_float_min(p)));
				rval.push_back(IPC::Value(obs_property_float_max(p)));
				rval.push_back(IPC::Value(obs_property_float_step(p)));
				rval.push_back(IPC::Value(obs_property_float_type(p)));
				break;
			case OBS_PROPERTY_INT:
				rval.push_back(IPC::Value(obs_property_int_min(p)));
				rval.push_back(IPC::Value(obs_property_int_max(p)));
				rval.push_back(IPC::Value(obs_property_int_step(p)));
				rval.push_back(IPC::Value(obs_property_int_type(p)));
				break;
			case OBS_PROPERTY_TEXT:
				rval.push_back(IPC::Value(obs_proprety_text_type(p)));
				break;
			case OBS_PROPERTY_PATH:
				rval.push_back(IPC::Value(obs_property_path_type(p)));
				rval.push_back(IPC::Value(obs_property_path_filter(p)));
				rval.push_back(IPC::Value(obs_property_path_default_path(p)));
				break;
			case OBS_PROPERTY_LIST:
			case OBS_PROPERTY_EDITABLE_LIST:
				rval.push_back(IPC::Value(obs_property_list_type(p)));
				rval.push_back(IPC::Value(obs_property_list_format(p)));
				break;
		}
	}
	return;
}

void osn::Source::GetSettings(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_data_t* sets = obs_source_get_settings(src);
	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_data_get_json(sets)));
	obs_data_release(sets);
	return;
}

void osn::Source::Update(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_data_t* sets = obs_data_create_from_json(args[1].value_str.c_str());
	obs_source_update(src, sets);
	obs_data_release(sets);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::Load(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_source_load(src);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::Save(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_source_save(src);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	return;
}

void osn::Source::GetType(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_type(src)));
	return;
}

void osn::Source::GetName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_name(src)));
	return;
}

void osn::Source::SetName(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_source_set_name(src, args[1].value_str.c_str());

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_name(src)));
	return;
}

void osn::Source::GetOutputFlags(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_output_flags(src)));
	return;
}

void osn::Source::GetFlags(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_flags(src)));
	return;
}

void osn::Source::SetFlags(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	obs_source_set_flags(src, args[1].value.ui32);

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_get_flags(src)));
	return;
}

void osn::Source::GetStatus(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value((uint64_t)true));
	return;
}

void osn::Source::GetId(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	const char* sid = obs_source_get_id(src);
	rval.push_back(IPC::Value(sid ? sid : ""));
	return;
}

void osn::Source::Muted(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_muted(src)));
	return;
}

void osn::Source::Enabled(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	// Attempt to find the source asked to load.
	obs_source_t* src = osn::Source::GetInstance()->Get(args[0].value.ui64);
	if (src == nullptr) {
		rval.push_back(IPC::Value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(IPC::Value("Source reference is not valid."));
		return;
	}

	rval.push_back(IPC::Value((uint64_t)ErrorCode::Ok));
	rval.push_back(IPC::Value(obs_source_enabled(src)));
	return;	
}

osn::Source::SingletonObjectManager::SingletonObjectManager() {

}

osn::Source::SingletonObjectManager::~SingletonObjectManager() {

}
