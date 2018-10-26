#include "nodeobs_api.h"
#include "osn-source.hpp"
#include "util/lexer.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0502

#include <ShlObj.h>
#include <codecvt>
#include <locale>
#include <mutex>
#include <string>
#include "nodeobs_content.h"
#endif

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#endif

#define WIN32_LEAN_AND_MEAN

#include <audiopolicy.h>
#include <mmdeviceapi.h>

#include <util/windows/ComPtr.hpp>
#include <util/windows/HRError.hpp>
#include <util/windows/WinHandle.hpp>

#include "error.hpp"
#include "shared.hpp"

namespace std
{
	template<>
	struct default_delete<obs_encoder_t> {
		void operator()(obs_encoder_t* ptr) {
			if (!obs_initialized())
				throw "Trying to delete an obs object but the service isn't active";
			obs_encoder_release(ptr);
		}
	};
} // namespace std

std::string                                            appdata_path;
vector<pair<obs_module_t*, int>>                       listModules;
os_cpu_usage_info_t*                                   cpuUsageInfo      = nullptr;
uint64_t                                               lastBytesSent     = 0;
uint64_t                                               lastBytesSentTime = 0;
std::string                                            pathConfigDirectory;
std::string                                            OBS_pathConfigDirectory;
std::string                                            OBS_currentProfile;
std::string                                            OBS_currentSceneCollection;
bool                                                   useOBS_configFiles = false;
bool                                                   isOBS_installedValue;
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

std::string g_moduleDirectory = "";

void OBS_API::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("API");

	cls->register_function(std::make_shared<ipc::function>(
	    "OBS_API_initAPI", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, OBS_API_initAPI));
	cls->register_function(
	    std::make_shared<ipc::function>("OBS_API_destroyOBS_API", std::vector<ipc::type>{}, OBS_API_destroyOBS_API));
	cls->register_function(std::make_shared<ipc::function>(
	    "OBS_API_getPerformanceStatistics", std::vector<ipc::type>{}, OBS_API_getPerformanceStatistics));
	cls->register_function(std::make_shared<ipc::function>(
	    "OBS_API_getOBS_existingProfiles", std::vector<ipc::type>{}, OBS_API_getOBS_existingProfiles));
	cls->register_function(std::make_shared<ipc::function>(
	    "OBS_API_getOBS_existingSceneCollections", std::vector<ipc::type>{}, OBS_API_getOBS_existingSceneCollections));
	cls->register_function(
	    std::make_shared<ipc::function>("OBS_API_isOBS_installed", std::vector<ipc::type>{}, OBS_API_isOBS_installed));
	cls->register_function(std::make_shared<ipc::function>(
	    "SetWorkingDirectory", std::vector<ipc::type>{ipc::type::String}, SetWorkingDirectory));
	cls->register_function(std::make_shared<ipc::function>(
	    "StopCrashHandler", std::vector<ipc::type>{}, StopCrashHandler));

	srv.register_collection(cls);
}

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
};

void OBS_API::SetWorkingDirectory(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	g_moduleDirectory = args[0].value_str;
	replaceAll(g_moduleDirectory, "\\", "/");
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(g_moduleDirectory));
	AUTO_DEBUG;
}

std::string OBS_API::getModuleDirectory(void)
{
	return g_moduleDirectory;
}

/* FIXME Platform specific and uses ASCII functions */
static bool dirExists(const std::string& path)
{
	DWORD ftyp = GetFileAttributesA(path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	return false;
}

static string GenerateTimeDateFilename(const char* extension)
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

	return string(file);
}

static bool GetToken(lexer* lex, string& str, base_token_type type)
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
	string year, month, day, hour, minute, second;

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
	string            oldestLog;
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
		string delPath;

		delPath = delPath + location + "/" + oldestLog;
		os_unlink(delPath.c_str());
	}
}

#include <chrono>
#include <cstdarg>
#include <varargs.h>

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#else
#include <unistd.h>
#endif

inline std::string nodeobs_log_formatted_message(const char* format, va_list& args)
{
	size_t            length  = _vscprintf(format, args);
	std::vector<char> buf     = std::vector<char>(length + 1, '\0');
	size_t            written = vsprintf_s(buf.data(), buf.size(), format, args);
	return std::string(buf.begin(), buf.begin() + length);
}

