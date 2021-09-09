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
		// obs_volmeter_remove_callback(meter->self, OBSCallback, meter->id2);
		delete meter->id2;
		meter->id2 = nullptr;
	}
	meter.reset();
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
	uint64_t uid,
	obs_volmeter_updated_t callback,
	void* jsThread)
{
	std::unique_lock<std::mutex> ulock(mtx);
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		blog(LOG_ERROR, "Invalid Meter reference.");
		return;
	}

	meter->m_jsThread = jsThread;
	meter->callback_count++;
	meter->cbReady = true;
	meter->lastProcessed = std::chrono::high_resolution_clock::now();
	if (meter->callback_count == 1) {
		meter->id2  = new uint64_t;
		*meter->id2 = meter->id;
		obs_volmeter_add_callback(meter->self, callback, meter.get());
	}
}

void obs::Volmeter::RemoveCallback(uint64_t uid, obs_volmeter_updated_t callback)
{
	std::unique_lock<std::mutex> ulock(mtx);
	auto meter = Manager::GetInstance().find(uid);
	if (!meter) {
		blog(LOG_ERROR, "Invalid Meter reference.");
		return;
	}

	meter->m_jsThread = nullptr;
	meter->callback_count--;
	meter->cbReady = false;
	if (meter->callback_count == 0) {
		obs_volmeter_remove_callback(meter->self, callback, meter.get());
		delete meter->id2;
		meter->id2 = nullptr;
	}
}