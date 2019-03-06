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
#include <nan.h>
#include <node.h>
#include <thread>
#include "utility-v8.hpp"

struct SourceSizeInfo
{
	std::string name;
	uint32_t    width;
	uint32_t    height;
};

struct SourceSizeInfoData
{
	std::vector<SourceSizeInfo*> items;
	void*                        param;
};

class CallbackManager;
typedef utilv8::managed_callback<std::shared_ptr<SourceSizeInfoData>> cm_Callback;
CallbackManager*                                              cm;

class CallbackManager : public Nan::ObjectWrap,
                        public utilv8::InterfaceObject<CallbackManager>,
                        public utilv8::ManagedObject<CallbackManager>
{
	friend utilv8::InterfaceObject<CallbackManager>;
	friend utilv8::ManagedObject<CallbackManager>;
	friend utilv8::CallbackData<SourceSizeInfoData, CallbackManager>;

	uint32_t sleepIntervalMS = 1000;

	public:
	std::thread   m_worker;
	bool          m_worker_stop = true;
	std::mutex    m_worker_lock;
	cm_Callback*  m_async_callback = nullptr;
	Nan::Callback m_callback_function;

	CallbackManager(){};
	~CallbackManager(){};

	void start_async_runner();
	void stop_async_runner();
	void callback_handler(void* data, std::shared_ptr<SourceSizeInfoData> sourceSizes);
	void start_worker();
	void stop_worker();
	void worker();
	void set_keepalive(v8::Local<v8::Object>);

	std::list<cm_Callback*> callbacks;
};

static void RegisterSourceCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
