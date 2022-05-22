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