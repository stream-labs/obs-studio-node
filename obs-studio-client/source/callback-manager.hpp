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

#include <mutex>
#include <napi.h>
#include <thread>
#include <map>
#include "utility-v8.hpp"

struct SourceSizeInfo
{
	std::string name;
	uint32_t    width;
	uint32_t    height;
	uint32_t    flags;
};

struct SourceSizeInfoData
{
	std::vector<SourceSizeInfo*> items;
};

namespace sourceCallback
{
	extern bool isWorkerRunning;
	extern bool worker_stop;
	extern uint32_t sleepIntervalMS;
	extern std::thread* worker_thread;
	extern Napi::ThreadSafeFunction js_thread;
	extern bool m_all_workers_stop;

	void worker(void);
	void start_worker(napi_env env, Napi::Function async_callback);
	void stop_worker(void);

	void Init(Napi::Env env, Napi::Object exports);

	Napi::Value RegisterSourceCallback(const Napi::CallbackInfo& info);
	Napi::Value RemoveSourceCallback(const Napi::CallbackInfo& info);
}
