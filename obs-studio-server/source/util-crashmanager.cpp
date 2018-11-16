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
#include <sstream>
#include <obs.h>

#ifndef _DEBUG

#include <WinBase.h>
#include "DbgHelp.h"
#pragma comment(lib, "Dbghelp.lib")

	#if defined(_WIN32)

	#include "Shlobj.h"
	#include <WinBase.h>
	#include "DbgHelp.h"
	#pragma comment(lib, "Dbghelp.lib")
	#include <fcntl.h>
	#include <io.h>
	#include <iostream>
	#include <windows.h>

	#endif

#endif

util::CrashManager::CrashpadInfo*	s_CrashpadInfo = nullptr;
std::map<std::string, std::string>	s_CustomAnnotations;

// Forward
std::string FormatVAString(const char* const format, va_list args);
LONG        mywindowsterminate(struct _EXCEPTION_POINTERS* ExceptionInfo);
void        myterminate();
void        BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr);

// Class specific
util::CrashManager::~CrashManager()
{
	if (s_CrashpadInfo != nullptr)
		delete s_CrashpadInfo;
}

bool util::CrashManager::Initialize()
{
#ifndef _DEBUG

	std::wstring             appdata_path;
	crashpad::CrashpadClient client;

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

	if (!client.StartHandler(handler, db, db, url, annotations, arguments, true, true))
		return false;

	if (!client.WaitForHandlerStart(INFINITE))
		return false;

	// Setup the crashpad info that will be used in case the obs throws an error
	s_CrashpadInfo = new CrashpadInfo({handler, db, url, arguments, client});

#endif

	return true;
}

void util::CrashManager::Configure() 
{
#ifndef _DEBUG

	// Handler for obs errors (mainly for bcrash() calls)
	base_set_crash_handler(
	    [](const char* format, va_list args, void* param) {
		    s_CustomAnnotations.insert({"OBS bcrash", FormatVAString(format, args)});

		    throw "Induced obs crash";

	    }, nullptr);

	std::set_terminate(myterminate);

#if defined(_WIN32)
	SetUnhandledExceptionFilter(mywindowsterminate);
#endif

#endif
}

void util::CrashManager::OpenConsole()
{
#if defined(_WIN32)

	// Allocate a console window for this process
	AllocConsole();

	// Update the C/C++ runtime standard input, output, and error targets to use the console window
	BindCrtHandlesToStdHandles(true, true, true);

#endif
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

LONG mywindowsterminate(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	static bool inside_handler = false;

	/* don't use if a debugger is present */
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;

	if (inside_handler)
		return EXCEPTION_CONTINUE_SEARCH;

	inside_handler = true;

	// Call our terminate method to unwing the callstack
	myterminate();

	inside_handler = false;

	return EXCEPTION_CONTINUE_SEARCH;
}

void myterminate()
{
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

		std::stringstream buffer;
		buffer << callers_stack[i] << " " << symbol->Name << " - 0x" << symbol->Address
		          << std::endl;
		s_CustomAnnotations.insert({std::string("CallStack " + std::to_string(i)), buffer.str()});
	}

	 bool rc = s_CrashpadInfo->client.StartHandler(
	    s_CrashpadInfo->handler,
	    s_CrashpadInfo->db,
	    s_CrashpadInfo->db,
	    s_CrashpadInfo->url,
	    s_CustomAnnotations,
	    s_CrashpadInfo->arguments,
	    true,
	    true);

	rc = s_CrashpadInfo->client.WaitForHandlerStart(INFINITE);

	free(symbol);

	abort(); // forces abnormal termination
}

void BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr)
{
	// Re-initialize the C runtime "FILE" handles with clean handles bound to "nul". We do this because it has been
	// observed that the file number of our standard handle file objects can be assigned internally to a value of -2
	// when not bound to a valid target, which represents some kind of unknown internal invalid state. In this state our
	// call to "_dup2" fails, as it specifically tests to ensure that the target file number isn't equal to this value
	// before allowing the operation to continue. We can resolve this issue by first "re-opening" the target files to
	// use the "nul" device, which will place them into a valid state, after which we can redirect them to our target
	// using the "_dup2" function.
	if (bindStdIn) {
		FILE* dummyFile;
		freopen_s(&dummyFile, "nul", "r", stdin);
	}
	if (bindStdOut) {
		FILE* dummyFile;
		freopen_s(&dummyFile, "nul", "w", stdout);
	}
	if (bindStdErr) {
		FILE* dummyFile;
		freopen_s(&dummyFile, "nul", "w", stderr);
	}

	// Redirect unbuffered stdin from the current standard input handle
	if (bindStdIn) {
		HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE) {
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1) {
				FILE* file = _fdopen(fileDescriptor, "r");
				if (file != NULL) {
					int dup2Result = _dup2(_fileno(file), _fileno(stdin));
					if (dup2Result == 0) {
						setvbuf(stdin, NULL, _IONBF, 0);
					}
				}
			}
		}
	}

	// Redirect unbuffered stdout to the current standard output handle
	if (bindStdOut) {
		HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE) {
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1) {
				FILE* file = _fdopen(fileDescriptor, "w");
				if (file != NULL) {
					int dup2Result = _dup2(_fileno(file), _fileno(stdout));
					if (dup2Result == 0) {
						setvbuf(stdout, NULL, _IONBF, 0);
					}
				}
			}
		}
	}

	// Redirect unbuffered stderr to the current standard error handle
	if (bindStdErr) {
		HANDLE stdHandle = GetStdHandle(STD_ERROR_HANDLE);
		if (stdHandle != INVALID_HANDLE_VALUE) {
			int fileDescriptor = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
			if (fileDescriptor != -1) {
				FILE* file = _fdopen(fileDescriptor, "w");
				if (file != NULL) {
					int dup2Result = _dup2(_fileno(file), _fileno(stderr));
					if (dup2Result == 0) {
						setvbuf(stderr, NULL, _IONBF, 0);
					}
				}
			}
		}
	}

	// Clear the error state for each of the C++ standard stream objects. We need to do this, as attempts to access the
	// standard streams before they refer to a valid target will cause the iostream objects to enter an error state. In
	// versions of Visual Studio after 2005, this seems to always occur during startup regardless of whether anything
	// has been read from or written to the targets or not.
	if (bindStdIn) {
		std::wcin.clear();
		std::cin.clear();
	}
	if (bindStdOut) {
		std::wcout.clear();
		std::cout.clear();
	}
	if (bindStdErr) {
		std::wcerr.clear();
		std::cerr.clear();
	}
}