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

#include "nodeobs_api-server.h"
#include "osn-source.hpp"
#include "osn-scene.hpp"
#include "osn-sceneitem.hpp"
#include "osn-input.hpp"
#include "osn-transition.hpp"
#include "osn-filter.hpp"
#include "osn-volmeter.hpp"
#include "osn-fader.hpp"
#include "nodeobs_autoconfig-server.h"
#include "util/lexer.h"
#include "util-crashmanager.h"
#include "util-metricsprovider.h"
#include "../obs-studio-client/source/shared.hpp"

#include <sys/types.h>

#ifdef __APPLE
#include <unistd.h>
#endif

#ifdef _WIN32

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0502

#include <ShlObj.h>
#include <codecvt>
#include <locale>
#include <mutex>
#include <string>
#endif
#include "nodeobs_content.h"

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <audiopolicy.h>
#include <mmdeviceapi.h>

#include <util/windows/ComPtr.hpp>
#include <util/windows/HRError.hpp>
#include <util/windows/WinHandle.hpp>
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "shared-server.hpp"

#include <fstream>

#define BUFFSIZE 512
#define CONNECTING_STATE 0
#define READING_STATE 1
#define MBYTE (1024ULL * 1024ULL)
#define GBYTE (1024ULL * 1024ULL * 1024ULL)
#define TBYTE (1024ULL * 1024ULL * 1024ULL * 1024ULL)

enum crashHandlerCommand {
	REGISTER = 0,
	UNREGISTER = 1,
	REGISTERMEMORYDUMP = 2,
	CRASHWITHCODE = 3
};

std::string g_moduleDirectory = "";
os_cpu_usage_info_t* cpuUsageInfo      = nullptr;

std::string                                            slobs_plugin;
std::vector<std::pair<std::string, obs_module_t*>>     obsModules;
OBS_API::LogReport                                     logReport;
OBS_API::OutputStats                                   streamingOutputStats;
OBS_API::OutputStats                                   recordingOutputStats;
std::mutex                                             logMutex;
std::string                                            currentVersion;
std::string                                            username("unknown");
std::chrono::high_resolution_clock::time_point         start_wait_acknowledge;

#ifdef WIN32
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
inline std::wstring make_wide_string(std::string text) {
	return converter.from_bytes(text);
}
#endif

void OBS_API::SetWorkingDirectory(std::string path)
{
	g_moduleDirectory = path;
	replaceAll(g_moduleDirectory, "\\", "/");
}

#ifdef _WIN32
static void SetPrivilegeForGPUPriority(void)
{
	const DWORD      flags = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY;
	TOKEN_PRIVILEGES tp;
	HANDLE           token;
	LUID             val;

	if (!OpenProcessToken(GetCurrentProcess(), flags, &token)) {
		return;
	}

	if (!!LookupPrivilegeValue(NULL, SE_INC_BASE_PRIORITY_NAME, &val)) {
		tp.PrivilegeCount           = 1;
		tp.Privileges[0].Luid       = val;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!AdjustTokenPrivileges(token, false, &tp, sizeof(tp), NULL, NULL)) {
			blog(LOG_INFO, "Could not set privilege to increase GPU priority");
		}
	}

	CloseHandle(token);
}
#endif

static std::string GenerateTimeDateFilename(const char* extension)
{
	time_t     now       = time(0);
	char       file[256] = {};
	struct tm* cur_time;

	cur_time = localtime(&now);
	snprintf(
	    file,
	    sizeof(file),
	    "%d-%02d-%02d %02d-%02d-%02d.%s",
	    cur_time->tm_year + 1900,
	    cur_time->tm_mon + 1,
	    cur_time->tm_mday,
	    cur_time->tm_hour,
	    cur_time->tm_min,
	    cur_time->tm_sec,
	    extension);

	return std::string(file);
}

static bool GetToken(lexer* lex, std::string& str, base_token_type type)
{
	base_token token;
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != type)
		return false;

	str.assign(token.text.array, token.text.len);
	return true;
}

static bool ExpectToken(lexer* lex, const char* str, base_token_type type)
{
	base_token token;
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != type)
		return false;

	return strref_cmp(&token.text, str) == 0;
}

/* os_dirent mimics POSIX dirent structure.
* Perhaps a better cross-platform solution can take
* place but this is as cross-platform as it gets
* for right now.  */
static uint64_t ConvertLogName(const char* name)
{
	lexer  lex;
	std::string year, month, day, hour, minute, second;

	lexer_init(&lex);
	lexer_start(&lex, name);

	if (!GetToken(&lex, year, BASETOKEN_DIGIT))
		return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER))
		return 0;
	if (!GetToken(&lex, month, BASETOKEN_DIGIT))
		return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER))
		return 0;
	if (!GetToken(&lex, day, BASETOKEN_DIGIT))
		return 0;
	if (!GetToken(&lex, hour, BASETOKEN_DIGIT))
		return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER))
		return 0;
	if (!GetToken(&lex, minute, BASETOKEN_DIGIT))
		return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER))
		return 0;
	if (!GetToken(&lex, second, BASETOKEN_DIGIT))
		return 0;

	std::string timestring(year);
	timestring += month + day + hour + minute + second;
	lexer_free(&lex);
	return std::stoull(timestring);
}

static void DeleteOldestFile(const char* location, unsigned maxLogs)
{
	std::string            oldestLog;
	uint64_t          oldest_ts = (uint64_t)-1;
	struct os_dirent* entry;

	os_dir_t* dir = os_opendir(location);

	if (!dir) {
		std::cout << "Failed to open log directory." << std::endl;
	}

	unsigned count = 0;

	while ((entry = os_readdir(dir)) != NULL) {
		if (entry->directory || *entry->d_name == '.')
			continue;

		uint64_t ts = ConvertLogName(entry->d_name);

		if (ts) {
			if (ts < oldest_ts) {
				oldestLog = entry->d_name;
				oldest_ts = ts;
			}

			count++;
		}
	}

	os_closedir(dir);

	if (count > maxLogs) {
		std::string delPath;

		delPath = delPath + location + "/" + oldestLog;
		os_unlink(delPath.c_str());
	}
}

#include <chrono>
#include <cstdarg>
#include <stdarg.h>

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#else
#include <unistd.h>
#endif

outdated_driver_error * outdated_driver_error::inst = nullptr;

outdated_driver_error * outdated_driver_error::instance()
{
	if(inst == nullptr)
	{
		inst = new outdated_driver_error();
	}
	return inst;
}

void outdated_driver_error::set_active( bool state) 
{
	if (state) {
		if (!lookup_enabled)
		{
			line_1 = "";
			line_2 = "";
		}
		lookup_enabled++;
	} else {
		lookup_enabled--;
	}
}

std::string outdated_driver_error::get_error()
{
	if(line_1.size() && line_2.size())
		return line_1 + std::string("\n") + line_2;
	else 
		return "";
}

