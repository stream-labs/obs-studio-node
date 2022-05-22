/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

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

#include "osn-output-signals.hpp"

void osn::OutputSignals::createOutput(std::string type, std::string name)
{
	deleteOutput();
	output = obs_output_create(type.c_str(), name.c_str(), nullptr, nullptr);

	auto onStopped = [](void* data, calldata_t*) {
		osn::OutputSignals* context = reinterpret_cast<osn::OutputSignals*>(data);
		std::unique_lock<std::mutex> lock(context->mtxOutputStop);
		context->cvStop.notify_one();
	};

	signal_handler* sh = obs_output_get_signal_handler(output);
	signal_handler_connect(sh, "stop", onStopped, this);

	ConnectSignals();
}

void osn::OutputSignals::deleteOutput()
{
	if (!output)
		return;

	if (obs_output_active(output)) {
		obs_output_stop(output);
		std::unique_lock<std::mutex> lock(mtxOutputStop);
		cvStop.wait_for(lock, std::chrono::seconds(20));
	}
	obs_output_release(output);
	output = nullptr;
}

static inline void calbback(void* data, calldata_t* params)
{
	auto info =
		reinterpret_cast<osn::cbData*>(data);

	if (!info)
		return;

	std::string signal = info->signal;
	auto outputClass = info->outputClass;

	if (!outputClass->output)
		return;

	const char* error =
		obs_output_get_last_error(outputClass->output);

	std::unique_lock<std::mutex> ulock(outputClass->signalsMtx);
	outputClass->signalsReceived.push({
		signal,
		(int)calldata_int(params, "code"),
		error ? std::string(error) : ""
	});
}

void osn::OutputSignals::ConnectSignals()
{
	if (!output)
		return;

	signal_handler* handler = obs_output_get_signal_handler(output);
	for (const auto &signal: this->signals) {
		osn::cbData* cd = new cbData();
		cd->signal = signal;
		cd->outputClass = this;
		signal_handler_connect(
			handler,
			signal.c_str(),
			calbback,
			cd);
	}
}