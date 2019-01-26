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
#include <functional>
#include <inttypes.h>
#include <list>
#include <map>
#include <math.h>
#include <nan.h>
#include <node.h>
#include <uv.h>
#include <vector>
#include "utility.hpp"

#define FIELD_NAME(name) Nan::New(name).ToLocalChecked()

#define ASSERT_INFO_LENGTH_AT_LEAST(info, length)                                                        \
	if ((info).Length() < length) {                                                                      \
		Nan::ThrowError(FIELD_NAME(                                                                      \
		    std::string(__FUNCTION_NAME__) + ": Unexpected number of arguments, got " + std::to_string((info).Length()) \
		    + std::string(" but expected at least ") + std::to_string(length) + std::string(".")));      \
		return;                                                                                          \
	}

#define ASSERT_INFO_LENGTH(info, length)                                                                 \
	if ((info).Length() != (length)) {                                                                   \
		Nan::ThrowError(FIELD_NAME(                                                                      \
		    std::string(__FUNCTION_NAME__) + ": Unexpected number of arguments, got " + std::to_string((info).Length()) \
		    + std::string(" but expected exactly ") + std::to_string(length) + std::string(".")));       \
		return;                                                                                          \
	}

#define ASSERT_GET_OBJECT_FIELD(object, field, var)                                           \
	if (!utilv8::GetFromObject((object), (field), (var))) {                                   \
		Nan::ThrowTypeError(FIELD_NAME(std::string(std::string(__FUNCTION_NAME__) + ": Unexpected type."))); \
		return;                                                                               \
	}

#define ASSERT_GET_VALUE(value, var)                                                          \
	if (!utilv8::FromValue((value), (var))) {                                                 \
		Nan::ThrowTypeError(FIELD_NAME(                                                       \
		    std::string(std::string(__FUNCTION_NAME__) + ": Unexpected type, got '") + utilv8::TypeOf(value) \
		    + std::string("', expected '") + utility::TypeOf(var) + std::string("'.")));      \
		return;                                                                               \
	}

namespace utilv8
{
	// Integers
	inline v8::Local<v8::Value> ToValue(bool v)
	{
		return Nan::New<v8::Boolean>(v);
	}

	inline v8::Local<v8::Value> ToValue(int8_t v)
	{
		return Nan::New<v8::Int32>(v);
	}

	inline v8::Local<v8::Value> ToValue(uint8_t v)
	{
		return Nan::New<v8::Uint32>(v);
	}

	inline v8::Local<v8::Value> ToValue(int16_t v)
	{
		return Nan::New<v8::Int32>(v);
	}

	inline v8::Local<v8::Value> ToValue(uint16_t v)
	{
		return Nan::New<v8::Uint32>(v);
	}

	inline v8::Local<v8::Value> ToValue(int32_t v)
	{
		return Nan::New<v8::Int32>(v);
	}

