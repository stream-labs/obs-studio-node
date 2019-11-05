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

#include "util-crashmanager.h"
#include "util-metricsprovider.h"

#include <chrono>
#include <codecvt>
#include <iostream>
#include <locale>
#include <map>
#include <obs.h>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "StackWalker.h"
#include "nodeobs_api.h"
#include "error.hpp"

#if defined(_WIN32)

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

#endif

#ifdef ENABLE_CRASHREPORT
#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"
#endif

//////////////////////
// STATIC VARIABLES //
//////////////////////

// Global/static variables
std::vector<std::string>                   handledOBSCrashes;
PDH_HQUERY                                 cpuQuery;
PDH_HCOUNTER                               cpuTotal;
std::vector<nlohmann::json>                breadcrumbs;
std::queue<std::pair<int, nlohmann::json>> lastActions;
std::vector<std::string>                   warnings;
std::chrono::steady_clock::time_point      initialTime;
std::mutex                                 messageMutex;
util::MetricsProvider                      metricsClient;
bool                                       reportsEnabled = true;

// Crashpad variables
#ifdef ENABLE_CRASHREPORT
std::wstring                                   appdata_path;
crashpad::CrashpadClient                       client;
std::unique_ptr<crashpad::CrashReportDatabase> database;
std::string                                    url;
base::FilePath                                 db;
base::FilePath                                 handler;
std::vector<std::string>                       arguments;
std::map<std::string, std::string>             annotations;
LPTOP_LEVEL_EXCEPTION_FILTER                   crashpadInternalExceptionFilterMethod = nullptr;
#endif

/////////////
// FORWARD //
/////////////

std::string    FormatVAString(const char* const format, va_list args);
nlohmann::json RewindCallStack(std::string& crashedMethod);

// Transform a byte value into a string + sufix
std::string PrettyBytes(uint64_t bytes)
{
	const char* suffixes[7];
	char        temp[100];
	suffixes[0]    = "b";
	suffixes[1]    = "kb";
	suffixes[2]    = "mb";
	suffixes[3]    = "gb";
	suffixes[4]    = "tb";
	suffixes[5]    = "pb";
	suffixes[6]    = "eb";
	uint64_t s     = 0; // which suffix to use
	double   count = double(bytes);
	while (count >= 1024 && s < 7) {
		s++;
		count /= 1024;
	}
	if (count - floor(count) == 0.0)
		sprintf(temp, "%d%s", (int)count, suffixes[s]);
	else
		sprintf(temp, "%.1f%s", count, suffixes[s]);

	return std::string(temp);
}

void RequestComputerUsageParams(
    long long& totalPhysMem,
    long long& physMemUsed,
    size_t&    physMemUsedByMe,
    double&    totalCPUUsed)
{
#if defined(_WIN32)

	MEMORYSTATUSEX          memInfo;
	PROCESS_MEMORY_COUNTERS pmc;
	PDH_FMT_COUNTERVALUE    counterVal;

	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	PdhCollectQueryData(cpuQuery);
	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);

	DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;

	totalPhysMem    = memInfo.ullTotalPhys;
	physMemUsed     = (memInfo.ullTotalPhys - memInfo.ullAvailPhys);
	physMemUsedByMe = pmc.WorkingSetSize;
	totalCPUUsed    = counterVal.doubleValue;

#else

	// This link has info about the linux and Mac OS versions
	// https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
	totalPhysMem    = long long(-1);
	physMemUsed     = long long(-1);
	physMemUsedByMe = size_t(-1);
	totalCPUUsed    = double(-1.0);

#endif
}

