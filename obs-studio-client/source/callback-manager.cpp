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

#include "callback-manager.hpp"
#include "controller.hpp"
#include "error.hpp"
#include "utility-v8.hpp"

#include <node.h>
#include <sstream>
#include <string>
#include "shared.hpp"
#include "utility.hpp"

bool sourceCallback::isWorkerRunning = false;
bool sourceCallback::worker_stop = true;
uint32_t sourceCallback::sleepIntervalMS = 33;
sourceCallback::Worker* sourceCallback::asyncWorker = nullptr;
std::thread* sourceCallback::worker_thread = nullptr;
std::vector<std::thread*> sourceCallback::source_queue_task_workers;
bool sourceCallback::m_all_workers_stop = false;

#ifdef WIN32
const char* source_sem_name = nullptr; // Not used on Windows
HANDLE source_sem;
#else
const char* source_sem_name = "sourcecb-semaphore";
sem_t *source_sem;
#endif

void sourceCallback::start_worker(void)
{
	if (!worker_stop)
		return;

	worker_stop = false;
	source_sem = create_semaphore(source_sem_name);
	worker_thread = new std::thread(&sourceCallback::worker);
}

void sourceCallback::stop_worker(void)
{
	if (worker_stop != false)
		return;

	worker_stop = true;
	if (worker_thread->joinable()) {
		worker_thread->join();
	}
	for (auto queue_worker: source_queue_task_workers) {
		if (queue_worker->joinable()) {
			queue_worker->join();
		}
	}
	remove_semaphore(source_sem, source_sem_name);
}

void sourceCallback::queueTask(std::shared_ptr<SourceSizeInfoData> data) {
	wait_semaphore(source_sem);
	asyncWorker->SetData(data);
	asyncWorker->Queue();
}

Napi::Value sourceCallback::RegisterSourceCallback(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();

	asyncWorker = new sourceCallback::Worker(async_callback);
	asyncWorker->SuppressDestruct();

	start_worker();
	isWorkerRunning = true;

	return Napi::Boolean::New(info.Env(), true);
}

void sourceCallback::worker()
{
	size_t totalSleepMS = 0;

	while (!worker_stop && !m_all_workers_stop) {
		auto tp_start = std::chrono::high_resolution_clock::now();

		// Validate Connection
		auto conn = Controller::GetInstance().GetConnection();
		if (!conn) {
			goto do_sleep;
		}

		// Call
		{
			std::vector<ipc::value> response = conn->call_synchronous_helper("CallbackManager", "QuerySourceSize", {});
			if (!response.size() || (response.size() == 1)) {
				goto do_sleep;
			}

			ErrorCode error = (ErrorCode)response[0].value_union.ui64;
			if (error == ErrorCode::Ok) {
				std::shared_ptr<SourceSizeInfoData> data = std::make_shared<SourceSizeInfoData>();
				for (int i = 2; i < (response[1].value_union.ui32*4) + 2; i++) {
					SourceSizeInfo* item = new SourceSizeInfo;

					item->name   = response[i++].value_str;
					item->width  = response[i++].value_union.ui32;
					item->height = response[i++].value_union.ui32;
					item->flags  = response[i].value_union.ui32;
					data->items.push_back(item);
				}
				source_queue_task_workers.push_back(new std::thread(&sourceCallback::queueTask, data));
			}
		}

	do_sleep:
		auto tp_end  = std::chrono::high_resolution_clock::now();
		auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
		totalSleepMS = sleepIntervalMS - dur.count();
		std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	}
	return;
}

Napi::Value sourceCallback::RemoveSourceCallback(const Napi::CallbackInfo& info)
{
	if (isWorkerRunning)
		stop_worker();
	// delete asyncWorker;
	return info.Env().Undefined();
}

void sourceCallback::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		Napi::String::New(env, "RegisterSourceCallback"),
		Napi::Function::New(env, sourceCallback::RegisterSourceCallback));
	exports.Set(
		Napi::String::New(env, "RemoveSourceCallback"),
		Napi::Function::New(env, sourceCallback::RemoveSourceCallback));
}