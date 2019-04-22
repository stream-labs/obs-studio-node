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
	srv.register_collection(cls);
	g_srv = &srv;
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

void osn::VolMeter::Destroy(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
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

void osn::VolMeter::GetUpdateInterval(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
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

void osn::VolMeter::SetUpdateInterval(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
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

void osn::VolMeter::Detach(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
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

void updateVolmeters(std::shared_ptr<osn::VolMeter> meter)
{
	meter->previous = std::chrono::high_resolution_clock::now();
	while (!meter->stopWorker) {

#define MAKE_FLOAT_SANE(db) (std::isfinite(db) ? db : (db > 0 ? 0.0f : -65535.0f))
#define PREVIOUS_FRAME_WEIGHT

		std::vector<ipc::value> agrs;
		agrs.push_back(ipc::value(meter->id));
		agrs.push_back(ipc::value(obs_volmeter_get_nr_channels(meter->self)));

		std::vector<char> binData;
		binData.resize(agrs.at(1).value_union.i32 * 3 * sizeof(float));
		uint32_t indexBuffer = 0;

		// Critical section
		{
			std::unique_lock<std::mutex> lck(meter->mutex);

			if (meter->values.size() == 0) {
				meter->stopWorker = true;
				continue;
			}

			int channels = obs_volmeter_get_nr_channels(meter->self);

			for (size_t ch = 0; ch < obs_volmeter_get_nr_channels(meter->self); ch++) {
				*reinterpret_cast<float*>(binData.data() + indexBuffer) =
				    MAKE_FLOAT_SANE(meter->values.back()->magnitude[ch]);
				indexBuffer += sizeof(float);
				*reinterpret_cast<float*>(binData.data() + indexBuffer) =
				    MAKE_FLOAT_SANE(meter->values.back()->peak[ch]);
				indexBuffer += sizeof(float);
				*reinterpret_cast<float*>(binData.data() + indexBuffer) =
				    MAKE_FLOAT_SANE(meter->values.back()->input_peak[ch]);
				indexBuffer += sizeof(float);
			}
			meter->values.clear();
		}

		agrs.push_back(ipc::value(binData));

#undef MAKE_FLOAT_SANE

		std::chrono::high_resolution_clock::time_point current = std::chrono::high_resolution_clock::now();
		auto                                           delta   = current - meter->previous;

		if (std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() < 50) {
			std::this_thread::sleep_for(
			    std::chrono::milliseconds(50 - std::chrono::duration_cast<std::chrono::milliseconds>(delta).count()));
		}

		meter->previous = std::chrono::high_resolution_clock::now();

		if (g_srv) {
			std::unique_lock<std::mutex> ul(g_srv->m_clients_mtx);
			if (meter->values.size() > 0) {
				for (auto client : g_srv->m_clients) {
					if (client.second->host)
						client.second->call("Volmeter", "UpdateVolmeter", agrs);
				}
			}
		}
	}
	std::unique_lock<std::mutex> lck(meter->mutex);
	meter->isRunning = false;
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
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
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
		rval.push_back(ipc::value((uint64_t)ErrorCode::InvalidReference));
		rval.push_back(ipc::value("Invalid Meter Reference."));
		AUTO_DEBUG;
		return;
	}

	meter->stopWorker = true;
	if (meter->worker.joinable())
		meter->worker.join();

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

	osn::AudioData* ad = new osn::AudioData();
	uint32_t        count_ch = obs_volmeter_get_nr_channels(meter->self);
	ad->magnitude.resize(count_ch);
	ad->peak.resize(count_ch);
	ad->input_peak.resize(count_ch);

	for (size_t ch = 0; ch < count_ch; ch++) {
		ad->magnitude[ch]     = magnitude[ch];
		ad->peak[ch]          = peak[ch];
		ad->input_peak[ch]    = input_peak[ch];
	}

	std::unique_lock<std::mutex> lck(meter->mutex);
	meter->values.push_back(ad);

	if (!meter->isRunning) {
		meter->stopWorker = false;
		meter->isRunning  = true;
		meter->worker     = std::thread(updateVolmeters, meter);
	}

}