void GetUserInfo(std::string& computerName)
{
	TCHAR infoBuf[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD bufCharCount = MAX_COMPUTERNAME_LENGTH + 1;

	if (!GetComputerName(infoBuf, &bufCharCount))
		return;

	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	computerName = converterX.to_bytes(std::wstring(infoBuf));
}

nlohmann::json RequestProcessList()
{
	DWORD          aProcesses[1024], cbNeeded, cProcesses;
	nlohmann::json result = nlohmann::json::object();

#if defined(_WIN32)

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
		return 1;
	}

	// Calculate how many process identifiers were returned
	cProcesses = cbNeeded / sizeof(DWORD);

	// Get the name and process identifier for each process
	for (unsigned i = 0; i < cProcesses; i++) {
		if (aProcesses[i] != 0) {
			TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

			// Get a handle to the process
			DWORD  processID = aProcesses[i];
			HANDLE hProcess  = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

			// Get the process name.
			if (NULL != hProcess) {
				HMODULE hMod;
				DWORD   cbNeeded;

				if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
					GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));

					result.push_back(
					    {std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(szProcessName),
					     std::to_string(processID)});

					CloseHandle(hProcess);
				}
			}
		}
	}

#endif

	return result;
}

//////////////////
// CrashManager //
//////////////////

bool util::CrashManager::Initialize()
{
#ifdef ENABLE_CRASHREPORT
	annotations.insert({{"crashpad_status", "internal crash handler missed"}});

	if (!SetupCrashpad()) {
		return false;
	}

	// Handler for obs errors (mainly for bcrash() calls)
	base_set_crash_handler(
	    [](const char* format, va_list args, void* param) {
		    std::string errorMessage;
		    if (format == nullptr)
			    errorMessage = "unknown error";
		    else
			    errorMessage = FormatVAString(format, args);

		    // Check if this crash error is handled internally (if this is a known
		    // error that we can't do anything about it, just let the application
		    // crash normally
		    if (!TryHandleCrash(std::string(format), errorMessage))
			    HandleCrash(errorMessage);
	    },
	    nullptr);

	// Redirect all the calls from std::terminate
	std::set_terminate([]() { HandleCrash("Direct call to std::terminate"); });
	
#if defined(_WIN32)

	// Setup the windows exeption filter
	auto ExceptionHandlerMethod = [](struct _EXCEPTION_POINTERS* ExceptionInfo) {
		HandleCrash("UnhandledExceptionFilter", false);

		// Call the crashpad internal exception filter method since we overrided it here and
		// it must be called to proper generate a report
		return crashpadInternalExceptionFilterMethod(ExceptionInfo);
	};

	// This method will substitute the crashpad unhandled exception filter method by our one, returning
	// the old method used by it, we will store this method pointer to be able to call it directly
	crashpadInternalExceptionFilterMethod = SetUnhandledExceptionFilter(ExceptionHandlerMethod);

	// Setup the metrics query for the CPU usage
	// Ref: https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
	PdhOpenQuery(NULL, NULL, &cpuQuery);
	PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhCollectQueryData(cpuQuery);

#endif

	// The atexit will check if obs was safelly closed
	std::atexit(HandleExit);
	std::at_quick_exit(HandleExit);

#endif

	initialTime = std::chrono::steady_clock::now();

	return true;
}

void util::CrashManager::Configure()
{
	// Add all obs crash messages that are supposed to be handled by our application and
	// shouldn't cause a crash report (because there is no point on reporting since we
	// cannot control them)
	// You don't need to set the entire message, we will just check for a substring match
	// in the main error message
	{
		handledOBSCrashes.push_back("Failed to recreate D3D11");
		// ...
	}
}

bool util::CrashManager::SetupCrashpad()
{
	if (!reportsEnabled) {
		return false;
	}

	// Define if this is a preview or live version
	bool isPreview = OBS_API::getCurrentVersion().find("preview") != std::string::npos;

#ifdef ENABLE_CRASHREPORT

#if defined(_WIN32)
	HRESULT hResult;
	PWSTR   ppszPath;

	hResult = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath);

	appdata_path.assign(ppszPath);
	appdata_path.append(L"\\obs-studio-node-server");

	CoTaskMemFree(ppszPath);