void outdated_driver_error::catch_error(const char* msg)
{
	if (!lookup_enabled)
		return;

	const std::string msg_string = msg;

	if (line_1.size()==0) {
		const std::string line_1_pattern = "Driver does not support the required nvenc API version";
		auto pattern_position = msg_string.find(line_1_pattern);
		if ( pattern_position != std::string::npos) {
			line_1 = msg_string.substr(pattern_position);
			return;
		}
	} else if (line_2.size()==0) {
		const std::string line_2_pattern = "The minimum required Nvidia driver for nvenc";
		auto pattern_position = msg_string.find(line_2_pattern);
		if ( pattern_position != std::string::npos) {
			line_2 = msg_string.substr(pattern_position);
		}
	}
}

inline std::string nodeobs_log_formatted_message(const char* format, va_list args)
{
	if (!format)
		return "";
#ifdef WIN32
	size_t            length  = _vscprintf(format, args);
#else
	va_list argcopy;
	va_copy(argcopy, args);
	size_t            length  = vsnprintf(NULL, 0, format, argcopy);
#endif
	std::vector<char> buf     = std::vector<char>(length + 1, '\0');
	size_t            written = vsprintf(buf.data(), format, args);
	return std::string(buf.begin(), buf.begin() + length);
}

std::chrono::high_resolution_clock             hrc;
std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
static void                                    node_obs_log(int log_level, const char* msg, va_list args, void* param)
{
	std::lock_guard<std::mutex> lock(logMutex);
	if (param == nullptr)
		return;
	
	outdated_driver_error::instance()->catch_error(msg);

	// Calculate log time.
	auto timeSinceStart = (std::chrono::high_resolution_clock::now() - tp);
	auto days           = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<86400>>>(timeSinceStart);
	timeSinceStart -= days;
	auto hours = std::chrono::duration_cast<std::chrono::hours>(timeSinceStart);
	timeSinceStart -= hours;
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(timeSinceStart);
	timeSinceStart -= minutes;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeSinceStart);
	timeSinceStart -= seconds;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceStart);
	timeSinceStart -= milliseconds;
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(timeSinceStart);
	timeSinceStart -= microseconds;
	auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(timeSinceStart);

	// Generate timestamp and log_level part.
	/// Convert level int to human readable name
	std::string levelname = "";
	switch (log_level) {
	case LOG_INFO:
		levelname = "Info";
		break;
	case LOG_WARNING:
		levelname = "Warning";
		break;
	case LOG_ERROR:
		levelname = "Error";
		break;
	case LOG_DEBUG:
		levelname = "Debug";
		break;
	default:
		if (log_level <= 50) {
			levelname = "Critical";
		} else if (log_level > 50 && log_level < LOG_ERROR) {
			levelname = "Error";
		} else if (log_level > LOG_ERROR && log_level < LOG_WARNING) {
			levelname = "Alert";
		} else if (log_level > LOG_WARNING && log_level < LOG_INFO) {
			levelname = "Hint";
		} else if (log_level > LOG_INFO) {
			levelname = "Notice";
		}
		break;
	}

	std::vector<char> timebuf(128, '\0');
	std::string       timeformat = "[%.3d:%.2d:%.2d:%.2d.%.3d.%.3d.%.3d][%*s]"; // "%*s";
#ifdef WIN32
	int length     = sprintf_s(
        timebuf.data(),
        timebuf.size(),
        timeformat.c_str(),
        days.count(),
        hours.count(),
        minutes.count(),
        seconds.count(),
        milliseconds.count(),
        microseconds.count(),
        nanoseconds.count(),
        levelname.length(),
        levelname.c_str());
#else
	int length     = snprintf(
        timebuf.data(),
        timebuf.size(),
        timeformat.c_str(),
        days.count(),
        hours.count(),
        minutes.count(),
        seconds.count(),
        milliseconds.count(),
        microseconds.count(),
        nanoseconds.count(),
        levelname.length(),
        levelname.c_str());
#endif
	if (length < 0)
		return;

	std::string time_and_level = std::string(timebuf.data(), length);

	// Format incoming text
	std::string text = nodeobs_log_formatted_message(msg, args);

	std::fstream* logStream = reinterpret_cast<std::fstream*>(param);

	// Split by \n (new-line)
	size_t last_valid_idx = 0;
	for (size_t idx = 0; idx <= text.length(); idx++) {
		if ((idx == text.length()) || (text[idx] == '\n')) {
			std::string newmsg = time_and_level + " " + std::string(&text[last_valid_idx], idx - last_valid_idx) + '\n';
			last_valid_idx     = idx + 1;

			// File Log
			*logStream << newmsg << std::flush;

            // Internal Log
			logReport.push(newmsg, log_level);

			// Std Out / Std Err
			/// Why fwrite and not std::cout and std::cerr?
			/// Well, it seems that std::cout and std::cerr break if you click in the console window and paste.
			/// Which is really bad, as nothing gets logged into the console anymore.
			if (log_level <= LOG_WARNING) {
				fwrite(newmsg.data(), sizeof(char), newmsg.length(), stderr);
			}
			fwrite(newmsg.data(), sizeof(char), newmsg.length(), stdout);

			// Debugger
#ifdef _WIN32
			if (IsDebuggerPresent()) {
				int wNum = MultiByteToWideChar(CP_UTF8, 0, newmsg.c_str(), -1, NULL, 0);
				if (wNum > 1) {
					std::wstring wide_buf;
					std::mutex   wide_mutex;

					std::lock_guard<std::mutex> lock(wide_mutex);
					wide_buf.reserve(wNum + 1);
					wide_buf.resize(wNum - 1);
					MultiByteToWideChar(CP_UTF8, 0, newmsg.c_str(), -1, &wide_buf[0], wNum);

					OutputDebugStringW(wide_buf.c_str());
				}
			}
#endif
		}
	}
	*logStream << std::flush;

#if defined(_WIN32) && defined(OBS_DEBUGBREAK_ON_ERROR)
	if (log_level <= LOG_ERROR && IsDebuggerPresent())
		__debugbreak();
#endif
}

#ifdef WIN32
uint32_t pid = GetCurrentProcessId();
#else
uint32_t pid = (uint32_t)getpid();
#endif

std::vector<char> registerProcess(void)
{
	std::vector<char> buffer;
	uint8_t action     = crashHandlerCommand::REGISTER;
	bool    isCritical = true;
	buffer.resize(sizeof(action) + sizeof(isCritical) + sizeof(pid));

	uint32_t offset = 0;

	memcpy(buffer.data(), &action, sizeof(action));
	offset++;
	memcpy(buffer.data() + offset, &isCritical, sizeof(isCritical));
	offset++;
	memcpy(buffer.data() + offset, &pid, sizeof(pid));

	return buffer;
}

