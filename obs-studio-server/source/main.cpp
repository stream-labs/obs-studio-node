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

#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <ipc-class.hpp>
#include <ipc-function.hpp>
#include <ipc-server.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "error.hpp"
#include "nodeobs_api.h"
#include "nodeobs_autoconfig.h"
#include "nodeobs_content.h"
#include "nodeobs_service.h"
#include "nodeobs_settings.h"
#include "osn-fader.hpp"
#include "osn-filter.hpp"
#include "osn-global.hpp"
#include "osn-input.hpp"
#include "osn-module.hpp"
#include "osn-properties.hpp"
#include "osn-scene.hpp"
#include "osn-sceneitem.hpp"
#include "osn-source.hpp"
#include "osn-transition.hpp"
#include "osn-video.hpp"
#include "osn-volmeter.hpp"

#ifndef _DEBUG
#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"
#endif

#if defined(_WIN32)
#include "Shlobj.h"

// Checks ForceGPUAsRenderDevice setting
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = [] {
	LPWSTR       roamingPath;
	std::wstring filePath;
	std::string  line;
	std::fstream file;
	bool         settingValue = true; // Default value (NvOptimusEnablement = 1)

	if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roamingPath))) {
		// Couldn't find roaming app data folder path, assume default value
		return settingValue;
	} else {
		filePath.assign(roamingPath);
		filePath.append(L"\\slobs-client\\basic.ini");
		CoTaskMemFree(roamingPath);
	}

	file.open(filePath);

	if (file.is_open()) {
		while (std::getline(file, line)) {
			if (line.find("ForceGPUAsRenderDevice", 0) != std::string::npos) {
				if (line.substr(line.find('=') + 1) == "false") {
					settingValue = false;
					file.close();
					break;
				}
			}
		}
	} else {
		//Couldn't open config file, assume default value
		return settingValue;
	}

	// Return setting value
	return settingValue;
}();
#endif

#define BUFFSIZE 512

struct ServerData
{
	std::mutex                                     mtx;
	std::chrono::high_resolution_clock::time_point last_connect, last_disconnect;
	size_t                                         count_connected = 0;
};

bool ServerConnectHandler(void* data, int64_t)
{
	ServerData*                  sd = reinterpret_cast<ServerData*>(data);
	std::unique_lock<std::mutex> ulock(sd->mtx);
	sd->last_connect = std::chrono::high_resolution_clock::now();
	sd->count_connected++;
	return true;
}

void ServerDisconnectHandler(void* data, int64_t)
{
	ServerData*                  sd = reinterpret_cast<ServerData*>(data);
	std::unique_lock<std::mutex> ulock(sd->mtx);
	sd->last_disconnect = std::chrono::high_resolution_clock::now();
	sd->count_connected--;
}

namespace System
{
	static void
	    Shutdown(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval)
	{
		bool* shutdown = (bool*)data;
		*shutdown      = true;
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		return;
	}
} // namespace System

std::string FormatVAString(const char* const format, va_list args)
{
	auto         temp   = std::vector<char>{};
	auto         length = std::size_t{63};
	while (temp.size() <= length) {
		temp.resize(length + 1);
		const auto status = std::vsnprintf(temp.data(), temp.size(), format, args);
		if (status < 0)
			throw std::runtime_error{"string formatting error"};
		length = static_cast<std::size_t>(status);
	}
	return std::string{temp.data(), length};
}

std::map<void*, std::pair<size_t, std::vector<std::string>>> s_Allocations;
std::mutex              s_AllocationMutex;

#include <WinBase.h>
#include "DbgHelp.h"
#include "Shlobj.h"
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "pdh.lib")
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#include "TCHAR.h"
#include "pdh.h"
#include "psapi.h"
#include <Windows.h>