#endif

	arguments.push_back("--no-rate-limit");

	std::wstring handler_path(L"crashpad_handler.exe");

	url = isPreview
	          ? std::string("https://sentry.io/api/1406061/minidump/?sentry_key=7376a60665cd40bebbd59d6bf8363172")
	          : std::string("https://sentry.io/api/1283431/minidump/?sentry_key=ec98eac4e3ce49c7be1d83c8fb2005ef");

	db      = base::FilePath(appdata_path);
	handler = base::FilePath(handler_path);

	database = crashpad::CrashReportDatabase::Initialize(db);
	if (database == nullptr || database->GetSettings() == NULL)
		return false;

	database->GetSettings()->SetUploadsEnabled(true);

	bool rc = client.StartHandler(handler, db, db, url, annotations, arguments, true, true);
	if (!rc)
		return false;

	rc = client.WaitForHandlerStart(INFINITE);
	if (!rc)
		return false;

#endif

	return true;
}

void util::CrashManager::HandleExit() noexcept
{
	// If we are exiting normally and obs is active, we have a problem because some
	// modules and threads could potentially be running and this will result in a
	// crash that is masked. The real issue here is why we are exiting without
	// finishing the obs first.
	if (obs_initialized()) {
		// Proceed to add more info to our crash reporter but don't call abort, we
		// cannot ensure that when at exit a call to obs_initialized will be safe, it
		// could be in an invalid state, we will let the application continue and if
		// this results in a crash at least we will know what caused it
		HandleCrash("AtExit", false);
	}
}

void util::CrashManager::HandleCrash(std::string _crashInfo, bool callAbort) noexcept
{
#ifdef ENABLE_CRASHREPORT

	// If for any reason this is true, it means that we are crashing inside this same
	// method, if that happens just call abort and ignore any remaining processing since
	// we cannot continue.
	static bool insideCrashMethod = false;
	static bool insideRewindCallstack = false; //if this is true then we already crashed inside StackWalker and try to skip it this time.
	if (insideCrashMethod && !insideRewindCallstack)
		abort();

	insideCrashMethod = true;
	annotations.clear();
	// This will manually rewind the callstack, we will use this info to populate an
	// crash report attribute, avoiding some cases that the memory dump is corrupted
	// and we don't have access to the callstack.
	std::string    crashedMethodName;
	nlohmann::json callStack;
	try {
		if(!insideRewindCallstack)
		{
			insideRewindCallstack = true;
			callStack = RewindCallStack(crashedMethodName);
			insideRewindCallstack = false;
		} else {
			annotations.insert({{"Recrashed_in", "RewindCallStack" }});
			callStack =  nlohmann::json::array();
		}
	} catch (...) {
		//ignore exceptions to not loose current crash info 
		callStack =  nlohmann::json::array();
	}
	
	int  known_crash_id = 0;
	
	if (is_allocator_failed()) {
		known_crash_id = 0x1;
	}

	if (known_crash_id != 0) {
		OBS_API::InformCrashHandler(known_crash_id);
	}

	// Get the information about the total of CPU and RAM used by this user
	long long totalPhysMem = 1;
	long long physMemUsed = 0;
	double    totalCPUUsed = 0.0;
	size_t    physMemUsedByMe = 0;
	std::string computerName;
	
	try {
		RequestComputerUsageParams(totalPhysMem, physMemUsed, physMemUsedByMe, totalCPUUsed);

		GetUserInfo(computerName);
	} catch (...) { }

	auto timeElapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - initialTime);


	// Setup all the custom annotations that are important too our crash report
	annotations.insert({{"Time elapsed: ", std::to_string(timeElapsed.count()) + "s"}});
	annotations.insert({{"Status", obs_initialized() ? "initialized" : "shutdown"}});
	annotations.insert({{"Leaks", std::to_string(bnum_allocs())}});
	annotations.insert({{"Total memory", PrettyBytes(totalPhysMem)}});
	annotations.insert({{"Total used memory",
	                     PrettyBytes(physMemUsed) + " - percentage: "
	                         + std::to_string(double(physMemUsed * 100) / double(totalPhysMem)) + "%"}});
	annotations.insert({{"Total SLOBS memory",
	                     PrettyBytes(physMemUsedByMe) + " - percentage: "
	                         + std::to_string(double(physMemUsedByMe * 100) / double(totalPhysMem)) + "%"}});
	annotations.insert({{"CPU usage", std::to_string(int(totalCPUUsed)) + "%"}});
	
	try {
		annotations.insert({{"Process List", RequestProcessList().dump(4)}});
	} catch (...) {}
	
	try {
		annotations.insert({{"OBS log general", RequestOBSLog(OBSLogType::General).dump(4)}});
		annotations.insert({{"Crash reason", _crashInfo}});
		annotations.insert({{"Computer name", computerName}});
		annotations.insert({{"Breadcrumbs", ComputeBreadcrumbs().dump(4)}});
		annotations.insert({{"Last actions", ComputeActions().dump(4)}});
		annotations.insert({{"Warnings", ComputeWarnings().dump(4)}});
	} catch (...) {}

	annotations.insert({{"sentry[release]", OBS_API::getCurrentVersion()}});
	annotations.insert({{"sentry[user][username]", OBS_API::getUsername()}});

	// If the callstack rewind operation returned an error, use it instead its result
	annotations.insert({{"Manual callstack", callStack.dump(4)}});

	// Recreate crashpad instance, this is a well defined/supported operation
	SetupCrashpad();

	// Finish the execution and let crashpad handle the crash
	if (callAbort)
		abort();

	insideCrashMethod = false;

