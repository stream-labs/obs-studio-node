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

#include "osn-Properties.hpp"
#include "obs.h"
#include "osn-source.hpp"
#include "shared.hpp"
#include "error.hpp"

void osn::Properties::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("Properties");
	cls->register_function(std::make_shared<ipc::function>("Modified", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String, ipc::type::String}, Modified));
	cls->register_function(std::make_shared<ipc::function>("Clicked", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::String}, Clicked));
	srv.register_collection(cls);
}

void osn::Properties::Modified(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	uint64_t sourceId = args[0].value_union.ui64;
	std::string name = args[1].value_str;

	obs_source_t* source = osn::Source::GetInstance()->Get(sourceId);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Reference."));
		AUTO_DEBUG;
		return;
	}
	obs_data_t* settings = obs_data_create_from_json(args[2].value_str.c_str());

	obs_properties_t* props = obs_source_properties(source);
	obs_property_t* prop = obs_properties_get(props, name.c_str());
	if (!prop) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to find property in source."));
	} else {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		rval.push_back(ipc::value((int32_t)obs_property_modified(prop, settings)));
	}
	obs_properties_destroy(props);
	obs_data_release(settings);

	AUTO_DEBUG;
}

void osn::Properties::Clicked(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	uint64_t sourceId = args[0].value_union.ui64;
	std::string name = args[1].value_str;

	obs_source_t* source = osn::Source::GetInstance()->Get(sourceId);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_properties_t* props = obs_source_properties(source);
	obs_property_t* prop = obs_properties_get(props, name.c_str());
	if (!prop) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to find property in source."));
	} else {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		rval.push_back(ipc::value((int32_t)obs_property_button_clicked(prop, source)));
	}
	obs_properties_destroy(props);

	AUTO_DEBUG;
}
