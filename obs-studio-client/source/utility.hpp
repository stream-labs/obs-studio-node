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
#include <string>
#include <vector>
#include <memory>

#include <nan.h>

#include "error.hpp"
#include "controller.hpp"

#ifdef __cplusplus
#define INITIALIZER(f) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void)
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU",read)
#define INITIALIZER2_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
#ifdef _WIN64
#define INITIALIZER(f) INITIALIZER2_(f,"")
#else
#define INITIALIZER(f) INITIALIZER2_(f,"_")
#endif
#else
#define INITIALIZER(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void)
#endif

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64)   //WINDOWS
#define __FUNCTION_NAME__   __FUNCTION__
#else          //*NIX
#define __FUNCTION_NAME__   __func__
#endif
#endif

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE __attribute__((always_inline))
#endif
#define force_inline FORCE_INLINE

#ifdef __GNUC__
#define __deprecated__ __attribute_deprecated__
#else
#define __deprecated__ __declspec(deprecated)
#endif

#define dstr(s) #s
#define vstr(s) dstr(s)

static FORCE_INLINE bool ValidateResponse(std::vector<ipc::value> &response) {
	if (response.size() == 0) {
		Nan::Error("Failed to make IPC call, verify IPC status.");
		return false;
	}

	if ((response.size() == 1) && (response[0].type == ipc::type::Null)) {
		Nan::ThrowError(Nan::New<v8::String>(response[0].value_str).ToLocalChecked());
		return false;
	}

	{
		ErrorCode error = (ErrorCode)response[0].value_union.ui64;
		if (error != ErrorCode::Ok) {
			Nan::ThrowError(Nan::New<v8::String>(response[1].value_str).ToLocalChecked());
			return false;
		}
	}

	if (!response.size()) {
		Nan::ThrowError("Failed to make IPC call, verify IPC status.");
		return false;
	}

	return true;
}

static FORCE_INLINE std::shared_ptr<ipc::client> GetConnection() {
	auto conn = Controller::GetInstance().GetConnection();
	if (!conn)
		Nan::ThrowError("Failed to obtain IPC connection.");
	return conn;
}

namespace utility {
	template<typename T>
	inline std::string TypeOf(T v) {
		return "unknown";
	}

#define AUTO_TYPEOF(x) template<> \
	inline std::string TypeOf<x>(x v) { \
		return dstr(x); \
	}

#define AUTO_TYPEOF_NAME(x, y) template<> \
	inline std::string TypeOf<x>(x v) { \
		return y; \
	}

	AUTO_TYPEOF_NAME(bool, "boolean");
	AUTO_TYPEOF(int8_t);
	AUTO_TYPEOF(int16_t);
	AUTO_TYPEOF(int32_t);
	AUTO_TYPEOF(int64_t);
	AUTO_TYPEOF(uint8_t);
	AUTO_TYPEOF(uint16_t);
	AUTO_TYPEOF(uint32_t);
	AUTO_TYPEOF(uint64_t);
	AUTO_TYPEOF_NAME(const char*, "string");
	AUTO_TYPEOF_NAME(std::string, "string");
	AUTO_TYPEOF(std::vector<char>);

	AUTO_TYPEOF_NAME(v8::Local<v8::Value>, "value");
	AUTO_TYPEOF_NAME(v8::Local<v8::Object>, "object");
	AUTO_TYPEOF_NAME(v8::Local<v8::Function>, "function");

	// This is from enc-amf
#if (defined _WIN32) || (defined _WIN64)
	void SetThreadName(uint32_t dwThreadID, const char* threadName);
	void SetThreadName(const char* threadName);
	//void SetThreadName(std::thread* pthread, const char* threadName);
#else
	void SetThreadName(std::thread* pthread, const char* threadName);
	void SetThreadName(const char* threadName);
#endif
}

std::string from_utf16_wide_to_utf8(const wchar_t *from, size_t length = -1);
std::wstring from_utf8_to_utf16_wide(const char *from, size_t length = -1);