#endif
}

bool util::CrashManager::TryHandleCrash(std::string _format, std::string _crashMessage)
{
	// This method can only be called by the obs-studio crash handler method, this means that
	// an internal error occurred.
	// handledOBSCrashes will contain all error messages that we should ignore from obs-studio,
	// like Dx11 errors for example, here we will check if this error is known, if true we will
	// try to finalize SLOBS in an attempt to NOT generate a crash report for it
	bool crashIsHandled = false;
	for (auto& handledCrashes : handledOBSCrashes) {
		if (std::string(_format).find(handledCrashes) != std::string::npos) {
			crashIsHandled = true;
			break;
		}
	}

	if (!crashIsHandled)
		return false;

	// If we are here this is a known crash that we don't wanna propagate to the crashpad
	// (because we don't need to handle it or we cannot control it), the idea would be just
	// a call to stop the crashpad handler but since it doesn't have a method like this we
	// can try to finish the application normally to avoid any crash report

	// Optionally send a message to the user here informing that SLOBS found an error and will
	// now close, potentially we could retrieve CPU and RAM usage and show a rich message
	// telling the user that he is using too much cpu/ram or process any Dx11 message and output
	// that to the user

	// Disable reports and kill the obs process
	DWORD pid = GetCurrentProcessId();
	HANDLE hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, pid);
	if (hnd != nullptr) {

		DisableReports();

		// Directly blame the user for this error since it was caused by the user side
		util::CrashManager::GetMetricsProvider()->SendStatus("Handled Crash");

		TerminateProcess(hnd, 0);
	}

	// Something really bad went wrong when killing this process, generate a crash report!
	util::CrashManager::HandleCrash(_crashMessage);

	// Unreachable statement
	return true;
}

// Format a var arg string into a c++ std::string type
std::string FormatVAString(const char* const format, va_list args)
{
	static const int MaximumVAStringSize = 63;
	auto temp   = std::vector<char>{};
	auto length = std::size_t{MaximumVAStringSize};
	while (temp.size() <= length) {
		temp.resize(length + 1);
		const auto status = std::vsnprintf(temp.data(), temp.size(), format, args);
		if (status < 0)
			throw std::runtime_error{"string formatting error"};
		length = static_cast<std::size_t>(status);
	}

	if (length > MaximumVAStringSize) {
		return "unknown error string";
	}

	return std::string{temp.data(), length};
}

