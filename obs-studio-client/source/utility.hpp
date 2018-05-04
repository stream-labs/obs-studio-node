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

#define dstr(s) #s
#define vstr(s) dstr(s)

/* Validate the response to make sure the entire
 * request didn't fail. */
#define ASSERT_RESPONSE_VALID(Response) \
        if ((Response.size() == 1) && (Response[0].type == ipc::type::Null)) { \
                Nan::ThrowError(Nan::New<v8::String>(Response[1].value_str).ToLocalChecked()); \
                return; \
        } \
        { \
                ErrorCode error = (ErrorCode)Response[0].value_union.ui64; \
                if (error != ErrorCode::Ok) { \
                        Nan::ThrowError(Nan::New<v8::String>(Response[0].value_str).ToLocalChecked()); \
                        return; \
                } \
        }