// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.
#pragma once

#include <mutex>
#include <nan.h>
#include <node.h>
#include <thread>
#include <functional>
#include "utility-v8.hpp"

template <typename DataType>
class CallbackManager
{
	public:
	typedef utilv8::managed_callback<std::shared_ptr<DataType>> DataCallback;

	private:
	uint32_t		m_SleepIntervalMS;
	std::thread		m_Worker;
	bool			m_WorkerStop;
	std::mutex		m_WorkerLock;
	DataCallback*	m_AsyncCallback;
	Nan::Callback	m_CallbackFunction;
	std::function<void(DataCallback*)>										m_UpdateMethod;
	std::function<void(void*, std::shared_ptr<DataType>&, Nan::Callback&)>	m_HandlerMethod;

	public:

	CallbackManager()
	{
	}

	~CallbackManager()
	{
		Shutdown();
	}

	void Initialize(
	    v8::Local<v8::Function>& callback,
	    v8::Local<v8::Object>&   keepAliveObject, 
		std::function<void(DataCallback*)> updateMethod, 
		std::function<void(void*, std::shared_ptr<DataType>&, Nan::Callback&)> handlerMethod, 
	    uint32_t                 sleepInterval = 33)
	{
		// Set the initial data
		m_WorkerStop      = true;
		m_AsyncCallback   = nullptr;
		m_SleepIntervalMS = sleepInterval;
		m_UpdateMethod    = updateMethod;
		m_HandlerMethod   = handlerMethod;

		// Reset the callback function and setup the worker
		m_CallbackFunction.Reset(callback);
		StartAsyncRunner();
		SetKeepAlive(keepAliveObject);
		StartWorker();
	}

	void Shutdown()
	{
		StopWorker();
		StopAsyncRunner();
		m_CallbackFunction.Reset();
	}

	void SetUpdateInterval(uint32_t sleepInterval)
	{
		m_SleepIntervalMS = sleepInterval;
	}

	private:

	void StartAsyncRunner()
	{
		if (m_AsyncCallback)
			return;

		std::unique_lock<std::mutex> ul(m_WorkerLock);
		
		using namespace std::placeholders;
		m_AsyncCallback = new DataCallback();
		m_AsyncCallback->set_handler(std::bind(&CallbackManager::CallbackHandler, this, _1, _2), nullptr);
	}

	void StopAsyncRunner()
	{
		if (!m_AsyncCallback)
			return;

		std::unique_lock<std::mutex> ul(m_WorkerLock);

		m_AsyncCallback->clear();
		m_AsyncCallback->finalize();
		m_AsyncCallback = nullptr;
	}

	void CallbackHandler(void* data, std::shared_ptr<DataType> item)
	{
		m_HandlerMethod(data, item, m_CallbackFunction);
	}

	void StartWorker()
	{
		if (!m_WorkerStop)
			return;

		m_WorkerStop = false;
		m_Worker     = std::thread(std::bind(&CallbackManager::Worker, this));
	}

	void StopWorker()
	{
		if (m_WorkerStop != false)
			return;

		m_WorkerStop = true;
		if (m_Worker.joinable()) {
			m_Worker.join();
		}
	}

	void Worker()
	{
		size_t totalSleepMS = 0;

		while (!m_WorkerStop) {
			auto tp_start = std::chrono::high_resolution_clock::now();

			if (m_UpdateMethod && m_AsyncCallback) {
				std::unique_lock<std::mutex> ul(m_WorkerLock);
				m_UpdateMethod(m_AsyncCallback);
			}
			
		do_sleep:
			auto tp_end  = std::chrono::high_resolution_clock::now();
			auto dur     = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_start);
			totalSleepMS = m_SleepIntervalMS - dur.count();
			std::this_thread::sleep_for(std::chrono::milliseconds(totalSleepMS));
		}
		return;
	}

	void SetKeepAlive(v8::Local<v8::Object> obj)
	{
		if (!m_AsyncCallback)
			return;
		m_AsyncCallback->set_keepalive(obj);
	}
};