std::vector<std::string> RewindCallStack()
{
	std::vector<std::string> result;

#ifndef _DEBUG
#if defined(_WIN32)

	// Get the function to rewing the callstack
	typedef USHORT(WINAPI * CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG);
	CaptureStackBackTraceType func =
	    (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary(L"kernel32.dll"), "RtlCaptureStackBackTrace"));
	if (func == NULL)
		return result; // WOE 29.SEP.2010

	// Quote from Microsoft Documentation:
	// ## Windows Server 2003 and Windows XP:
	// ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
	const int       kMaxCallers = 62;
	void*           callers_stack[kMaxCallers];
	unsigned short  frames;
	SYMBOL_INFO*    symbol;
	HANDLE          process;
	DWORD           dwDisplacement = 0;
	IMAGEHLP_LINE64 line;

	// Get the callstack information (values/constants here retrieve from microsoft page)
	SymSetOptions(SYMOPT_LOAD_LINES);
	process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	frames               = (func)(0, kMaxCallers, callers_stack, NULL);
	symbol               = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen   = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	std::vector<std::string> callstack;
	int                      writingIndex = -1;

	// Currently 50 is the maximum that we can display on backtrace in one single attribute
	const unsigned short MAX_CALLERS_SHOWN = 50;
	frames                                 = frames < MAX_CALLERS_SHOWN ? frames : MAX_CALLERS_SHOWN;
	std::vector<int> missingFrames;
	for (int i = frames - 1; i >= 0; i--) {
		if (!SymFromAddr(process, (DWORD64)(callers_stack[i]), 0, symbol)
		    || !SymGetLineFromAddr64(process, (DWORD64)(callers_stack[i]), &dwDisplacement, &line)) {
			// Add the frame index to the missing frames vector
			missingFrames.push_back(i);
			continue;
		}

		std::stringstream buffer;
		std::string       fullPath = line.FileName;
		std::string       fileName;
		std::string       functionName = symbol->Name;
		std::string       instructionAddress;
		std::string       symbolAddress = std::to_string(symbol->Address);

		// Get the filename from the fullpath
		size_t pos = fullPath.rfind("\\", fullPath.length());
		if (pos != std::string::npos) {
			fileName = (fullPath.substr(pos + 1, fullPath.length() - pos));
		}

		// Ignore any report that refers to this file
		if (fileName == "util-crashmanager.cpp")
			continue;

		// Store the callstack address into the buffer (void* to std::string)
		buffer << callers_stack[i];

		// A little of magic to shorten the address lenght
		instructionAddress = buffer.str();
		pos                = instructionAddress.find_first_not_of("0");
		if (pos != std::string::npos && pos <= 4) {
			instructionAddress = (instructionAddress.substr(pos + 1, instructionAddress.length() - pos));
		}

		// Transform the addresses to lowercase
		transform(instructionAddress.begin(), instructionAddress.end(), instructionAddress.begin(), ::tolower);
		transform(symbolAddress.begin(), symbolAddress.end(), symbolAddress.begin(), ::tolower);

        std::string entry = functionName + " - " + fileName + std::to_string(line.LineNumber);
		result.push_back(entry);

        // entry["function"]         = functionName;
		// entry["filename"]         = fileName;
		// entry["lineno"]           = line.LineNumber;
		// entry["instruction addr"] = "0x" + instructionAddress;
		// entry["symbol addr"]      = "0x" + symbolAddress;
	}

	free(symbol);

#endif
#endif

	return result;
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

static void* a_malloc(size_t size)
{
#ifdef ALIGNED_MALLOC
	return _aligned_malloc(size, ALIGNMENT);
#elif ALIGNMENT_HACK
	void* ptr = NULL;
	long  diff;

	ptr = malloc(size + ALIGNMENT);
	if (ptr) {
		diff             = ((~(long)ptr) & (ALIGNMENT - 1)) + 1;
		ptr              = (char*)ptr + diff;
		((char*)ptr)[-1] = (char)diff;
	}

	return ptr;
#else
	return malloc(size);
#endif
}

static void* Alloc(size_t size)
{
	void* ptr = a_malloc(size);

    std::lock_guard<std::mutex> lock(s_AllocationMutex);
	s_Allocations.insert({ptr, {size, RewindCallStack()}});

    return ptr;
}

static void* a_realloc(void* ptr, size_t size)
{
#ifdef ALIGNED_MALLOC
	return _aligned_realloc(ptr, size, ALIGNMENT);
#elif ALIGNMENT_HACK
	long diff;

	if (!ptr)
		return a_malloc(size);
	diff = ((char*)ptr)[-1];
	ptr  = realloc((char*)ptr - diff, size + diff);
	if (ptr)
		ptr = (char*)ptr + diff;
	return ptr;
#else
	return realloc(ptr, size);
#endif
}

static void* Realloc(void* ptr, size_t size)
{
	s_AllocationMutex.lock();
	s_Allocations.erase(ptr);
	s_AllocationMutex.unlock();

	ptr = a_realloc(ptr, size);

    s_AllocationMutex.lock();
	s_Allocations.insert({ptr, {size, RewindCallStack()}});
	s_AllocationMutex.unlock();

    return ptr;
}

static void a_free(void* ptr)
{
	s_AllocationMutex.lock();
	auto size = s_Allocations[ptr].first;
	s_AllocationMutex.unlock();

	memset(ptr, 0, size);
	
    s_AllocationMutex.lock();
    s_Allocations.erase(ptr);
	s_AllocationMutex.unlock();

#ifdef ALIGNED_MALLOC
	_aligned_free(ptr);
#elif ALIGNMENT_HACK
	if (ptr)
		free((char*)ptr - ((char*)ptr)[-1]);
#else
	free(ptr);
#endif
}

static struct base_allocator alloc = {Alloc, Realloc, a_free};

#include <set>

