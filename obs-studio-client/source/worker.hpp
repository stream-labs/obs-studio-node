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
#include <napi.h>

class Worker
{
	public:
    Worker(std::string n) {
        name = n;
        isWorkerRunning = false;
        workerStop = true;
        sleepIntervalMS = 33;
        workerThread = nullptr;
    };
    ~Worker() {};
    
	virtual void worker(void) = 0;

    protected:
    std::string name;
    bool isWorkerRunning;
	bool workerStop;
	uint32_t sleepIntervalMS;
	std::thread* workerThread;
	Napi::ThreadSafeFunction jsThread;
	Napi::FunctionReference cb;

	void startWorker(napi_env env, Napi::Function asyncCallback) {
        if (!workerStop)
		    return;

        workerStop = false;
        jsThread = Napi::ThreadSafeFunction::New(
            env,
            asyncCallback,
            name.c_str(),
            0,
            1,
            []( Napi::Env ) {} );
        workerThread = new std::thread(&Worker::worker, this);
    }

	void stopWorker(void) {
        if (workerStop != false)
            return;

        workerStop = true;
        if (workerThread->joinable()) {
            workerThread->join();
        }
    }
};