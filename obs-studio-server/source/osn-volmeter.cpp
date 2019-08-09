/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

#include "osn-volmeter.hpp"
#include "error.hpp"
#include "obs.h"
#include "osn-source.hpp"
#include "shared.hpp"
#include "utility.hpp"

osn::VolMeter::Manager& osn::VolMeter::Manager::GetInstance()
{
	static Manager _inst;
	return _inst;
}

osn::VolMeter::VolMeter(obs_fader_type type)
{
	self = obs_volmeter_create(type);
	if (!self)
		throw std::exception();
}

osn::VolMeter::~VolMeter()
{
	obs_volmeter_destroy(self);
}

void osn::VolMeter::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("VolMeter");
	cls->register_function(std::make_shared<ipc::function>("Create", std::vector<ipc::type>{ipc::type::Int32}, Create));
	cls->register_function(
	    std::make_shared<ipc::function>("Destroy", std::vector<ipc::type>{ipc::type::UInt64}, Destroy));
	cls->register_function(std::make_shared<ipc::function>(
	    "GetUpdateInterval", std::vector<ipc::type>{ipc::type::UInt64}, GetUpdateInterval));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetUpdateInterval", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt32}, SetUpdateInterval));
	cls->register_function(std::make_shared<ipc::function>(
	    "Attach", std::vector<ipc::type>{ipc::type::UInt64, ipc::type::UInt64}, Attach));
	cls->register_function(
	    std::make_shared<ipc::function>("Detach", std::vector<ipc::type>{ipc::type::UInt64}, Detach));
	cls->register_function(
	    std::make_shared<ipc::function>("AddCallback", std::vector<ipc::type>{ipc::type::UInt64}, AddCallback));
	cls->register_function(
	    std::make_shared<ipc::function>("RemoveCallback", std::vector<ipc::type>{ipc::type::UInt64}, RemoveCallback));
	cls->register_function(std::make_shared<ipc::function>("Query", std::vector<ipc::type>{ipc::type::UInt64}, Query));
	srv.register_collection(cls);
}

void osn::VolMeter::ClearVolmeters()
{
    Manager::GetInstance().for_each([](const std::shared_ptr<osn::VolMeter>& volmeter)
    {
        if (volmeter->id2) {
            obs_volmeter_remove_callback(volmeter->self, OBSCallback, volmeter->id2);
            delete volmeter->id2;
            volmeter->id2 = nullptr;
        }
    });

    Manager::GetInstance().clear();
}

void osn::VolMeter::Create(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	obs_fader_type            type = (obs_fader_type)args[0].value_union.i32;
	std::shared_ptr<VolMeter> meter;

	try {
		meter = std::make_shared<osn::VolMeter>(type);
	} catch (...) {
		PRETTY_ERROR_RETURN(ErrorCode::Error, "Failed to create Meter.");
	}

	meter->id = Manager::GetInstance().allocate(meter);
	if (meter->id == std::numeric_limits<utility::unique_id::id_t>::max()) {
		meter.reset();
		PRETTY_ERROR_RETURN(ErrorCode::CriticalError, "Failed to allocate unique id for Meter.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(meter->id));
	rval.push_back(ipc::value(obs_volmeter_get_update_interval(meter->self)));
	AUTO_DEBUG;
}

void osn::VolMeter::Destroy(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
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

void osn::VolMeter::GetUpdateInterval(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_volmeter_get_update_interval(meter->self)));
	AUTO_DEBUG;
}

void osn::VolMeter::SetUpdateInterval(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}

	obs_volmeter_set_update_interval(meter->self, args[1].value_union.ui32);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(obs_volmeter_get_update_interval(meter->self)));
	AUTO_DEBUG;
}

void osn::VolMeter::Attach(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid_fader  = args[0].value_union.ui64;
	auto uid_source = args[1].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid_fader);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}

	auto source = osn::Source::Manager::GetInstance().find(uid_source);
	if (!source) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Source reference.");
	}

	if (!obs_volmeter_attach_source(meter->self, source)) {
		PRETTY_ERROR_RETURN(ErrorCode::Error, "Error attaching source.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::VolMeter::Detach(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid = args[0].value_union.ui64;

	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}

	obs_volmeter_detach_source(meter->self);

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void osn::VolMeter::AddCallback(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid   = args[0].value_union.ui64;
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}

	meter->callback_count++;
	if (meter->callback_count == 1) {
		meter->id2  = new uint64_t;
		*meter->id2 = meter->id;
		obs_volmeter_add_callback(meter->self, OBSCallback, meter->id2);
	}

	rval.push_back(ipc::value(uint64_t(ErrorCode::Ok)));
	rval.push_back(ipc::value(meter->callback_count));
	AUTO_DEBUG;
}

void osn::VolMeter::RemoveCallback(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid   = args[0].value_union.ui64;
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
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

void osn::VolMeter::Query(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid   = args[0].value_union.ui64;
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	std::unique_lock<std::mutex> ulock(meter->current_data_mtx);

	// Reset audio data if OBSCallBack is idle
	if (CheckIdle(GetTime(), meter->current_data.lastUpdateTime)) {
		meter->current_data.resetData();
	}

	rval.push_back(ipc::value(meter->current_data.ch));

	for (size_t ch = 0; ch < meter->current_data.ch; ch++) {
		rval.push_back(ipc::value(meter->current_data.magnitude[ch]));
		rval.push_back(ipc::value(meter->current_data.peak[ch]));
		rval.push_back(ipc::value(meter->current_data.input_peak[ch]));
	}

	ulock.unlock();

	AUTO_DEBUG;
}

void osn::VolMeter::OBSCallback(
    void*       param,
    const float magnitude[MAX_AUDIO_CHANNELS],
    const float peak[MAX_AUDIO_CHANNELS],
    const float input_peak[MAX_AUDIO_CHANNELS])
{
	auto meter = Manager::GetInstance().find(*reinterpret_cast<uint64_t*>(param));
	if (!meter) {
		return;
	}

#define MAKE_FLOAT_SANE(db) (std::isfinite(db) ? db : (db > 0 ? 0.0f : -65535.0f))
#define PREVIOUS_FRAME_WEIGHT

	std::unique_lock<std::mutex> ulock(meter->current_data_mtx);

	meter->current_data.lastUpdateTime = GetTime();
	meter->current_data.ch = obs_volmeter_get_nr_channels(meter->self);
	for (size_t ch = 0; ch < MAX_AUDIO_CHANNELS; ch++) {
		meter->current_data.magnitude[ch]  = MAKE_FLOAT_SANE(magnitude[ch]);
		meter->current_data.peak[ch]       = MAKE_FLOAT_SANE(peak[ch]);
		meter->current_data.input_peak[ch] = MAKE_FLOAT_SANE(input_peak[ch]);
	}

#undef MAKE_FLOAT_SANE
}

std::chrono::milliseconds osn::VolMeter::GetTime()
{
	auto currentTime   = std::chrono::high_resolution_clock::now();
	auto currentTimeMs = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
	return currentTimeMs.time_since_epoch();
}

bool osn::VolMeter::CheckIdle(std::chrono::milliseconds currentTime, std::chrono::milliseconds lastUpdateTime)
{
	auto idleTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime);

	if (lastUpdateTime != std::chrono::milliseconds(0) && idleTime > std::chrono::milliseconds(300)) {
		return true;
	}

	return false;
}