std::chrono::high_resolution_clock             hrc;
std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
static void                                    node_obs_log(int log_level, const char* msg, va_list args, void* param)
{
	// Calculate log time.
	auto timeSinceStart = (std::chrono::high_resolution_clock::now() - tp);
	auto days           = std::chrono::duration_cast<std::chrono::duration<int, ratio<86400>>>(timeSinceStart);
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

	std::vector<char> timebuf(65535, '\0');
	std::string       timeformat = "[%.3d:%.2d:%.2d:%.2d.%.3d.%.3d.%.3d][%*s]"; // "%*s";
	int               length     = sprintf_s(
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
	std::string time_and_level = std::string(timebuf.data(), length);

	// Format incoming text
	std::string text = nodeobs_log_formatted_message(msg, args);

	std::fstream* logStream = reinterpret_cast<std::fstream*>(param);

	// Split by \n (new-line)
	size_t last_valid_idx = 0;
	for (size_t idx = 0; idx <= text.length(); idx++) {
		char& ch = text[idx];
		if ((ch == '\n') || (idx == text.length())) {
			std::string newmsg = time_and_level + " " + std::string(&text[last_valid_idx], idx - last_valid_idx) + '\n';
			last_valid_idx     = idx + 1;

			// File Log
			*logStream << newmsg << std::flush;

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

					lock_guard<mutex> lock(wide_mutex);
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

uint32_t pid = GetCurrentProcessId();

std::vector<char> registerProcess(void) {
	std::vector<char> buffer;
	buffer.resize(sizeof(uint8_t) + sizeof(bool) + sizeof(uint32_t));
	uint8_t action = 0;
	bool    isCritical = true;

	uint32_t offset = 0;

	memcpy(buffer.data(), &action, sizeof(action));
	offset++;
	memcpy(buffer.data() + offset, &isCritical, sizeof(isCritical));
	offset++;
	memcpy(buffer.data() + offset, &pid, sizeof(pid));

	return buffer;
}

std::vector<char> unregisterProcess(void)
{
	std::vector<char> buffer;
	buffer.resize(sizeof(uint8_t) + sizeof(uint32_t));
	uint8_t action = 1;

	uint32_t offset = 0;

	memcpy(buffer.data(), &action, sizeof(action));
	offset++;
	memcpy(buffer.data() + offset, &pid, sizeof(pid));

	return buffer;
}

std::vector<char> terminateCrashHandler(void)
{
	std::vector<char> buffer;
	buffer.resize(sizeof(uint8_t) + sizeof(uint32_t));
	uint8_t action = 2;

	uint32_t offset = 0;

	memcpy(buffer.data(), &action, sizeof(action));
	offset++;
	memcpy(buffer.data() + offset, &pid, sizeof(pid));

	return buffer;
}

void writeCrashHandler(std::vector<char> buffer) {
	HANDLE hPipe = CreateFile(
	    TEXT("\\\\.\\pipe\\slobs-crash-handler"),
	    GENERIC_READ |
	    GENERIC_WRITE,
	    0,
	    NULL,
	    OPEN_EXISTING,
	    0,
	    NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
		return;

	if (GetLastError() == ERROR_PIPE_BUSY)
		return;

	WriteFile(
	    hPipe,
	    buffer.data(),
	    buffer.size(),
	    NULL,
	    NULL);

	CloseHandle(hPipe);
}

void OBS_API::OBS_API_initAPI(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	writeCrashHandler(registerProcess());
	/* Map base DLLs as soon as possible into the current process space.
	* In particular, we need to load obs.dll into memory before we call
	* any functions from obs else if we delay-loaded the dll, it will
	* fail miserably. */

	/* FIXME These should be configurable */
	/* FIXME g_moduleDirectory really needs to be a wstring */
	std::string pathOBS = g_moduleDirectory;
	std::string locale;

	/* Also note that this method is possible on POSIX
	* as well. You can call dlopen with RTLD_GLOBAL
	* Order matters here. Loading a library out of order
	* will cause a failure to resolve dependencies. */
	static const char* g_modules[] = {
	    "zlib.dll",           "libopus-0.dll",    "libogg-0.dll",    "libvorbis-0.dll",
	    "libvorbisenc-2.dll", "libvpx-1.dll",     "libx264-152.dll", "avutil-55.dll",
	    "swscale-4.dll",      "swresample-2.dll", "avcodec-57.dll",  "avformat-57.dll",
	    "avfilter-6.dll",     "avdevice-57.dll",  "libcurl.dll",     "libvorbisfile-3.dll",
	    "w32-pthreads.dll",   "obsglad.dll",      "obs.dll",         "libobs-d3d11.dll",
	    "libobs-opengl.dll"};

	static const int g_modules_size = sizeof(g_modules) / sizeof(g_modules[0]);

	for (int i = 0; i < g_modules_size; ++i) {
		std::string module_path;
		void*       handle = NULL;

		module_path.reserve(pathOBS.size() + strlen(g_modules[i]) + 1);
		module_path.append(pathOBS);
		module_path.append("/");
		module_path.append(g_modules[i]);

#ifdef _WIN32
		handle = LoadLibraryW(converter.from_bytes(module_path).c_str());
#endif

		if (!handle) {
			std::cerr << "Failed to open dependency " << module_path << std::endl;
		}

		/* This is an intentional leak.
		* We leave these open and let the
		* OS clean these up for us as
		* they should be available through
		* out the application */
	}
	pathConfigDirectory = args[0].value_str.c_str();
	appdata_path        = args[0].value_str.c_str();
	appdata_path += "/node-obs/";

	/* libobs will use three methods of finding data files:
	* 1. ${CWD}/data/libobs <- This doesn't work for us
	* 2. ${OBS_DATA_PATH}/libobs <- This works but is inflexible
	* 3. getenv(OBS_DATA_PATH) + /libobs <- Can be set anywhere
	*    on the cli, in the frontend, or the backend. */
	obs_add_data_path((g_moduleDirectory + "/libobs/data/libobs/").c_str());

	std::vector<char> userData = std::vector<char>(1024);
	os_get_config_path(userData.data(), userData.capacity() - 1, "slobs-client/plugin_config");
	obs_startup(args[1].value_str.c_str(), userData.data(), NULL);

	/* Logging */
	string filename = GenerateTimeDateFilename("txt");
	string log_path = appdata_path;
	log_path.append("/logs/");

	/* Make sure the path is created
	before attempting to make a file there. */
	if (os_mkdirs(log_path.c_str()) == MKDIR_ERROR) {
		cerr << "Failed to open log file" << endl;
	}

	DeleteOldestFile(log_path.c_str(), 3);
	log_path.append(filename);

#if defined(_WIN32) && defined(UNICODE)
	fstream* logfile = new fstream(converter.from_bytes(log_path.c_str()).c_str(), ios_base::out | ios_base::trunc);
#else
	fstream* logfile = new fstream(log_path, ios_base::out | ios_base::trunc);
#endif

	if (!logfile) {
		cerr << "Failed to open log file" << endl;
	}

	/* Delete oldest file in the folder to imitate rotating */
	base_set_log_handler(node_obs_log, logfile);

	/* INJECT osn::Source::Manager */
	// Alright, you're probably wondering: Why is osn code here?
	// Well, simply because the hooks need to run as soon as possible. We don't
	//  want to miss a single create or destroy signal OBS gives us for the
	//  osn::Source::Manager.
	osn::Source::initialize_global_signals();
	/* END INJECT osn::Source::Manager */

	cpuUsageInfo = os_cpu_usage_info_start();

	//Setting obs-studio config directory
	char path[512];
	int  ret = os_get_config_path(path, 512, "obs-studio");

	if (ret > 0) {
		OBS_pathConfigDirectory = path;
	}

	std::string profiles = OBS_pathConfigDirectory + "\\basic\\profiles";
	std::string scenes   = OBS_pathConfigDirectory + "\\basic\\scenes";

	/* Profiling */
	//profiler_start();

	obs_data_t *private_settings = obs_data_create();
	obs_data_set_bool(private_settings, "BrowserHWAccel", true);
	obs_apply_private_data(private_settings);
	obs_data_release(private_settings);

	openAllModules();
	OBS_service::createStreamingOutput();
	OBS_service::createRecordingOutput();

	OBS_service::createVideoStreamingEncoder();
	OBS_service::createVideoRecordingEncoder();

	auto audioStreamingEncoder = OBS_service::getAudioStreamingEncoder();
	auto audioRecordingEncoder = OBS_service::getAudioRecordingEncoder();

	OBS_service::createAudioEncoder(audioStreamingEncoder); // Why we aren't retaining a reference to it?
	OBS_service::createAudioEncoder(audioRecordingEncoder); // Why we aren't retaining a reference to it?

	OBS_service::resetAudioContext();
	OBS_service::resetVideoContext(NULL);

	OBS_service::associateAudioAndVideoToTheCurrentStreamingContext();
	OBS_service::associateAudioAndVideoToTheCurrentRecordingContext();

	OBS_service::createService();

	OBS_service::associateAudioAndVideoEncodersToTheCurrentStreamingOutput();
	OBS_service::associateAudioAndVideoEncodersToTheCurrentRecordingOutput();

	OBS_service::setServiceToTheStreamingOutput();

	setAudioDeviceMonitoring();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void OBS_API::OBS_API_destroyOBS_API(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	/* INJECT osn::Source::Manager */
	// Alright, you're probably wondering: Why is osn code here?
	// Well, simply because the hooks need to run as soon as possible. We don't
	//  want to miss a single create or destroy signal OBS gives us for the
	//  osn::Source::Manager.
	osn::Source::finalize_global_signals();
	/* END INJECT osn::Source::Manager */
	destroyOBS_API();
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void OBS_API::OBS_API_getPerformanceStatistics(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	double percentage              = getCPU_Percentage();
	int    numberOfDroppedFrames   = getNumberOfDroppedFrames();
	double droppedFramesPercentage = getDroppedFramesPercentage();
	double bandwidth               = getCurrentBandwidth();
	double frameRate               = getCurrentFrameRate();

	rval.push_back(ipc::value(percentage));
	rval.push_back(ipc::value(numberOfDroppedFrames));
	rval.push_back(ipc::value(droppedFramesPercentage));
	rval.push_back(ipc::value(bandwidth));
	rval.push_back(ipc::value(frameRate));
	AUTO_DEBUG;
}

void OBS_API::OBS_API_getOBS_existingProfiles(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::string pathProfiles;
	pathProfiles += OBS_pathConfigDirectory;
	pathProfiles += "\\basic\\profiles\\";

	std::vector<std::string> existingProfiles = exploreDirectory(pathProfiles, "directories");

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint32_t)existingProfiles.size()));

	for (int i = 0; i < existingProfiles.size(); i++) {
		rval.push_back(ipc::value(existingProfiles.at(i).c_str()));
	}
	AUTO_DEBUG;
}

void OBS_API::OBS_API_getOBS_existingSceneCollections(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::string pathSceneCollections;
	pathSceneCollections += OBS_pathConfigDirectory;
	pathSceneCollections += "\\basic\\scenes\\";

	std::vector<std::string> existingSceneCollections = exploreDirectory(pathSceneCollections, "files");

	int indexArray = 0;

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint32_t)existingSceneCollections.size()));

	for (int i = 0; i < existingSceneCollections.size(); i++) {
		if (existingSceneCollections.at(i).substr(existingSceneCollections.at(i).find_last_of(".") + 1) == "json") {
			existingSceneCollections.at(i).erase(
			    existingSceneCollections.at(i).end() - 5, existingSceneCollections.at(i).end());

			rval.push_back(ipc::value(existingSceneCollections.at(i).c_str()));
		}
	}
	AUTO_DEBUG;
}

void OBS_API::OBS_API_isOBS_installed(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((bool)isOBS_installed()));
	AUTO_DEBUG;
}