nlohmann::json RewindCallStack(std::string& crashedMethod)
{
	class MyStackWalker : public StackWalker
	{
		public:
		MyStackWalker(nlohmann::json& _outJson, std::string& _crashMethod)
		    : StackWalker(), m_OutJson(_outJson), m_OutCrashMethodName(_crashMethod)
		{}

		protected:
		virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry)
		{
			// If this entry is valid
			if (entry.offset == 0)
				return;

			// If the entry is inside this file
			std::string fileName = std::string(entry.lineFileName);
			if (fileName.find("util-crashmanager.cpp") != std::string::npos
			    || fileName.find("stackwalker.cpp") != std::string::npos)
				return;

			nlohmann::json jsonEntry;
			if(strlen(entry.name) > 0)
			{
				jsonEntry["function"] = std::string(entry.name);
				entry.name[0] = 0x00;

				if(strlen(entry.lineFileName) > 0 )
					jsonEntry["filename"] = entry.lineFileName;
				entry.lineFileName[0] = 0x00;

				if(entry.lineNumber > 0)
					jsonEntry["lineno"] = entry.lineNumber;

				if(strlen(entry.moduleName) > 0)
					jsonEntry["module"] = std::string(entry.moduleName);
					entry.moduleName[0] = 0x00;

			} else {
				jsonEntry["function"] = "unknown";
			}
			
			// Check if we should update the crash method variable
			if (m_OutCrashMethodName.length() == 0)
				m_OutCrashMethodName = std::string(entry.name);

			m_OutJson.push_back(jsonEntry);
		}

		private:
		nlohmann::json& m_OutJson;
		std::string&    m_OutCrashMethodName;
	};

	nlohmann::json result = nlohmann::json::array();
	MyStackWalker  sw(result, crashedMethod);
	sw.ShowCallstack();

	return result;
}

nlohmann::json util::CrashManager::RequestOBSLog(OBSLogType type)
{
	nlohmann::json result;

	switch (type) {
	case OBSLogType::Errors: {
		auto& errors = OBS_API::getOBSLogErrors();
		for (auto& msg : errors)
			result.push_back(msg);
		break;
	}

	case OBSLogType::Warnings: {
		auto& warnings = OBS_API::getOBSLogWarnings();
		for (auto& msg : warnings)
			result.push_back(msg);
		break;
	}

	case OBSLogType::General: {
		auto& general = OBS_API::getOBSLogGeneral();
		while (!general.empty()) {
			result.push_back(general.front());
			general.pop();
		}

		break;
	}
	}

	std::reverse(result.begin(), result.end());

	return result;
}

nlohmann::json util::CrashManager::ComputeBreadcrumbs()
{
	nlohmann::json result = nlohmann::json::array();

	for (auto& msg : breadcrumbs)
		result.push_back(msg);

	return result;
}

nlohmann::json util::CrashManager::ComputeActions()
{
	nlohmann::json result = nlohmann::json::array();

	while (!lastActions.empty()) {
		auto counter = lastActions.front().first;
		auto message = lastActions.front().second;

		// Update the message to reflect the count amount, if applicable
		if (counter > 0) {
			message["repeat"] = counter;
		}

		result.push_back(message);
		lastActions.pop();
	}

	return result;
}

