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
#include "shared-server.hpp"
#include "utility-server.hpp"
#include <cmath>

std::mutex mtx;

obs::Volmeter::Manager& obs::Volmeter::Manager::GetInstance()
{
	static Manager _inst;
	return _inst;
}

obs::Volmeter::Volmeter(obs_fader_type type)
{
	self = obs_volmeter_create(type);
	if (!self)
		throw std::exception();
}

obs::Volmeter::~Volmeter()
{
	obs_volmeter_destroy(self);
}

void obs::Volmeter::ClearVolmeters()
{
    Manager::GetInstance().for_each([](const std::shared_ptr<obs::Volmeter>& volmeter)
    {
        if (volmeter->id2) {
            obs_volmeter_remove_callback(volmeter->self, OBSCallback, volmeter->id2);
            delete volmeter->id2;
            volmeter->id2 = nullptr;
        }
    });

    Manager::GetInstance().clear();
}

std::pair<uint64_t, uint32_t> obs::Volmeter::Create(int32_t a_type)
{
	obs_fader_type            type = (obs_fader_type)a_type;
	std::shared_ptr<Volmeter> meter;

	std::unique_lock<std::mutex> ulock(mtx);
	try {
		meter = std::make_shared<obs::Volmeter>(type);
	} catch (...) {
		blog(LOG_ERROR, "Failed to create Meter.");
		return std::make_pair(UINT64_MAX, UINT32_MAX);
	}

	meter->id = Manager::GetInstance().allocate(meter);
	if (meter->id == std::numeric_limits<utility_server::unique_id::id_t>::max()) {
		meter.reset();
		blog(LOG_ERROR, "Failed to allocate unique id for Meter.");
		return std::make_pair(UINT64_MAX, UINT32_MAX);
	}

	return std::make_pair(meter->id, obs_volmeter_get_update_interval(meter->self));
}

void obs::Volmeter::Destroy(uint64_t uid)
{
	std::unique_lock<std::mutex> ulock(mtx);
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		blog(LOG_ERROR, "Invalid Meter reference.");
		return;
	}

	Manager::GetInstance().free(uid);
	if (meter->id2) { // Ensure there are no more callbacks
		obs_volmeter_remove_callback(meter->self, OBSCallback, meter->id2);
		delete meter->id2;
		meter->id2 = nullptr;
	}
}

void obs::Volmeter::Attach(uint64_t uid_fader, uint64_t uid_source)
{
	std::unique_lock<std::mutex> ulock(mtx);
	auto meter = Manager::GetInstance().find(uid_fader);
	if (!meter) {
		blog(LOG_ERROR, "Invalid Meter reference.");
		return;
	}

	auto source = obs::Source::Manager::GetInstance().find(uid_source);
	if (!source) {
		blog(LOG_ERROR, "Invalid Meter reference.");
		return;
	}

	if (!obs_volmeter_attach_source(meter->self, source)) {
		blog(LOG_ERROR, "Error attaching source.");
		return;
	}

	meter->uid_source = uid_source;
}

void obs::Volmeter::Detach(uint64_t uid)
{
	std::unique_lock<std::mutex> ulock(mtx);
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		blog(LOG_ERROR, "Invalid Meter reference.");
		return;
	}

	meter->uid_source = 0;
	obs_volmeter_detach_source(meter->self);
}

void obs::Volmeter::AddCallback(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid   = args[0].value_union.ui64;
	std::unique_lock<std::mutex> ulock(mtx);
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
	rval.push_back(ipc::value(uint64_t(meter->callback_count)));
	AUTO_DEBUG;
}

void obs::Volmeter::RemoveCallback(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid   = args[0].value_union.ui64;
	std::unique_lock<std::mutex> ulock(mtx);
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
	rval.push_back(ipc::value(uint64_t(meter->callback_count)));
	AUTO_DEBUG;
}

void obs::Volmeter::Query(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	auto uid   = args[0].value_union.ui64;
	std::unique_lock<std::mutex> ulockMutex(mtx);
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	std::unique_lock<std::mutex> ulock(meter->current_data_mtx);

	// Reset audio data if OBSCallBack is idle
	if (meter->current_data.lastUpdateTime != std::chrono::milliseconds(0)) {
		if (CheckIdle(GetTime(), meter->current_data.lastUpdateTime)) {
			meter->current_data.resetData();
		}
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

void obs::Volmeter::OBSCallback(
    void*       param,
    const float magnitude[MAX_AUDIO_CHANNELS],
    const float peak[MAX_AUDIO_CHANNELS],
    const float input_peak[MAX_AUDIO_CHANNELS])
{
	std::unique_lock<std::mutex> ulockMutex(mtx);
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

std::chrono::milliseconds obs::Volmeter::GetTime()
{
	auto currentTime   = std::chrono::high_resolution_clock::now();
	auto currentTimeMs = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
	return currentTimeMs.time_since_epoch();
}

bool obs::Volmeter::CheckIdle(std::chrono::milliseconds currentTime, std::chrono::milliseconds lastUpdateTime)
{
	auto idleTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime);

	if (idleTime > std::chrono::milliseconds(300)) {
		return true;
	}

	return false;
}

void obs::Volmeter::getAudioData(uint64_t id, std::vector<ipc::value>& rval)
{
	std::unique_lock<std::mutex> ulockMutex(mtx);

	auto meter = Manager::GetInstance().find(id);
	if (!meter) {
		PRETTY_ERROR_RETURN(ErrorCode::InvalidReference, "Invalid Meter reference.");
	}
	
	std::unique_lock<std::mutex> ulock(meter->current_data_mtx);

	rval.push_back(ipc::value(meter->current_data.ch));

	auto source = obs::Source::Manager::GetInstance().find(meter->uid_source);
	bool isMuted = source ? obs_source_muted(source) : true;
	rval.push_back(ipc::value(isMuted));

	if (isMuted)
		return;

	for (size_t ch = 0; ch < meter->current_data.ch; ch++) {
		rval.push_back(ipc::value(meter->current_data.magnitude[ch]));
		rval.push_back(ipc::value(meter->current_data.peak[ch]));
		rval.push_back(ipc::value(meter->current_data.input_peak[ch]));
	}
}