std::vector<char> registerMemoryDump(void)
{
	const uint8_t action = crashHandlerCommand::REGISTERMEMORYDUMP;

	//str
	std::wstring eventName_Start = util::CrashManager::GetMemoryDumpEventName_Start();
	uint32_t eventName_Start_Size = (eventName_Start.size() + 1) * sizeof(wchar_t);

	//str
	std::wstring eventName_Fail = util::CrashManager::GetMemoryDumpEventName_Fail();
	uint32_t eventName_Fail_Size = (eventName_Fail.size() + 1) * sizeof(wchar_t);

	//str
	std::wstring eventName_Success = util::CrashManager::GetMemoryDumpEventName_Success();
	uint32_t eventName_Success_Size = (eventName_Success.size() + 1) * sizeof(wchar_t);

	//str
	std::wstring dumpPath = util::CrashManager::GetMemoryDumpPath();
	uint32_t dumpPathSize = (dumpPath.size() + 1) * sizeof(wchar_t);

	//str
	std::wstring dumpName = util::CrashManager::GetMemoryDumpName();
	uint32_t dumpNameSize = (dumpName.size() + 1) * sizeof(wchar_t);

	// Buffer
	std::vector<char> buffer;
	buffer.resize(sizeof(action) + sizeof(pid) + sizeof(int) + eventName_Start_Size + sizeof(int) + eventName_Fail_Size + sizeof(int) + eventName_Success_Size + sizeof(int) + dumpPathSize + sizeof(int) + dumpNameSize);
	uint32_t offset = 0;

	//@uint32_t - pid
	memcpy(buffer.data(), &action, sizeof(action));
	offset++;
	memcpy(buffer.data() + offset, &pid, sizeof(pid));
	offset += sizeof(pid);

	//@str - eventName_Start
	memcpy(buffer.data() + offset, &eventName_Start_Size, sizeof(eventName_Start_Size));
	offset += sizeof(eventName_Start_Size);
	memcpy(buffer.data() + offset, &eventName_Start[0], eventName_Start_Size);
	offset += eventName_Start_Size;

	//@str - eventName_Fail
	memcpy(buffer.data() + offset, &eventName_Fail_Size, sizeof(eventName_Fail_Size));
	offset += sizeof(eventName_Fail_Size);
	memcpy(buffer.data() + offset, &eventName_Fail[0], eventName_Fail_Size);
	offset += eventName_Fail_Size;

	//@str - eventName_Success
	memcpy(buffer.data() + offset, &eventName_Success_Size, sizeof(eventName_Success_Size));
	offset += sizeof(eventName_Success_Size);
	memcpy(buffer.data() + offset, &eventName_Success[0], eventName_Success_Size);
	offset += eventName_Success_Size;

	//@str - dumpPath
	memcpy(buffer.data() + offset, &dumpPathSize, sizeof(dumpPathSize));
	offset+=sizeof(dumpPathSize);
	memcpy(buffer.data() + offset, &dumpPath[0], dumpPathSize);
	offset+=dumpPathSize;

	//@str - dumpName
	memcpy(buffer.data() + offset, &dumpNameSize, sizeof(dumpNameSize));
	offset += sizeof(dumpNameSize);
	memcpy(buffer.data() + offset, &dumpName[0], dumpNameSize);
	offset += dumpNameSize;

	return buffer;
}

std::vector<char> unregisterProcess(void)
{
	std::vector<char> buffer;
	uint8_t action = crashHandlerCommand::UNREGISTER;
	buffer.resize(sizeof(action) + sizeof(pid));

	uint32_t offset = 0;

	memcpy(buffer.data(), &action, sizeof(action));
	offset++;
	memcpy(buffer.data() + offset, &pid, sizeof(pid));

	return buffer;
}

std::vector<char> crashedProcess(uint32_t crash_id)
{
	std::vector<char> buffer;
	uint8_t action = crashHandlerCommand::CRASHWITHCODE;
	buffer.resize(sizeof(action) + sizeof(crash_id) + sizeof(pid));

	uint32_t offset = 0;

	memcpy(buffer.data(), &action, sizeof(action));
	offset++;
	memcpy(buffer.data() + offset, &crash_id, sizeof(crash_id));
	offset+=sizeof(crash_id);
	memcpy(buffer.data() + offset, &pid, sizeof(pid));

	return buffer;
}

#ifdef WIN32
std::wstring crash_handler_pipe;

void OBS_API::SetCrashHandlerPipe(const std::wstring &new_pipe)
{
	crash_handler_pipe = std::wstring(L"\\\\.\\pipe\\") + new_pipe + std::wstring(L"-crash-handler");
}

