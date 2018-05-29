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

osn::VolMeter::Manager& osn::VolMeter::Manager::GetInstance() {
	static Manager _inst;
	return _inst;
}

osn::VolMeter::VolMeter(obs_fader_type type) {
	self = obs_volmeter_create(type);
	if (!self)
		throw std::exception();
}

osn::VolMeter::~VolMeter() {
	obs_volmeter_destroy(self);
}

void osn::VolMeter::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("VolMeter");
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::Int32}, Create));
	cls->register_function(std::make_shared<ipc::function>("Destroy", std::vector<ipc::type>{ipc::type::UInt64}, Destroy));
	cls->register_function(std::make_shared<ipc::function>("GetUpdateInterval", std::vector<ipc::type>{ipc::type::UInt64}, GetUpdateInterval));
	cls->register_function(std::make_shared<ipc::function>("SetUpdateInterval", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, SetUpdateInterval));
	cls->register_function(std::make_shared<ipc::function>("Attach", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, Attach));
	cls->register_function(std::make_shared<ipc::function>("Detach", std::vector<ipc::type>{ipc::type::UInt64}, Detach));
	cls->register_function(std::make_shared<ipc::function>("AddCallback", std::vector<ipc::type>{ipc::type::UInt64}, AddCallback));
	cls->register_function(std::make_shared<ipc::function>("RemoveCallback", std::vector<ipc::type>{ipc::type::UInt64}, RemoveCallback));
	cls->register_function(std::make_shared<ipc::function>("Query", std::vector<ipc::type>{ipc::type::UInt64}, Query));
	srv.register_collection(cls);
}

void osn::VolMeter::Create(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	obs_fader_type type = (obs_fader_type)args[0].value_union.i32;
	std::shared_ptr<VolMeter> meter;

	try {
		meter = std::make_shared<osn::VolMeter>(type);
	} catch (...) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Error));
		rval.push_back(ipc::value("Failed to create Meter."));
		AUTO_DEBUG;
		return;
	}

	meter->id = Manager::GetInstance().allocate(meter);
	if (meter->id == std::numeric_limits<utility::unique_id::id_t>::max()) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::CriticalError));
		rval.push_back(ipc::value("Failed to allocate unique id for Meter."));
		meter.reset();
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(meter->id));
	rval.push_back(ipc::value(obs_volmeter_get_update_interval(meter->self)));
	AUTO_DEBUG;
}

void osn::VolMeter::Destroy(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	Manager::GetInstance().free(uid);
	if (meter->id2) { // Ensure there are no more callbacks
		obs_volmeter_remove_callback(meter->self, OBSCallback, meter->id2);
		delete meter->id2;
		meter->id2 = nullptr;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::VolMeter::GetUpdateInterval(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
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

void osn::VolMeter::SetUpdateInterval(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
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

void osn::VolMeter::Attach(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid_fader = args[0].value_union.ui64;
	auto uid_source = args[1].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid_fader);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	auto source = osn::Source::Manager::GetInstance().find(uid_source);
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

void osn::VolMeter::Detach(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	obs_volmeter_detach_source(meter->self);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::VolMeter::AddCallback(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	meter->callback_count++;
	if (meter->callback_count == 1) {
		meter->id2 = new uint64_t;
		*meter->id2 = meter->id;
		obs_volmeter_add_callback(meter->self, OBSCallback, meter->id2);
	}

	rval.push_back(ipc::value(uint64_t(ErrorCode::Ok)));
	rval.push_back(ipc::value(meter->callback_count));
	AUTO_DEBUG;
}

void osn::VolMeter::RemoveCallback(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	meter->callback_count--;
	if (meter->callback_count == 0) {
		obs_volmeter_remove_callback(meter->self, OBSCallback, meter->id2);
		delete meter->id2;
		meter->id2 = nullptr;
	}

	rval.push_back(ipc::value(uint64_t(ErrorCode::Ok)));
	rval.push_back(ipc::value(meter->callback_count));
	AUTO_DEBUG;
}

void osn::VolMeter::Query(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	shared::LogWarnTimer warntimer(__FUNCTION_NAME__);
	auto uid = args[0].value_union.ui64;
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	std::shared_ptr<AudioData> ad;
	if (meter->current_data.size() > 0) {
		ad = meter->current_data.front();
		meter->current_data.pop();
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	if (ad) {
		rval.push_back(ipc::value(ad->ch));
		for (size_t ch = 0; ch < ad->ch; ch++) {
			rval.push_back(ipc::value(ad->magnitude[ch]));
			rval.push_back(ipc::value(ad->peak[ch]));
			rval.push_back(ipc::value(ad->input_peak[ch]));
		}
	} else {
		rval.push_back(ipc::value(0));
	}

	if (ad) {
		meter->free_data.push(ad);
	}

	AUTO_DEBUG;
}

void osn::VolMeter::OBSCallback(void *param, const float magnitude[MAX_AUDIO_CHANNELS], const float peak[MAX_AUDIO_CHANNELS], const float input_peak[MAX_AUDIO_CHANNELS]) {
	auto meter = Manager::GetInstance().find(*reinterpret_cast<uint64_t*>(param));
	if (!meter) {
		return;
	}

#define MAKE_FLOAT_SANE(db) (std::isfinite(db) ? db : (db > 0 ? 0.0f : -128.0f))
	if (meter->current_data.size() > 4) {
		// Data is not being used, so force re-use.
		while (meter->current_data.size() > 1) {
			meter->free_data.push(meter->current_data.front());
			meter->current_data.pop();
		}
	}

	std::shared_ptr<AudioData> ad;
	if (meter->free_data.size() > 0) {
		ad = meter->free_data.front();
		meter->free_data.pop();
	} else {
		ad = std::make_shared<AudioData>();
	}

	ad->ch = obs_volmeter_get_nr_channels(meter->self);
	for (size_t ch = 0; ch < MAX_AUDIO_CHANNELS; ch++) {
		ad->magnitude[ch] = MAKE_FLOAT_SANE(magnitude[ch]);
		ad->peak[ch] = MAKE_FLOAT_SANE(peak[ch]);
		ad->input_peak[ch] = MAKE_FLOAT_SANE(input_peak[ch]);
	}

	meter->current_data.push(ad);

#undef MAKE_FLOAT_SANE
}
