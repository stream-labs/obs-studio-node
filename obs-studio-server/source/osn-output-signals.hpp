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

#pragma once
#include <obs.h>
#include <mutex>
#include <queue>
#include <vector>

namespace osn {
struct signalInfo {
	std::string signal;
	int code;
	std::string errorMessage;
};

class OutputSignals {
public:
	OutputSignals()
	{
		output = nullptr;
		canvas = nullptr;
	}
	virtual ~OutputSignals() {}

public:
	std::mutex signalsMtx;
	std::queue<signalInfo> signalsReceived;
	std::vector<std::string> signals;
	obs_output_t *output;
	obs_video_info *canvas;

	void ConnectSignals();

public:
	std::condition_variable cvStop;
	std::mutex mtxOutputStop;
	void createOutput(const std::string &type, const std::string &name);
	void deleteOutput();
	void startOutput();
};

struct cbData {
	std::string signal;
	OutputSignals *outputClass;
};
}
