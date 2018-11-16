/*
* Modern effects for a modern Streamer
* Copyright (C) 2017 Michael Fabian Dirks
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "util-crashmanager.h"
#include <iostream>
#include <obs.h>

#ifndef _DEBUG

#include <WinBase.h>
#include "DbgHelp.h"
#pragma comment(lib, "Dbghelp.lib")

#if defined(_WIN32)

#include "Shlobj.h"

#endif
#endif

util::CrashManager::~CrashManager()
{
	if (s_CrashpadInfo != nullptr)
		delete s_CrashpadInfo;
}

std::string FormatVAString(const char* const format, va_list args)
{
	auto temp   = std::vector<char>{};
	auto length = std::size_t{63};
	while (temp.size() <= length) {
		temp.resize(length + 1);
		const auto status = std::vsnprintf(temp.data(), temp.size(), format, args);
		if (status < 0)
			throw std::runtime_error{"string formatting error"};
		length = static_cast<std::size_t>(status);
	}
	return std::string{temp.data(), length};
}

void myterminate();

LONG mywindowsterminate(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	static bool inside_handler = false;

	/* don't use if a debugger is present */
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;

	if (inside_handler)
		return EXCEPTION_CONTINUE_SEARCH;

	// Call our terminate method to unwing the callstack
	myterminate();

	return EXCEPTION_CONTINUE_SEARCH;
}

void myterminate()
{
	for (int i = 0; i < 100000; i++) {
		std::cout << i << " - ";
	}

	typedef USHORT(WINAPI * CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
	CaptureStackBackTraceType func =
	    (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary(L"kernel32.dll"), "RtlCaptureStackBackTrace"));

	if (func == NULL)
		abort(); // WOE 29.SEP.2010

	// Quote from Microsoft Documentation:
	// ## Windows Server 2003 and Windows XP:
	// ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
	const int kMaxCallers = 62;

	void*          callers_stack[kMaxCallers];
	unsigned short frames;
	SYMBOL_INFO*   symbol;
	HANDLE         process;
	process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	frames               = (func)(0, kMaxCallers, callers_stack, NULL);
	symbol               = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen   = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	// std::cout << "(" << (int)this << "): " << std::endl;
	const unsigned short MAX_CALLERS_SHOWN = 30;
	frames                                 = frames < MAX_CALLERS_SHOWN ? frames : MAX_CALLERS_SHOWN;
	for (unsigned int i = 0; i < frames; i++) {
		SymFromAddr(process, (DWORD64)(callers_stack[i]), 0, symbol);
		std::cout << "*** " << i << ": " << callers_stack[i] << " " << symbol->Name << " - 0x" << symbol->Address
		          << std::endl;
	}

	free(symbol);

	abort(); // forces abnormal termination
}

bool util::CrashManager::Initialize()
{
#ifndef _DEBUG

	std::wstring             appdata_path;
	crashpad::CrashpadClient client;
	bool                     rc;

#if defined(_WIN32)
	HRESULT hResult;
	PWSTR   ppszPath;

	hResult = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath);

	appdata_path.assign(ppszPath);
	appdata_path.append(L"\\obs-studio-node-server");

	CoTaskMemFree(ppszPath);
#endif

	std::map<std::string, std::string> annotations;
	std::vector<std::string>           arguments;
	arguments.push_back("--no-rate-limit");

	std::wstring handler_path(L"crashpad_handler.exe");
	std::string  url(
        "https://submit.backtrace.io/streamlabs/513fa5577d6a193ed34965e18b93d7b00813e9eb2f4b0b7059b30e66afebe4fe/"
        "minidump");

	base::FilePath db(appdata_path);
	base::FilePath handler(handler_path);

	std::unique_ptr<crashpad::CrashReportDatabase> database = crashpad::CrashReportDatabase::Initialize(db);
	if (database == nullptr || database->GetSettings() == NULL)
		return false;

	database->GetSettings()->SetUploadsEnabled(true);

	rc = client.StartHandler(handler, db, db, url, annotations, arguments, true, true);
	/* TODO Check rc value for errors */

	rc = client.WaitForHandlerStart(INFINITE);
	/* TODO Check rc value for errors */

	// Setup the crashpad info that will be used in case the obs throws an error
	s_CrashpadInfo = new CrashpadInfo({handler, db, url, arguments, client});

	//
	//
	//

	// Handler for obs errors (mainly for bcrash() calls)
	base_set_crash_handler(
	    [](const char* format, va_list args, void* param) {
		    CrashpadInfo*                      crashpadInfo = (CrashpadInfo*)param;
		    std::map<std::string, std::string> annotations;

		    annotations.insert({"OBS bcrash", FormatVAString(format, args)});

		    bool rc = crashpadInfo->client.StartHandler(
		        crashpadInfo->handler,
		        crashpadInfo->db,
		        crashpadInfo->db,
		        crashpadInfo->url,
		        annotations,
		        crashpadInfo->arguments,
		        true,
		        true);

		    rc = crashpadInfo->client.WaitForHandlerStart(INFINITE);

		    throw "Induced obs crash";
	    },
	    s_CrashpadInfo);

	std::set_terminate(myterminate);
	SetUnhandledExceptionFilter(mywindowsterminate);

#endif

	return true;
}