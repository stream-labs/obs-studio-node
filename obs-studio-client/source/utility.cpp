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

#include <codecvt>
#include <locale>
#include "utility.hpp"

// This is from enc-amf
#if (defined _WIN32) || (defined _WIN64) // Windows
#define WIN32_LEAN_AND_MEAN
#define WINVER 0x601
#define _WIN32_WINNT 0x601
#include <windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO {
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void utility::SetThreadName(uint32_t dwThreadID, const char* threadName) {

	// DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	} __except (EXCEPTION_EXECUTE_HANDLER) {}
}
void utility::SetThreadName(const char* threadName) {
	utility::SetThreadName(GetCurrentThreadId(), threadName);
}
//void utility::SetThreadName(std::thread* pthread, const char* threadName) {
//	DWORD threadId = ::GetThreadId(static_cast<HANDLE>(pthread->native_handle()));
//	utility::SetThreadName(threadId, threadName);
//}
#else // Linux, Mac
#include <sys/prctl.h>

void Utility::SetThreadName(std::thread* pthread, const char* threadName) {
	auto handle = pthread->native_handle();
	pthread_setname_np(handle, threadName);
}
void Utility::SetThreadName(const char* threadName) {
	prctl(PR_SET_NAME, threadName, 0, 0, 0);
}
#endif

static thread_local std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

std::string from_utf16_wide_to_utf8(const wchar_t *from, size_t length)
{
	const wchar_t *from_end;

	if (length == 0)
		return {};
	else if (length != -1)
		from_end = from + length;
	else
		return converter.to_bytes(from);

	return converter.to_bytes(from, from_end);
}

std::wstring from_utf8_to_utf16_wide(const char *from, size_t length)
{
	const char *from_end;

	if (length == 0)
		return {};
	else if (length != -1)
		from_end = from + length;
	else
		return converter.from_bytes(from);

	return converter.from_bytes(from, from_end);
}