void writeCrashHandler(std::vector<char> buffer)
{
	HANDLE hPipe = CreateFile( crash_handler_pipe.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
		hPipe = CreateFile( TEXT("\\\\.\\pipe\\slobs-crash-handler"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
		return;

	DWORD bytesWritten;

	WriteFile(hPipe, buffer.data(), buffer.size(), &bytesWritten, NULL);

	CloseHandle(hPipe);
}
#else
std::string crash_handler_pipe;

void OBS_API::SetCrashHandlerPipe(const std::wstring &new_pipe)
{
	crash_handler_pipe = std::string(new_pipe.begin(), new_pipe.end()) + std::string("-crash-handler");
}

void writeCrashHandler(std::vector<char> buffer)
{
	int file_descriptor = open(crash_handler_pipe.c_str(), O_WRONLY | O_DSYNC);
	if (file_descriptor < 0) {
		blog(LOG_DEBUG, "failed to open pipe %s ", crash_handler_pipe.c_str());
		file_descriptor = open("/tmp/slobs-crash-handler", O_WRONLY | O_DSYNC);
		if (file_descriptor < 0) {
			blog(LOG_DEBUG, "failed to open pipe /tmp/slobs-crash-handler ");
			return;
		}
	}

	::write(file_descriptor, buffer.data(), buffer.size());
	close(file_descriptor);
}
#endif
int OBS_API::OBS_API_initAPI(
    std::string appdata,
    std::string locale,
    std::string a_currentVersion,
	std::string crashserverurl)
{
	writeCrashHandler(registerProcess());

	/* Map base DLLs as soon as possible into the current process space.
	* In particular, we need to load obs.dll into memory before we call
	* any functions from obs else if we delay-loaded the dll, it will
	* fail miserably. */

	currentVersion = a_currentVersion;
	utility_server::osn_current_version(currentVersion);

#ifdef ENABLE_CRASHREPORT
// 	util::CrashManager crashManager;
// 	crashManager.SetVersionName(currentVersion);
// 	blog(LOG_INFO, "INIT - 1");
// 	crashManager.SetReportServerUrl(crashserverurl);
// 	blog(LOG_INFO, "INIT - 2");
// 	char* path = new char [g_moduleDirectory.length()+1];
// 	blog(LOG_INFO, "INIT - 3");
// 	std::strcpy (path, g_moduleDirectory.c_str());
// 	blog(LOG_INFO, "INIT - 4");
// 	if (crashManager.Initialize(path, appdata)) {
// 		blog(LOG_INFO, "INIT - 5");
// 		crashManager.Configure();
// 		blog(LOG_INFO, "INIT - 6");
// 		if (crashManager.InitializeMemoryDump()) {
// 			writeCrashHandler(registerMemoryDump());
// 		}
// 		blog(LOG_INFO, "INIT - 7");
//    }

// 	blog(LOG_INFO, "INIT - 1");
#ifdef WIN32
	// Register the pre and post server callbacks to log the data into the crashmanager
	// g_server->set_pre_callback([](std::string cname, std::string fname, const std::vector<ipc::value>& args, void* data)
	// { 
	// 	util::CrashManager& crashManager = *static_cast<util::CrashManager*>(data);
	// 	crashManager.ProcessPreServerCall(cname, fname, args);

	// }, &crashManager);
	// g_server->set_post_callback([](std::string cname, std::string fname, const std::vector<ipc::value>& args, void* data)
	// {
	// 	util::CrashManager& crashManager = *static_cast<util::CrashManager*>(data);
	// 	crashManager.ProcessPostServerCall(cname, fname, args);
	// }, &crashManager);

#endif
#endif

#ifdef WIN32
	// Connect the metrics provider with our crash handler process, sending our current version tag
	// and enabling metrics
	util::CrashManager::GetMetricsProvider()->Initialize("\\\\.\\pipe\\metrics_pipe", currentVersion, false);
#endif
	blog(LOG_INFO, "g_moduleDirectory: %s", g_moduleDirectory.c_str());
	obs_add_data_path((g_moduleDirectory + "/data/libobs/").c_str());
	slobs_plugin = appdata.substr(0, appdata.size() - strlen("/slobs-client"));
	slobs_plugin.append("/slobs-plugins");
	obs_add_data_path((slobs_plugin + "/data/").c_str());

	blog(LOG_INFO, "INIT - 0");
	std::vector<char> userData = std::vector<char>(1024);
	os_get_config_path(userData.data(), userData.capacity() - 1, "slobs-client/plugin_config");
    if (!obs_startup(locale.c_str(), userData.data(), NULL)) {
            // TODO: We should return an error code if obs fails to initialize.
            // This was added as a temporary measure to detect what could be happening in some
            // cases (if the user data path is wrong for ex). This will be correctly adjusted
            // when init API supports more return codes.
#ifdef WIN32
            std::string userDataPath = std::string(userData.begin(), userData.end());
            util::CrashManager::AddWarning("Failed to start OBS, locale: " + locale + " user data: " + userDataPath);
#endif
	}

	blog(LOG_INFO, "INIT - 1");
	/* Logging */
	std::string filename = GenerateTimeDateFilename("txt");
	std::string log_path = appdata;
	log_path.append("/node-obs/logs/");

	blog(LOG_INFO, "INIT - 2");
	/* Make sure the path is created
	before attempting to make a file there. */
	if (os_mkdirs(log_path.c_str()) == MKDIR_ERROR) {
		std::cerr << "Failed to open log file" << std::endl;
#ifdef WIN32
		util::CrashManager::AddWarning("Error on log file, failed to create path: " + log_path);
#endif	
	}

	blog(LOG_INFO, "INIT - 3");
	/* Delete oldest file in the folder to imitate rotating */
	DeleteOldestFile(log_path.c_str(), 3);
	log_path.append(filename);

#if defined(_WIN32) && defined(UNICODE)
	std::fstream* logfile =
	    new std::fstream(converter.from_bytes(log_path.c_str()).c_str(), std::ios_base::out | std::ios_base::trunc);
#else
	blog(LOG_INFO, "INIT - 3.1");
	blog(LOG_INFO, "log_path: %s", log_path.c_str());
	std::fstream* logfile = new std::fstream(log_path, std::ios_base::out | std::ios_base::trunc);
#endif
	if (!logfile->is_open()) {
		logfile = nullptr;
		util::CrashManager::AddWarning("Error on log file, failed to open: " + log_path);
		std::cerr << "Failed to open log file" << std::endl;
	}
	blog(LOG_INFO, "INIT - 3.2");
	base_set_log_handler(node_obs_log, logfile);
	blog(LOG_INFO, "INIT - 3.3");
#ifndef _DEBUG
	// Redirect the ipc log callbacks to our log handler
	// ipc::register_log_callback([](void* data, const char* fmt, va_list args) { 
	// 	blogva(LOG_ERROR, fmt, args);
	// }, nullptr);
#endif

	blog(LOG_INFO, "INIT - 4");
#ifdef _WIN32
	SetPrivilegeForGPUPriority();
#endif

	obs::Source::initialize_global_signals();

	cpuUsageInfo = os_cpu_usage_info_start();
	ConfigManager::getInstance().setAppdataPath(appdata);

	blog(LOG_INFO, "INIT - 5");
	/* Set global private settings for whomever it concerns */
	bool        browserHWAccel   = config_get_bool(ConfigManager::getInstance().getGlobal(), "General", "BrowserHWAccel");
	obs_data_t* private_settings = obs_data_create();
	obs_data_set_bool(private_settings, "BrowserHWAccel", browserHWAccel);
	obs_apply_private_data(private_settings);
	obs_data_release(private_settings);

	int videoError;
	if (!openAllModules(videoError)) {
#ifdef WIN32
		util::CrashManager::GetMetricsProvider()->BlameUser();

		blog(LOG_INFO, "Error returning now");
		return videoError;
#endif
	}

	OBS_service::createService();
	OBS_service::createStreamingOutput();
	OBS_service::createRecordingOutput();
	OBS_service::createReplayBufferOutput();

	OBS_service::createVideoStreamingEncoder();
	OBS_service::createVideoRecordingEncoder();

	OBS_service::resetAudioContext();
	OBS_service::resetVideoContext();

	OBS_service::setupAudioEncoder();

	setAudioDeviceMonitoring();

	// Enable the hotkey callback rerouting that will be used when manually handling hotkeys on the frontend
	obs_hotkey_enable_callback_rerouting(true);

	// Init replay buffer rendering mode
	const char* currentOutputMode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");
	bool        simple            = true;

	if (currentOutputMode)
		simple = strcmp(currentOutputMode, "Simple") == 0;

	enum obs_replay_buffer_rendering_mode mode = OBS_STREAMING_REPLAY_BUFFER_RENDERING;

	bool useStreamOutput = config_get_bool(
	    ConfigManager::getInstance().getBasic(), simple ? "SimpleOutput" : "AdvOut", "replayBufferUseStreamOutput");

	obs_set_replay_buffer_rendering_mode(
		useStreamOutput ? OBS_STREAMING_REPLAY_BUFFER_RENDERING : OBS_RECORDING_REPLAY_BUFFER_RENDERING);

	util::CrashManager::setAppState("idle");

	// We are returning a video result here because the frontend needs to know if we sucessfully
	// initialized the Dx11 API
	return OBS_VIDEO_SUCCESS;
}

void OBS_API::OBS_API_destroyOBS_API()
{
	/* INJECT obs::Source::Manager */
	// Alright, you're probably wondering: Why is osn code here?
	// Well, simply because the hooks need to run as soon as possible. We don't
	//  want to miss a single create or destroy signal OBS gives us for the
	//  obs::Source::Manager.
	obs::Source::finalize_global_signals();
	/* END INJECT obs::Source::Manager */
	destroyOBS_API();
}

std::vector<OBS_API::HotkeyInfo> OBS_API::QueryHotkeys()
{
	// For each registered hotkey
	std::vector<HotkeyInfo> hotkeyInfos;
	obs_enum_hotkeys(
	    [](void* data, obs_hotkey_id id, obs_hotkey_t* key) {
		    // Make sure every word has an initial capital letter
		    auto ToTitle = [](std::string s) {
			    bool last = true;
			    for (char& c : s) {
				    int full_c = (uint8_t)c;
				    full_c     = last ? ::toupper(full_c) : ::tolower(full_c);
				    last       = ::isspace(full_c);
			    }
			    return s;
		    };
		    std::vector<HotkeyInfo>& hotkeyInfos     = *static_cast<std::vector<HotkeyInfo>*>(data);
		    auto                     registerer_type = obs_hotkey_get_registerer_type(key);
		    void*                    registerer      = obs_hotkey_get_registerer(key);
		    HotkeyInfo               currentHotkeyInfo;
		    if (registerer == nullptr)
			    return true;

		    // Discover the type of object registered with this hotkey
		    switch (registerer_type) {
		    case OBS_HOTKEY_REGISTERER_NONE: 
		    case OBS_HOTKEY_REGISTERER_FRONTEND: {
			    // Ignore any frontend hotkey
			    return true;
			    break;
		    }
		    case OBS_HOTKEY_REGISTERER_SOURCE: {
			    auto* weak_source            = static_cast<obs_weak_source_t*>(registerer);
			    auto  key_source             = OBSGetStrongRef(weak_source);
			    if (key_source == nullptr)
				    return true;
			    currentHotkeyInfo.objectName = obs_source_get_name(key_source);
			    currentHotkeyInfo.objectType = OBS_HOTKEY_REGISTERER_SOURCE;
			    break;
		    }
		    case OBS_HOTKEY_REGISTERER_OUTPUT: {
			    auto* weak_output            = static_cast<obs_weak_output_t*>(registerer);
			    auto  key_output             = OBSGetStrongRef(weak_output);
			    if (key_output == nullptr)
				    return true;
			    currentHotkeyInfo.objectName = obs_output_get_name(key_output);
			    currentHotkeyInfo.objectType = OBS_HOTKEY_REGISTERER_OUTPUT;
			    break;
		    }
		    case OBS_HOTKEY_REGISTERER_ENCODER: {
			    auto* weak_encoder           = static_cast<obs_weak_encoder_t*>(registerer);
			    auto  key_encoder            = OBSGetStrongRef(weak_encoder);
			    if (key_encoder == nullptr)
				    return true;
			    currentHotkeyInfo.objectName = obs_encoder_get_name(key_encoder);
			    currentHotkeyInfo.objectType = OBS_HOTKEY_REGISTERER_ENCODER;
			    break;
		    }
		    case OBS_HOTKEY_REGISTERER_SERVICE: {
			    auto* weak_service           = static_cast<obs_weak_service_t*>(registerer);
			    auto  key_service            = OBSGetStrongRef(weak_service);
			    if (key_service == nullptr)
				    return true;
			    currentHotkeyInfo.objectName = obs_service_get_name(key_service);
			    currentHotkeyInfo.objectType = OBS_HOTKEY_REGISTERER_SERVICE;
			    break;
		    }
		    }

		    // Key defs
		    const char* _key_name = obs_hotkey_get_name(key);
		    const char* _desc     = obs_hotkey_get_description(key);

		    if (!_key_name)
			    return true;

		    if (!_desc)
			    _desc = "";

		    auto       key_name = std::string(_key_name);
		    auto       desc     = std::string(_desc);
		    const auto hotkeyId = obs_hotkey_get_id(key);

		    // Parse the key name and the description
		    key_name = key_name.substr(key_name.find_first_of(".") + 1);
		    std::replace(key_name.begin(), key_name.end(), '-', '_');
		    std::transform(key_name.begin(), key_name.end(), key_name.begin(), ::toupper);
		    std::replace(desc.begin(), desc.end(), '-', ' ');
		    desc = ToTitle(desc);

		    currentHotkeyInfo.hotkeyName = key_name;
		    currentHotkeyInfo.hotkeyDesc = desc;
		    currentHotkeyInfo.hotkeyId   = hotkeyId;
		    hotkeyInfos.push_back(currentHotkeyInfo);

		    return true;
	    },
	    &hotkeyInfos);

	return hotkeyInfos;
}

void OBS_API::ProcessHotkeyStatus(obs_hotkey_id hotkeyId, bool pressed)
{
	// TODO: Check if the hotkey ID is valid
	obs_hotkey_trigger_routed_callback(hotkeyId, pressed);
}

void OBS_API::SetUsername(std::string a_username)
{
	username = a_username;
	util::CrashManager::SetUsername(username);
}

void OBS_API::SetProcessPriority(const char* priority)
{
	if (!priority)
		return;
#ifdef WIN32
	if (strcmp(priority, "High") == 0)
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	else if (strcmp(priority, "AboveNormal") == 0)
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	else if (strcmp(priority, "Normal") == 0)
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	else if (strcmp(priority, "BelowNormal") == 0)
		SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
	else if (strcmp(priority, "Idle") == 0)
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif
}

void OBS_API::UpdateProcessPriority()
{
	const char* priority = config_get_string(ConfigManager::getInstance().getGlobal(), "General", "ProcessPriority");
	if (priority && strcmp(priority, "Normal") != 0)
		SetProcessPriority(priority);
}

bool DisableAudioDucking(bool disable)
{
#ifdef WIN32
	ComPtr<IMMDeviceEnumerator>   devEmum;
	ComPtr<IMMDevice>             device;
	ComPtr<IAudioSessionManager2> sessionManager2;
	ComPtr<IAudioSessionControl>  sessionControl;
	ComPtr<IAudioSessionControl2> sessionControl2;

	HRESULT result = CoCreateInstance(
	    __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&devEmum);
	if (FAILED(result))
		return false;

	result = devEmum->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	if (FAILED(result))
		return false;

	result = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, nullptr, (void**)&sessionManager2);
	if (FAILED(result))
		return false;

	result = sessionManager2->GetAudioSessionControl(nullptr, 0, &sessionControl);
	if (FAILED(result))
		return false;

	result = sessionControl->QueryInterface(&sessionControl2);
	if (FAILED(result))
		return false;

	result = sessionControl2->SetDuckingPreference(disable);
	return SUCCEEDED(result);
#else
    return false;
#endif
}

void OBS_API::setAudioDeviceMonitoring(void)
{
/* load audio monitoring */
#if defined(_WIN32) || defined(__APPLE__)
	const char* device_name =
	    config_get_string(ConfigManager::getInstance().getBasic(), "Audio", "MonitoringDeviceName");
	const char* device_id = config_get_string(ConfigManager::getInstance().getBasic(), "Audio", "MonitoringDeviceId");

	obs_set_audio_monitoring_device(device_name, device_id);

	blog(LOG_INFO, "Audio monitoring device:\n\tname: %s\n\tid: %s", device_name, device_id);

	bool disableAudioDucking = config_get_bool(ConfigManager::getInstance().getBasic(), "Audio", "DisableAudioDucking");
	if (disableAudioDucking)
		DisableAudioDucking(true);
#endif
}

std::shared_ptr<std::thread> crash_handler_responce_thread;
bool crash_handler_timeout_activated = false;
bool crash_handler_exit = false;

#ifdef WIN32
typedef struct
{
	OVERLAPPED        oOverlap;
	HANDLE            hPipeInst;
	std::vector<char> chRequest;
	DWORD             cbRead;
	TCHAR             chReply[BUFFSIZE];
	DWORD             cbToWrite;
	DWORD             dwState;
	BOOL              fPendingIO;
} PIPEINST, *LPPIPEINST;

PIPEINST Pipe = {0};
HANDLE   hEvents = {0};

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;
	fConnected = ConnectNamedPipe(hPipe, lpo);

	if (fConnected) {
		return 0;
	}

	switch (GetLastError()) {
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;
	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;
	default: {
		return 0;
	}
	}

	return fPendingIO;
}

VOID DisconnectAndReconnect(void)
{
	if (!DisconnectNamedPipe(Pipe.hPipeInst)) {
		return;
	}

	Pipe.fPendingIO = ConnectToNewClient(Pipe.hPipeInst, &(Pipe.oOverlap));

	Pipe.dwState = Pipe.fPendingIO ? CONNECTING_STATE : READING_STATE;
}

bool prepareTerminationPipe()
{
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\exit-slobs-crash-handler");

	hEvents = CreateEvent(NULL, TRUE, TRUE, NULL);

	if (hEvents == NULL) {
		return false;
	}

	Pipe.oOverlap.hEvent = hEvents;

	Pipe.hPipeInst = CreateNamedPipe(
	    lpszPipename,
	    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
	    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
	    1,
	    BUFFSIZE * sizeof(TCHAR),
	    BUFFSIZE * sizeof(TCHAR),
	    5000,
	    NULL);

	if (Pipe.hPipeInst == INVALID_HANDLE_VALUE) {
		CloseHandle(hEvents);
		hEvents = NULL;
		Pipe.oOverlap.hEvent = NULL;
		return false;
	}

	Pipe.fPendingIO = ConnectToNewClient(Pipe.hPipeInst, &(Pipe.oOverlap));

	Pipe.dwState = Pipe.fPendingIO ? CONNECTING_STATE : READING_STATE;
	
	return true;
}

void acknowledgeTerminate()
{
	BOOL   fSuccess;
	DWORD i, dwWait, cbRet, dwErr;

	while (!crash_handler_exit) {
		if (crash_handler_timeout_activated) {
			auto tp    = std::chrono::high_resolution_clock::now();
			auto delta = tp - start_wait_acknowledge;

			if (std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() > 5000) {
				// We timeout, crash handler failed to send the shutdown acknowledge,
				// we move forward with the shutdown procedure
				crash_handler_exit = true;
				CloseHandle(Pipe.hPipeInst);
				break;
			}
		}
		dwWait = WaitForSingleObject(hEvents, 500);

		if (dwWait == WAIT_OBJECT_0) {
			if (Pipe.fPendingIO) {
				fSuccess = GetOverlappedResult(Pipe.hPipeInst, &(Pipe.oOverlap), &cbRet, FALSE);

				switch (Pipe.dwState) {
				case CONNECTING_STATE: {
					if (!fSuccess) {
						break;
					}
					Pipe.dwState = READING_STATE;
					break;
				}
				case READING_STATE: {
					if (!fSuccess || cbRet == 0) {
						crash_handler_exit = true;
						DisconnectAndReconnect();
						break;
					}
					Pipe.cbRead  = cbRet;
					Pipe.dwState = READING_STATE;
					break;
				}
				default: {
					break;
				}
				}
			}

			switch (Pipe.dwState) {
			case READING_STATE: {
				Pipe.chRequest.resize(BUFFSIZE);
				fSuccess = ReadFile(
				    Pipe.hPipeInst,
				    Pipe.chRequest.data(),
				    BUFFSIZE * sizeof(TCHAR),
				    &Pipe.cbRead,
				    &Pipe.oOverlap);

				GetOverlappedResult(Pipe.hPipeInst, &Pipe.oOverlap, &Pipe.cbRead, false);

				// The read operation completed successfully.
				if (Pipe.cbRead > 0) {
					Pipe.fPendingIO = FALSE;
				}
				dwErr = GetLastError();
				if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
					Pipe.fPendingIO = TRUE;
					break;
				}
				crash_handler_exit = true;
				DisconnectAndReconnect();
				break;
			}
			}
		}
	}
}
#else __APPLE__
void acknowledgeTerminate (void) {
	std::vector<char> buffer;
	buffer.resize(64);
	int file_descriptor = open("exit-slobs-crash-handler", O_RDONLY);
	if (file_descriptor < 0) {
		blog(LOG_ERROR, "Could not open crash-handler exit fifo");
        return;
    }

	if (::read(file_descriptor, buffer.data(), buffer.size()) <= 0)
		blog(LOG_ERROR, "Error while reading crash-handler exit message");
}

