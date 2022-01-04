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

std::mutex sources_sizes_mtx;
std::map<std::string, SourceSizeInfo*> sources;

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

	start_worker(info.Env(), async_callback);
	isWorkerRunning = true;
	worker_stop = false;

	worker_thread = new std::thread(&globalCallback::worker);

	return Napi::Boolean::New(info.Env(), true);
}

Napi::Value globalCallback::RemoveGlobalCallback(const Napi::CallbackInfo& info)
{
	if (isWorkerRunning)
		stop_worker();

	return info.Env().Undefined();
}

void globalCallback::start_worker(napi_env env, Napi::Function async_callback)
{
	if (!worker_stop)
		return;

	js_thread = Napi::ThreadSafeFunction::New(
		env,
		async_callback,
		"GlobalCallback",
		0,
		1,
		[]( Napi::Env ) {} );
}

void globalCallback::stop_worker(void)
{
	if (worker_stop != false)
		return;

	worker_stop = true;
	if (worker_thread->joinable()) {
		worker_thread->join();
	}
	js_thread.Release();

	std::unique_lock<std::mutex> ulock(sources_sizes_mtx);
	sources.clear();
}

void globalCallback::worker()
{
	auto sources_callback = []( Napi::Env env,
			Napi::Function jsCallback,
			SourceSizeInfoData* data ) {
		try {
			Napi::Array result = Napi::Array::New(env, data->items.size());

			for (size_t i = 0; i < data->items.size(); i++) {
				Napi::Object obj = Napi::Object::New(env);
				obj.Set("name", Napi::String::New(env, data->items[i]->name));
				obj.Set("width", Napi::Number::New(env, data->items[i]->width));
				obj.Set("height", Napi::Number::New(env, data->items[i]->height));
				obj.Set("flags", Napi::Number::New(env, data->items[i]->flags));
				result.Set(i, obj);
			}
			jsCallback.Call({ result });
		} catch (...) {}
		delete data;
	};

	size_t totalSleepMS = 0;

	while (!worker_stop && !m_all_workers_stop) {
		auto tp_start = std::chrono::high_resolution_clock::now();

		if (!sources.empty()) {
			std::unique_lock<std::mutex> ulock(sources_sizes_mtx);
			SourceSizeInfoData* data = new SourceSizeInfoData{ {} };

			for (auto item : sources) {
				SourceSizeInfo* si = item.second;
				// See if width or height changed here
				uint32_t newWidth  = obs_source_get_width(si->source);
				uint32_t newHeight = obs_source_get_height(si->source);
				uint32_t newFlags  = obs_source_get_output_flags(si->source);

				if (si->width != newWidth ||
					si->height != newHeight ||
					si->flags != newFlags) {
					si->width = newWidth;
					si->height = newHeight;
					si->flags  = newFlags;

					data->items.push_back(si);
				}
			}

			if (data->items.size() > 0) {
				napi_status status = js_thread.BlockingCall( data, sources_callback );
				if (status != napi_ok) {
					delete data;
				}
			}
		}

	do_sleep:
		auto tp_end  = std::chrono::high_resolution_clock::now();
		auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
		totalSleepMS = sleepIntervalMS - dur.count();
		if (totalSleepMS > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
	}
}

void globalCallback::addSource(obs_source_t* source)
{
	uint32_t flags= obs_source_get_output_flags(source);
	if ((flags & OBS_SOURCE_VIDEO) == 0)
		return;

	if (!source ||
		obs_source_get_type(source) == OBS_SOURCE_TYPE_FILTER ||
		obs_source_get_type(source) == OBS_SOURCE_TYPE_TRANSITION ||
		obs_source_get_type(source) == OBS_SOURCE_TYPE_SCENE)
		return;

	std::unique_lock<std::mutex> ulock(sources_sizes_mtx);

	SourceSizeInfo* si = new SourceSizeInfo;
	si->source = source;
	si->name = obs_source_get_name(source);
	si->width = obs_source_get_width(source);
	si->height = obs_source_get_height(source);

	sources.emplace(std::make_pair(si->name, si));
}

void globalCallback::removeSource(obs_source_t* source)
{
	std::unique_lock<std::mutex> ulock(sources_sizes_mtx);

	if (!source)
		return;

	const char* name = obs_source_get_name(source);
	
	if (name)
		sources.erase(name);
}