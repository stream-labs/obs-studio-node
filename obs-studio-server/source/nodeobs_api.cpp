#include "nodeobs_api.h"
#include "osn-source.hpp"
#include "util/lexer.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0502

#include "nodeobs_content.h"
#include <mutex>
#include <string>
#include <ShlObj.h>
#include <locale>
#include <codecvt>
#endif

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#endif

#define WIN32_LEAN_AND_MEAN

#include <mmdeviceapi.h>
#include <audiopolicy.h>

#include <util/windows/WinHandle.hpp>
#include <util/windows/HRError.hpp>
#include <util/windows/ComPtr.hpp>

#include "error.hpp"
#include "shared.hpp"


std::string g_moduleDirectory = "";
os_cpu_usage_info_t *cpuUsageInfo = nullptr;
uint64_t lastBytesSent = 0;
uint64_t lastBytesSentTime = 0;
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

void OBS_API::Register(ipc::server& srv) {
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("API");

	cls->register_function(std::make_shared<ipc::function>("OBS_API_initAPI", std::vector<ipc::type>{ipc::type::String, ipc::type::String}, OBS_API_initAPI));
	cls->register_function(std::make_shared<ipc::function>("OBS_API_destroyOBS_API", std::vector<ipc::type>{}, OBS_API_destroyOBS_API));
	cls->register_function(std::make_shared<ipc::function>("OBS_API_getPerformanceStatistics", std::vector<ipc::type>{}, OBS_API_getPerformanceStatistics));
	cls->register_function(std::make_shared<ipc::function>("SetWorkingDirectory", std::vector<ipc::type>{ipc::type::String}, SetWorkingDirectory));

	srv.register_collection(cls);
}

void replaceAll(std::string &str, const std::string &from,
	const std::string &to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos +=
			to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
};

void OBS_API::SetWorkingDirectory(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval)
{
	g_moduleDirectory = args[0].value_str;
	replaceAll(g_moduleDirectory, "\\", "/");
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value(g_moduleDirectory));
	AUTO_DEBUG;
}

static string GenerateTimeDateFilename(const char *extension)
{
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

	return string(file);
}

static bool GetToken(lexer *lex, string &str, base_token_type type)
{
	base_token token;
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != type)
		return false;

	str.assign(token.text.array, token.text.len);
	return true;
}

static bool ExpectToken(lexer *lex, const char *str, base_token_type type)
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
static uint64_t ConvertLogName(const char *name)
{
	lexer  lex;
	string     year, month, day, hour, minute, second;

	lexer_init(&lex);
	lexer_start(&lex, name);

	if (!GetToken(&lex, year, BASETOKEN_DIGIT)) return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!GetToken(&lex, month, BASETOKEN_DIGIT)) return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!GetToken(&lex, day, BASETOKEN_DIGIT)) return 0;
	if (!GetToken(&lex, hour, BASETOKEN_DIGIT)) return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!GetToken(&lex, minute, BASETOKEN_DIGIT)) return 0;
	if (!ExpectToken(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!GetToken(&lex, second, BASETOKEN_DIGIT)) return 0;

	std::string timestring(year);
	timestring += month + day + hour + minute + second;
	lexer_free(&lex);
	return std::stoull(timestring);
}