bool prepareTerminationPipe() {
	// TODO
	return false;
}
#endif

void OBS_API::CreateCrashHandlerExitPipe() 
{
	if (prepareTerminationPipe()) {
		crash_handler_responce_thread = std::make_shared<std::thread>( acknowledgeTerminate );
	} else {
#ifdef WIN32
		blog(LOG_ERROR, "Failed to create pipe for crash-handler exit message");
#endif
	}
}

void OBS_API::WaitCrashHandlerClose(bool waitBeforeClosing)
{
	if (crash_handler_responce_thread) {
		if (!waitBeforeClosing)
			crash_handler_exit = true;
			
		if (crash_handler_responce_thread->joinable())
			crash_handler_responce_thread->join();
	}
}

void OBS_API::InformCrashHandler(const int crash_id)
{
	writeCrashHandler(crashedProcess(crash_id));
}

void OBS_API::destroyOBS_API(void)
{
	blog(LOG_DEBUG, "OBS_API::destroyOBS_API started, objects allocated %d", bnum_allocs());
	blog(LOG_INFO, "destroyOBS_API - 0");
	os_cpu_usage_info_destroy(cpuUsageInfo);

#ifdef _WIN32
	config_t* basicConfig         = ConfigManager::getInstance().getBasic();
	if (basicConfig) {
		bool disableAudioDucking = config_get_bool(basicConfig, "Audio", "DisableAudioDucking");
		if (disableAudioDucking)
			DisableAudioDucking(false);
	}
#endif
	OBS_content::OBS_content_shutdownDisplays();

	blog(LOG_INFO, "destroyOBS_API - 1");
	obs::autoConfig::WaitPendingTests();
	OBS_service::stopAllOutputs();
	OBS_service::setStreamingEncoder(nullptr);
	OBS_service::setRecordingEncoder(nullptr);
	OBS_service::setAudioSimpleStreamingEncoder(nullptr);
	OBS_service::setAudioSimpleRecordingEncoder(nullptr);
	OBS_service::setArchiveEncoder(nullptr);
	OBS_service::setStreamingOutput(nullptr);
	OBS_service::setRecordingOutput(nullptr);
	OBS_service::setReplayBufferOutput(nullptr);

	blog(LOG_INFO, "destroyOBS_API - 2");
	obs_output* virtualWebcamOutput = OBS_service::getVirtualWebcamOutput();
	if (virtualWebcamOutput != NULL) {
		if (obs_output_active(virtualWebcamOutput))
			obs_output_stop(virtualWebcamOutput);

		obs_output_release(virtualWebcamOutput);
	}

	blog(LOG_INFO, "destroyOBS_API - 3");
	OBS_service::setService(nullptr);
	blog(LOG_INFO, "destroyOBS_API - 3.1");
    OBS_service::waitReleaseWorker();
	blog(LOG_INFO, "destroyOBS_API - 3.2");
    OBS_service::clearAudioEncoder();
	blog(LOG_INFO, "destroyOBS_API - 3.3");
    obs::Fader::ClearFaders();
	blog(LOG_INFO, "destroyOBS_API - 3");

	blog(LOG_INFO, "destroyOBS_API - 4");
	// Check if the frontend was able to shutdown correctly:
	// If there are some sources here it's because it ended unexpectedly, this represents a 
	// problem since obs doesn't handle releasing leaked sources very well. The best we can
	// do is to insert a try-catch block and disable the crash handler to avoid false positives
	if (obs::Source::Manager::GetInstance().size() > 0		||
		obs::Scene::Manager::GetInstance().size() > 0		||
		obs::SceneItem::Manager::GetInstance().size() > 0	||
		obs::Transition::Manager::GetInstance().size() > 0	||
		obs::Filter::Manager::GetInstance().size() > 0		||
		obs::Input::Manager::GetInstance().size() > 0) {

		for (int i = 0; i < MAX_CHANNELS; i++)
			obs_set_output_source(i, nullptr);

		std::vector<obs_source_t*> sources;
		obs::Source::Manager::GetInstance().for_each([&sources](obs_source_t* source)
		{
			if (source)
				sources.push_back(source);
		});

		for (const auto &source: sources) {
			if (!source)
				continue;

			const char* source_id = obs_source_get_id(source);
			if (!source_id)
				continue;

			if (!strcmp(source_id, "scene")) {
				std::list<obs_sceneitem_t*> items;
				auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
					if (item) {
						obs_sceneitem_release(item);
						obs_sceneitem_remove(item);
					}
					return true;
				};
				obs_scene_t* scene = obs_scene_from_source(source);
				if (scene)
					obs_scene_enum_items(scene, cb, nullptr);
			}
		}

		// Release filters only
		for (int i = 0; i < sources.size(); i++) {
			if (sources[i] && obs_source_get_type(sources[i]) == OBS_SOURCE_TYPE_FILTER) {
				obs_source_release(sources[i]);
				sources[i] = nullptr;
			}
		}

		// Release all remaining sources that are not transitions
		for (int i = 0; i < sources.size(); i++) {
			if (sources[i] && obs_source_get_type(sources[i]) != OBS_SOURCE_TYPE_TRANSITION) {
				obs_source_release(sources[i]);
				sources[i] = nullptr;
			}
		}

		// Release all remaning transitions
		for (auto source: sources) {
			if (source)
				obs_source_release(source);
		}

#ifdef WIN32
		// Directly blame the frontend since it didn't release all objects and that could cause 
		// a crash on the backend
		// This is necessary since the frontend could still finish after the backend, causing the
		// crash manager to think the backend crashed first while the real culprit is the frontend
		util::CrashManager::GetMetricsProvider()->BlameFrontend();

		util::CrashManager::DisableReports();
#endif
		blog(LOG_DEBUG, "OBS_API::destroyOBS_API unreleased objects detected before obs_shutdown, objects allocated %d", bnum_allocs());
		// Try-catch should suppress any error message that could be thrown to the user
		try {
			obs_shutdown();
		} catch (...) {}

	} else {
		blog(LOG_DEBUG, "OBS_API::destroyOBS_API calling obs_shutdown, objects allocated %d", bnum_allocs());
		obs_shutdown();
	}

	blog(LOG_INFO, "destroyOBS_API - 5");
	// Release each obs module (dlls for windows)
	// TODO: We should release these modules (dlls) manually and not let the garbage
	// collector do this for us on shutdown
	for (auto& moduleInfo : obsModules) {
	}

	// The goal is to reduce this number to zero and add a throw here, so if in the future
	// a leak is detected, any developer will know for sure what is causing it
	int totalLeaks = bnum_allocs();
	std::cout << "Total leaks: " << totalLeaks << std::endl;
	if (totalLeaks) {
		// throw "OBS has memory leaks";
	}
	blog(LOG_DEBUG, "OBS_API::destroyOBS_API after obs_shutdown, objects allocated %d", bnum_allocs());
}