void OBS_API::SetProcessPriority(const char* priority)
{
	if (!priority)
		return;

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
}

void OBS_API::UpdateProcessPriority()
{
	std::string globalConfigFile = OBS_API::getGlobalConfigPath();
	config_t*   globalConfig     = OBS_API::openConfigFile(globalConfigFile);

	const char* priority = config_get_string(globalConfig, "General", "ProcessPriority");
	if (priority && strcmp(priority, "Normal") != 0)
		SetProcessPriority(priority);
}

bool DisableAudioDucking(bool disable)
{
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
}

void OBS_API::setAudioDeviceMonitoring(void)
{
	/* load audio monitoring */
#if defined(_WIN32) || defined(__APPLE__)
	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t*   config          = OBS_API::openConfigFile(basicConfigFile);

	const char* device_name = config_get_string(config, "Audio", "MonitoringDeviceName");
	const char* device_id   = config_get_string(config, "Audio", "MonitoringDeviceId");

	obs_set_audio_monitoring_device(device_name, device_id);

	blog(LOG_INFO, "Audio monitoring device:\n\tname: %s\n\tid: %s", device_name, device_id);

	bool disableAudioDucking = config_get_bool(config, "Audio", "DisableAudioDucking");
	if (disableAudioDucking)
		DisableAudioDucking(true);
#endif
}

