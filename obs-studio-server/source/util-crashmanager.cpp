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
#include "nodeobs_api.h"
#include <iostream>
#include <sstream>
#include <obs.h>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include "Shlobj.h"
#include <WinBase.h>

#if defined(_WIN32)

#include "DbgHelp.h"
#pragma comment(lib, "Dbghelp.lib")
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <windows.h>

#endif

// Global/static variables
util::CrashManager::CrashpadInfo*	s_CrashpadInfo = nullptr;
std::map<std::string, std::string>	s_CustomAnnotations;
std::vector<std::string>            s_HandledOBSCrashes;
bool                                s_IgnoreFutureCrashes = false;

// Forward
std::string FormatVAString(const char* const format, va_list args);
long        UnhandledExecptionMethod(struct _EXCEPTION_POINTERS* ExceptionInfo);
void        TerminationMethod();
void        BindCrtHandlesToStdHandles(bool bindStdIn, bool bindStdOut, bool bindStdErr);
void        AtExitMethod();
bool        RewindCallStack() noexcept;

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

	// Setup the crashpad info that will be used in case the obs throws an error
	s_CrashpadInfo = new CrashpadInfo();
	s_CrashpadInfo->handler = handler;
	s_CrashpadInfo->db      = db;
	s_CrashpadInfo->url     = url;
	s_CrashpadInfo->arguments = arguments;

	if (!s_CrashpadInfo->client.StartHandler(handler, db, db, url, annotations, arguments, true, true))
		return false;

	if (!s_CrashpadInfo->client.WaitForHandlerStart(INFINITE))
		return false;

#endif

	return true;
}

void util::CrashManager::Configure() 
{
	// Add all obs crashes that arr supposed to be handled by our application
	s_HandledOBSCrashes.push_back("Failed to recreate D3D11");

// #ifndef _DEBUG

	// Handler for obs errors (mainly for bcrash() calls)
	base_set_crash_handler(
	    [](const char* format, va_list args, void* param) {

			for (auto& handledCrashes : s_HandledOBSCrashes) {
			    if (std::string(format).find(handledCrashes) != std::string::npos) {
			    
					/*
						Do something here that will show to the user the error and that
						we cannot continue (the server will crash)
					*/

					// Disable crashpad
				    if (s_CrashpadInfo != nullptr) {
					    delete s_CrashpadInfo;
					    s_CrashpadInfo = nullptr;
				    }

					/*
					std::string errorMessage = std::string(
				        "The Streamlabs OBS encontered an internal error and will now close, if you are receiving this "
				        "type of error frequently please contact our support! Error message: "
				        + FormatVAString(format, args));

					MessageBox(
				        nullptr,
				        std::wstring(errorMessage.begin(), errorMessage.end()).c_str(),
				        TEXT("Streamlabs OBS Crash"),
				        MB_OK);
					*/

					s_IgnoreFutureCrashes = true;
				    break;
				}
			}

		    s_CustomAnnotations.insert({"OBS bcrash", FormatVAString(format, args)});

		    throw "Induced obs crash";

	    }, nullptr);

	std::set_terminate(TerminationMethod);

#if defined(_WIN32)
	SetUnhandledExceptionFilter(UnhandledExecptionMethod);
#endif

	// The atexit will check if obs was safelly closed
	std::atexit(AtExitMethod);
	std::at_quick_exit(AtExitMethod);

// #endif
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

// Format a var arg string into a c++ std::string type
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

void AtExitMethod()
{
	// If we are exiting normally and obs is active, we have a problem because some
	// modules and threads could potentially be running and this will result in a 
	// crash that is masked. The real issue here is why we are exiting without 
	// finishing the obs first.
	if (obs_initialized()) {
		
		// This will make sure a correct crash report will be made in case we have
		// a crash on shutdown
		RewindCallStack();

		// Setting this will make sure that if the code breaks on a DLL or any other
		// future location after this atexit callback, we won't generate another crash
		// report. This will make sure that we will maintain the correct log on crashpad.
		s_IgnoreFutureCrashes = true;
	}
}

long UnhandledExecptionMethod(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	static bool inside_handler = false;

	/* don't use if a debugger is present */
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;

	if (inside_handler || s_IgnoreFutureCrashes)
		return EXCEPTION_CONTINUE_SEARCH;

	// Call our terminate method to unwing the callstack
	inside_handler = true;
	TerminationMethod();
	inside_handler = false;

	return EXCEPTION_CONTINUE_SEARCH;
}

bool RewindCallStack() noexcept
{
#ifndef _DEBUG
#if defined(_WIN32)

	// Get the function to rewing the callstack
	typedef USHORT(WINAPI * CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
	CaptureStackBackTraceType func =
	    (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary(L"kernel32.dll"), "RtlCaptureStackBackTrace"));
	if (func == NULL)
		return false; // WOE 29.SEP.2010

	// Quote from Microsoft Documentation:
	// ## Windows Server 2003 and Windows XP:
	// ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
	const int      kMaxCallers = 62;
	void*          callers_stack[kMaxCallers];
	unsigned short frames;
	SYMBOL_INFO*   symbol;
	HANDLE         process;

	// Get the callstack information
	process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	frames               = (func)(0, kMaxCallers, callers_stack, NULL);
	symbol               = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen   = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	std::string callStackString;

	// Currently 18 is the maximum that we can display on backtrace in one single attribute
	const unsigned short MAX_CALLERS_SHOWN = 18;
	frames                                 = frames < MAX_CALLERS_SHOWN ? frames : MAX_CALLERS_SHOWN;
	for (unsigned int i = 0; i < frames; i++) {
		SymFromAddr(process, (DWORD64)(callers_stack[i]), 0, symbol);

		// Setup a readable callstack string
		std::stringstream buffer;
		buffer << callers_stack[i] << " " << symbol->Name << " - 0x" << symbol->Address << std::endl;
		callStackString.append(symbol->Name + (i == frames - 1 ? "" : std::string(" -> ")));
	}

	// Use a custom annotation entry for it
	s_CustomAnnotations.insert({std::string("CallStack"), callStackString});

	// Insert the obs init status to detect if the obs is currently initialized or if it was shutdown
	s_CustomAnnotations.insert({"OBS Status", obs_initialized() ? "Initialized" : "Shutdown"});

	// Add the total number of obs leaks
	s_CustomAnnotations.insert({"OBS Total Leaks", std::to_string(bnum_allocs())});

	// Setup the obs log queue as an attribute
	if (OBS_API::getOBSLogQueue().size() > 0) {
		std::queue<std::string> obsLogQueue = OBS_API::getOBSLogQueue();
		std::string             obsLogMessage;
		while (obsLogQueue.size() > 0) {
			obsLogMessage.append(obsLogQueue.front().append(obsLogQueue.size() == 1 ? " " : " -> "));
			obsLogQueue.pop();
		}

		s_CustomAnnotations.insert({"OBS Log Queue", obsLogMessage});
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

	return true;

#endif
#endif
}

void TerminationMethod()
{
	if (s_IgnoreFutureCrashes)
		abort();

	RewindCallStack();

	abort();
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