struct ci_char_traits : public std::char_traits<char>
{
	static bool eq(char c1, char c2)
	{
		return toupper(c1) == toupper(c2);
	}
	static bool ne(char c1, char c2)
	{
		return toupper(c1) != toupper(c2);
	}
	static bool lt(char c1, char c2)
	{
		return toupper(c1) < toupper(c2);
	}
	static int compare(const char* s1, const char* s2, size_t n)
	{
		while (n-- != 0) {
			if (toupper(*s1) < toupper(*s2))
				return -1;
			if (toupper(*s1) > toupper(*s2))
				return 1;
			++s1;
			++s2;
		}
		return 0;
	}
	static const char* find(const char* s, int n, char a)
	{
		while (n-- > 0 && toupper(*s) != toupper(a)) {
			++s;
		}
		return s;
	}
};

typedef std::basic_string<char, ci_char_traits> istring;

/* This should be reusable outside of node-obs, especially
* if we go a server/client route. */
bool OBS_API::openAllModules(int& video_err)
{
	video_err = OBS_service::resetVideoContext();
	if (video_err != OBS_VIDEO_SUCCESS) {
		blog(LOG_INFO, "Reset video failed with error: %d", video_err);
		return false;
	}
	std::string plugins_paths[] = {g_moduleDirectory + "/obs-plugins/64bit",
	                               g_moduleDirectory + "/obs-plugins",
	                               slobs_plugin + "/obs-plugins/64bit"};

	std::string plugins_data_paths[] = {
	    g_moduleDirectory + "/data/obs-plugins", plugins_data_paths[0], slobs_plugin + "/data/obs-plugins"};

	size_t num_paths = sizeof(plugins_paths) / sizeof(plugins_paths[0]);

	for (int i = 0; i < num_paths; ++i) {
		std::string& plugins_path      = plugins_paths[i];
		std::string& plugins_data_path = plugins_data_paths[i];

		/* FIXME Plugins could be in individual folders, maybe
		* with some metainfo so we don't attempt just any
		* shared library. */
		if (!os_file_exists(plugins_path.c_str())) {
			blog(LOG_ERROR, "Plugin Path provided is invalid: %s", plugins_path.c_str());
			std::cerr << "Plugin Path provided is invalid: " << plugins_path << std::endl;
			continue;
		}

		os_dir_t* plugin_dir = os_opendir(plugins_path.c_str());
		if (!plugin_dir) {
			blog(LOG_ERROR, "Failed to open plugin diretory: %s", plugins_path.c_str());
			std::cerr << "Failed to open plugin diretory: " << plugins_path << std::endl;
			continue;
		}

#ifdef WIN32
		SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS);
		AddDllDirectory(make_wide_string(g_moduleDirectory).c_str());
		WCHAR system[MAX_PATH];
		GetSystemDirectory(system, sizeof(system));
		AddDllDirectory(system);
#endif
		for (os_dirent* ent = os_readdir(plugin_dir); ent != nullptr; ent = os_readdir(plugin_dir)) {
			std::string fullname = ent->d_name;
			std::string basename = fullname.substr(0, fullname.find_last_of('.'));

			std::string plugin_path      = plugins_path + "/" + fullname;
			std::string plugin_data_path = plugins_data_path + "/" + basename;
			if (ent->directory) {
				continue;
			}

#ifdef _WIN32
			if (fullname.substr(fullname.find_last_of(".") + 1) != "dll") {
				continue;
			}
#endif

			obs_module_t* module = nullptr;
			int           result = MODULE_ERROR;

			try {
				result = obs_open_module(&module, plugin_path.c_str(), plugin_data_path.c_str());
			} catch (std::string errorMsg) {
				blog(LOG_ERROR, "Failed to load module: %s - %s", basename.c_str(), errorMsg.c_str());
				continue;
			} catch (...) {
				blog(LOG_ERROR, "Failed to load module: %s", basename.c_str());
				continue;
			}

			switch (result) {
			case MODULE_SUCCESS:
				obsModules.push_back(std::make_pair(fullname, module));
				break;
			case MODULE_FILE_NOT_FOUND:
				std::cerr << "Unable to load '" << plugin_path << "', could not find file." << std::endl;
				continue;
			case MODULE_MISSING_EXPORTS:
				std::cerr << "Unable to load '" << plugin_path << "', missing exports." << std::endl;
				continue;
			case MODULE_INCOMPATIBLE_VER:
				std::cerr << "Unable to load '" << plugin_path << "', incompatible version." << std::endl;
				continue;
			case MODULE_ERROR:
				std::cerr << "Unable to load '" << plugin_path << "', generic error." << std::endl;
				continue;
			default:
				continue;
			}

			try {
				bool success = obs_init_module(module);
				if (!success) {
					std::cerr << "Failed to initialize module " << plugin_path << std::endl;
					/* Just continue to next one */
				}
			} catch (std::string errorMsg) {
				blog(LOG_ERROR, "Failed to initialize module: %s - %s", basename.c_str(), errorMsg.c_str());
				continue;
			} catch (...) {
				blog(LOG_ERROR, "Failed to initialize module: %s", basename.c_str());
				continue;
			}
		}

		os_closedir(plugin_dir);
	}

	return true;
}

