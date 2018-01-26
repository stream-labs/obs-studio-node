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

#include "osn-NodeOBS.hpp"
#include "utility.hpp"
#include "shared.hpp"
#include <iostream>
#include <fstream>

/// OBS
#include <obs.h>
#include <util/platform.h>
#include <util/lexer.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Dwmapi.h>
#include <psapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <util/windows/WinHandle.hpp>
#include <util/windows/HRError.hpp>
#include <util/windows/ComPtr.hpp>
#include <util/windows/win-version.h>

// These collide with functions.
#undef RegisterClass 
#endif

#ifdef _MSC_VER

#endif


// Global Storage (Should be changed in the future)
static std::string g_moduleDirectory = ""; // Module Directory
static std::string appdata_path = "";
static std::vector<std::pair<obs_module_t *, int>> listModules;
static os_cpu_usage_info_t *cpuUsageInfo = nullptr;
static uint64_t lastBytesSent = 0;
static uint64_t lastBytesSentTime = 0;
static std::string pathConfigDirectory;
static std::string OBS_pathConfigDirectory;
static std::string OBS_currentProfile;
static std::string OBS_currentSceneCollection;
static bool useOBS_configFiles = false;
static bool isOBS_installedValue;

// Functions
static bool get_token(lexer *lex, std::string &str, base_token_type type) {
	base_token token;
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != type)
		return false;

	str.assign(token.text.array, token.text.len);
	return true;
}