static void SaveProfilerData(const profiler_snapshot_t* snap)
{
	string dst(appdata_path);
	dst.append("profiler_data/");

	if (os_mkdirs(dst.c_str()) == MKDIR_ERROR) {
		cerr << "Failed to open profiler snapshot for writing" << endl;
	}

	dst.append(GenerateTimeDateFilename("csv.gz"));

	if (!profiler_snapshot_dump_csv_gz(snap, dst.c_str()))
		blog(LOG_WARNING, "Could not save profiler data to '%s'", dst.c_str());
}

void OBS_API::StopCrashHandler(
	void*                          data,
	const int64_t                  id,
	const std::vector<ipc::value>& args,
	std::vector<ipc::value>&       rval)
{
	writeCrashHandler(unregisterProcess());
	writeCrashHandler(terminateCrashHandler());

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	AUTO_DEBUG;
}

void OBS_API::destroyOBS_API(void)
{
	os_cpu_usage_info_destroy(cpuUsageInfo);

#ifdef _WIN32
	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t*   config          = OBS_API::openConfigFile(basicConfigFile);

	bool disableAudioDucking = config_get_bool(config, "Audio", "DisableAudioDucking");
	if (disableAudioDucking)
		DisableAudioDucking(false);
#endif

	OBS_service::setStreamingEncoder(nullptr);
	OBS_service::setRecordingEncoder(nullptr);
	OBS_service::setAudioStreamingEncoder(nullptr);
	OBS_service::setAudioRecordingEncoder(nullptr);
	OBS_service::setStreamingOutput(nullptr);
	OBS_service::setRecordingOutput(nullptr);
	OBS_service::setService(nullptr);

	// Sanity checks (any race condition will be visible here)
	if (OBS_service::getStreamingEncoder() 
		|| OBS_service::getRecordingEncoder()
	    || OBS_service::getAudioStreamingEncoder() 
		|| OBS_service::getAudioRecordingEncoder()
	    || OBS_service::getStreamingOutput() 
		|| OBS_service::getRecordingOutput() 
		|| OBS_service::getService()) {
		throw "Some of the service objects are still on use when performing shutdown!";
	}
	obs_shutdown();

	/*profiler_stop();
	auto snapshot = profile_snapshot_create();
	profiler_print(snapshot);
	profiler_print_time_between_calls(snapshot);
	SaveProfilerData(snapshot);
	profile_snapshot_free(snapshot);
	profiler_free();*/
}