double OBS_API::getCPU_Percentage(void)
{
	double cpuPercentage = os_cpu_usage_info_query(cpuUsageInfo);

	cpuPercentage *= 10;
	cpuPercentage = trunc(cpuPercentage);
	cpuPercentage /= 10;

	return 0;
}

int OBS_API::getNumberOfDroppedFrames(void)
{
	obs_output_t* streamOutput = OBS_service::getStreamingOutput();

	int totalDropped = 0;

	if (streamOutput && obs_output_active(streamOutput)) {
		totalDropped = obs_output_get_frames_dropped(streamOutput);
	}

	return totalDropped;
}

double OBS_API::getDroppedFramesPercentage(void)
{
	obs_output_t* streamOutput = OBS_service::getStreamingOutput();

	double percent = 0;

	if (streamOutput && obs_output_active(streamOutput)) {
		int totalDropped = obs_output_get_frames_dropped(streamOutput);
		int totalFrames  = obs_output_get_total_frames(streamOutput);
		if (totalFrames == 0) {
			percent = 0.0;
		} else {
			percent = (double)totalDropped / (double)totalFrames * 100.0;
		}
	}

	return percent;
}

OBS_API::OutputStats OBS_API::getCurrentOutputStats(std::string type) // obs_output_t* output, OBS_API::OutputStats &outputStats)
{
	OBS_API::OutputStats outputStats = {0};
	outputStats.kbitsPerSec = 0.0;
	outputStats.dataOutput  = 0.0;
	obs_output_t* output = nullptr;

	if (type.compare("stream") == 0) {
		output = OBS_service::getStreamingOutput();
	} else if (type.compare("recording")) {
		output = OBS_service::getRecordingOutput();
	} else {
		return outputStats;
	}

	if (!output)
		return outputStats;

	if (obs_output_active(output)) {
		uint64_t bytesSent = obs_output_get_total_bytes(output);
		uint64_t bytesSentTime = os_gettime_ns();

		if (bytesSent < outputStats.lastBytesSent)
			bytesSent = 0;
		if (bytesSent == 0)
			outputStats.lastBytesSent = 0;

		uint64_t bitsBetween = (bytesSent - outputStats.lastBytesSent) * 8;

		double timePassed = double(bytesSentTime - outputStats.lastBytesSentTime) / 1000000000.0;
		if (timePassed < std::numeric_limits<double>::epsilon()
		    && timePassed > -std::numeric_limits<double>::epsilon()) {
			outputStats.kbitsPerSec = 0.0;
		} else {
			outputStats.kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;
		}

		outputStats.lastBytesSent = bytesSent;
		outputStats.lastBytesSentTime = bytesSentTime;
		outputStats.dataOutput = bytesSent / (1024.0 * 1024.0);
	}

	return outputStats;
}

