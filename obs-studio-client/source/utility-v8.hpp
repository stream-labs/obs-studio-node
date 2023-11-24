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

#pragma once
#include <functional>
#include <inttypes.h>
#include <list>
#include <map>
#include <math.h>
#include <napi.h>
#include <uv.h>
#include <vector>
#include <mutex>
#include "utility.hpp"

// #define FIELD_NAME(name) Nan::New(name).ToLocalChecked()
#define FIELD_NAME(info, name) Napi::String::New(info.Env(), name)

// Napi::Error::New(info.Env(), "Too few arguments, usage: host(uri).").ThrowAsJavaScriptException();

#define ASSERT_INFO_LENGTH_AT_LEAST(info, length)                                                                                                \
	if ((info).Length() < length) {                                                                                                          \
		Napi::Error::New(info.Env(), FIELD_NAME(info, std::string(__FUNCTION_NAME__) + ": Unexpected number of arguments, got " +        \
								      std::to_string((info).Length()) + std::string(" but expected at least ") + \
								      std::to_string(length) + std::string(".")));                               \
		return;                                                                                                                          \
	}

#define ASSERT_INFO_LENGTH(info, length)                                                                                                        \
	if ((info).Length() != (length)) {                                                                                                      \
		Napi::Error::New(info.Env(), FIELD_NAME(info, std::string(__FUNCTION_NAME__) + ": Unexpected number of arguments, got " +       \
								      std::to_string((info).Length()) + std::string(" but expected exactly ") + \
								      std::to_string(length) + std::string(".")));                              \
		return;                                                                                                                         \
	}

#define ASSERT_GET_OBJECT_FIELD(object, field, var)                                                                    \
	if (!utilv8::GetFromObject((object), (field), (var))) {                                                        \
		Napi::Error::New(info.Env(), FIELD_NAME(info, std::string(__FUNCTION_NAME__) + ": Unexpected type.")); \
		return;                                                                                                \
	}

#define ASSERT_GET_VALUE(info, value, var)                                                                             \
	if (!utilv8::FromValue((value), (var))) {                                                                      \
		Napi::Error::New(info.Env(), FIELD_NAME(info, std::string(__FUNCTION_NAME__) + ": Unexpected type.")); \
		return info.Env().Undefined();                                                                         \
	}

namespace utilv8 {
// Integers
inline Napi::Value ToValue(const Napi::CallbackInfo &info, bool v)
{
	return Napi::Boolean::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, int8_t v)
{
	// return Nan::New<v8::Int32>(v);
	return Napi::Number::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, uint8_t v)
{
	// return Nan::New<v8::Uint32>(v);
	return Napi::Number::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, int16_t v)
{
	// return Nan::New<v8::Int32>(v);
	return Napi::Number::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, uint16_t v)
{
	// return Nan::New<v8::Uint32>(v);
	return Napi::Number::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, int32_t v)
{
	// return Nan::New<v8::Int32>(v);
	return Napi::Number::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, uint32_t v)
{
	// return Nan::New<v8::Uint32>(v);
	return Napi::Number::New(info.Env(), v);
}

// Floating Point
inline Napi::Value ToValue(const Napi::CallbackInfo &info, float_t v)
{
	// return Nan::New<v8::Number>(v);
	return Napi::Number::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, double_t v)
{
	// return Nan::New<v8::Number>(v);
	return Napi::Number::New(info.Env(), v);
}

// Text
inline Napi::Value ToValue(const Napi::CallbackInfo &info, const char *v)
{
	// return Nan::New<v8::String>(v).ToLocalChecked();
	return Napi::String::New(info.Env(), v);
}

inline Napi::Value ToValue(const Napi::CallbackInfo &info, std::string v)
{
	// return Nan::New<v8::String>(v).ToLocalChecked();
	return Napi::String::New(info.Env(), v);
}

// Arrays, Lists (both are v8::Arrays here for simplicity and efficiency)
template<typename T> inline Napi::Value ToValue(const Napi::CallbackInfo &info, std::vector<T> v)
{
	// auto rv = v8::Array::New(v8::Isolate::GetCurrent());
	auto rv = Napi::Array::New(info.Env(), v.size());
	for (size_t idx = 0; idx < v.size(); idx++) {
		rv[(uint32_t)idx] = ToValue(info, v[idx]);
	}
	return rv;
}

template<typename T> inline Napi::Value ToValue(const Napi::CallbackInfo &info, std::list<T> v)
{
	// auto rv = v8::Array::New(v8::Isolate::GetCurrent());
	auto rv = Napi::Array::New(info.Env(), v.size());
	for (size_t idx = 0; idx < v.size(); idx++) {
		rv[(uint32_t)idx] = ToValue(info, v[idx]);
	}
	return rv;
}

// Maps
template<typename A, typename B> inline Napi::Value ToValue(const Napi::CallbackInfo &info, std::map<A, B> v)
{
	// auto rv = v8::Object::New(v8::Isolate::GetCurrent());
	auto rv = Napi::Object::New(info.Env());
	for (auto kv : v) {
		rv.Set(ToValue(kv.first), ToValue(kv.second));
	}
	return rv;
}

template<typename T> inline Napi::Value ToValue(const Napi::CallbackInfo &info, std::map<uint32_t, T> v)
{
	// auto rv = v8::Object::New(v8::Isolate::GetCurrent());
	auto rv = Napi::Object::New(info.Env());
	for (auto kv : v) {
		rv.Set(kv.first, ToValue(kv.second));
	}
	return rv;
}

// Helpers (Allows simply just using ToValue with already correct types)
inline Napi::Value ToValue(Napi::Object v)
{
	return v;
}

inline Napi::Value ToValue(Napi::Value v)
{
	return v;
}

inline Napi::Value ToValue(Napi::Array v)
{
	return v;
}

inline Napi::Value ToValue(Napi::Function v)
{
	return v;
}

// Integers
inline bool FromValue(Napi::Value l, bool &r)
{
	if (l.IsBoolean()) {
		r = l.ToBoolean().Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, int8_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Int32Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, uint8_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Uint32Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, int16_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Int32Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, uint16_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Uint32Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, int32_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Int32Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, uint32_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Uint32Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, int64_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Int64Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, uint64_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().Int64Value(); // Uint64 not available
		return true;
	}
	return false;
}

// Floating Point
inline bool FromValue(Napi::Value l, float_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().FloatValue();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, double_t &r)
{
	if (l.IsNumber()) {
		r = l.ToNumber().DoubleValue();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, std::string &r)
{
	if (l.IsString()) {
		r = l.ToString().Utf8Value();
		return true;
	}
	return false;
}

inline bool FromValue(Napi::Value l, Napi::Object &r)
{
	if (l.IsObject()) {
		r = l.ToObject();
		return true;
	}
	return false;
}
}