struct ci_char_traits : public char_traits<char>
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
void OBS_API::openAllModules(void)
{
	OBS_service::resetVideoContext(NULL);

	std::string plugins_paths[] = {g_moduleDirectory + "/obs-plugins/64bit", g_moduleDirectory + "/obs-plugins"};

	std::string plugins_data_paths[] = {g_moduleDirectory + "/data/obs-plugins", plugins_data_paths[0]};

	size_t num_paths = sizeof(plugins_paths) / sizeof(plugins_paths[0]);

	for (int i = 0; i < num_paths; ++i) {
		std::string& plugins_path      = plugins_paths[i];
		std::string& plugins_data_path = plugins_data_paths[i];

		/* FIXME Plugins could be in individual folders, maybe
		* with some metainfo so we don't attempt just any
		* shared library. */
		if (!os_file_exists(plugins_path.c_str())) {
			std::cerr << "Plugin Path provided is invalid: " << plugins_path << std::endl;
			return;
		}

		os_dir_t* plugin_dir = os_opendir(plugins_path.c_str());
		if (!plugin_dir) {
			std::cerr << "Failed to open plugin diretory: " << plugins_path << std::endl;
			return;
		}

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

			obs_module_t* module;
			int           result = obs_open_module(&module, plugin_path.c_str(), plugin_data_path.c_str());

			switch (result) {
			case MODULE_SUCCESS:
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

			bool success = obs_init_module(module);

			if (!success) {
				std::cerr << "Failed to initialize module " << plugin_path << std::endl;
				/* Just continue to next one */
			}
		}

		os_closedir(plugin_dir);
	}
}

double OBS_API::getCPU_Percentage(void)
{
	double cpuPercentage = os_cpu_usage_info_query(cpuUsageInfo);

	cpuPercentage *= 10;
	cpuPercentage = trunc(cpuPercentage);
	cpuPercentage /= 10;

	return cpuPercentage;
}

int OBS_API::getNumberOfDroppedFrames(void)
{
	auto streamOutput = OBS_service::getStreamingOutput();

	int totalDropped = 0;

	if (obs_output_active(streamOutput.get())) {
		totalDropped = obs_output_get_frames_dropped(streamOutput.get());
	}

	return totalDropped;
}

double OBS_API::getDroppedFramesPercentage(void)
{
	auto streamOutput = OBS_service::getStreamingOutput();

	double percent = 0;

	if (obs_output_active(streamOutput.get())) {
		int totalDropped = obs_output_get_frames_dropped(streamOutput.get());
		int totalFrames  = obs_output_get_total_frames(streamOutput.get());
		percent          = (double)totalDropped / (double)totalFrames * 100.0;
	}

	return percent;
}

double OBS_API::getCurrentBandwidth(void)
{
	auto streamOutput = OBS_service::getStreamingOutput();

	double kbitsPerSec = 0;

	if (obs_output_active(streamOutput.get())) {
		uint64_t bytesSent     = obs_output_get_total_bytes(streamOutput.get());
		uint64_t bytesSentTime = os_gettime_ns();

		if (bytesSent < lastBytesSent)
			bytesSent = 0;
		if (bytesSent == 0)
			lastBytesSent = 0;

		uint64_t bitsBetween = (bytesSent - lastBytesSent) * 8;

		double timePassed = double(bytesSentTime - lastBytesSentTime) / 1000000000.0;

		kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;

		lastBytesSent     = bytesSent;
		lastBytesSentTime = bytesSentTime;
	}

	return kbitsPerSec;
}

