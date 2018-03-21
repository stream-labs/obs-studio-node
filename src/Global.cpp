#include "spdlog/spdlog.h"
#include "spdlog/sinks/ansicolor_sink.h"

#include "obspp/obspp.hpp"
#include "obspp/obspp-module.hpp" /* For libobs data path */
#include <util/base.h>
#include <util/platform.h>

#include "Global.h"
#include "Common.h"

#include "Input.h"
#include "Scene.h"
#include "Transition.h"
#include "Properties.h"

namespace osn {

std::shared_ptr<spdlog::logger> logger;

static void custom_log_handler(int level, const char *format, 
	                           va_list args, void *data)
{
	char out[4096];
	vsnprintf(out, sizeof(out), format, args);

	switch (level) {
	case LOG_ERROR:
		logger->error("{0}", out);
		break;
	case LOG_WARNING:
		logger->warn("{0}", out);
		break;
	case LOG_INFO:
		logger->info("{0}", out);
		break;
	case LOG_DEBUG:
		logger->debug("{0}", out);
		break;
	default:
		logger->info("{0}", out);
	}
}

static void custom_crash_handler(const char *msg, va_list args, void *param)
{
	/* Frontend handles crash catching right now. So just
	 * immediately crash in a way the crash report will catch. */
	*(char *)0 = 0;

	/* The above will normally cause a crash.
	 * If it doesn't, we manually raise the segfault signal. */
	raise(SIGSEGV);

	/* If we get to this point... 
	 * the OS is doing something really dumb and insecure. */
}

#define OBS_VALID \
    if (!obs::initialized()) \
        return;

NAN_MODULE_INIT(Init)
{
    auto ObsGlobal = Nan::New<v8::Object>();

    common::SetObjectField(ObsGlobal, "startup", startup);
    common::SetObjectField(ObsGlobal, "shutdown", shutdown);
    common::SetObjectField(ObsGlobal, "getOutputSource", getOutputSource);
    common::SetObjectField(ObsGlobal, "setOutputSource", setOutputSource);
    common::SetObjectField(ObsGlobal, "getOutputFlagsFromId", getOutputFlagsFromId);
    common::SetObjectField(ObsGlobal, "getProperties", getProperties);
    common::SetObjectField(ObsGlobal, "getActiveFps", getActiveFps);
    common::SetObjectField(ObsGlobal, "getAudioMonitoringDevices", getAudioMonitoringDevices);
    common::SetObjectField(ObsGlobal, "setAudioMonitoringDevice", setAudioMonitoringDevice);
    common::SetObjectField(ObsGlobal, "getAudioMonitoringDevice", getAudioMonitoringDevice);
    common::SetObjectLazyAccessor(ObsGlobal, "laggedFrames", laggedFrames);
    common::SetObjectLazyAccessor(ObsGlobal, "totalFrames", totalFrames);
    common::SetObjectLazyAccessor(ObsGlobal, "initialized", initialized);
    common::SetObjectLazyAccessor(ObsGlobal, "locale", get_locale, set_locale);

    Nan::Set(target, FIELD_NAME("Global"), ObsGlobal);
}

NAN_METHOD(startup)
{
	/* Map base DLLs as soon as possible into the current process space.
	 * In particular, we need to load obs.dll into memory before we call
	 * any functions from obs else if we delay-loaded the dll, it will
	 * fail miserably. */

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

	std::string locale;
	std::string libobs_path;
	std::string libobs_data_path;

	ASSERT_INFO_LENGTH_AT_LEAST(info, 1);
	ASSERT_GET_VALUE(info[0], locale);

	if (info.Length() > 1) {
		const char data_path_suffix[] = "/data";

		ASSERT_GET_VALUE(info[1], libobs_path);

		libobs_data_path.reserve(
			libobs_path.size() +
			(sizeof(data_path_suffix) / sizeof(char)));

		libobs_data_path.append(libobs_path);
		libobs_data_path.append(data_path_suffix);
	}

	/* Note that we shouldn't use obs functions here yet
	 * as obs.dll is possibly not loaded yet. */
	for (int i = 0; i < g_modules_size; ++i) {
		std::string module_path;
		void *handle = NULL;
		
		/* libobs_path should be an absolute path. 
		 * If it's not provided, assume that it can
		 * be found in dll search path. */

		if (!libobs_path.empty()) {
			const char rel_bin_path[] = "/bin/64bit/";

			module_path.reserve(
				libobs_path.size() + 
				(sizeof(rel_bin_path) / sizeof(char)) +
				(sizeof(g_modules[i]) / sizeof(char)));

			module_path.append(libobs_path);
			module_path.append(rel_bin_path);
		} else {
			module_path.reserve(sizeof(g_modules[i]) / sizeof(char));
		}

		module_path.append(g_modules[i]);

		#ifdef _WIN32
			handle = LoadLibrary(module_path.c_str());
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

	/* At this point, we can use OBS functions. If delayload failed
	 * we will never reach this point. 
	 * POSSIBLE TODO - Customize the delayload function to give more
	 * information on when it fails. */

	/* We are assuming here the slobs-client folder is already */

	char *log_path  = os_get_config_path_ptr("slobs-client/libobs/logs");
	char *log_file_base_path = 
		os_get_config_path_ptr("slobs-client/libobs/logs/log.txt");
	char *plugin_config_path = 
		os_get_config_path_ptr("slobs-client/libobs/plugin-config");

	std::array<spdlog::sink_ptr, 2> sinks;
	int status = 0;

	/* Windows uses _wenviron which ends up causing confusing
	 * issues where ANSI and Wide Character env maps get
	 * desynchronized. Even more confusing is compiling in debug
	 * and release showcase this issue. As a result, we use
	 * SetEnvironmentVariable which will always use the wide
	 * version of the function internally. For POSIX, there
	 * is no wide-character version of set/getenv */

	/* libobs will use three methods of finding data files:
	 * 1. ${CWD}/data/libobs <- This doesn't work for us
	 * 2. ${OBS_DATA_PATH}/libobs <- This works but is inflexible
	 * 3. getenv(OBS_DATA_PATH) + /libobs <- Can be set anywhere
	 *    on the cli, in the frontend, or the backend. */

	/* Don't overwrite the environment variable if nothing is passed.
	 * This allows the user to pass an environment variable instead
	 * passing it directly to the API */
	if (!libobs_data_path.empty()) {
		#ifdef _WIN32
			SetEnvironmentVariable("OBS_DATA_PATH", libobs_data_path.c_str());
		#else
			setenv("OBS_DATA_PATH", libobs_data_path.c_str());
		#endif
	}

	/* Make sure our config folders exist */
	status = os_mkdirs(log_path);
	status = status == MKDIR_ERROR ? status : os_mkdirs(plugin_config_path);

	if (status == MKDIR_ERROR) {
		Nan::ThrowError("Failed to create config directories!");
		return;
	}

	/* Even though we're single-threaded, we still need multi-threaded 
	 * loggers since libobs itself can log from multiple threads. */
	sinks[0] = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
	sinks[1] = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
		log_file_base_path, 1024 * 1024 * 2 /* 1 MB */, 20);

	/* This will throw an exception on failure. I'm alright with that */
	logger = spdlog::create("libobs", sinks.begin(), sinks.end());

	/* If the previous log resulted from a crash, it won't have an new line */
	logger->info("\n");
	logger->info("***************************************************************************");
	logger->info("* Starting up libobs... ");
	logger->info("***************************************************************************");

	base_set_log_handler(custom_log_handler, 0);
	base_set_crash_handler(custom_crash_handler, 0);

	status = obs_startup(locale.c_str(), plugin_config_path, 0);

	if (!status) {
		Nan::ThrowError("Failed to start obs!");
		return;
	}

	/* Note that these must be delayed until
	 * after we call obs. obs needs to be initialized
	 * in order to add modules paths */
	if (!libobs_data_path.empty()) {
		std::string plugin_bin_path(libobs_path);
		std::string plugin_data_path(libobs_path);

		plugin_bin_path.append("/obs-plugins");
		plugin_data_path.append("/data/obs-plugins/%module%");

		obs::module::add_path(plugin_bin_path, plugin_data_path);

		/* The top-most directory was originally used to hold binaries.
		 * We moved to one folder deeper since that's what obs does.
		 * So we now support both for compatibility reasons. */
		plugin_bin_path.append("/64bit");
		obs::module::add_path(plugin_bin_path, plugin_data_path);
	}

	bfree(log_path);
	bfree(log_file_base_path);
	bfree(plugin_config_path);
}

NAN_METHOD(shutdown)
{
	logger->info("***************************************************************************");
	logger->info("* Shutting libobs down... ");
	logger->info("***************************************************************************");
	obs::shutdown();
}

NAN_METHOD(laggedFrames)
{
    OBS_VALID

    info.GetReturnValue().Set(common::ToValue(obs::lagged_frames()));
}

NAN_METHOD(totalFrames)
{
    OBS_VALID

    info.GetReturnValue().Set(common::ToValue(obs::total_frames()));
}

NAN_METHOD(get_locale)
{
    OBS_VALID

    info.GetReturnValue().Set(common::ToValue(obs::locale()));
}

NAN_METHOD(set_locale)
{
    OBS_VALID

    std::string locale;

    ASSERT_GET_VALUE(info[0], locale);

    obs::locale(locale);
}

NAN_METHOD(initialized)
{
    info.GetReturnValue().Set(common::ToValue(obs::initialized()));
}

NAN_METHOD(version)
{
    info.GetReturnValue().Set(common::ToValue(obs::version()));
}

NAN_METHOD(getOutputFlagsFromId)
{
    std::string id;

    ASSERT_GET_VALUE(info[0], id);

    uint32_t flags = obs::output_flags_from_id(id.c_str());

    info.GetReturnValue().Set(flags);
}

NAN_METHOD(setOutputSource)
{
    uint32_t channel;
    v8::Local<v8::Object> source_object;

    ASSERT_INFO_LENGTH(info, 2);
    ASSERT_GET_VALUE(info[0], channel);
    
    if (info[1]->IsNull()) {
        obs::output_source(channel, obs::source(nullptr));
        return;
    }
    
    ASSERT_GET_VALUE(info[1], source_object);

    obs::source source = ISource::GetHandle(source_object);
    obs::output_source(channel, source);
}

NAN_METHOD(getOutputSource)
{
    ASSERT_INFO_LENGTH(info, 1);

    uint32_t channel;

    ASSERT_GET_VALUE(info[0], channel);

    obs::source source = obs::output_source(channel);

    if (source.type() == OBS_SOURCE_TYPE_INPUT) {
        Input *binding = new Input(source.dangerous());

        v8::Local<v8::Object> object = 
            Input::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }
    else if (source.type() == OBS_SOURCE_TYPE_SCENE) {
        Scene *binding = new Scene(source.dangerous());

        v8::Local<v8::Object> object = 
            Scene::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }
    else if (source.type() == OBS_SOURCE_TYPE_TRANSITION) {
        Transition *binding = new Transition(source.dangerous());

        v8::Local<v8::Object> object = 
            Transition::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }

    obs_source_release(source.dangerous());
}

NAN_METHOD(getProperties)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 1);

    std::string id;
    uint32_t type;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], type);

    obs::properties props(id.c_str(), (obs::properties::object_type)type);

    if (props.status() != obs::properties::status_type::okay) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Properties *bindings = new Properties(std::move(props));
    auto object = Properties::Object::GenerateObject(bindings);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(getActiveFps)
{
    info.GetReturnValue().Set(common::ToValue(obs_get_active_fps()));
}

NAN_METHOD(getAudioMonitoringDevices)
{
    auto monitoring_devices = obs::monitoring_devices();

    int count = static_cast<int>(monitoring_devices.size());
    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        v8::Local<v8::Object> element = Nan::New<v8::Object>();

        common::SetObjectField(element, "name", monitoring_devices[i].first);
        common::SetObjectField(element, "id", monitoring_devices[i].second);

        Nan::Set(array, i, element);
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(setAudioMonitoringDevice)
{
    std::string name, id;

    ASSERT_INFO_LENGTH_AT_LEAST(info, 2);
    ASSERT_GET_VALUE(info[0], name);
    ASSERT_GET_VALUE(info[1], id);

    obs::monitoring_device(name, id);
}

NAN_METHOD(getAudioMonitoringDevice)
{
    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    auto device = obs::monitoring_device();

    common::SetObjectField(result, "name", device.first);
    common::SetObjectField(result, "id", device.second);

    info.GetReturnValue().Set(result);
}

}