nlohmann::json util::CrashManager::ComputeWarnings()
{
	nlohmann::json result;

	for (auto& msg : warnings)
		result.push_back(msg);

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

void util::CrashManager::OpenConsole()
{
#if defined(_WIN32)

	// Allocate a console window for this process
	AllocConsole();

	// Update the C/C++ runtime standard input, output, and error targets to use the console window
	BindCrtHandlesToStdHandles(true, true, true);

#endif
}

void util::CrashManager::IPCValuesToData(const std::vector<ipc::value>& values, nlohmann::json& data)
{
	int paramCounter = 0;
	for (auto& value : values) {
		switch (value.type) {
		case ipc::type::Null: {
			data.push_back({{"arg" + std::to_string(paramCounter), "null"}});
			break;
		}
		case ipc::type::Float: {
			data.push_back({{"arg" + std::to_string(paramCounter), std::to_string(value.value_union.fp32)}});
			break;
		}
		case ipc::type::Double: {
			data.push_back({{"arg" + std::to_string(paramCounter), std::to_string(value.value_union.fp64)}});
			break;
		}
		case ipc::type::Int32: {
			data.push_back({{"arg" + std::to_string(paramCounter), std::to_string(value.value_union.i32)}});
			break;
		}
		case ipc::type::Int64: {
			data.push_back({{"arg" + std::to_string(paramCounter), std::to_string(value.value_union.i64)}});
			break;
		}
		case ipc::type::UInt32: {
			data.push_back({{"arg" + std::to_string(paramCounter), std::to_string(value.value_union.ui32)}});
			break;
		}
		case ipc::type::UInt64: {
			data.push_back({{"arg" + std::to_string(paramCounter), std::to_string(value.value_union.ui64)}});
			break;
		}
		case ipc::type::String: {
			data.push_back({{"arg" + std::to_string(paramCounter), value.value_str}});
			break;
		}
		case ipc::type::Binary: {
			data.push_back({{"arg" + std::to_string(paramCounter), ""}});
			break;
		}
		}

		paramCounter++;
	}
}

void util::CrashManager::AddWarning(const std::string& warning)
{
	std::lock_guard<std::mutex> lock(messageMutex);
	warnings.push_back(warning);
}

void RegisterAction(const nlohmann::json& message)
{
	static const int            MaximumActionsRegistered = 50;
	std::lock_guard<std::mutex> lock(messageMutex);

	// Check if this and the last message are the same, if true just add a counter
	if (lastActions.size() > 0 && lastActions.back().second == message) {
		lastActions.back().first++;
	} else {
		lastActions.push({0, message});
		if (lastActions.size() >= MaximumActionsRegistered) {
			lastActions.pop();
		}
	}
}

void util::CrashManager::AddBreadcrumb(const nlohmann::json& message)
{
	std::lock_guard<std::mutex> lock(messageMutex);
	breadcrumbs.push_back(message);
}

void util::CrashManager::AddBreadcrumb(const std::string& message)
{
	nlohmann::json j = nlohmann::json::array();
	j.push_back({{message}});

	std::lock_guard<std::mutex> lock(messageMutex);
	breadcrumbs.push_back(j);
}

void util::CrashManager::ClearBreadcrumbs()
{
	std::lock_guard<std::mutex> lock(messageMutex);
	breadcrumbs.clear();
}

void util::CrashManager::ProcessPreServerCall(std::string cname, std::string fname, const std::vector<ipc::value>& args)
{
	nlohmann::json jsonEntry;
	jsonEntry["cname"] = cname;
	jsonEntry["fname"] = fname;

	// Perform this only if this user have a high crash rate (TODO: this check must be implemented)
	/*
	nlohmann::json ipcValues = nlohmann::json::array();
	IPCValuesToData(args, ipcValues);
	jsonEntry["ipc values"] = ipcValues;
	*/

	RegisterAction(jsonEntry);
}

void util::CrashManager::ProcessPostServerCall(
    std::string                    cname,
    std::string                    fname,
    const std::vector<ipc::value>& args)
{
	if (args.size() == 0) {
		AddWarning(std::string("No return params on method ") + fname + std::string(" for class ") + cname);
	} else if ((ErrorCode)args[0].value_union.ui64 != ErrorCode::Ok) {
		AddWarning(
		    std::string("Server call returned error number ") + std::to_string(args[0].value_union.ui64) + " on method "
		    + fname + std::string(" for class ") + cname);
	}
}

void util::CrashManager::DisableReports()
{
	reportsEnabled = false;

#ifdef ENABLE_CRASHREPORT

	client.~CrashpadClient();
	database->~CrashReportDatabase();
	database = nullptr;

#endif
}

util::MetricsProvider* const util::CrashManager::GetMetricsProvider()
{
	return &metricsClient;
}