double OBS_API::getCurrentFrameRate(void)
{
	return obs_get_active_fps();
}

std::string OBS_API::getPathConfigDirectory(void)
{
	return pathConfigDirectory;
}

void OBS_API::setPathConfigDirectory(std::string newPathConfigDirectory)
{
	if (!newPathConfigDirectory.empty() && !useOBS_configFiles) {
		pathConfigDirectory = newPathConfigDirectory;
	}
}

std::vector<std::string> OBS_API::exploreDirectory(std::string directory, std::string typeToReturn)
{
	std::vector<std::string> listElements;

	char originalDirectory[_MAX_PATH];

	// Get the current directory so we can return to it
	_getcwd(originalDirectory, _MAX_PATH);

	_chdir(directory.c_str()); // Change to the working directory
	_finddata_t fileinfo;

	// This will grab the first file in the directory
	// "*" can be changed if you only want to look for specific files
	intptr_t handle = _findfirst("*", &fileinfo);

	if (handle == -1) // No files or directories found
	{
		perror("Error searching for file");
		exit(1);
	}

	do {
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
			continue;
		if (fileinfo.attrib & _A_SUBDIR && typeToReturn.compare("directories") == 0) {
			listElements.push_back(fileinfo.name);
		} else if (typeToReturn.compare("files") == 0) {
			listElements.push_back(fileinfo.name);
		}
	} while (_findnext(handle, &fileinfo) == 0);

	_findclose(handle); // Close the stream

	_chdir(originalDirectory);

	return listElements;
}

std::string OBS_API::getOBS_currentProfile(void)
{
	return OBS_currentProfile;
}

void OBS_API::setOBS_currentProfile(std::string profileName)
{
	OBS_currentProfile = profileName;
}

std::string OBS_API::getOBS_currentSceneCollection(void)
{
	return OBS_currentSceneCollection;
}

void OBS_API::setOBS_currentSceneCollection(std::string sceneCollectionName)
{
	OBS_currentSceneCollection = sceneCollectionName;
}

bool OBS_API::isOBS_configFilesUsed(void)
{
	return useOBS_configFiles;
}

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO info;
	info.cbSize = sizeof(info);
	if (GetMonitorInfo(hMonitor, &info)) {
		std::vector<Screen>* resolutions = reinterpret_cast<std::vector<Screen>*>(dwData);

		Screen screen;

		screen.width  = std::abs(info.rcMonitor.left - info.rcMonitor.right);
		screen.height = std::abs(info.rcMonitor.top - info.rcMonitor.bottom);

		resolutions->push_back(screen);
	}
	return true;
}

std::vector<Screen> OBS_API::availableResolutions(void)
{
	std::vector<Screen> resolutions;
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&resolutions));

	return resolutions;
}

std::string OBS_API::getGlobalConfigPath(void)
{
	std::string globalConfigPath;

	globalConfigPath += pathConfigDirectory;
	globalConfigPath += "\\global.ini";

	return globalConfigPath;
}

std::string OBS_API::getBasicConfigPath(void)
{
	std::string basicConfigPath;

	basicConfigPath += pathConfigDirectory;

	if (useOBS_configFiles && !OBS_currentProfile.empty()) {
		basicConfigPath += "\\basic\\profiles\\";
		basicConfigPath += OBS_currentProfile;
	}

	basicConfigPath += "\\basic.ini";

	return basicConfigPath;
}

std::string OBS_API::getServiceConfigPath(void)
{
	std::string serviceConfigPath;

	serviceConfigPath += pathConfigDirectory;

	if (useOBS_configFiles && !OBS_currentProfile.empty()) {
		serviceConfigPath += "\\basic\\profiles\\";
		serviceConfigPath += OBS_currentProfile;
	}

	serviceConfigPath += "\\service.json";

	return serviceConfigPath;
}

std::string OBS_API::getContentConfigPath(void)
{
	std::string contentConfigPath;

	contentConfigPath += pathConfigDirectory;

	if (useOBS_configFiles && !OBS_currentSceneCollection.empty()) {
		contentConfigPath += "\\basic\\scenes\\";
		contentConfigPath += OBS_currentSceneCollection;
		;
		contentConfigPath += ".json";
	} else {
		contentConfigPath += "\\config.json";
	}

	return contentConfigPath;
}

std::string OBS_API::getStreamingEncoderConfigPath(void)
{
	std::string contentConfigPath;
	contentConfigPath += pathConfigDirectory;
	contentConfigPath += "\\streamEncoder.json";
	return contentConfigPath;
}

