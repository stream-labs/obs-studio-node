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

CallbackManager* cm;

void CallbackManager::start_async_runner()
{
	if (m_async_callback)
		return;
	// Start v8/uv asynchronous runner.
	m_async_callback = new cm_Callback();
	m_async_callback->set_handler(
	    std::bind(&CallbackManager::callback_handler, this, std::placeholders::_1, std::placeholders::_2), nullptr);
}
void CallbackManager::stop_async_runner()
{
	if (!m_async_callback)
		return;
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
	uint32_t             i    = 0;
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

	std::vector<ipc::value> response = conn->call_synchronous_helper("CallbackManager", "StartWorker", {});

	args.GetReturnValue().Set(utilv8::ToValue(true));
}

void CallbackManager::set_keepalive(v8::Local<v8::Object> obj)
{
	if (!m_async_callback)
		return;
	m_async_callback->set_keepalive(obj);
}

void RemoveSourceCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::shared_ptr<ipc::client> conn = nullptr;
	if (!(conn = GetConnection())) {
		return;
	}

	std::vector<ipc::value> response = conn->call_synchronous_helper("CallbackManager", "StopWorker", {});
	cm->stop_async_runner();
}

void CallbackManager::UpdateSourceSize(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::vector<char>                   buffer      = args[0].value_bin;
	uint32_t                            size        = *reinterpret_cast<uint32_t*>(buffer.data());
	uint32_t                            indexBuffer = sizeof(uint32_t);
	std::shared_ptr<SourceSizeInfoData> dataSource  = std::make_shared<SourceSizeInfoData>();

	for (size_t i = 0; i < size; i++) {
		SourceSizeInfo* item = new SourceSizeInfo;

		size_t sizeName = *reinterpret_cast<size_t*>(buffer.data() + indexBuffer);
		indexBuffer += sizeof(size_t);
		item->name = std::string(buffer.data() + indexBuffer, sizeName);
		indexBuffer += sizeName;

		item->width = *reinterpret_cast<uint32_t*>(buffer.data() + indexBuffer);
		indexBuffer += sizeof(uint32_t);
		item->height = *reinterpret_cast<uint32_t*>(buffer.data() + indexBuffer);
		indexBuffer += sizeof(uint32_t);
		item->flags = *reinterpret_cast<uint32_t*>(buffer.data() + indexBuffer);
		indexBuffer += sizeof(uint32_t);

		dataSource->items.push_back(item);
	}

	if (cm->m_async_callback)
		cm->m_async_callback->queue(std::move(dataSource));
}

INITIALIZER(callback_manager)
{
	initializerFunctions.push([](v8::Local<v8::Object> exports) {
		NODE_SET_METHOD(exports, "RegisterSourceCallback", RegisterSourceCallback);
		NODE_SET_METHOD(exports, "RemoveSourceCallback", RemoveSourceCallback);
	});
}