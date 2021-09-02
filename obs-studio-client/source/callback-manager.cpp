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
#include "volmeter.hpp"
#include <future>
#include <thread>

bool globalCallback::isWorkerRunning = false;
bool globalCallback::worker_stop = true;
uint32_t globalCallback::sleepIntervalMS = 50;
std::thread* globalCallback::worker_thread = nullptr;
Napi::ThreadSafeFunction globalCallback::js_thread;
bool globalCallback::m_all_workers_stop = false;
std::mutex globalCallback::mtx_volmeters;
std::map<uint64_t, Napi::ThreadSafeFunction> globalCallback::volmeters;

void globalCallback::Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		Napi::String::New(env, "RegisterSourceCallback"),
		Napi::Function::New(env, globalCallback::RegisterGlobalCallback));
	exports.Set(
		Napi::String::New(env, "RemoveSourceCallback"),
		Napi::Function::New(env, globalCallback::RemoveGlobalCallback));
}

Napi::Value globalCallback::RegisterGlobalCallback(const Napi::CallbackInfo& info)
{
	Napi::Function async_callback = info[0].As<Napi::Function>();

	// start_worker(info.Env(), async_callback);
	// isWorkerRunning = true;
	// worker_stop = false;

	// worker_thread = new std::thread(&globalCallback::worker);



	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value globalCallback::RemoveGlobalCallback(const Napi::CallbackInfo& info)
{
	// if (isWorkerRunning)
	// 	stop_worker();

	return info.Env().Undefined();
}

void globalCallback::start_worker(napi_env env, Napi::Function async_callback)
{
	// if (!worker_stop)
	// 	return;

	// js_thread = Napi::ThreadSafeFunction::New(
    //   env,
    //   async_callback,
    //   "GlobalCallback",
    //   0,
    //   1,
    //   []( Napi::Env ) {} );
}

void globalCallback::stop_worker(void)
{
	// if (worker_stop != false)
	// 	return;

	// worker_stop = true;
	// if (worker_thread->joinable()) {
	// 	worker_thread->join();
	// }
	// js_thread.Release();
}

void globalCallback::worker()
{
    // auto sources_callback = []( Napi::Env env, 
	// 		Napi::Function jsCallback,
	// 		SourceSizeInfoData* data ) {
	// 	Napi::Array result = Napi::Array::New(env, data->items.size());

	// 	for (size_t i = 0; i < data->items.size(); i++) {
	// 		Napi::Object obj = Napi::Object::New(env);
	// 		obj.Set("name", Napi::String::New(env, data->items[i]->name));
	// 		obj.Set("width", Napi::Number::New(env, data->items[i]->width));
	// 		obj.Set("height", Napi::Number::New(env, data->items[i]->height));
	// 		obj.Set("flags", Napi::Number::New(env, data->items[i]->flags));
	// 		result.Set(i, obj);
	// 	}
	// 	jsCallback.Call({ result });
    // };

	// size_t totalSleepMS = 0;

	// while (!worker_stop && !m_all_workers_stop) {
	// 	auto tp_start = std::chrono::high_resolution_clock::now();

	// 	auto conn = Controller::GetInstance().GetConnection();
	// 	if (!conn)
	// 		return;

	// 	{
	// 		std::vector<ipc::value> response =
	// 			conn->call_synchronous_helper("CallbackManager", "GlobalQuery",
	// 			{
	// 			});
	// 		if (!response.size() || (response.size() == 1)) {
	// 			goto do_sleep;
	// 		}

	// 		uint32_t index = 1;

	// 		SourceSizeInfoData* data = new SourceSizeInfoData{ {} };
	// 		for (int i = 2; i < (response[1].value_union.ui32*4) + 2; i++) {
	// 			SourceSizeInfo* item = new SourceSizeInfo;

	// 			item->name   = response[i++].value_str;
	// 			item->width  = response[i++].value_union.ui32;
	// 			item->height = response[i++].value_union.ui32;
	// 			item->flags  = response[i].value_union.ui32;
	// 			data->items.push_back(item);
	// 			index = i;
	// 		}

	// 		if (data->items.size() > 0)
	// 			js_thread.NonBlockingCall( data, sources_callback );

	// 		index++;
	// 	}

	// do_sleep:
	// 	mtx_volmeters.unlock();
	// 	auto tp_end  = std::chrono::high_resolution_clock::now();
	// 	auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
	// 	totalSleepMS = sleepIntervalMS - dur.count();
	// 	if (totalSleepMS > 0)
	// 		std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	// }
	// return;
}

void globalCallback::add_volmeter(napi_env env, uint64_t id, Napi::Function cb)
{
	// Napi::ThreadSafeFunction vol_thread = Napi::ThreadSafeFunction::New(
    //   env,
    //   cb,
    //   "Volmeter",
    //   0,
    //   1,
    //   []( Napi::Env ) {} );
	// volmeters.insert(std::make_pair(id, vol_thread));
}

void globalCallback::remove_volmeter(uint64_t id)
{
	// if (volmeters.find(id) == volmeters.end())
	// 	return;
	
	// volmeters[id].Release();
	// volmeters.erase(id);
}