std::string OBS_API::getRecordingEncoderConfigPath(void)
{
	std::string contentConfigPath;
	contentConfigPath += pathConfigDirectory;
	contentConfigPath += "\\recordEncoder.json";
	return contentConfigPath;
}

static inline string GetDefaultVideoSavePath()
{
	wchar_t path_utf16[MAX_PATH];
	char    path_utf8[MAX_PATH] = {};

	SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT, path_utf16);

	os_wcs_to_utf8(path_utf16, wcslen(path_utf16), path_utf8, MAX_PATH);
	return string(path_utf8);
}

static const double scaled_vals[] = {1.0, 1.25, (1.0 / 0.75), 1.5, (1.0 / 0.6), 1.75, 2.0, 2.25, 2.5, 2.75, 3.0, 0.0};

std::vector<std::pair<std::string, config_t*>> configFiles;

config_t* OBS_API::openConfigFile(std::string configFile)
{
	std::vector<std::pair<std::string, config_t*>>::iterator it =
	    std::find_if(configFiles.begin(), configFiles.end(), [&configFile](const pair<std::string, config_t*> value) {
		    return (value.first.compare(configFile) == 0);
	    });

	// if(it == configFiles.end()) {
	config_t* config;

	int result = config_open(&config, configFile.c_str(), CONFIG_OPEN_EXISTING);

	if (result != CONFIG_SUCCESS) {
		config = config_create(configFile.c_str());
		config_open(&config, configFile.c_str(), CONFIG_OPEN_EXISTING);
	}

	std::string basic     = "basic.ini";
	std::string subString = configFile.substr(configFile.size() - basic.size(), basic.size()).c_str();

	if (subString.compare(basic) == 0) {
		// Base resolution
		uint32_t cx = 0;
		uint32_t cy = 0;

		/* ----------------------------------------------------- */
		/* move over mixer values in advanced if older config */
		if (config_has_user_value(config, "AdvOut", "RecTrackIndex")
		    && !config_has_user_value(config, "AdvOut", "RecTracks")) {
			uint64_t track = config_get_uint(config, "AdvOut", "RecTrackIndex");
			track          = 1ULL << (track - 1);
			config_set_uint(config, "AdvOut", "RecTracks", track);
			config_remove_value(config, "AdvOut", "RecTrackIndex");
			config_save_safe(config, "tmp", nullptr);
		}

		config_set_default_string(config, "Output", "Mode", "Simple");
		std::string filePath = GetDefaultVideoSavePath();
		config_set_default_string(config, "SimpleOutput", "FilePath", filePath.c_str());
		config_set_default_string(config, "SimpleOutput", "RecFormat", "flv");
		config_set_default_uint(config, "SimpleOutput", "VBitrate", 2500);
		config_set_default_string(config, "SimpleOutput", "StreamEncoder", SIMPLE_ENCODER_X264);
		config_set_default_uint(config, "SimpleOutput", "ABitrate", 160);
		config_set_default_bool(config, "SimpleOutput", "UseAdvanced", false);
		config_set_default_bool(config, "SimpleOutput", "EnforceBitrate", true);
		config_set_default_string(config, "SimpleOutput", "Preset", "veryfast");
		config_set_default_string(config, "SimpleOutput", "RecQuality", "Stream");
		config_set_default_string(config, "SimpleOutput", "RecEncoder", SIMPLE_ENCODER_X264);
		config_set_default_bool(config, "SimpleOutput", "RecRB", false);
		config_set_default_int(config, "SimpleOutput", "RecRBTime", 20);
		config_set_default_int(config, "SimpleOutput", "RecRBSize", 512);
		config_set_default_string(config, "SimpleOutput", "RecRBPrefix", "Replay");

		config_set_default_bool(config, "AdvOut", "ApplyServiceSettings", true);
		config_set_default_bool(config, "AdvOut", "UseRescale", false);
		config_set_default_uint(config, "AdvOut", "TrackIndex", 1);
		config_set_default_string(config, "AdvOut", "Encoder", "obs_x264");

		config_set_default_string(config, "AdvOut", "RecType", "Standard");

		config_set_default_string(config, "AdvOut", "RecFilePath", GetDefaultVideoSavePath().c_str());
		config_set_default_string(config, "AdvOut", "RecFormat", "flv");
		config_set_default_bool(config, "AdvOut", "RecUseRescale", false);
		config_set_default_uint(config, "AdvOut", "RecTracks", (1 << 0));
		config_set_default_string(config, "AdvOut", "RecEncoder", "none");

		config_set_default_bool(config, "AdvOut", "FFOutputToFile", true);
		config_set_default_string(config, "AdvOut", "FFFilePath", GetDefaultVideoSavePath().c_str());
		config_set_default_string(config, "AdvOut", "FFExtension", "mp4");
		config_set_default_uint(config, "AdvOut", "FFVBitrate", 2500);
		config_set_default_uint(config, "AdvOut", "FFVGOPSize", 250);
		config_set_default_bool(config, "AdvOut", "FFUseRescale", false);
		config_set_default_bool(config, "AdvOut", "FFIgnoreCompat", false);
		config_set_default_uint(config, "AdvOut", "FFABitrate", 160);
		config_set_default_uint(config, "AdvOut", "FFAudioTrack", 1);

		config_set_default_uint(config, "AdvOut", "Track1Bitrate", 160);
		config_set_default_uint(config, "AdvOut", "Track2Bitrate", 160);
		config_set_default_uint(config, "AdvOut", "Track3Bitrate", 160);
		config_set_default_uint(config, "AdvOut", "Track4Bitrate", 160);
		config_set_default_uint(config, "AdvOut", "Track5Bitrate", 160);
		config_set_default_uint(config, "AdvOut", "Track6Bitrate", 160);

		config_set_default_uint(config, "Video", "BaseCX", cx);
		config_set_default_uint(config, "Video", "BaseCY", cy);

		/* don't allow BaseCX/BaseCY to be susceptible to defaults changing */
		if (!config_has_user_value(config, "Video", "BaseCX") || !config_has_user_value(config, "Video", "BaseCY")) {
			config_set_uint(config, "Video", "BaseCX", cx);
			config_set_uint(config, "Video", "BaseCY", cy);
			config_save_safe(config, "tmp", nullptr);
		}

		config_set_default_string(config, "Output", "FilenameFormatting", "%CCYY-%MM-%DD %hh-%mm-%ss");

		config_set_default_bool(config, "Output", "DelayEnable", false);
		config_set_default_uint(config, "Output", "DelaySec", 20);
		config_set_default_bool(config, "Output", "DelayPreserve", true);

		config_set_default_bool(config, "Output", "Reconnect", true);
		config_set_default_uint(config, "Output", "RetryDelay", 10);
		config_set_default_uint(config, "Output", "MaxRetries", 20);

		config_set_default_string(config, "Output", "BindIP", "default");
		config_set_default_bool(config, "Output", "NewSocketLoopEnable", false);
		config_set_default_bool(config, "Output", "LowLatencyEnable", false);

		int      i        = 0;
		uint32_t scale_cx = 0;
		uint32_t scale_cy = 0;

		/* use a default scaled resolution that has a pixel count no higher
		* than 1280x720 */
		while (((scale_cx * scale_cy) > (1280 * 720)) && scaled_vals[i] > 0.0) {
			double scale = scaled_vals[i++];
			scale_cx     = uint32_t(double(cx) / scale);
			scale_cy     = uint32_t(double(cy) / scale);
		}

		config_set_default_uint(config, "Video", "OutputCX", scale_cx);
		config_set_default_uint(config, "Video", "OutputCY", scale_cy);

		/* don't allow OutputCX/OutputCY to be susceptible to defaults
		* changing */
		if (!config_has_user_value(config, "Video", "OutputCX")
		    || !config_has_user_value(config, "Video", "OutputCY")) {
			config_set_uint(config, "Video", "OutputCX", scale_cx);
			config_set_uint(config, "Video", "OutputCY", scale_cy);
			config_save_safe(config, "tmp", nullptr);
		}

		config_set_default_uint(config, "Video", "FPSType", 0);
		config_set_default_string(config, "Video", "FPSCommon", "30");
		config_set_default_uint(config, "Video", "FPSInt", 30);
		config_set_default_uint(config, "Video", "FPSNum", 30);
		config_set_default_uint(config, "Video", "FPSDen", 1);
		config_set_default_string(config, "Video", "ScaleType", "bicubic");
		config_set_default_string(config, "Video", "ColorFormat", "NV12");
		config_set_default_string(config, "Video", "ColorSpace", "601");
		config_set_default_string(config, "Video", "ColorRange", "Partial");

		config_set_default_string(config, "Audio", "MonitoringDeviceId", "default");
		config_set_default_string(config, "Audio", "MonitoringDeviceName", "Default");
		config_set_default_uint(config, "Audio", "SampleRate", 44100);
		config_set_default_string(config, "Audio", "ChannelSetup", "Stereo");

		config_save_safe(config, "tmp", nullptr);
	}

	configFiles.push_back(std::make_pair(configFile, config));
	return config;
	// } else {
	// 	return (*it).second;
	// }
}

bool OBS_API::isOBS_installed(void)
{
	return isOBS_installedValue;
}