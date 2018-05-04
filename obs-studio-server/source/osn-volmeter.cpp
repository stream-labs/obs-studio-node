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

#include "osn-volmeter.hpp"
#include "obs.h"
#include "error.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include "osn-source.hpp"

osn::VolumeMeter::Manager& osn::VolumeMeter::Manager::GetInstance() {
	static Manager _inst;
	return _inst;
}

osn::VolumeMeter::VolumeMeter(obs_fader_type type) {
	self = obs_volmeter_create(type);
	if (!self)
		throw std::exception();
}

osn::VolumeMeter::~VolumeMeter() {
	obs_volmeter_destroy(self);
}

void osn::VolumeMeter::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("VolumeMeter");
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::Int32}, Create));
	cls->register_function(std::make_shared<ipc::function>("Destroy", std::vector<ipc::type>{ipc::type::Int32}, Destroy));
	cls->register_function(std::make_shared<ipc::function>("GetUpdateInterval", std::vector<ipc::type>{ipc::type::UInt64}, GetUpdateInterval));
	cls->register_function(std::make_shared<ipc::function>("SetUpdateInterval", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, SetUpdateInterval));
	cls->register_function(std::make_shared<ipc::function>("Attach", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, Attach));
	cls->register_function(std::make_shared<ipc::function>("Detach", std::vector<ipc::type>{ipc::type::UInt64}, Detach));
	cls->register_function(std::make_shared<ipc::function>("AddCallback", std::vector<ipc::type>{ipc::type::UInt64}, AddCallback));
	cls->register_function(std::make_shared<ipc::function>("RemoveCallback", std::vector<ipc::type>{ipc::type::UInt64}, RemoveCallback));
	srv.register_collection(cls);
}

void osn::VolumeMeter::Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	obs_fader_type type = (obs_fader_type)args[0].value_union.i32;
	VolumeMeter* meter = nullptr;

	try {
		meter = new VolumeMeter(type);
	} catch (...) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create Meter."));
		AUTO_DEBUG;
		return;
	}

	auto uid = Manager::GetInstance().allocate(meter);
	if (uid == std::numeric_limits<utility::unique_id::id_t>::max()) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Failed to allocate unique id for Meter."));
		delete meter;
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(uid));
	AUTO_DEBUG;
}

void osn::VolumeMeter::Destroy(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	delete meter;
	Manager::GetInstance().free(uid);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::VolumeMeter::GetUpdateInterval(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_volmeter_get_update_interval(meter->self)));
	AUTO_DEBUG;
}

void osn::VolumeMeter::SetUpdateInterval(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_volmeter_set_update_interval(meter->self, args[1].value_union.ui32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_volmeter_get_update_interval(meter->self)));
	AUTO_DEBUG;
}

void osn::VolumeMeter::Attach(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	auto uid_fader = args[0].value_union.ui64;
	auto uid_source = args[1].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid_fader);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	auto source = osn::Source::GetInstance()->Get(uid_source);
	if (!source) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Source Reference."));
		AUTO_DEBUG;
		return;
	}

	if (!obs_volmeter_attach_source(meter->self, source)) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Error attaching source."));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::VolumeMeter::Detach(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Fader Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_volmeter_detach_source(meter->self);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::VolumeMeter::AddCallback(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	//!FIXME!
}

void osn::VolumeMeter::RemoveCallback(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	//!FIXME!
}
