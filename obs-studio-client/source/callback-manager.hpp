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
	void*                        param;
};

enum MouseEventType: uint8_t {
    mouseDown = 0,
    mouseUp = 1,
    mouseDragged = 2,
    mouseMoved = 3,
    mouseEntered = 4
};

struct MouseEvent {
    float x;
    float y;
    bool altKey;
    bool ctrlKey;
    bool shiftKey;
    int button;
    int buttons;
};

struct MouseEventInfo {
	MouseEventType type;
	MouseEvent event;
};

struct MouseEventInfoData
{
	std::vector<MouseEventInfo*> items;
	void*                        param;
};

class CallbackManager;
class SourceCallback;
class MouseEventCallback;
typedef utilv8::managed_callback<std::shared_ptr<SourceSizeInfoData>> cm_sourcesCallback;
typedef utilv8::managed_callback<std::shared_ptr<MouseEventInfoData>> cm_mouseEventCallback;
SourceCallback*                                                        cm_sources;
MouseEventCallback*                                                   cm_mouseEvents;

class CallbackManager : public Nan::ObjectWrap,
                        public utilv8::InterfaceObject<CallbackManager>,
                        public utilv8::ManagedObject<CallbackManager>
{
	friend utilv8::InterfaceObject<CallbackManager>;
	friend utilv8::ManagedObject<CallbackManager>;

	protected:
	uint32_t sleepIntervalMS = 1000;

	public:
	std::thread   m_worker;
	bool          m_worker_stop = true;
	std::mutex    m_worker_lock;

	virtual void start_async_runner() = 0;
	virtual void stop_async_runner() = 0;
	void start_worker();
	void stop_worker();
	virtual void worker() = 0;
	virtual void set_keepalive(v8::Local<v8::Object>) = 0;
};

class SourceCallback : public CallbackManager
{
	friend utilv8::CallbackData<SourceSizeInfoData, CallbackManager>;

	cm_sourcesCallback*  m_async_callback = nullptr;

	public:
	Nan::Callback        m_callback_function;

	virtual void start_async_runner();
	virtual void stop_async_runner();
	virtual void worker();
	virtual void set_keepalive(v8::Local<v8::Object>);
	void callback_handler(void* data, std::shared_ptr<SourceSizeInfoData> sourceSizes);
};

class MouseEventCallback : public CallbackManager
{
	friend utilv8::CallbackData<MouseEventInfoData, CallbackManager>;

	cm_mouseEventCallback*  m_async_callback = nullptr;

	public:
	std::map<MouseEventType, Nan::Callback*> m_callback_functions;

	virtual void start_async_runner();
	virtual void stop_async_runner();
	virtual void worker();
	virtual void set_keepalive(v8::Local<v8::Object>);
	void callback_handler(void* data, std::shared_ptr<MouseEventInfoData> mouseEvents);
};

static void RegisterSourceCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
static void RemoveSourceCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

// Mouse events
static void RegisterMouseEventsCallbacks(const v8::FunctionCallbackInfo<v8::Value>& args);
static void RemoveMouseEventsCallbacks(const v8::FunctionCallbackInfo<v8::Value>& args);