static bool expect_token(lexer *lex, const char *str, base_token_type type) {
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
static uint64_t convert_log_name(const char *name) {
	lexer  lex;
	std::string     year, month, day, hour, minute, second;

	lexer_init(&lex);
	lexer_start(&lex, name);

	if (!get_token(&lex, year, BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, month, BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, day, BASETOKEN_DIGIT)) return 0;
	if (!get_token(&lex, hour, BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, minute, BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, second, BASETOKEN_DIGIT)) return 0;

	std::string timestring(year);
	timestring += month + day + hour + minute + second;
	lexer_free(&lex);
	return std::stoull(timestring);
}

static void delete_oldest_file(const char *location, unsigned maxLogs) {
	std::string           oldestLog;
	uint64_t         oldest_ts = (uint64_t)-1;
	struct os_dirent *entry;

	os_dir_t *dir = os_opendir(location);

	if (!dir) {
		std::cout << "Failed to open log directory." << std::endl;
	}

	unsigned count = 0;

	while ((entry = os_readdir(dir)) != NULL) {
		if (entry->directory || *entry->d_name == '.')
			continue;

		uint64_t ts = convert_log_name(entry->d_name);

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

// Actual Implementation
void NodeOBS::Module::Register(IPC::Server& srv) {
	std::shared_ptr<IPC::Class> cls = std::make_shared<IPC::Class>("NodeOBSModule");
	cls->RegisterFunction(std::make_shared<IPC::Function>("SetWorkingDirectory", std::vector<IPC::Type>{IPC::Type::String}, SetWorkingDirectory));
	srv.RegisterClass(cls);
}

void NodeOBS::Module::SetWorkingDirectory(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	g_moduleDirectory = args[0].value_str;
}

void NodeOBS::API::Register(IPC::Server& srv) {
	std::shared_ptr<IPC::Class> cls = std::make_shared<IPC::Class>("NodeOBSAPI");

	// InitAPI has an optional String parameter.
	cls->RegisterFunction(std::make_shared<IPC::Function>("InitAPI", std::vector<IPC::Type>{IPC::Type::String}, InitAPI));
	cls->RegisterFunction(std::make_shared<IPC::Function>("InitAPI", std::vector<IPC::Type>{}, InitAPI));

	//cls->RegisterFunction(std::make_shared<IPC::Function>("SetWorkingDirectory", std::vector<IPC::Type>{IPC::Type::String}, SetWorkingDirectory));

	srv.RegisterClass(cls);
}

void NodeOBS::API::InitAPI(void* data, const int64_t id, const std::vector<IPC::Value>& args, std::vector<IPC::Value>& rval) {
	if ((args.size() > 0) && (args[0].type == IPC::Type::String)) {
		appdata_path = args[0].value_str;
		appdata_path += "/node-obs/";
		pathConfigDirectory = args[0].value_str;
	} else {
		char *tmp;
		appdata_path = tmp = os_get_config_path_ptr("node-obs/");
		bfree(tmp);
	}

	initAPI();
}

void NodeOBS::API::initAPI() {
	initOBS_API();
	openAllModules();
	initAllModules();
	OBS_service::createStreamingOutput();
	OBS_service::createRecordingOutput();

	OBS_service::createVideoStreamingEncoder();
	OBS_service::createVideoRecordingEncoder();

	OBS_service::createAudioEncoder();

	OBS_service::resetAudioContext();
	OBS_service::resetVideoContext(NULL);

	OBS_service::associateAudioAndVideoToTheCurrentStreamingContext();
	OBS_service::associateAudioAndVideoToTheCurrentRecordingContext();

	OBS_service::createService();

	OBS_service::associateAudioAndVideoEncodersToTheCurrentStreamingOutput();
	OBS_service::associateAudioAndVideoEncodersToTheCurrentRecordingOutput();

	OBS_service::setServiceToTheStreamingOutput();

	setAudioDeviceMonitoring();
}

bool NodeOBS::API::initOBS_API() {
	std::vector<char> userData = std::vector<char>(1024);
	os_get_config_path(userData.data(), userData.capacity() - 1, "slobs-client/plugin_config");
	obs_startup("en-US", userData.data(), NULL);
	cpuUsageInfo = os_cpu_usage_info_start();

	//Setting obs-studio config directory
	char path[512];
	int ret = GetConfigPath(path, 512, "obs-studio");

	if (ret > 0) {
		OBS_pathConfigDirectory = path;
	}

	std::string profiles = OBS_pathConfigDirectory + "\\basic\\profiles";
	std::string scenes = OBS_pathConfigDirectory + "\\basic\\scenes";

	isOBS_installedValue = dirExists(OBS_pathConfigDirectory) &&
		containsDirectory(profiles) &&
		os_file_exists(scenes.c_str());

	if (isOBS_installedValue) {
		v8::String::Utf8Value firstProfile(getOBS_existingProfiles()->Get(0)->ToString());
		OBS_currentProfile = std::string(*firstProfile);

		v8::String::Utf8Value firstSceneCollection(getOBS_existingSceneCollections()->Get(0)->ToString());
		OBS_currentSceneCollection = std::string(*firstSceneCollection);
	}

	/* Logging */
	std::string filename = GenerateTimeDateFilename("txt");
	std::string log_path = appdata_path;
	log_path.append("/logs/");

	/* Make sure the path is created
	before attempting to make a file there. */
	if (os_mkdirs(log_path.c_str()) == MKDIR_ERROR) {
		std::cerr << "Failed to open log file" << std::endl;
	}

	delete_oldest_file(log_path.c_str(), 3);
	log_path.append(filename);

	/* Leak although not that big of a deal since it should always be open. */
	std::fstream *logfile = new std::fstream(log_path, std::ios_base::out | std::ios_base::trunc);

	if (!logfile) {
		std::cerr << "Failed to open log file" << std::endl;
	}

	/* Delete oldest file in the folder to imitate rotating */
	base_set_log_handler(node_obs_log, logfile);

	/* Profiling */
	//profiler_start();



	return obs_initialized();
}

std::string NodeOBS::API::GenerateTimeDateFilename(const char* extension) {
	time_t    now = time(0);
	char      file[256] = {};
	struct tm *cur_time;

	cur_time = localtime(&now);
	snprintf(file, sizeof(file), "%d-%02d-%02d %02d-%02d-%02d.%s",
		cur_time->tm_year + 1900,
		cur_time->tm_mon + 1,
		cur_time->tm_mday,
		cur_time->tm_hour,
		cur_time->tm_min,
		cur_time->tm_sec,
		extension);

	return std::string(file);
}

void NodeOBS::API::stdout_log_handler(int log_level, const char *format, va_list args, void *param) {
	char out[4096];
	vsnprintf(out, sizeof(out), format, args);

	switch (log_level) {
		case LOG_DEBUG:
			fprintf(stdout, "debug: %s\n", out);
			fflush(stdout);
			break;

		case LOG_INFO:
			fprintf(stdout, "info: %s\n", out);
			fflush(stdout);
			break;

		case LOG_WARNING:
			fprintf(stdout, "warning: %s\n", out);
			fflush(stdout);
			break;

		case LOG_ERROR:
			fprintf(stderr, "error: %s\n", out);
			fflush(stderr);
	}

	UNUSED_PARAMETER(param);
}

void NodeOBS::API::node_obs_log(int log_level, const char *msg, va_list args, void *param) {
	std::fstream &logFile = *static_cast<std::fstream*>(param);
	char str[4096];

	va_list args2;
	va_copy(args2, args);

	stdout_log_handler(log_level, msg, args, nullptr);
	vsnprintf(str, 4095, msg, args2);

#ifdef _WIN32
	if (IsDebuggerPresent()) {
		int wNum = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
		if (wNum > 1) {
			static std::wstring wide_buf;
			static std::mutex wide_mutex;

			std::lock_guard<std::mutex> lock(wide_mutex);
			wide_buf.reserve(wNum + 1);
			wide_buf.resize(wNum - 1);
			MultiByteToWideChar(CP_UTF8, 0, str, -1, &wide_buf[0],
				wNum);
			wide_buf.push_back('\n');

			OutputDebugStringW(wide_buf.c_str());
		}
	}
#endif

	char *tmp_str = str;
	char *nextLine = tmp_str;

	while (*nextLine) {
		char *nextLine = strchr(tmp_str, '\n');
		if (!nextLine)
			break;

		if (nextLine != tmp_str && nextLine[-1] == '\r') {
			nextLine[-1] = 0;
		} else {
			nextLine[0] = 0;
		}

		logFile << tmp_str << std::endl;
		nextLine++;
		tmp_str = nextLine;
	}

	logFile << tmp_str << std::endl;

#if defined(_WIN32) && defined(OBS_DEBUGBREAK_ON_ERROR)
	if (log_level <= LOG_ERROR && IsDebuggerPresent())
		__debugbreak();
#endif
}

int NodeOBS::API::GetConfigPath(char *path, size_t size, const char *name) {
	return os_get_config_path(path, size, name);
}