void PrintLog()
{
	std::set<std::pair<size_t, uint32_t>> processedStuff;
	std::ofstream                         myfile;
	myfile.open("Leaks.txt");

	for (auto& log : s_Allocations)
    {
        auto iter = processedStuff.find({log.second.first, uint32_t(log.second.second.size())});
		if (iter != processedStuff.end()) {
            // Repeated stuff, ignore
			continue;
        }

        myfile << "-------------------------" << "\n";
		std::cout << "-------------------------" << std::endl;

        processedStuff.insert({log.second.first, uint32_t(log.second.second.size())});

		auto valuePair = log.second;
		for (auto& entry : valuePair.second)
        {
			myfile << entry << "\n";

			std::cout << entry << std::endl;
        }

         myfile << "-------------------------" << "\n";
		std::cout << "-------------------------" << std::endl;
    }

    while (true)
    {

    }
}

int main(int argc, char* argv[])
{
#ifndef _DEBUG

	struct CrashpadInfo
	{
		base::FilePath&           handler;
		base::FilePath&           db;
		std::string&              url;
		std::vector<std::string>& arguments;
		crashpad::CrashpadClient& client;
	};

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
        "https://sentry.io/api/1283431/minidump/?sentry_key=ec98eac4e3ce49c7be1d83c8fb2005ef");

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
	CrashpadInfo crashpadInfo = {handler, db, url, arguments, client};

    // Allocate a console window for this process
	AllocConsole();

	// Update the C/C++ runtime standard input, output, and error targets to use the console window
	BindCrtHandlesToStdHandles(true, true, true);

#endif

	// Usage:
	// argv[0] = Path to this application. (Usually given by default if run via path-based command!)
	// argv[1] = Path to a named socket.

	if (argc != 2) {
		std::cerr << "There must be exactly one parameter." << std::endl;
		return -1;
	}

	// Instance
	ipc::server myServer;
	bool        doShutdown = false;
	ServerData  sd;
	sd.last_disconnect = sd.last_connect = std::chrono::high_resolution_clock::now();
	sd.count_connected                   = 0;

	// Classes
	/// System
	{
		std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("System");
		cls->register_function(
		    std::make_shared<ipc::function>("Shutdown", std::vector<ipc::type>{}, System::Shutdown, &doShutdown));
		myServer.register_collection(cls);
	};

	/// OBS Studio Node
	osn::Global::Register(myServer);
	osn::Source::Register(myServer);
	osn::Input::Register(myServer);
	osn::Filter::Register(myServer);
	osn::Transition::Register(myServer);
	osn::Scene::Register(myServer);
	osn::SceneItem::Register(myServer);
	osn::Fader::Register(myServer);
	osn::VolMeter::Register(myServer);
	osn::Properties::Register(myServer);
	osn::Video::Register(myServer);
	osn::Module::Register(myServer);
	OBS_API::Register(myServer);
	OBS_content::Register(myServer);
	OBS_service::Register(myServer);
	OBS_settings::Register(myServer);
	OBS_settings::Register(myServer);
	autoConfig::Register(myServer);

    base_set_allocator(&alloc);

	// Register Connect/Disconnect Handlers
	myServer.set_connect_handler(ServerConnectHandler, &sd);
	myServer.set_disconnect_handler(ServerDisconnectHandler, &sd);

#ifndef _DEBUG

	// Handler for obs errors (mainly for bcrash() calls)
	base_set_crash_handler([](const char* format, va_list args, void* param) { 

		CrashpadInfo* crashpadInfo = (CrashpadInfo*)param;
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

	}, &crashpadInfo);

#endif

	// Initialize Server
	try {
		myServer.initialize(argv[1]);
	} catch (std::exception& e) {
		std::cerr << "Initialization failed with error " << e.what() << "." << std::endl;
		return -2;
	} catch (...) {
		std::cerr << "Failed to initialize server" << std::endl;
		return -2;
	}

	// Reset Connect/Disconnect time.
	sd.last_disconnect = sd.last_connect = std::chrono::high_resolution_clock::now();

	bool waitBeforeClosing = false;

	while (!doShutdown) {
		if (sd.count_connected == 0) {
			auto tp    = std::chrono::high_resolution_clock::now();
			auto delta = tp - sd.last_disconnect;
			if (std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() > 5000) {
				doShutdown = true;
				waitBeforeClosing = true;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Wait on receive the exit message from the crash-handler
	if (waitBeforeClosing) {
		HANDLE hPipe;
		TCHAR  chBuf[BUFFSIZE];
		DWORD  cbRead;
		hPipe = CreateNamedPipe(
		    TEXT("\\\\.\\pipe\\exit-slobs-crash-handler"),
		    PIPE_ACCESS_DUPLEX,
		    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		    1,
		    BUFFSIZE * sizeof(TCHAR),
		    BUFFSIZE * sizeof(TCHAR),
		    NULL,
		    NULL);

		if (hPipe != INVALID_HANDLE_VALUE) {
			if (ConnectNamedPipe(hPipe, NULL) != FALSE) {
				BOOL fSuccess = ReadFile(hPipe, chBuf, BUFFSIZE * sizeof(TCHAR), &cbRead, NULL);

				if (!fSuccess) {
					PrintLog();
					return 0;
                }
				CloseHandle(hPipe);
			}
		}
	}
	PrintLog();
	// Finalize Server
	myServer.finalize();

	return 0;
}
