#include "nodeobs_api.h"
#include "nodeobs_module.h"
#include "util/platform.h"
#include "util/lexer.h"

#ifdef _WIN32
#define _WIN32_WINNT 0x0502

#include "nodeobs_content.h"
#include <mutex>
#include <fstream>
#include <locale>
#include <codecvt>
#include <string>
// #include <windows.h>
#include <ShlObj.h>
#endif

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#endif

#include <util/windows/win-version.h>
#include <util/platform.h>

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

std::string appdata_path;
vector<pair<obs_module_t *, int>> listModules;
os_cpu_usage_info_t *cpuUsageInfo = nullptr;
uint64_t lastBytesSent = 0;
uint64_t lastBytesSentTime = 0;
std::string pathConfigDirectory;
std::string OBS_pathConfigDirectory;
std::string OBS_currentProfile;
std::string OBS_currentSceneCollection;
bool useOBS_configFiles = false;
bool isOBS_installedValue;

OBS_API::OBS_API() {

}
OBS_API::~OBS_API() {
	while (listModules.size() != 0) {
		listModules.pop_back();
	}
}

void OBS_API::OBS_API_initAPI(const FunctionCallbackInfo<Value>& args)
{
	String::Utf8Value path(args[0]);

	if (args[0]->IsString()) {
		appdata_path = *path;
		appdata_path += "/node-obs/";
		pathConfigDirectory = *path;
	}
	else {
		char *tmp;
		appdata_path = tmp = os_get_config_path_ptr("node-obs/");
		bfree(tmp);
	}

	initAPI();
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

static void stdout_log_handler(int log_level, const char *format,
		va_list args, void *param)
{
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

void node_obs_log
		(int log_level, const char *msg,
		 va_list args, void *param)
{
	fstream &logFile = *static_cast<fstream*>(param);
	char str[4096];

	va_list args2;
	va_copy(args2, args);

	stdout_log_handler(log_level, msg, args, nullptr);
	vsnprintf(str, 4095, msg, args2);

#ifdef _WIN32
	if (IsDebuggerPresent()) {
		int wNum = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
		if (wNum > 1) {
			static wstring wide_buf;
			static mutex wide_mutex;

			lock_guard<mutex> lock(wide_mutex);
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

		logFile << tmp_str << endl;
		nextLine++;
		tmp_str = nextLine;
	}

	logFile << tmp_str << endl;

#if defined(_WIN32) && defined(OBS_DEBUGBREAK_ON_ERROR)
	if (log_level <= LOG_ERROR && IsDebuggerPresent())
		__debugbreak();
#endif
}

static bool get_token(lexer *lex, string &str, base_token_type type)
{
	base_token token;
	if (!lexer_getbasetoken(lex, &token, IGNORE_WHITESPACE))
		return false;
	if (token.type != type)
		return false;

	str.assign(token.text.array, token.text.len);
	return true;
}

static bool expect_token(lexer *lex, const char *str, base_token_type type)
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
static uint64_t convert_log_name(const char *name)
{
	lexer  lex;
	string     year, month, day, hour, minute, second;

	lexer_init(&lex);
	lexer_start(&lex, name);

	if (!get_token(&lex, year,   BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, month,  BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, day,    BASETOKEN_DIGIT)) return 0;
	if (!get_token(&lex, hour,   BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, minute, BASETOKEN_DIGIT)) return 0;
	if (!expect_token(&lex, "-", BASETOKEN_OTHER)) return 0;
	if (!get_token(&lex, second, BASETOKEN_DIGIT)) return 0;

	std::string timestring(year);
	timestring += month + day + hour + minute + second;
	lexer_free(&lex);
	return std::stoull(timestring);
}

static void delete_oldest_file(const char *location, unsigned maxLogs)
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
		string delPath;

		delPath = delPath + location + "/" + oldestLog;
		os_unlink(delPath.c_str());
	}
}

void OBS_API::OBS_API_initOBS_API(const FunctionCallbackInfo<Value>& args) {
	String::Utf8Value path(args[0]);

	if (args[0]->IsString()) {
		appdata_path = *path;
		appdata_path += "/node-obs/";
		pathConfigDirectory = *path;
	}
	else {
		char *tmp;
		appdata_path = tmp = os_get_config_path_ptr("node-obs/");
		bfree(tmp);
	}

	initOBS_API();
}

void OBS_API::OBS_API_destroyOBS_API(const FunctionCallbackInfo<Value>& args) {
    destroyOBS_API();
}

void OBS_API::OBS_API_openAllModules(const FunctionCallbackInfo<Value>& args) {
	openAllModules();
}

void OBS_API::OBS_API_initAllModules(const FunctionCallbackInfo<Value>& args) {
	initAllModules();
}

void OBS_API::OBS_API_getPerformanceStatistics(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	Local<Object> statistics = getPerformanceStatistics();

	args.GetReturnValue().Set(statistics);	
}

void OBS_API::OBS_API_getPathConfigDirectory(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	args.GetReturnValue().Set(String::NewFromUtf8(isolate, getPathConfigDirectory().c_str()));
}

void OBS_API::OBS_API_setPathConfigDirectory(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string pathConfigDirectory = std::string(*param1);	

	setPathConfigDirectory(pathConfigDirectory);
}

void OBS_API::OBS_API_getOBS_existingProfiles(const FunctionCallbackInfo<Value>& args)
{
	args.GetReturnValue().Set(getOBS_existingProfiles());
}

void OBS_API::OBS_API_getOBS_existingSceneCollections(const FunctionCallbackInfo<Value>& args)
{
	args.GetReturnValue().Set(getOBS_existingSceneCollections());
}

void OBS_API::OBS_API_getOBS_currentProfile(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	args.GetReturnValue().Set(String::NewFromUtf8(isolate, getOBS_currentProfile().c_str()));
}

void OBS_API::OBS_API_setOBS_currentProfile(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string currentProfile = std::string(*param1);	

	setOBS_currentProfile(currentProfile);	
}

void OBS_API::OBS_API_getOBS_currentSceneCollection(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	args.GetReturnValue().Set(String::NewFromUtf8(isolate, getOBS_currentSceneCollection().c_str()));
}

void OBS_API::OBS_API_setOBS_currentSceneCollection(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string currentSceneCollection = std::string(*param1);	

	setOBS_currentSceneCollection(currentSceneCollection);	
}

void OBS_API::OBS_API_isOBS_installed(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	args.GetReturnValue().Set(Integer::New(isolate, isOBS_installed()));
}

void OBS_API::OBS_API_useOBS_config(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	useOBS_configFiles = true;

	String::Utf8Value currentProfile(args[0]);
	String::Utf8Value currentSceneCollection(args[1]);

	setOBS_currentProfile(*currentProfile);
	setOBS_currentSceneCollection(*currentSceneCollection);

	pathConfigDirectory = OBS_pathConfigDirectory;
}

void OBS_API::OBS_API_test_openAllModules(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	openAllModules();

	string result = "SUCCESS";

	for (int i = 0; i < listModules.size(); i++) {
		if (listModules.at(i).second != MODULE_SUCCESS) {
			result = "FAILURE";
		}
	}

	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_API::OBS_API_test_initAllModules(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	string result;
	if (initAllModules() == 0) {
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

int GetConfigPath(char *path, size_t size, const char *name)
{
	return os_get_config_path(path, size, name);
}

bool dirExists(const std::string& path)
{
  DWORD ftyp = GetFileAttributesA(path.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;   

  return false;
}

bool containsDirectory(const std::string& path)
{
	const char* pszDir = path.c_str();
    char szBuffer[MAX_PATH];

    DWORD dwRet = GetCurrentDirectory(MAX_PATH, szBuffer);
    SetCurrentDirectory(pszDir);

    WIN32_FIND_DATA fd;

    HANDLE hFind = ::FindFirstFile("*.", &fd);

    // Get all sub-folders:

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            char* pszName = fd.cFileName;
            if (_stricmp(pszName, ".") != 0 && _stricmp(pszName, "..") != 0)
            {
				//Only look for at least one directory
				::FindClose(hFind);
				SetCurrentDirectory(szBuffer);
				return true;
            }

        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
    // Set the current folder back to what it was:
    SetCurrentDirectory(szBuffer);
    return false;
}

void OBS_API::initAPI(void)
{
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
    std::string basicConfigFile = OBS_API::getBasicConfigPath();
    config_t* config = OBS_API::openConfigFile(basicConfigFile);

    const char *device_name = config_get_string(config, "Audio",
            "MonitoringDeviceName");
    const char *device_id = config_get_string(config, "Audio",
            "MonitoringDeviceId");

    obs_set_audio_monitoring_device(device_name, device_id);

    blog(LOG_INFO, "Audio monitoring device:\n\tname: %s\n\tid: %s",
			device_name, device_id);

	bool disableAudioDucking = config_get_bool(config, "Audio",
			"DisableAudioDucking");
	if (disableAudioDucking)
		DisableAudioDucking(true);
#endif
}

bool OBS_API::initOBS_API()
{
	std::vector<char> userData = std::vector<char>(1024);
	os_get_config_path(userData.data(), userData.capacity() - 1, "slobs-client/plugin_config");
	obs_startup("en-US", userData.data(), NULL);
    cpuUsageInfo = os_cpu_usage_info_start();

	//Setting obs-studio config directory
	char path[512];
	int ret = GetConfigPath(path, 512, "obs-studio");

	if(ret > 0) {
		OBS_pathConfigDirectory = path;
	}

	std::string profiles = OBS_pathConfigDirectory + "\\basic\\profiles";
	std::string scenes = OBS_pathConfigDirectory + "\\basic\\scenes";

	isOBS_installedValue = dirExists(OBS_pathConfigDirectory) &&
		containsDirectory(profiles) &&
		os_file_exists(scenes.c_str());

	if(isOBS_installedValue) {
	    v8::String::Utf8Value firstProfile(getOBS_existingProfiles()->Get(0)->ToString());
	    OBS_currentProfile = std::string(*firstProfile);

	    v8::String::Utf8Value firstSceneCollection(getOBS_existingSceneCollections()->Get(0)->ToString());
	    OBS_currentSceneCollection = std::string(*firstSceneCollection);
	}

	/* Logging */
	string filename = GenerateTimeDateFilename("txt");
	string log_path = appdata_path;
	log_path.append("/logs/");

	/* Make sure the path is created
	   before attempting to make a file there. */
	if (os_mkdirs(log_path.c_str()) == MKDIR_ERROR) {
		cerr << "Failed to open log file" << endl;
	}

	delete_oldest_file(log_path.c_str(), 3);
	log_path.append(filename);

	/* Leak although not that big of a deal since it should always be open. */
	fstream *logfile = new fstream(log_path, ios_base::out | ios_base::trunc);

	if (!logfile) {
		cerr << "Failed to open log file" << endl;
	}

	/* Delete oldest file in the folder to imitate rotating */
	base_set_log_handler(node_obs_log, logfile);

	/* Profiling */
	//profiler_start();



	return obs_initialized();
}

static void SaveProfilerData(const profiler_snapshot_t *snap)
{
	string dst(appdata_path);
	dst.append("profiler_data/");

	if (os_mkdirs(dst.c_str()) == MKDIR_ERROR) {
		cerr << "Failed to open profiler snapshot for writing" << endl;
	}

	dst.append(GenerateTimeDateFilename("csv.gz"));

	if (!profiler_snapshot_dump_csv_gz(snap, dst.c_str()))
		blog(LOG_WARNING, "Could not save profiler data to '%s'",
				dst.c_str());
}

void OBS_API::destroyOBS_API(void) {
    os_cpu_usage_info_destroy(cpuUsageInfo);

#ifdef _WIN32
	std::string basicConfigFile = OBS_API::getBasicConfigPath();
	config_t* config = OBS_API::openConfigFile(basicConfigFile);

	bool disableAudioDucking = config_get_bool(config, "Audio",
			"DisableAudioDucking");
	if (disableAudioDucking)
		DisableAudioDucking(false);
#endif

	obs_encoder_t* streamingEncoder = OBS_service::getStreamingEncoder();
	if(streamingEncoder != NULL)
		obs_encoder_release(streamingEncoder);

	obs_encoder_t* recordingEncoder = OBS_service::getRecordingEncoder();
	if(recordingEncoder != NULL)
		obs_encoder_release(recordingEncoder);

	obs_encoder_t* audioEncoder = OBS_service::getAudioEncoder();
	if(audioEncoder != NULL)
		obs_encoder_release(audioEncoder);

	obs_output_t* streamingOutput = OBS_service::getStreamingOutput();
	if(streamingOutput != NULL)
		obs_output_release(streamingOutput);

	obs_output_t* recordingOutput = OBS_service::getRecordingOutput();
	if(recordingOutput != NULL)
		obs_output_release(recordingOutput);

	obs_service_t* service = OBS_service::getService();
	if(service != NULL)
		obs_service_release(service);

    obs_shutdown();

    /*profiler_stop();
	auto snapshot = profile_snapshot_create();
	profiler_print(snapshot);
	profiler_print_time_between_calls(snapshot);
	SaveProfilerData(snapshot);
	profile_snapshot_free(snapshot);
	profiler_free();*/
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

vector<pair<obs_module_t *, int>>  OBS_API::openAllModules(void) {
	OBS_service::resetVideoContext(NULL);

	// Set up several directories used.
	std::string pathOBS = g_moduleDirectory;
	std::string pathOBSPlugins = pathOBS + "/obs-plugins";
	std::string pathOBSPluginData = pathOBS + "/data/obs-plugins";

	// Switch working directory to where node-obs is loaded from.
	_chdir(pathOBS.c_str());
	#ifdef _WIN32
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32);
	SetDllDirectory(NULL);
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wide = converter.from_bytes(pathOBS);
		AddDllDirectory(wide.c_str());
	}
	#endif

	if (os_file_exists(pathOBSPlugins.c_str())) { // Test if basePluginPath exists.
		os_dir_t* odPlugins = os_opendir(pathOBSPlugins.c_str());
		if (odPlugins) { // Check if we were able to open it
			//std::cout << "reading " << basePluginPath << std::endl;
			for (os_dirent* ent = os_readdir(odPlugins); ent != nullptr; ent = os_readdir(odPlugins)) {
				if (!ent->directory) { // Only list actual files
					std::string pathFile = ent->d_name;
					std::string pathFileName = pathFile.substr(0, pathFile.find_last_of('.'));
					std::string pathPlugin = pathOBSPlugins + "/" + pathFile;
					std::string pathPluginData = pathOBSPluginData + "/" + pathFileName;

					#ifdef _WIN32
					// Only try to load .dll files
					istring tempFile = pathFile.c_str();
					if (tempFile.length() >= 4) {
						if (tempFile.substr(tempFile.length() - 4, 4).compare(".dll") != 0) {
							continue;
						}
					} else { // Don't bother with files that have no name
						continue;
					}
					#else

					#endif

					obs_module_t *currentModule;
					int result = obs_open_module(&currentModule, pathPlugin.c_str(), pathPluginData.c_str());
					switch (result) {
						case MODULE_SUCCESS:
							listModules.push_back(make_pair(currentModule, result));
							break;
						case MODULE_FILE_NOT_FOUND:
							std::cerr << "Unable to load '" << pathPlugin << "', could not find file." << std::endl;
							break;
						case MODULE_MISSING_EXPORTS:
							std::cerr << "Unable to load '" << pathPlugin << "', missing exports." << std::endl;
							break;
						case MODULE_INCOMPATIBLE_VER:
							std::cerr << "Unable to load '" << pathPlugin << "', incompatible version." << std::endl;
							break;
						case MODULE_ERROR:
							std::cerr << "Unable to load '" << pathPlugin << "', generic error." << std::endl;
							break;
					}
					}
				}
			os_closedir(odPlugins);
			}
		}
	// Restore old working directory.
	_chdir(g_moduleDirectory.c_str());

	return listModules;
	}

int OBS_API::initAllModules(void) {
	int error = 0;
	for (int i = 0; i < listModules.size(); i++) {
		if (listModules.at(i).second == MODULE_SUCCESS) {
			if (!obs_init_module(listModules.at(i).first)) {
				error--;
			}
		}
	}
	getPerformanceStatistics();
	return error;
}

Local<Object> OBS_API::getPerformanceStatistics(void) 
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Object> statistics = Object::New(isolate);

	statistics->Set(String::NewFromUtf8(isolate, "CPU"), Number::New(isolate, getCPU_Percentage()));
	statistics->Set(String::NewFromUtf8(isolate, "numberDroppedFrames"), Number::New(isolate, getNumberOfDroppedFrames()));
	statistics->Set(String::NewFromUtf8(isolate, "percentageDroppedFrames"), Number::New(isolate, getDroppedFramesPercentage()));
	statistics->Set(String::NewFromUtf8(isolate, "bandwidth"), Number::New(isolate, getCurrentBandwidth()));
	statistics->Set(String::NewFromUtf8(isolate, "frameRate"), Number::New(isolate, getCurrentFrameRate()));

	return statistics;
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

	if(obs_output_active(streamOutput))
	{
		totalDropped = obs_output_get_frames_dropped(streamOutput);
	}

	return totalDropped;
}

double OBS_API::getDroppedFramesPercentage(void)
{
	obs_output_t* streamOutput = OBS_service::getStreamingOutput();
	
	double percent = 0;

	if(obs_output_active(streamOutput))
	{
		int totalDropped = obs_output_get_frames_dropped(streamOutput);
		int totalFrames  = obs_output_get_total_frames(streamOutput);
		percent   = (double)totalDropped / (double)totalFrames * 100.0;
	}

	return percent;
}

double OBS_API::getCurrentBandwidth(void)
{
	obs_output_t* streamOutput = OBS_service::getStreamingOutput();

	double kbitsPerSec = 0;

	if(obs_output_active(streamOutput))
	{
		uint64_t bytesSent     = obs_output_get_total_bytes(streamOutput);
		uint64_t bytesSentTime = os_gettime_ns();

		if (bytesSent < lastBytesSent)
			bytesSent = 0;
		if (bytesSent == 0)
			lastBytesSent = 0;

		uint64_t bitsBetween   = (bytesSent - lastBytesSent) * 8;

		double timePassed = double(bytesSentTime - lastBytesSentTime) /
			1000000000.0;

		kbitsPerSec = double(bitsBetween) / timePassed / 1000.0;

		lastBytesSent        = bytesSent;
		lastBytesSentTime    = bytesSentTime;
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
	if(!newPathConfigDirectory.empty() && !useOBS_configFiles)
	{
		pathConfigDirectory = newPathConfigDirectory;
	} 
}

std::vector<std::string> exploreDirectory(std::string directory, std::string typeToReturn) 
{
	std::vector<std::string> listElements;

    char originalDirectory[_MAX_PATH];

    // Get the current directory so we can return to it
    _getcwd(originalDirectory, _MAX_PATH);

    _chdir(directory.c_str());  // Change to the working directory
    _finddata_t fileinfo;

    // This will grab the first file in the directory
    // "*" can be changed if you only want to look for specific files
    intptr_t handle = _findfirst("*", &fileinfo);

    if(handle == -1)  // No files or directories found
    {
        perror("Error searching for file");
        exit(1);
    }

    do
    {
        if(strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
            continue;
        if(fileinfo.attrib & _A_SUBDIR && typeToReturn.compare("directories") == 0) {
        	listElements.push_back(fileinfo.name);
        } else if (typeToReturn.compare("files") == 0) {
        	listElements.push_back(fileinfo.name);
        }
    } while(_findnext(handle, &fileinfo) == 0);

    _findclose(handle); // Close the stream

    _chdir(originalDirectory);

	return listElements;
}

Local<Array> OBS_API::getOBS_existingProfiles(void)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> listExistingProfiles = Array::New(isolate);

	std::string pathProfiles;
	pathProfiles += OBS_pathConfigDirectory;
	pathProfiles += "\\basic\\profiles\\";

	std::vector<std::string> vectorExistingProfiles = exploreDirectory(pathProfiles, "directories");

    for(int i=0;i<vectorExistingProfiles.size();i++) {
    	listExistingProfiles->Set(i, String::NewFromUtf8(isolate, vectorExistingProfiles.at(i).c_str()));
    }

	return listExistingProfiles;
}

Local<Array> OBS_API::getOBS_existingSceneCollections(void)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Array> listExistingSceneCollections = Array::New(isolate);

	std::string pathSceneCollections;
	pathSceneCollections += OBS_pathConfigDirectory;
	pathSceneCollections += "\\basic\\scenes\\";

	std::vector<std::string> files = exploreDirectory(pathSceneCollections, "files");

	int indexArray = 0;

	for(int i=0;i<files.size();i++) {
		if(files.at(i).substr(files.at(i).find_last_of(".") + 1) == "json") {
			files.at(i).erase(files.at(i).end()-5, files.at(i).end());
			listExistingSceneCollections->Set(indexArray++, String::NewFromUtf8(isolate, files.at(i).c_str()));
		} 
	}

	return listExistingSceneCollections;
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
        screen.height = std::abs(info.rcMonitor.top  - info.rcMonitor.bottom);

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

	if(useOBS_configFiles && !OBS_currentProfile.empty()) {
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

	if(useOBS_configFiles && !OBS_currentProfile.empty()) {
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

	if(useOBS_configFiles && !OBS_currentSceneCollection.empty()) {
		contentConfigPath += "\\basic\\scenes\\";
		contentConfigPath += OBS_currentSceneCollection;;
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

	SHGetFolderPathW(NULL, CSIDL_MYVIDEO, NULL, SHGFP_TYPE_CURRENT,
		path_utf16);

	os_wcs_to_utf8(path_utf16, wcslen(path_utf16), path_utf8, MAX_PATH);
	return string(path_utf8);
}

static const double scaled_vals[] =
{
	1.0,
	1.25,
	(1.0/0.75),
	1.5,
	(1.0/0.6),
	1.75,
	2.0,
	2.25,
	2.5,
	2.75,
	3.0,
	0.0
};

std::vector<std::pair<std::string, config_t*>> configFiles;

config_t* OBS_API::openConfigFile(std::string configFile)
{

	std::vector<std::pair<std::string, config_t*>>::iterator it =
		std::find_if(configFiles.begin(), configFiles.end(),
			[&configFile] (const pair<std::string, config_t*> value)
			{
				return (value.first.compare(configFile) == 0);
			});

	// if(it == configFiles.end()) {
		config_t* config;

		int result = config_open(&config, configFile.c_str(), CONFIG_OPEN_EXISTING);

		if(result != CONFIG_SUCCESS) {
			config = config_create(configFile.c_str());
			config_open(&config, configFile.c_str(), CONFIG_OPEN_EXISTING);
		}

		std::string basic = "basic.ini";
		std::string subString = configFile.substr(configFile.size() - basic.size(), basic.size()).c_str();

		if (subString.compare(basic) == 0) {
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
			config_set_default_uint  (config, "SimpleOutput", "VBitrate",
					2500);
			config_set_default_string(config, "SimpleOutput", "StreamEncoder",
					SIMPLE_ENCODER_X264);
			config_set_default_uint  (config, "SimpleOutput", "ABitrate", 160);
			config_set_default_bool  (config, "SimpleOutput", "UseAdvanced",
					false);
			config_set_default_bool  (config, "SimpleOutput", "EnforceBitrate",
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

			config_set_default_bool  (config, "AdvOut", "ApplyServiceSettings",
					true);
			config_set_default_bool  (config, "AdvOut", "UseRescale", false);
			config_set_default_uint  (config, "AdvOut", "TrackIndex", 1);
			config_set_default_string(config, "AdvOut", "Encoder", "obs_x264");

			config_set_default_string(config, "AdvOut", "RecType", "Standard");

			config_set_default_string(config, "AdvOut", "RecFilePath",
					GetDefaultVideoSavePath().c_str());
			config_set_default_string(config, "AdvOut", "RecFormat", "flv");
			config_set_default_bool  (config, "AdvOut", "RecUseRescale",
					false);
			config_set_default_uint  (config, "AdvOut", "RecTracks", (1<<0));
			config_set_default_string(config, "AdvOut", "RecEncoder",
					"none");

			config_set_default_bool  (config, "AdvOut", "FFOutputToFile",
					true);
			config_set_default_string(config, "AdvOut", "FFFilePath",
					GetDefaultVideoSavePath().c_str());
			config_set_default_string(config, "AdvOut", "FFExtension", "mp4");
			config_set_default_uint  (config, "AdvOut", "FFVBitrate", 2500);
			config_set_default_uint  (config, "AdvOut", "FFVGOPSize", 250);
			config_set_default_bool  (config, "AdvOut", "FFUseRescale",
					false);
			config_set_default_bool  (config, "AdvOut", "FFIgnoreCompat",
					false);
			config_set_default_uint  (config, "AdvOut", "FFABitrate", 160);
			config_set_default_uint  (config, "AdvOut", "FFAudioTrack", 1);

			config_set_default_uint  (config, "AdvOut", "Track1Bitrate", 160);
			config_set_default_uint  (config, "AdvOut", "Track2Bitrate", 160);
			config_set_default_uint  (config, "AdvOut", "Track3Bitrate", 160);
			config_set_default_uint  (config, "AdvOut", "Track4Bitrate", 160);
			config_set_default_uint  (config, "AdvOut", "Track5Bitrate", 160);
			config_set_default_uint  (config, "AdvOut", "Track6Bitrate", 160);

			config_set_default_uint  (config, "Video", "BaseCX",   cx);
			config_set_default_uint  (config, "Video", "BaseCY",   cy);

			/* don't allow BaseCX/BaseCY to be susceptible to defaults changing */
			if (!config_has_user_value(config, "Video", "BaseCX") ||
				!config_has_user_value(config, "Video", "BaseCY")) {
				config_set_uint(config, "Video", "BaseCX", cx);
				config_set_uint(config, "Video", "BaseCY", cy);
				config_save_safe(config, "tmp", nullptr);
			}

			config_set_default_string(config, "Output", "FilenameFormatting",
					"%CCYY-%MM-%DD %hh-%mm-%ss");

			config_set_default_bool  (config, "Output", "DelayEnable", false);
			config_set_default_uint  (config, "Output", "DelaySec", 20);
			config_set_default_bool  (config, "Output", "DelayPreserve", true);

			config_set_default_bool  (config, "Output", "Reconnect", true);
			config_set_default_uint  (config, "Output", "RetryDelay", 10);
			config_set_default_uint  (config, "Output", "MaxRetries", 20);

			config_set_default_string(config, "Output", "BindIP", "default");
			config_set_default_bool  (config, "Output", "NewSocketLoopEnable",
					false);
			config_set_default_bool  (config, "Output", "LowLatencyEnable",
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

			config_set_default_uint  (config, "Video", "OutputCX", scale_cx);
			config_set_default_uint  (config, "Video", "OutputCY", scale_cy);

			/* don't allow OutputCX/OutputCY to be susceptible to defaults
				* changing */
			if (!config_has_user_value(config, "Video", "OutputCX") ||
				!config_has_user_value(config, "Video", "OutputCY")) {
				config_set_uint(config, "Video", "OutputCX", scale_cx);
				config_set_uint(config, "Video", "OutputCY", scale_cy);
				config_save_safe(config, "tmp", nullptr);
			}

			config_set_default_uint  (config, "Video", "FPSType", 0);
			config_set_default_string(config, "Video", "FPSCommon", "30");
			config_set_default_uint  (config, "Video", "FPSInt", 30);
			config_set_default_uint  (config, "Video", "FPSNum", 30);
			config_set_default_uint  (config, "Video", "FPSDen", 1);
			config_set_default_string(config, "Video", "ScaleType", "bicubic");
			config_set_default_string(config, "Video", "ColorFormat", "NV12");
			config_set_default_string(config, "Video", "ColorSpace", "601");
			config_set_default_string(config, "Video", "ColorRange",
					"Partial");

			config_set_default_string(config, "Audio", "MonitoringDeviceId",
					"default");
			config_set_default_string(config, "Audio", "MonitoringDeviceName",
					"Default");
			config_set_default_uint  (config, "Audio", "SampleRate", 44100);
			config_set_default_string(config, "Audio", "ChannelSetup",
					"Stereo");

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