double OBS_API::getCurrentFrameRate(void)
{
	return obs_get_active_fps();
}

double OBS_API::getAverageTimeToRenderFrame()
{
	return (double)obs_get_average_frame_time_ns() / 1000000.0;
}

std::string OBS_API::getDiskSpaceAvailable()
{
	const char* path = nullptr;
	const char* mode = config_get_string(ConfigManager::getInstance().getBasic(), "Output", "Mode");

	if (strcmp(mode, "Advanced") == 0) {
		const char* advanced_mode = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecType");

		if (strcmp(advanced_mode, "FFmpeg") == 0) {
			path = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "FFFilePath");
		} else {
			path = config_get_string(ConfigManager::getInstance().getBasic(), "AdvOut", "RecFilePath");
		}
	} else {
		path = config_get_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "FilePath");
	}

	uint64_t bytes = os_get_free_disk_space(path);

	double free_bytes = 0;
	std::string type;

	if (bytes > TBYTE) {
		free_bytes = (double)bytes / TBYTE;
		type = " TB";
	} else if (bytes > GBYTE) {
		free_bytes = (double)bytes / GBYTE;
		type = " GB";
	} else {
		free_bytes = (double)bytes / MBYTE;
		type = " MB";
	}

	std::stringstream remainingHDSpace;
	remainingHDSpace << free_bytes << type;
	return remainingHDSpace.str();
}

double OBS_API::getMemoryUsage()
{
	return (double)os_get_proc_resident_size() / (1024.0 * 1024.0);
}

const std::vector<std::string>& OBS_API::getOBSLogErrors()
{
	return logReport.errors;
}

const std::vector<std::string>& OBS_API::getOBSLogWarnings()
{
	return logReport.warnings;
}

std::queue<std::string>& OBS_API::getOBSLogGeneral()
{
	return logReport.general;
}

std::string OBS_API::getCurrentVersion()
{
	return currentVersion;
}

std::string OBS_API::getUsername()
{
	return username;
}
#ifdef WIN32
static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO info;
	info.cbSize = sizeof(info);
	if (GetMonitorInfo(hMonitor, &info)) {
		std::vector<std::pair<uint32_t, uint32_t>>* resolutions =
			reinterpret_cast<std::vector<std::pair<uint32_t, uint32_t>>*>(dwData);

		resolutions->push_back(std::make_pair(
			std::abs(info.rcMonitor.left - info.rcMonitor.right),
			std::abs(info.rcMonitor.top - info.rcMonitor.bottom)
		));
	}
	return true;
}
#endif
std::vector<std::pair<uint32_t, uint32_t>> OBS_API::availableResolutions(void)
{
	std::vector<std::pair<uint32_t, uint32_t>> resolutions;
#ifdef WIN32
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&resolutions));
#else
	resolutions = g_util_osx_server->getAvailableScreenResolutions();
#endif
	return resolutions;
}