	inline v8::Local<v8::Value> ToValue(uint32_t v)
	{
		return Nan::New<v8::Uint32>(v);
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value>
	    ToValue(int64_t v)
	{
		return Nan::New<v8::Number>((double_t)v);
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value>
	    ToValue(uint64_t v)
	{
		return Nan::New<v8::Number>((double_t)v);
	}

	// Floating Point
	inline v8::Local<v8::Value> ToValue(float_t v)
	{
		return Nan::New<v8::Number>(v);
	}

	inline v8::Local<v8::Value> ToValue(double_t v)
	{
		return Nan::New<v8::Number>(v);
	}

	// Text
	inline v8::Local<v8::Value> ToValue(const char* v)
	{
		return Nan::New<v8::String>(v).ToLocalChecked();
	}

	inline v8::Local<v8::Value> ToValue(std::string v)
	{
		return Nan::New<v8::String>(v).ToLocalChecked();
	}

	// Arrays, Lists (both are v8::Arrays here for simplicity and efficiency)
	template<typename T>
	inline v8::Local<v8::Value> ToValue(std::vector<T> v)
	{
		auto rv = v8::Array::New(v8::Isolate::GetCurrent());
		for (size_t idx = 0; idx < v.size(); idx++) {
			rv->Set((uint32_t)idx, ToValue(v[idx]));
		}
		return rv;
	}

	template<typename T>
	inline v8::Local<v8::Value> ToValue(std::list<T> v)
	{
		auto rv = v8::Array::New(v8::Isolate::GetCurrent());
		for (size_t idx = 0; idx < v.size(); idx++) {
			rv->Set((uint32_t)idx, ToValue(v[idx]));
		}
		return rv;
	}

	// Maps
	template<typename A, typename B>
	inline v8::Local<v8::Value> ToValue(std::map<A, B> v)
	{
		auto rv = v8::Object::New(v8::Isolate::GetCurrent());
		for (auto kv : v) {
			rv->Set(ToValue(kv.first), ToValue(kv.second));
		}
		return rv;
	}

	template<typename T>
	inline v8::Local<v8::Value> ToValue(std::map<uint32_t, T> v)
	{
		auto rv = v8::Object::New(v8::Isolate::GetCurrent());
		for (auto kv : v) {
			rv->Set(kv.first, ToValue(kv.second));
		}
		return rv;
	}

	// Functions (Nan)
	inline v8::Local<v8::Function> ToValue(Nan::FunctionCallback v)
	{
		return Nan::New<v8::Function>(v);
	}

	// Helpers (Allows simply just using ToValue with already correct types)
	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Object> v)
	{
		return v;
	}

	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Value> v)
	{
		return v;
	}

	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Array> v)
	{
		return v;
	}

	inline v8::Local<v8::Value> ToValue(v8::Local<v8::Function> v)
	{
		return v;
	}

	// Integers
	inline bool FromValue(v8::Local<v8::Value> l, bool& r)
	{
		if (l->IsBoolean()) {
			r = l->BooleanValue();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, int8_t& r)
	{
		if (l->IsInt32()) {
			r = l->Int32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, uint8_t& r)
	{
		if (l->IsUint32()) {
			r = l->Uint32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, int16_t& r)
	{
		if (l->IsInt32()) {
			r = l->Int32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, uint16_t& r)
	{
		if (l->IsUint32()) {
			r = l->Uint32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, int32_t& r)
	{
		if (l->IsInt32()) {
			r = l->Int32Value();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, uint32_t& r)
	{
		if (l->IsUint32()) {
			r = l->Uint32Value();
			return true;
		}
		return false;
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline bool
	    FromValue(v8::Local<v8::Value> l, int64_t& r)
	{
		if (l->IsNumber()) {
			r = (int64_t)l->NumberValue();
			return true;
		}
		return false;
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline bool
	    FromValue(v8::Local<v8::Value> l, uint64_t& r)
	{
		if (l->IsNumber()) {
			r = (uint64_t)l->NumberValue();
			return true;
		}
		return false;
	}

	// Floating Point
	inline bool FromValue(v8::Local<v8::Value> l, float_t& r)
	{
		if (l->IsNumber()) {
			r = (float_t)l->NumberValue();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, double_t& r)
	{
		if (l->IsNumber()) {
			r = (double_t)l->NumberValue();
			return true;
		}
		return false;
	}

	// Text
	inline bool FromValue(v8::Local<v8::Value> l, char*& r, size_t length)
	{
		if (l->IsString()) {
			auto v8s = l->ToString();
			if ((v8s->Utf8Length() + 1) <= length) {
				v8::String::Utf8Value utfv8(v8s);
				if (*utfv8) {
					memcpy(r, *utfv8, v8s->Utf8Length());
					r[v8s->Utf8Length()] = 0;
					return true;
				}
			}
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, char*& r)
	{
		if (l->IsString()) {
			auto                  v8s = l->ToString();
			v8::String::Utf8Value utfv8(v8s);
			if (*utfv8) {
				r = (char*)malloc(v8s->Utf8Length() + 1);
				memcpy(r, *utfv8, v8s->Utf8Length());
				r[v8s->Utf8Length()] = 0;
				return true;
			}
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, std::string& r)
	{
		if (l->IsString()) {
			auto                  v8s = l->ToString();
			v8::String::Utf8Value utfv8(v8s);
			if (*utfv8) {
				r = std::string(*utfv8, v8s->Utf8Length());
				return true;
			}
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, v8::Local<v8::Object>& r)
	{
		if (l->IsObject()) {
			r = l->ToObject();
			return true;
		}
		return false;
	}

	inline bool FromValue(v8::Local<v8::Value> l, v8::Local<v8::Function>& r)
	{
		if (l->IsFunction()) {
			r = l.As<v8::Function>();
			return true;
		}
		return false;
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<int8_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int8_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Int8Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<uint8_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint8_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint8Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<int16_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int16_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Int16Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<uint16_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint16_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint16Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<int32_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int32_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint32Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<uint32_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint32_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Uint32Array::New(rv, 0, v.size());
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value>
	    ToArrayBuffer(std::vector<int64_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(int64_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float64Array::New(rv, 0, v.size());
	}

	[[deprecated("v8::Number does not map perfectly to 64-bit Integers")]] inline v8::Local<v8::Value>
	    ToArrayBuffer(std::vector<uint64_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(uint64_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float64Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<float_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(float_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float32Array::New(rv, 0, v.size());
	}

	inline v8::Local<v8::Value> ToArrayBuffer(std::vector<double_t> v)
	{
		auto rv = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), v.size() * sizeof(double_t));
		memcpy(rv->GetContents().Data(), v.data(), v.size());
		return v8::Float64Array::New(rv, 0, v.size());
	}

	template<typename T>
	inline void SetObjectField(v8::Local<v8::Object> object, const char* field, T value)
	{
		Nan::Set(object, ToValue(field), ToValue(value));
	}

	template<typename T>
	inline void SetObjectField(v8::Local<v8::Object> object, uint32_t field, T value)
	{
		Nan::Set(object, field, ToValue(value));
	}

	template<>
	inline void SetObjectField<Nan::FunctionCallback>(
	    v8::Local<v8::Object> object,
	    const char*           name,
	    Nan::FunctionCallback value)
	{
		Nan::SetMethod(object, name, value);
	}

	template<typename Type>
	bool GetFromObject(v8::Local<v8::Object> object, const char* field, Type& var)
	{
		auto field_value = Nan::Get(object, ToValue(field)).ToLocalChecked();
		return FromValue(field_value, var);
	}

	inline void SetObjectAccessor(
	    v8::Local<v8::Object> object,
	    const char*           name,
	    Nan::GetterCallback   get,
	    Nan::SetterCallback   set = nullptr)
	{
		Nan::SetAccessor(object, FIELD_NAME(name), get, set);
	}

	inline void SetObjectAccessorProperty(
	    v8::Local<v8::Object> object,
	    const char*           name,
	    Nan::FunctionCallback get,
	    Nan::FunctionCallback set = nullptr)
	{
		object->SetAccessorProperty(FIELD_NAME(name), ToValue(get), ToValue(set));
	}

	template<typename T>
	inline void SetTemplateField(v8::Local<v8::Template> object, const char* field, T value)
	{
		Nan::SetMethod(object, ToValue(field), ToValue(value));
	}

	template<typename T>
	inline void SetTemplateField(v8::Local<v8::Template> object, uint32_t field, T value)
	{
		Nan::SetMethod(object, field, ToValue(value));
	}

	template<>
	inline void SetTemplateField<Nan::FunctionCallback>(
	    v8::Local<v8::Template> object,
	    const char*             name,
	    Nan::FunctionCallback   value)
	{
		Nan::SetMethod(object, name, value);
	}

	inline void SetTemplateAccessorProperty(
	    v8::Local<v8::Template> object,
	    const char*             name,
	    Nan::FunctionCallback   get,
	    Nan::FunctionCallback   set = nullptr)
	{
		object->SetAccessorProperty(
		    FIELD_NAME(name), Nan::New<v8::FunctionTemplate>(get), Nan::New<v8::FunctionTemplate>(set));
	}

	inline void SetObjectTemplateAccessor(
	    v8::Local<v8::ObjectTemplate> object,
	    const char*                   name,
	    Nan::GetterCallback           get,
	    Nan::SetterCallback           set = nullptr)
	{
		Nan::SetAccessor(object, FIELD_NAME(name), get, set);
	}

	template<typename T>
	inline bool SafeUnwrap(Nan::NAN_METHOD_ARGS_TYPE info, T*& valp)
	{
		T* val = Nan::ObjectWrap::Unwrap<T>(info.This());
		if (!val) {
			info.GetIsolate()->ThrowException(
			    v8::Exception::TypeError(Nan::New<v8::String>("No wrapped object.").ToLocalChecked()));
			return false;
		}
		valp = val;
		return true;
	}

	template<typename T>
	class InterfaceObject
	{
		public:
		static bool Retrieve(v8::Local<v8::Object> object, T*& valp)
		{
			T* val = Nan::ObjectWrap::Unwrap<T>(object);
			if (!val) {
				v8::Isolate::GetCurrent()->ThrowException(
				    v8::Exception::TypeError(Nan::New<v8::String>("No wrapped object.").ToLocalChecked()));
				return false;
			}
			valp = val;
			return true;
		}
	};

	template<typename T>
	class ManagedObject
	{
		public:
		static v8::Local<v8::Object> Store(T* object)
		{
//            auto obj =
//                Nan::NewInstance(T::prototype.Get(v8::Isolate::GetCurrent())->InstanceTemplate()).ToLocalChecked();
//            object->Wrap(obj);
//            return obj;
		}
	};

	/* DynamicCast is incorrect now, since dynamic_cast never should have been used.
	 * The name has been kept for legacy reasons. */
	template<typename T, typename C>
	static bool RetrieveDynamicCast(v8::Local<v8::Object> object, C*& value_ptr)
	{
		T* inner_ptr = nullptr;

		if (!T::Retrieve(object, inner_ptr)) {
			return false;
		}

		value_ptr = static_cast<C*>(inner_ptr);

		return !!value_ptr;
	}

	inline std::string TypeOf(v8::Local<v8::Value> v)
	{
		v8::Local<v8::String> type = v->TypeOf(v8::Isolate::GetCurrent());
		return std::string(*v8::String::Utf8Value(type));
	}

	/* This structure is an adaptation of one used in SQLite Node bindings.
	* You can find the original here:
	* https://github.com/mapbox/node-sqlite3/blob/master/src/async.h */

	template<class Item, class Parent>
	class Async
	{
		public:
		typedef void (*Callback)(Parent* parent, Item* item);

		protected:
		uint32_t           interval;
		uint64_t           last_event_ms;
		uv_async_t         watcher;
		std::mutex         mutex;
		std::vector<Item*> data;
		Callback           callback;

		public:
		Parent* parent;

		public:
		Async(Parent* parent_, Callback cb_, uint32_t interval_ = 0)
		    : callback(cb_), parent(parent_), interval(interval_), last_event_ms(0)
		{
			watcher.data = this;
			uv_async_init(uv_default_loop(), &watcher, reinterpret_cast<uv_async_cb>(listener));
		}

		static void listener(uv_async_t* handle, int status)
		{
			Async*             async = static_cast<Async*>(handle->data);
			std::vector<Item*> rows;

			{
				std::unique_lock<std::mutex> lock(async->mutex);
				rows.swap(async->data);
			}

			for (size_t i = 0, size = rows.size(); i < size; i++) {
				async->callback(async->parent, rows[i]);
			}
		}

		static void close(uv_handle_t* handle)
		{
			assert(handle != NULL);
			assert(handle->data != NULL);
			Async* async = static_cast<Async*>(handle->data);
			delete async;
		}

		void close()
		{
			listener(&watcher, 0);
			uv_close((uv_handle_t*)&watcher, close);
		}

		void add(Item* item)
		{
			std::unique_lock<std::mutex> lock(mutex);
			data.push_back(item);
		}

		void send()
		{
			uint64_t ts = uv_hrtime();

			uint64_t ts_ms = ts / 1000000;

			if (ts_ms - last_event_ms >= interval) {
				last_event_ms = ts_ms;
				uv_async_send(&watcher);
			}
		}

		void send(Item* item)
		{
			add(item);
			send();
		}
	};

	template<typename Item, typename Parent>
	struct CallbackData : public Nan::ObjectWrap,
	                      utilv8::InterfaceObject<utilv8::CallbackData<Item, Parent>>,
	                      utilv8::ManagedObject<utilv8::CallbackData<Item, Parent>>
	{
		friend class utilv8::InterfaceObject<utilv8::CallbackData<Item, Parent>>;
		friend class utilv8::ManagedObject<utilv8::CallbackData<Item, Parent>>;

		public:
//        static Nan::Persistent<v8::FunctionTemplate> prototype;

		static NAN_MODULE_INIT(Init)
		{
			auto locProto = Nan::New<v8::FunctionTemplate>();
			locProto->InstanceTemplate()->SetInternalFieldCount(1);
//            prototype.Reset(locProto);
		}

		CallbackData(
		    Parent*                                parent,
		    typename Async<Item, Parent>::Callback callback,
		    v8::Local<v8::Function>                func,
		    uint32_t                               interval = 0)
		    : handle(this), cb(func), queue(parent, callback, interval), stopped(false)
		{}

		Async<Item, Parent> queue;
		CallbackData*       handle;

//        Nan::Persistent<v8::Object> obj_ref;
		Nan::Callback               cb;
		void*                       user_data; /* Extra user data */

		/* Since events are deferred, we need
						 * some way to know if we should actually
						 * fire off an event when it's finally
						 * being executed. */
		bool stopped;
	};

	// Template class for asynchronous v8 Callbacks
	//
	// A relatively simple to use wrapper around the libuv asynchronous call-
	//  back system. It is self-cleaning in order to not crash in the event that
	//  there are still events queued in the v8 engine. This means that calling
	//  'delete' or using smart pointers will lead to undefined behavior.
	//
	// To use, simply create a new instance of this, set the handler, set the
	//  object to keep alive for callbacks and then just call queue(). Ideally
	//  use smart pointers for T instead of manually tracked pointers.
	template<typename T>
	class managed_callback
	{
		public:
		typedef std::function<void(void* data, T obj)> callback_t;

		private:
		uv_async_t m_async_runner;
		bool       finalizing = false;
		bool       finalized  = false;

		std::mutex                  m_data_mutex;
		std::list<T>                m_objects;
//        Nan::Persistent<v8::Object> m_keepalive;
		callback_t                  m_callback;
		void*                       m_callback_data;

		static void worker(uv_async_t* handle)
		{
			std::list<T>                l_objects;
//            Nan::Persistent<v8::Object> l_keepalive;
			callback_t                  l_callback;
			void*                       l_callback_data;
			managed_callback<T>*        self = reinterpret_cast<managed_callback<T>*>(handle->data);

			if (!self->m_callback) {
				// If there is no callback, simply clear the queue and return.
				std::unique_lock<std::mutex> ul(self->m_data_mutex);
				self->m_objects.clear();
				return;
			}

			{
				// Keep a thread-local copy for thread-safety.
				std::unique_lock<std::mutex> ul(self->m_data_mutex);
				self->m_objects.swap(l_objects);
				l_callback      = self->m_callback;
				l_callback_data = self->m_callback_data;
//                l_keepalive.Reset(self->m_keepalive);
			}

			// Execute the callback for all queued elements.
			for (T v : l_objects) {
				l_callback(l_callback_data, v);
			}
		}

		static void close_handler(uv_handle_t* handle)
		{
			managed_callback<T>* self = reinterpret_cast<managed_callback<T>*>(handle->data);
			{
				std::unique_lock<std::mutex> ul(self->m_data_mutex);
				self->m_objects.clear();
				self->finalized = true;
//                self->m_keepalive.Reset();
			}
			delete self;
		}

		~managed_callback()
		{
			if (finalizing == false)
				throw std::runtime_error("Destructor called before cleaning up. Unable to continue.");
			if (finalized == false)
				throw std::runtime_error(
				    "Destructor called before clean up finished. Race condition that must be fixed.");
		}

		public:
		managed_callback()
		{
			m_async_runner.data = this;
			uv_async_init(uv_default_loop(), &m_async_runner, worker);
		}

		// Finalize the asynchronous callback runner.
		//
		// This needs to be called in order to properly clean things up. It is
		//  a replacement for the destructor. You do not call delete on this
		//  object once you've called finalize(), it will do it by itself.
		void finalize()
		{
			{
				std::unique_lock<std::mutex> ul(m_data_mutex);
				m_objects.clear();
				finalizing = true;
			}
			uv_close((uv_handle_t*)&m_async_runner, close_handler);
		}

		// Set the object that needs to be kept alive until finalized.
		void set_keepalive(v8::Local<v8::Object> obj)
		{
			std::unique_lock<std::mutex> ul(m_data_mutex);
//            m_keepalive.Reset(obj);
		}

		// Set the callback handler and data.
		void set_handler(callback_t handler, void* data)
		{
			std::unique_lock<std::mutex> ul(m_data_mutex);
			m_callback_data = data;
			m_callback      = handler;
		}

		// Clear the queue.
		void clear()
		{
			std::unique_lock<std::mutex> ul(m_data_mutex);
			m_objects.clear();
		}

		// Enqueue another object for the next call.
		void queue(T object)
		{
			std::unique_lock<std::mutex> ul(m_data_mutex);
			m_objects.push_back(object);
			uv_async_send(&m_async_runner);
		}
	};
} // namespace utilv8