static void DeleteOldestFile(const char *location, unsigned maxLogs)
{
	string           oldestLog;
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

#pragma region Logging
#include <chrono>
#include <cstdarg>
#include <varargs.h>

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#else
#include <unistd.h>
#endif

inline std::string nodeobs_log_formatted_message(const char* format, va_list& args) {
	size_t length = _vscprintf(format, args);
	std::vector<char> buf = std::vector<char>(length + 1, '\0');
	size_t written = vsprintf_s(buf.data(), buf.size(), format, args);
	return std::string(buf.begin(), buf.begin() + length);
}

std::chrono::high_resolution_clock hrc;
std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
static void node_obs_log(int log_level, const char *msg, va_list args, void *param) {
	// Calculate log time.
	auto timeSinceStart = (std::chrono::high_resolution_clock::now() - tp);
	auto days = std::chrono::duration_cast<std::chrono::duration<int, ratio<86400>>>(timeSinceStart);
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
		}
		else if (log_level > 50 && log_level < LOG_ERROR) {
			levelname = "Error";
		}
		else if (log_level > LOG_ERROR && log_level < LOG_WARNING) {
			levelname = "Alert";
		}
		else if (log_level > LOG_WARNING && log_level < LOG_INFO) {
			levelname = "Hint";
		}
		else if (log_level > LOG_INFO) {
			levelname = "Notice";
		}
		break;
	}

	std::vector<char> timebuf(65535, '\0');
	std::string timeformat = "[%.3d:%.2d:%.2d:%.2d.%.3d.%.3d.%.3d][%*s]";// "%*s";
	int length = sprintf_s(
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
		levelname.length(), levelname.c_str());
	std::string time_and_level = std::string(timebuf.data(), length);

	// Format incoming text
	std::string text = nodeobs_log_formatted_message(msg, args);


	std::fstream *logStream = reinterpret_cast<std::fstream*>(param);

	// Split by \n (new-line)
	size_t last_valid_idx = 0;
	for (size_t idx = 0; idx <= text.length(); idx++) {
		char& ch = text[idx];
		if ((ch == '\n') || (idx == text.length())) {
			std::string newmsg = time_and_level + " " + std::string(&text[last_valid_idx], idx - last_valid_idx) + '\n';
			last_valid_idx = idx + 1;

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
					std::mutex wide_mutex;

					lock_guard<mutex> lock(wide_mutex);
					wide_buf.reserve(wNum + 1);
					wide_buf.resize(wNum - 1);
					MultiByteToWideChar(CP_UTF8, 0, newmsg.c_str(), -1, &wide_buf[0],
						wNum);

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
#pragma endregion Logging

static inline string GetDefaultVideoSavePath()
{
	wchar_t path_utf16[MAX_PATH];
	char    path_utf8[MAX_PATH] = {};

	SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT,
		path_utf16);

	os_wcs_to_utf8(path_utf16, wcslen(path_utf16), path_utf8, MAX_PATH);
	return string(path_utf8);
}

static const double scaled_vals[] =
{
	1.0,
	1.25,
	(1.0 / 0.75),
	1.5,
	(1.0 / 0.6),
	1.75,
	2.0,
	2.25,
	2.5,
	2.75,
	3.0,
	0.0
};

void initGlobalDefault(config_t* config) {
	config_set_bool(config, "BasicWindow", "SnappingEnabled", true);
	config_set_double(config, "BasicWindow", "SnapDistance", 10);
	config_set_bool(config, "BasicWindow", "ScreenSnapping", true);
	config_set_bool(config, "BasicWindow", "SourceSnapping", true);
	config_set_bool(config, "BasicWindow", "CenterSnapping", false);

	config_save_safe(config, "tmp", nullptr);
}

void initBasicDefault(config_t* config) {
	// Base resolution
	uint32_t cx = 0;
	uint32_t cy = 0;

	/* ----------------------------------------------------- */
	/* move over mixer values in advanced if older config */
	if (config_has_user_value(config, "AdvOut", "RecTrackIndex") &&
		!config_has_user_value(config, "AdvOut", "RecTracks")) {

		uint64_t track = config_get_uint(config, "AdvOut",
			"RecTrackIndex");
		track = 1ULL << (track - 1);
		config_set_uint(config, "AdvOut", "RecTracks", track);
		config_remove_value(config, "AdvOut", "RecTrackIndex");
		config_save_safe(config, "tmp", nullptr);
	}

	config_set_default_string(config, "Output", "Mode", "Simple");
	std::string filePath = GetDefaultVideoSavePath();
	config_set_default_string(config, "SimpleOutput", "FilePath",
		filePath.c_str());
	config_set_default_string(config, "SimpleOutput", "RecFormat",
		"flv");
	config_set_default_uint(config, "SimpleOutput", "VBitrate",
		2500);
	config_set_default_string(config, "SimpleOutput", "StreamEncoder",
		SIMPLE_ENCODER_X264);
	config_set_default_uint(config, "SimpleOutput", "ABitrate", 160);
	config_set_default_bool(config, "SimpleOutput", "UseAdvanced",
		false);
	config_set_default_bool(config, "SimpleOutput", "EnforceBitrate",
		true);
	config_set_default_string(config, "SimpleOutput", "Preset",
		"veryfast");
	config_set_default_string(config, "SimpleOutput", "RecQuality",
		"Stream");
	config_set_default_string(config, "SimpleOutput", "RecEncoder",
		SIMPLE_ENCODER_X264);
	config_set_default_bool(config, "SimpleOutput", "RecRB", false);
	config_set_default_int(config, "SimpleOutput", "RecRBTime", 20);
	config_set_default_int(config, "SimpleOutput", "RecRBSize", 512);
	config_set_default_string(config, "SimpleOutput", "RecRBPrefix",
		"Replay");

	config_set_default_bool(config, "AdvOut", "ApplyServiceSettings",
		true);
	config_set_default_bool(config, "AdvOut", "UseRescale", false);
	config_set_default_uint(config, "AdvOut", "TrackIndex", 1);
	config_set_default_string(config, "AdvOut", "Encoder", "obs_x264");

	config_set_default_string(config, "AdvOut", "RecType", "Standard");

	config_set_default_string(config, "AdvOut", "RecFilePath",
		GetDefaultVideoSavePath().c_str());
	config_set_default_string(config, "AdvOut", "RecFormat", "flv");
	config_set_default_bool(config, "AdvOut", "RecUseRescale",
		false);
	config_set_default_uint(config, "AdvOut", "RecTracks", (1 << 0));
	config_set_default_string(config, "AdvOut", "RecEncoder",
		"none");

	config_set_default_bool(config, "AdvOut", "FFOutputToFile",
		true);
	config_set_default_string(config, "AdvOut", "FFFilePath",
		GetDefaultVideoSavePath().c_str());
	config_set_default_string(config, "AdvOut", "FFExtension", "mp4");
	config_set_default_uint(config, "AdvOut", "FFVBitrate", 2500);
	config_set_default_uint(config, "AdvOut", "FFVGOPSize", 250);
	config_set_default_bool(config, "AdvOut", "FFUseRescale",
		false);
	config_set_default_bool(config, "AdvOut", "FFIgnoreCompat",
		false);
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
	if (!config_has_user_value(config, "Video", "BaseCX") ||
		!config_has_user_value(config, "Video", "BaseCY")) {
		config_set_uint(config, "Video", "BaseCX", cx);
		config_set_uint(config, "Video", "BaseCY", cy);
		config_save_safe(config, "tmp", nullptr);
	}

	config_set_default_string(config, "Output", "FilenameFormatting",
		"%CCYY-%MM-%DD %hh-%mm-%ss");

	config_set_default_bool(config, "Output", "DelayEnable", false);
	config_set_default_uint(config, "Output", "DelaySec", 20);
	config_set_default_bool(config, "Output", "DelayPreserve", true);

	config_set_default_bool(config, "Output", "Reconnect", true);
	config_set_default_uint(config, "Output", "RetryDelay", 10);
	config_set_default_uint(config, "Output", "MaxRetries", 20);

	config_set_default_string(config, "Output", "BindIP", "default");
	config_set_default_bool(config, "Output", "NewSocketLoopEnable",
		false);
	config_set_default_bool(config, "Output", "LowLatencyEnable",
		false);

	int i = 0;
	uint32_t scale_cx = 0;
	uint32_t scale_cy = 0;

	/* use a default scaled resolution that has a pixel count no higher
	* than 1280x720 */
	while (((scale_cx * scale_cy) > (1280 * 720)) && scaled_vals[i] > 0.0) {
		double scale = scaled_vals[i++];
		scale_cx = uint32_t(double(cx) / scale);
		scale_cy = uint32_t(double(cy) / scale);
	}

	config_set_default_uint(config, "Video", "OutputCX", scale_cx);
	config_set_default_uint(config, "Video", "OutputCY", scale_cy);

	/* don't allow OutputCX/OutputCY to be susceptible to defaults
	* changing */
	if (!config_has_user_value(config, "Video", "OutputCX") ||
		!config_has_user_value(config, "Video", "OutputCY")) {
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
	config_set_default_string(config, "Video", "ColorRange",
		"Partial");

	config_set_default_string(config, "Audio", "MonitoringDeviceId",
		"default");
	config_set_default_string(config, "Audio", "MonitoringDeviceName",
		"Default");
	config_set_default_uint(config, "Audio", "SampleRate", 44100);
	config_set_default_string(config, "Audio", "ChannelSetup",
		"Stereo");

	config_save_safe(config, "tmp", nullptr);
}

void OBS_API::OBS_API_initAPI(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	/* Map base DLLs as soon as possible into the current process space.
	* In particular, we need to load obs.dll into memory before we call
	* any functions from obs else if we delay-loaded the dll, it will
	* fail miserably. */

	/* FIXME These should be configurable */
	/* FIXME g_moduleDirectory really needs to be a wstring */
	std::string appdata = args[0].value_str;
	std::string locale = args[1].value_str;

	/* Also note that this method is possible on POSIX
	* as well. You can call dlopen with RTLD_GLOBAL
	* Order matters here. Loading a library out of order
	* will cause a failure to resolve dependencies. */
	static const char *g_modules[] = {
		"zlib.dll",
		"libopus-0.dll",
		"libogg-0.dll",
		"libvorbis-0.dll",
		"libvorbisenc-2.dll",
		"libvpx-1.dll",
		"libx264-152.dll",
		"avutil-55.dll",
		"swscale-4.dll",
		"swresample-2.dll",
		"avcodec-57.dll",
		"avformat-57.dll",
		"avfilter-6.dll",
		"avdevice-57.dll",
		"libcurl.dll",
		"libvorbisfile-3.dll",
		"w32-pthreads.dll",
		"obsglad.dll",
		"obs.dll",
		"libobs-d3d11.dll",
		"libobs-opengl.dll"
	};

	static const int g_modules_size = sizeof(g_modules) / sizeof(g_modules[0]);

	for (int i = 0; i < g_modules_size; ++i) {
		std::string module_path;
		void *handle = NULL;

		module_path.reserve(g_moduleDirectory.size() + strlen(g_modules[i]) + 1);
		module_path.append(g_moduleDirectory);
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
	
	/* libobs will use three methods of finding data files:
	* 1. ${CWD}/data/libobs <- This doesn't work for us
	* 2. ${OBS_DATA_PATH}/libobs <- This works but is inflexible
	* 3. getenv(OBS_DATA_PATH) + /libobs <- Can be set anywhere
	*    on the cli, in the frontend, or the backend. */
	obs_add_data_path((g_moduleDirectory + "/libobs/data/libobs/").c_str());

	std::vector<char> userData = std::vector<char>(1024);
	os_get_config_path(userData.data(), userData.capacity() - 1, "slobs-client/plugin_config");
	obs_startup(locale.c_str(), userData.data(), NULL);

#pragma region Logging
	/* Logging */
	string filename = GenerateTimeDateFilename("txt");
	string log_path = appdata + "/node-obs/";
	log_path.append("/logs/");

	/* Make sure the path is created
	before attempting to make a file there. */
	if (os_mkdirs(log_path.c_str()) == MKDIR_ERROR) {
		cerr << "Failed to open log file" << endl;
	}

	DeleteOldestFile(log_path.c_str(), 3);
	log_path.append(filename);

#if defined(_WIN32) && defined(UNICODE)
	fstream *logfile = new fstream(
		converter.from_bytes(log_path.c_str()).c_str(),
		ios_base::out |
		ios_base::trunc
	);
#else
	fstream *logfile = new fstream(
		log_path,
		ios_base::out | ios_base::trunc
	);
#endif

	if (!logfile) {
		cerr << "Failed to open log file" << endl;
	}

	/* Delete oldest file in the folder to imitate rotating */
	base_set_log_handler(node_obs_log, logfile);
#pragma endregion Logging

	/* INJECT osn::Source::Manager */
	// Alright, you're probably wondering: Why is osn code here?
	// Well, simply because the hooks need to run as soon as possible. We don't
	//  want to miss a single create or destroy signal OBS gives us for the
	//  osn::Source::Manager.
	osn::Source::initialize_global_signals();
	/* END INJECT osn::Source::Manager */

	cpuUsageInfo = os_cpu_usage_info_start();

	ConfigManager::getInstance().setAppdataPath(appdata);

	openAllModules();

	initGlobalDefault(ConfigManager::getInstance().getGlobal());
	initBasicDefault(ConfigManager::getInstance().getBasic());

	OBS_service::createStreamingOutput();
	OBS_service::createRecordingOutput();

	OBS_service::createVideoStreamingEncoder();
	OBS_service::createVideoRecordingEncoder();

	obs_encoder_t* audioStreamingEncoder = OBS_service::getAudioStreamingEncoder();
	obs_encoder_t* audioRecordingEncoder = OBS_service::getAudioRecordingEncoder();

	OBS_service::createAudioEncoder(&audioStreamingEncoder);
	OBS_service::createAudioEncoder(&audioRecordingEncoder);

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

void OBS_API::OBS_API_destroyOBS_API(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
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

void OBS_API::OBS_API_getPerformanceStatistics(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval) {
	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	rval.push_back(ipc::value(getCPU_Percentage()));
	rval.push_back(ipc::value(getNumberOfDroppedFrames()));
	rval.push_back(ipc::value(getDroppedFramesPercentage()));
	rval.push_back(ipc::value(getCurrentBandwidth()));
	rval.push_back(ipc::value(getCurrentFrameRate()));
	AUTO_DEBUG;
}

void OBS_API::SetProcessPriority(const char *priority)
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
	const char *priority = config_get_string(ConfigManager::getInstance().getGlobal(),
		"General", "ProcessPriority");
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

	HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
		nullptr, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator),
		(void **)&devEmum);
	if (FAILED(result))
		return false;

	result = devEmum->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	if (FAILED(result))
		return false;

	result = device->Activate(__uuidof(IAudioSessionManager2),
		CLSCTX_INPROC_SERVER, nullptr,
		(void **)&sessionManager2);
	if (FAILED(result))
		return false;

	result = sessionManager2->GetAudioSessionControl(nullptr, 0,
		&sessionControl);
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
	const char *device_name = config_get_string(ConfigManager::getInstance().getBasic(), "Audio",
		"MonitoringDeviceName");
	const char *device_id = config_get_string(ConfigManager::getInstance().getBasic(), "Audio",
		"MonitoringDeviceId");

	obs_set_audio_monitoring_device(device_name, device_id);

	blog(LOG_INFO, "Audio monitoring device:\n\tname: %s\n\tid: %s",
		device_name, device_id);

	bool disableAudioDucking = config_get_bool(ConfigManager::getInstance().getBasic(), "Audio",
		"DisableAudioDucking");
	if (disableAudioDucking)
		DisableAudioDucking(true);
#endif
}

void OBS_API::destroyOBS_API(void) {
	os_cpu_usage_info_destroy(cpuUsageInfo);

#ifdef _WIN32
	bool disableAudioDucking = config_get_bool(ConfigManager::getInstance().getBasic(), "Audio",
		"DisableAudioDucking");
	if (disableAudioDucking)
		DisableAudioDucking(false);
#endif

	obs_encoder_t* streamingEncoder = OBS_service::getStreamingEncoder();
	if (streamingEncoder != NULL)
		obs_encoder_release(streamingEncoder);

	obs_encoder_t* recordingEncoder = OBS_service::getRecordingEncoder();
	if (recordingEncoder != NULL)
		obs_encoder_release(recordingEncoder);

	obs_encoder_t* audioStreamingEncoder = OBS_service::getAudioStreamingEncoder();
	if (audioStreamingEncoder != NULL)
		obs_encoder_release(audioStreamingEncoder);

	obs_encoder_t* audioRecordingEncoder = OBS_service::getAudioRecordingEncoder();
	if (audioRecordingEncoder != NULL)
		obs_encoder_release(audioRecordingEncoder);

	obs_output_t* streamingOutput = OBS_service::getStreamingOutput();
	if (streamingOutput != NULL)
		obs_output_release(streamingOutput);

	obs_output_t* recordingOutput = OBS_service::getRecordingOutput();
	if (recordingOutput != NULL)
		obs_output_release(recordingOutput);

	obs_service_t* service = OBS_service::getService();
	if (service != NULL)
		obs_service_release(service);

	obs_shutdown();
}

#pragma region Case-Insensitive String
struct ci_char_traits : public char_traits<char> {
	static bool eq(char c1, char c2) {
		return toupper(c1) == toupper(c2);
	}
	static bool ne(char c1, char c2) {
		return toupper(c1) != toupper(c2);
	}
	static bool lt(char c1, char c2) {
		return toupper(c1) < toupper(c2);
	}
	static int compare(const char* s1, const char* s2, size_t n) {
		while (n-- != 0) {
			if (toupper(*s1) < toupper(*s2)) return -1;
			if (toupper(*s1) > toupper(*s2)) return 1;
			++s1; ++s2;
		}
		return 0;
	}
	static const char* find(const char* s, int n, char a) {
		while (n-- > 0 && toupper(*s) != toupper(a)) {
			++s;
		}
		return s;
	}
};

typedef std::basic_string<char, ci_char_traits> istring;
#pragma endregion Case-Insensitive String

/* This should be reusable outside of node-obs, especially
* if we go a server/client route. */
void OBS_API::openAllModules(void) {
	OBS_service::resetVideoContext(NULL);

	std::string plugins_paths[] = {
		g_moduleDirectory + "/obs-plugins/64bit",
		g_moduleDirectory + "/obs-plugins"
	};

	std::string plugins_data_paths[] = {
		g_moduleDirectory + "/data/obs-plugins",
		plugins_data_paths[0]
	};

	size_t num_paths = sizeof(plugins_paths) / sizeof(plugins_paths[0]);

	for (int i = 0; i < num_paths; ++i) {
		std::string &plugins_path = plugins_paths[i];
		std::string &plugins_data_path = plugins_data_paths[i];

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

			std::string plugin_path = plugins_path + "/" + fullname;
			std::string plugin_data_path = plugins_data_path + "/" + basename;
			if (ent->directory) {
				continue;
			}

#ifdef _WIN32
			if (fullname.substr(fullname.find_last_of(".") + 1) != "dll") {
				continue;
			}
#endif

			obs_module_t *module;
			int result = obs_open_module(&module, plugin_path.c_str(), plugin_data_path.c_str());

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
	obs_output_t* streamOutput = OBS_service::getStreamingOutput();

	int totalDropped = 0;

	if (obs_output_active(streamOutput))
	{
		totalDropped = obs_output_get_frames_dropped(streamOutput);
	}

	return totalDropped;
}

double OBS_API::getDroppedFramesPercentage(void)
{
	obs_output_t* streamOutput = OBS_service::getStreamingOutput();

	double percent = 0;

	if (obs_output_active(streamOutput))
	{
		int totalDropped = obs_output_get_frames_dropped(streamOutput);
		int totalFrames = obs_output_get_total_frames(streamOutput);
		percent = (double)totalDropped / (double)totalFrames * 100.0;
	}

	return percent;
}

double OBS_API::getCurrentBandwidth(void)
{
	obs_output_t* streamOutput = OBS_service::getStreamingOutput();

	double kbitsPerSec = 0;

	if (obs_output_active(streamOutput))
	{
		uint64_t bytesSent = obs_output_get_total_bytes(streamOutput);
		uint64_t bytesSentTime = os_gettime_ns();

		if (bytesSent < lastBytesSent)
			bytesSent = 0;
		if (bytesSent == 0)
			lastBytesSent = 0;

		uint64_t bitsBetween = (bytesSent - lastBytesSent) * 8;

		double timePassed = double(bytesSentTime - lastBytesSentTime) /
			1000000000.0;

		kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;

		lastBytesSent = bytesSent;
		lastBytesSentTime = bytesSentTime;
	}

	return kbitsPerSec;
}

double OBS_API::getCurrentFrameRate(void)
{
	return obs_get_active_fps();
}

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,
	HDC      hdcMonitor,
	LPRECT   lprcMonitor,
	LPARAM   dwData)
{
	MONITORINFO info;
	info.cbSize = sizeof(info);
	if (GetMonitorInfo(hMonitor, &info))
	{
		std::vector<Screen>* resolutions = reinterpret_cast<std::vector<Screen>*>(dwData);

		Screen screen;

		screen.width = std::abs(info.rcMonitor.left - info.rcMonitor.right);
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