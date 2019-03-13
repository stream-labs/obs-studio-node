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

void CallbackManager::start_async_runner()
{
	if (m_async_callback)
		return;
	std::unique_lock<std::mutex> ul(m_worker_lock);
	// Start v8/uv asynchronous runner.
	m_async_callback = new cm_Callback();
	m_async_callback->set_handler(
	    std::bind(&CallbackManager::callback_handler, this, std::placeholders::_1, std::placeholders::_2), nullptr);
}
void CallbackManager::stop_async_runner()
{
	if (!m_async_callback)
		return;
	std::unique_lock<std::mutex> ul(m_worker_lock);
	// Stop v8/uv asynchronous runner.
	m_async_callback->clear();
	m_async_callback->finalize();
	m_async_callback = nullptr;
}

void CallbackManager::callback_handler(void* data, std::shared_ptr<SourceSizeInfoData> sourceSizes)
{
	v8::Isolate*         isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::Value> args[1];
	Nan::HandleScope     scope;
	uint32_t             i = 0;
	v8::Local<v8::Array> rslt = v8::Array::New(isolate);

	if (sourceSizes->items.size() == 0)
		return;

	for (auto item : sourceSizes->items) {
		v8::Local<v8::Value> argv = v8::Object::New(isolate);
		argv->ToObject()->Set(utilv8::ToValue("name"), utilv8::ToValue(item->name));
		argv->ToObject()->Set(utilv8::ToValue("width"), utilv8::ToValue(item->width));
		argv->ToObject()->Set(utilv8::ToValue("height"), utilv8::ToValue(item->height));
		argv->ToObject()->Set(utilv8::ToValue("flags"), utilv8::ToValue(item->flags));
		rslt->Set(i++, argv);
	}	

	args[0] = rslt;
	Nan::Call(m_callback_function, 1, args);
}
void CallbackManager::start_worker()
{
	if (!m_worker_stop)
		return;
	// Launch worker thread.
	m_worker_stop = false;
	m_worker      = std::thread(std::bind(&CallbackManager::worker, this));
}
void CallbackManager::stop_worker()
{
	if (m_worker_stop != false)
		return;
	// Stop worker thread.
	m_worker_stop = true;
	if (m_worker.joinable()) {
		m_worker.join();
	}
}

static v8::Persistent<v8::Object> cm_CallbackObject;

void RegisterSourceCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Function> callback;
	ASSERT_GET_VALUE(args[0], callback);

	// Grab IPC Connection
	std::shared_ptr<ipc::client> conn = nullptr;
	if (!(conn = GetConnection())) {
		return;
	}

	// Callback
	cm = new CallbackManager();
	cm->m_callback_function.Reset(callback);
	cm->start_async_runner();
	cm->set_keepalive(args.This());
	cm->start_worker();
	args.GetReturnValue().Set(utilv8::ToValue(true));
}

void CallbackManager::worker()
{
	size_t totalSleepMS = 0;

	while (!m_worker_stop) {
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
				data->param = this;
				m_async_callback->queue(std::move(data));
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

void CallbackManager::set_keepalive(v8::Local<v8::Object> obj)
{
	if (!m_async_callback)
		return;
	m_async_callback->set_keepalive(obj);
}

void RemoveSourceCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	cm->stop_worker();
	cm->stop_async_runner();
}

INITIALIZER(callback_manager)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "RegisterSourceCallback", RegisterSourceCallback);
		NODE_SET_METHOD(exports, "RemoveSourceCallback", RemoveSourceCallback);
	});
}