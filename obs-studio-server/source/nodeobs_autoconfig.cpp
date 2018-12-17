#include "nodeobs_autoconfig.h"
#include "error.hpp"
#include "shared.hpp"

enum class Type
{
	Invalid,
	Streaming,
	Recording
};

enum class Service
{
	Twitch,
	Hitbox,
	Beam,
	Other
};

enum class Encoder
{
	x264,
	NVENC,
	QSV,
	AMD,
	Stream
};

enum class Quality
{
	Stream,
	High
};

enum class FPSType : int
{
	PreferHighFPS,
	PreferHighRes,
	UseCurrent,
	fps30,
	fps60
};

struct Event
{
	// obs::CallbackInfo *cb_info;

	std::string event;
	std::string description;
	int         percentage;
};

class AutoConfigInfo
{
	public:
	AutoConfigInfo(std::string a_event, std::string a_description, double a_percentage)
	{
		event       = a_event;
		description = a_description;
		percentage  = a_percentage;
	};
	~AutoConfigInfo(){};

	std::string event;
	std::string description;
	double      percentage;
};

std::mutex                 eventsMutex;
std::queue<AutoConfigInfo> events;

Service     serviceSelected   = Service::Other;
Quality     recordingQuality  = Quality::Stream;
Encoder     recordingEncoder  = Encoder::Stream;
Encoder     streamingEncoder  = Encoder::x264;
Type        type              = Type::Streaming;
FPSType     fpsType           = FPSType::PreferHighFPS;
uint64_t    idealBitrate      = 2500;
uint64_t    baseResolutionCX  = 1920;
uint64_t    baseResolutionCY  = 1080;
uint64_t    idealResolutionCX = 1280;
uint64_t    idealResolutionCY = 720;
int         idealFPSNum       = 60;
int         idealFPSDen       = 1;
std::string serviceName;
std::string serverName;
std::string server;
std::string key;

bool hardwareEncodingAvailable = false;
bool nvencAvailable            = false;
bool qsvAvailable              = false;
bool vceAvailable              = false;

int  startingBitrate = 2500;
bool customServer    = false;
bool bandwidthTest   = true;
bool testRegions     = true;

bool regionNA = false;
bool regionSA = false;
bool regionEU = false;
bool regionAS = false;
bool regionOC = false;

bool preferHighFPS  = true;
bool preferHardware = true;
int  specificFPSNum = 0;
int  specificFPSDen = 0;

std::thread             testThread;
std::condition_variable cv;
std::mutex              m;
bool                    cancel  = false;
bool                    started = false;

bool softwareTested = false;

struct ServerInfo
{
	std::string name;
	std::string address;
	int         bitrate = 0;
	int         ms      = -1;

	inline ServerInfo() {}

	inline ServerInfo(const char* name_, const char* address_) : name(name_), address(address_) {}
};

class TestMode
{
	obs_video_info ovi;
	OBSSource      source[6];

	static void render_rand(void*, uint32_t cx, uint32_t cy)
	{
		gs_effect_t* solid         = obs_get_base_effect(OBS_EFFECT_SOLID);
		gs_eparam_t* randomvals[3] = {gs_effect_get_param_by_name(solid, "randomvals1"),
		                              gs_effect_get_param_by_name(solid, "randomvals2"),
		                              gs_effect_get_param_by_name(solid, "randomvals3")};

		struct vec4 r;

		for (int i = 0; i < 3; i++) {
			vec4_set(
			    &r, rand_float(true) * 100.0f, rand_float(true) * 100.0f, rand_float(true) * 50000.0f + 10000.0f, 0.0f);
			gs_effect_set_vec4(randomvals[i], &r);
		}

		while (gs_effect_loop(solid, "Random"))
			gs_draw_sprite(nullptr, 0, cx, cy);
	}

	public:
	inline TestMode()
	{
		obs_get_video_info(&ovi);
		obs_add_main_render_callback(render_rand, this);

		for (uint32_t i = 0; i < 6; i++) {
			source[i] = obs_get_output_source(i);
			obs_source_release(source[i]);
			obs_set_output_source(i, nullptr);
		}
	}

	inline ~TestMode()
	{
		for (uint32_t i = 0; i < 6; i++)
			obs_set_output_source(i, source[i]);

		obs_remove_main_render_callback(render_rand, this);
		obs_reset_video(&ovi);
	}

	inline void SetVideo(int cx, int cy, int fps_num, int fps_den)
	{
		obs_video_info newOVI = ovi;

		newOVI.output_width  = (uint32_t)cx;
		newOVI.output_height = (uint32_t)cy;
		newOVI.fps_num       = (uint32_t)fps_num;
		newOVI.fps_den       = (uint32_t)fps_den;

		obs_reset_video(&newOVI);
	}
};

void autoConfig::Register(ipc::server& srv)
{
	std::shared_ptr<ipc::collection> cls = std::make_shared<ipc::collection>("AutoConfig");

	cls->register_function(std::make_shared<ipc::function>(
	    "InitializeAutoConfig",
	    std::vector<ipc::type>{ipc::type::String, ipc::type::String},
	    autoConfig::InitializeAutoConfig));
	cls->register_function(std::make_shared<ipc::function>(
	    "StartBandwidthTest", std::vector<ipc::type>{}, autoConfig::StartBandwidthTest));
	cls->register_function(std::make_shared<ipc::function>(
	    "StartStreamEncoderTest", std::vector<ipc::type>{}, autoConfig::StartStreamEncoderTest));
	cls->register_function(std::make_shared<ipc::function>(
	    "StartRecordingEncoderTest", std::vector<ipc::type>{}, autoConfig::StartRecordingEncoderTest));
	cls->register_function(std::make_shared<ipc::function>(
	    "StartCheckSettings", std::vector<ipc::type>{}, autoConfig::StartCheckSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "StartSetDefaultSettings", std::vector<ipc::type>{}, autoConfig::StartSetDefaultSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "StartSaveStreamSettings", std::vector<ipc::type>{}, autoConfig::StartSaveStreamSettings));
	cls->register_function(
	    std::make_shared<ipc::function>("StartSaveSettings", std::vector<ipc::type>{}, autoConfig::StartSaveSettings));
	cls->register_function(std::make_shared<ipc::function>(
	    "TerminateAutoConfig", std::vector<ipc::type>{}, autoConfig::TerminateAutoConfig));
	cls->register_function(std::make_shared<ipc::function>("Query", std::vector<ipc::type>{}, autoConfig::Query));

	srv.register_collection(cls);
}

void autoConfig::TestHardwareEncoding(void)
{
	size_t      idx = 0;
	const char* id;
	while (obs_enum_encoder_types(idx++, &id)) {
		if (strcmp(id, "ffmpeg_nvenc") == 0)
			hardwareEncodingAvailable = nvencAvailable = true;
		else if (strcmp(id, "obs_qsv11") == 0)
			hardwareEncodingAvailable = qsvAvailable = true;
		else if (strcmp(id, "amd_amf_h264") == 0)
			hardwareEncodingAvailable = vceAvailable = true;
	}
}

static inline void string_depad_key(std::string& key)
{
	while (!key.empty()) {
		char ch = key.back();
		if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
			key.pop_back();
		else
			break;
	}
}

bool autoConfig::CanTestServer(const char* server)
{
	if (!testRegions || (regionNA && regionSA && regionEU && regionAS && regionOC))
		return true;

	if (serviceSelected == Service::Twitch) {
		if (astrcmp_n(server, "NA:", 3) == 0 || astrcmp_n(server, "US West:", 8) == 0
		    || astrcmp_n(server, "US East:", 8) == 0 || astrcmp_n(server, "US Central:", 11) == 0) {
			return regionNA;
		} else if (astrcmp_n(server, "South America:", 14) == 0) {
			return regionSA;
		} else if (astrcmp_n(server, "EU:", 3) == 0) {
			return regionEU;
		} else if (astrcmp_n(server, "Asia:", 5) == 0) {
			return regionAS;
		} else if (astrcmp_n(server, "Australia:", 10) == 0) {
			return regionOC;
		} else {
			return true;
		}
	} else if (serviceSelected == Service::Hitbox) {
		if (strcmp(server, "Default") == 0) {
			return true;
		} else if (astrcmp_n(server, "US-West:", 8) == 0 || astrcmp_n(server, "US-East:", 8) == 0) {
			return regionNA;
		} else if (astrcmp_n(server, "South America:", 14) == 0) {
			return regionSA;
		} else if (astrcmp_n(server, "EU-", 3) == 0) {
			return regionEU;
		} else if (
		    astrcmp_n(server, "South Korea:", 12) == 0 || astrcmp_n(server, "Asia:", 5) == 0
		    || astrcmp_n(server, "China:", 6) == 0) {
			return regionAS;
		} else if (astrcmp_n(server, "Oceania:", 8) == 0) {
			return regionOC;
		} else {
			return true;
		}
	} else if (serviceSelected == Service::Beam) {
		if (astrcmp_n(server, "US:", 3) == 0 || astrcmp_n(server, "Canada:", 7) || astrcmp_n(server, "Mexico:", 7)) {
			return regionNA;
		} else if (astrcmp_n(server, "Brazil:", 7) == 0) {
			return regionSA;
		} else if (astrcmp_n(server, "EU:", 3) == 0) {
			return regionEU;
		} else if (
		    astrcmp_n(server, "South Korea:", 12) == 0 || astrcmp_n(server, "Asia:", 5) == 0
		    || astrcmp_n(server, "India:", 6) == 0) {
			return regionAS;
		} else if (astrcmp_n(server, "Australia:", 10) == 0) {
			return regionOC;
		} else {
			return true;
		}
	} else {
		return true;
	}

	return false;
}

void GetServers(std::vector<ServerInfo>& servers)
{
	OBSData settings = obs_data_create();
	obs_data_release(settings);
	// obs_data_set_string(settings, "service", wiz->serviceName.c_str());
	//FIX ME
	obs_data_set_string(settings, "service", serviceName.c_str());

	obs_properties_t* ppts = obs_get_service_properties("rtmp_common");
	obs_property_t*   p    = obs_properties_get(ppts, "service");
	obs_property_modified(p, settings);

	p            = obs_properties_get(ppts, "server");
	size_t count = obs_property_list_item_count(p);
	servers.reserve(count);

	for (size_t i = 0; i < count; i++) {
		const char* name   = obs_property_list_item_name(p, i);
		const char* server = obs_property_list_item_string(p, i);

		if (autoConfig::CanTestServer(name)) {
			ServerInfo info(name, server);
			servers.push_back(info);
		}
	}

	obs_properties_destroy(ppts);
}

void start_next_step(void (*task)(), std::string event, std::string description, int percentage)
{
	/*eventCallbackQueue.work_queue.push_back({cb, event, description, percentage});
    eventCallbackQueue.Signal();

    if(task)
    	std::thread(*task).detach();*/
}

void autoConfig::TerminateAutoConfig(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	StopThread();
}

void autoConfig::Query(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval)
{
	std::unique_lock<std::mutex> ulock(eventsMutex);
	if (events.empty()) {
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		AUTO_DEBUG;
		return;
	}

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));

	rval.push_back(ipc::value(events.front().event));
	rval.push_back(ipc::value(events.front().description));
	rval.push_back(ipc::value(events.front().percentage));

	events.pop();

	AUTO_DEBUG;
}

void autoConfig::StopThread(void)
{
	unique_lock<mutex> ul(m);
	cancel = true;
	cv.notify_one();
}

void autoConfig::InitializeAutoConfig(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	serverName = "Auto (Recommended)";
	server     = "auto";

	cancel = false;

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void autoConfig::StartBandwidthTest(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::thread(TestBandwidthThread).detach();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void autoConfig::StartStreamEncoderTest(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::thread(TestStreamEncoderThread).detach();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void autoConfig::StartRecordingEncoderTest(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::thread(TestRecordingEncoderThread).detach();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void autoConfig::StartSaveStreamSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::thread(SaveStreamSettings).detach();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void autoConfig::StartSaveSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::thread(SaveSettings).detach();

	cancel = false;

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

void autoConfig::StartCheckSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	bool sucess = CheckSettings();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
	rval.push_back(ipc::value((uint32_t)sucess));
}

void autoConfig::StartSetDefaultSettings(
    void*                          data,
    const int64_t                  id,
    const std::vector<ipc::value>& args,
    std::vector<ipc::value>&       rval)
{
	std::thread(SetDefaultSettings).detach();

	rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
}

int EvaluateBandwidth(
    ServerInfo& server,
    bool&       connected,
    bool&       stopped,
    bool&       success,
    OBSData&    service_settings,
    OBSService& service,
    OBSOutput&  output,
    OBSData&    vencoder_settings)
{
	// auto &server = servers[i];

	connected = false;
	stopped   = false;

	// int per = int((i + 1) * 100 / servers.size());

	obs_data_set_string(service_settings, "server", server.address.c_str());
	obs_service_update(service, service_settings);

	if (!obs_output_start(output))
		return -1;

	unique_lock<mutex> ul(m);
	if (cancel) {
		ul.unlock();
		obs_output_force_stop(output);
		return -1;
	}
	if (!stopped && !connected)
		cv.wait(ul);
	if (cancel) {
		ul.unlock();
		obs_output_force_stop(output);
		return -1;
	}
	if (!connected) {
		return -1;
	}

	uint64_t t_start = os_gettime_ns();

	cv.wait_for(ul, chrono::seconds(10));
	if (stopped)
		return -1;
	if (cancel) {
		ul.unlock();
		obs_output_force_stop(output);
		return -1;
	}

	obs_output_stop(output);

	while (!obs_output_active(output)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	cv.wait(ul);

	uint64_t total_time  = os_gettime_ns() - t_start;
	int      total_bytes = (int)obs_output_get_total_bytes(output);
	uint64_t bitrate     = 0;

	if (total_time > 0) {
		bitrate = (uint64_t)total_bytes * 8 * 1000000000 / total_time / 1000;
	}

	startingBitrate      = (int)obs_data_get_int(vencoder_settings, "bitrate");
	if (obs_output_get_frames_dropped(output) || (int)bitrate < (startingBitrate * 75 / 100)) {
		server.bitrate = (int)bitrate * 70 / 100;
	} else {
		server.bitrate = startingBitrate;
	}

	server.ms = obs_output_get_connect_time_ms(output);
	success   = true;
	return 0;
}

void sendErrorMessage(std::string message) {
	eventsMutex.lock();
	events.push(AutoConfigInfo("error", message.c_str(), 0));
	eventsMutex.unlock();
}

void autoConfig::TestBandwidthThread(void)
{
	eventsMutex.lock();
	events.push(AutoConfigInfo("starting_step", "bandwidth_test", 0));
	eventsMutex.unlock();

	bool connected = false;
	bool stopped   = false;

	obs_video_info ovi;
	obs_get_video_info(&ovi);

	ovi.output_width  = 128;
	ovi.output_height = 128;
	ovi.fps_num       = 60;
	ovi.fps_den       = 1;

	obs_reset_video(&ovi);

	const char* serverType = "rtmp_common";

	OBSEncoder vencoder = obs_video_encoder_create("obs_x264", "test_x264", nullptr, nullptr);
	OBSEncoder aencoder = obs_audio_encoder_create("ffmpeg_aac", "test_aac", nullptr, 0, nullptr);
	OBSService service  = obs_service_create(serverType, "test_service", nullptr, nullptr);
	OBSOutput  output   = obs_output_create("rtmp_output", "test_stream", nullptr, nullptr);
	obs_output_release(output);
	obs_encoder_release(vencoder);
	obs_encoder_release(aencoder);
	obs_service_release(service);

	/* -----------------------------------*/
	/* configure settings                 */

	// service: "service", "server", "key"
	// vencoder: "bitrate", "rate_control",
	//           obs_service_apply_encoder_settings
	// aencoder: "bitrate"
	// output: "bind_ip" via main config -> "Output", "BindIP"
	//         obs_output_set_service

	OBSData service_settings  = obs_data_create();
	OBSData vencoder_settings = obs_data_create();
	OBSData aencoder_settings = obs_data_create();
	OBSData output_settings   = obs_data_create();
	obs_data_release(service_settings);
	obs_data_release(vencoder_settings);
	obs_data_release(aencoder_settings);
	obs_data_release(output_settings);

	obs_service_t* currentService = OBS_service::getService();
	if (currentService) {
		obs_data_t* currentServiceSettings = obs_service_get_settings(currentService);
		if (currentServiceSettings) {
			if (serviceName.compare("") == 0)
				serviceName = obs_data_get_string(currentServiceSettings, "service");

			key = obs_service_get_key(currentService);
			if (key.empty()) {
				sendErrorMessage("invalid_stream_settings");
				return;
			}
		} else {
			sendErrorMessage("invalid_stream_settings");
			return;
		}
	} else {
		sendErrorMessage("invalid_stream_settings");
		return;
	}

	if (!customServer) {
		if (serviceName == "Twitch")
			serviceSelected = Service::Twitch;
		else if (serviceName == "hitbox.tv")
			serviceSelected = Service::Hitbox;
		else if (serviceName == "beam.pro")
			serviceSelected = Service::Beam;
		else
			serviceSelected = Service::Other;
	} else {
		serviceSelected = Service::Other;
	}

	std::string keyToEvaluate = key;

	if (serviceSelected == Service::Twitch) {
		string_depad_key(key);
		keyToEvaluate += "?bandwidthtest";
	}

	obs_data_set_string(service_settings, "service", serviceName.c_str());
	obs_data_set_string(service_settings, "key", keyToEvaluate.c_str());

	//Setting starting bitrate
	OBSData service_settingsawd = obs_data_create();
	obs_data_release(service_settingsawd);

	obs_data_set_string(service_settingsawd, "service", serviceName.c_str());

	OBSService servicewad = obs_service_create(serverType, "temp_service", service_settingsawd, nullptr);
	obs_service_release(servicewad);

	int bitrate = 10000;

	OBSData settings = obs_data_create();
	obs_data_release(settings);
	obs_data_set_int(settings, "bitrate", bitrate);
	obs_service_apply_encoder_settings(servicewad, settings, nullptr);

	int awstartingBitrate = (int)obs_data_get_int(settings, "bitrate");
	obs_data_set_int(vencoder_settings, "bitrate", awstartingBitrate);
	obs_data_set_string(vencoder_settings, "rate_control", "CBR");
	obs_data_set_string(vencoder_settings, "preset", "veryfast");
	obs_data_set_int(vencoder_settings, "keyint_sec", 2);

	obs_data_set_int(aencoder_settings, "bitrate", 32);

	const char *bind_ip = config_get_string(ConfigManager::getInstance().getBasic(), "Output",
			"BindIP");
	obs_data_set_string(output_settings, "bind_ip", bind_ip);

	/* -----------------------------------*/
	/* determine which servers to test    */

	std::vector<ServerInfo> servers;
	if (customServer)
		servers.emplace_back(server.c_str(), server.c_str());
	else
		GetServers(servers);

	/* just use the first server if it only has one alternate server */
	if (servers.size() < 3)
		servers.resize(1);

	/* -----------------------------------*/
	/* apply settings                     */

	obs_service_update(service, service_settings);
	obs_service_apply_encoder_settings(service, vencoder_settings, aencoder_settings);

	obs_encoder_update(vencoder, vencoder_settings);
	obs_encoder_update(aencoder, aencoder_settings);
	obs_output_update(output, output_settings);

	/* -----------------------------------*/
	/* connect encoders/services/outputs  */

	obs_encoder_set_video(vencoder, obs_get_video());
	obs_encoder_set_audio(aencoder, obs_get_audio());

	obs_output_set_video_encoder(output, vencoder);
	obs_output_set_audio_encoder(output, aencoder, 0);

	obs_output_set_service(output, service);

	/* -----------------------------------*/
	/* connect signals                    */

	auto on_started = [&]() {
		unique_lock<mutex> lock(m);
		connected = true;
		stopped   = false;
		cv.notify_one();
	};

	auto on_stopped = [&]() {
		unique_lock<mutex> lock(m);
		connected = false;
		stopped   = true;
		cv.notify_one();
	};

	using on_started_t = decltype(on_started);
	using on_stopped_t = decltype(on_stopped);

	auto pre_on_started = [](void* data, calldata_t*) {
		on_started_t& on_started = *reinterpret_cast<on_started_t*>(data);
		on_started();
	};

	auto pre_on_stopped = [](void* data, calldata_t*) {
		on_stopped_t& on_stopped = *reinterpret_cast<on_stopped_t*>(data);
		on_stopped();
	};

	signal_handler* sh = obs_output_get_signal_handler(output);
	signal_handler_connect(sh, "start", pre_on_started, &on_started);
	signal_handler_connect(sh, "stop", pre_on_stopped, &on_stopped);

	/* -----------------------------------*/
	/* test servers                       */

	int    bestBitrate = 0;
	int    bestMS      = 0x7FFFFFFF;
	string bestServer;
	string bestServerName;
	bool   success = false;

	if (serverName.compare("") != 0) {
		ServerInfo info(serverName.c_str(), server.c_str());

		if (EvaluateBandwidth(info, connected, stopped, success, service_settings, service, output, vencoder_settings)
		    < 0) {
			eventsMutex.lock();
			events.push(AutoConfigInfo("error", "invalid_stream_settings", 0));
			eventsMutex.unlock();
			return;
		}

		bestServer     = info.address;
		bestServerName = info.name;
		bestBitrate    = info.bitrate;

		eventsMutex.lock();
		events.push(AutoConfigInfo("progress", "bandwidth_test", 100));
		eventsMutex.unlock();

	} else {
		for (size_t i = 0; i < servers.size(); i++) {
			EvaluateBandwidth(
			    servers[i], connected, stopped, success, service_settings, service, output, vencoder_settings);
			eventsMutex.lock();
			events.push(AutoConfigInfo("progress", "bandwidth_test", (double)(i + 1) * 100 / servers.size()));
			eventsMutex.unlock();
		}
	}

	if (!success) {
		eventsMutex.lock();
		events.push(AutoConfigInfo("error", "invalid_stream_settings", 0));
		eventsMutex.unlock();
		return;
	}

	for (auto& server : servers) {
		bool close = abs(server.bitrate - bestBitrate) < 400;

		if ((!close && server.bitrate > bestBitrate) || (close && server.ms < bestMS)) {
			bestServer     = server.address;
			bestServerName = server.name;
			bestBitrate    = server.bitrate;
			bestMS         = server.ms;
		}
	}

	server       = bestServer;
	serverName   = bestServerName;
	idealBitrate = bestBitrate;

	eventsMutex.lock();
	events.push(AutoConfigInfo("stopping_step", "bandwidth_test", 100));
	eventsMutex.unlock();
}

/* this is used to estimate the lower bitrate limit for a given
 * resolution/fps.  yes, it is a totally arbitrary equation that gets
 * the closest to the expected values */
static long double EstimateBitrateVal(int cx, int cy, int fps_num, int fps_den)
{
	long        fps     = long((long double)fps_num / (long double)fps_den);
	long double areaVal = pow((long double)(cx * cy), 0.85l);
	return areaVal * sqrt(pow(fps, 1.1l));
}

static long double EstimateMinBitrate(int cx, int cy, int fps_num, int fps_den)
{
	long double val = EstimateBitrateVal((int)baseResolutionCX, (int)baseResolutionCY, 60, 1) / 5800.0l;
	if (val < std::numeric_limits<double>::epsilon() && val > -std::numeric_limits<double>::epsilon()) {
		return 0.0;
	}

	return EstimateBitrateVal(cx, cy, fps_num, fps_den) / val;
}

static long double EstimateUpperBitrate(int cx, int cy, int fps_num, int fps_den)
{
	long double val = EstimateBitrateVal(1280, 720, 30, 1) / 3000.0l;
	if (val < std::numeric_limits<double>::epsilon() && val > -std::numeric_limits<double>::epsilon()) {
		return 0.0;
	}

	return EstimateBitrateVal(cx, cy, fps_num, fps_den) / val;
}

struct Result
{
	int cx;
	int cy;
	int fps_num;
	int fps_den;

	inline Result(int cx_, int cy_, int fps_num_, int fps_den_) : cx(cx_), cy(cy_), fps_num(fps_num_), fps_den(fps_den_)
	{}
};

void autoConfig::FindIdealHardwareResolution()
{
	int baseCX = (int)baseResolutionCX;
	int baseCY = (int)baseResolutionCY;

	vector<Result> results;

	int pcores = os_get_physical_cores();
	int maxDataRate;
	if (pcores >= 4) {
		maxDataRate = int(baseResolutionCX * baseResolutionCY * 60 + 1000);
	} else {
		maxDataRate = 1280 * 720 * 30 + 1000;
	}

	auto testRes = [&](long double div, int fps_num, int fps_den, bool force) {
		if (results.size() >= 3)
			return;

		if (!fps_num || !fps_den) {
			fps_num = specificFPSNum;
			fps_den = specificFPSDen;
		}

		long double fps = ((long double)fps_num / (long double)fps_den);

		int cx = int((long double)baseCX / div);
		int cy = int((long double)baseCY / div);

		long double rate = (long double)cx * (long double)cy * fps;
		if (!force && rate > maxDataRate)
			return;

		int minBitrate = int(EstimateMinBitrate(cx, cy, fps_num, fps_den) * 114 / 100);
		if (type == Type::Recording)
			force = true;
		if (force || idealBitrate >= minBitrate)
			results.emplace_back(cx, cy, fps_num, fps_den);
	};

	if (specificFPSNum && specificFPSDen) {
		testRes(1.0, 0, 0, false);
		testRes(1.5, 0, 0, false);
		testRes(1.0 / 0.6, 0, 0, false);
		testRes(2.0, 0, 0, false);
		testRes(2.25, 0, 0, true);
	} else {
		testRes(1.0, 60, 1, false);
		testRes(1.0, 30, 1, false);
		testRes(1.5, 60, 1, false);
		testRes(1.5, 30, 1, false);
		testRes(1.0 / 0.6, 60, 1, false);
		testRes(1.0 / 0.6, 30, 1, false);
		testRes(2.0, 60, 1, false);
		testRes(2.0, 30, 1, false);
		testRes(2.25, 60, 1, false);
		testRes(2.25, 30, 1, true);
	}

	int minArea = 960 * 540 + 1000;

	if (!specificFPSNum && preferHighFPS && results.size() > 1) {
		Result& result1 = results[0];
		Result& result2 = results[1];

		if (result1.fps_num == 30 && result2.fps_num == 60) {
			int nextArea = result2.cx * result2.cy;
			if (nextArea >= minArea)
				results.erase(results.begin());
		}
	}

	Result result     = results.front();
	idealResolutionCX = result.cx;
	idealResolutionCY = result.cy;

	if (idealResolutionCX * idealResolutionCY > 1280 * 720) {
		idealResolutionCX = 1280;
		idealResolutionCY = 720;
	}

	idealFPSNum = result.fps_num;
	idealFPSDen = result.fps_den;
}

bool autoConfig::TestSoftwareEncoding()
{
	OBSEncoder vencoder = obs_video_encoder_create("obs_x264", "test_x264", nullptr, nullptr);
	OBSEncoder aencoder = obs_audio_encoder_create("ffmpeg_aac", "test_aac", nullptr, 0, nullptr);
	OBSOutput  output   = obs_output_create("null_output", "null", nullptr, nullptr);
	obs_output_release(output);
	obs_encoder_release(vencoder);
	obs_encoder_release(aencoder);

	/* -----------------------------------*/
	/* configure settings                 */

	OBSData aencoder_settings = obs_data_create();
	OBSData vencoder_settings = obs_data_create();
	obs_data_release(aencoder_settings);
	obs_data_release(vencoder_settings);
	obs_data_set_int(aencoder_settings, "bitrate", 32);

	if (type != Type::Recording) {
		obs_data_set_int(vencoder_settings, "keyint_sec", 2);
		obs_data_set_int(vencoder_settings, "bitrate", idealBitrate);
		obs_data_set_string(vencoder_settings, "rate_control", "CBR");
		obs_data_set_string(vencoder_settings, "profile", "main");
		obs_data_set_string(vencoder_settings, "preset", "veryfast");
	} else {
		obs_data_set_int(vencoder_settings, "crf", 20);
		obs_data_set_string(vencoder_settings, "rate_control", "CRF");
		obs_data_set_string(vencoder_settings, "profile", "high");
		obs_data_set_string(vencoder_settings, "preset", "veryfast");
	}

	/* -----------------------------------*/
	/* apply settings                     */

	obs_encoder_update(vencoder, vencoder_settings);
	obs_encoder_update(aencoder, aencoder_settings);

	/* -----------------------------------*/
	/* connect encoders/services/outputs  */

	obs_output_set_video_encoder(output, vencoder);
	obs_output_set_audio_encoder(output, aencoder, 0);

	/* -----------------------------------*/
	/* connect signals                    */

	auto on_stopped = [&]() {
		unique_lock<mutex> lock(m);
		cv.notify_one();
	};

	using on_stopped_t = decltype(on_stopped);

	auto pre_on_stopped = [](void* data, calldata_t*) {
		on_stopped_t& on_stopped = *reinterpret_cast<on_stopped_t*>(data);
		on_stopped();
	};

	signal_handler* sh = obs_output_get_signal_handler(output);
	signal_handler_connect(sh, "deactivate", pre_on_stopped, &on_stopped);

	/* -----------------------------------*/
	/* calculate starting resolution      */

	int baseCX = int(baseResolutionCX);
	int baseCY = int(baseResolutionCY);

	/* -----------------------------------*/
	/* calculate starting test rates      */

	int pcores = os_get_physical_cores();
	int lcores = os_get_logical_cores();
	int maxDataRate;
	if (lcores > 8 || pcores > 4) {
		/* superb */
		maxDataRate = int(baseResolutionCX * baseResolutionCY * 60 + 1000);

	} else if (lcores > 4 && pcores == 4) {
		/* great */
		maxDataRate = int(baseResolutionCX * baseResolutionCY * 60 + 1000);

	} else if (pcores == 4) {
		/* okay */
		maxDataRate = int(baseResolutionCX * baseResolutionCY * 30 + 1000);

	} else {
		/* toaster */
		maxDataRate = 960 * 540 * 30 + 1000;
	}

	/* -----------------------------------*/
	/* perform tests                      */

	vector<Result> results;
	int            i     = 0;
	int            count = 1;

	auto testRes = [&](long double div, int fps_num, int fps_den, bool force) {
		int per = ++i * 100 / count;

		/* no need for more than 3 tests max */
		if (results.size() >= 3)
			return true;

		if (!fps_num || !fps_den) {
			fps_num = specificFPSNum;
			fps_den = specificFPSDen;
		}

		long double fps = ((long double)fps_num / (long double)fps_den);

		int cx = int((long double)baseCX / div);
		int cy = int((long double)baseCY / div);

		if (!force && type != Type::Recording) {
			int est = int(EstimateMinBitrate(cx, cy, fps_num, fps_den));
			if (est > idealBitrate)
				return true;
		}

		long double rate = (long double)cx * (long double)cy * fps;
		if (!force && rate > maxDataRate)
			return true;

		obs_video_info ovi;
		obs_get_video_info(&ovi);

		ovi.output_width  = (uint32_t)cx;
		ovi.output_height = (uint32_t)cy;
		ovi.fps_num       = fps_num;
		ovi.fps_den       = fps_den;

		obs_reset_video(&ovi);

		obs_encoder_set_video(vencoder, obs_get_video());
		obs_encoder_set_audio(aencoder, obs_get_audio());
		obs_encoder_update(vencoder, vencoder_settings);

		obs_output_set_media(output, obs_get_video(), obs_get_audio());

		unique_lock<mutex> ul(m);
		if (cancel)
			return false;

		if (!obs_output_start(output)) {
			return false;
		}

		cv.wait_for(ul, chrono::seconds(5));

		obs_output_stop(output);
		cv.wait(ul);

		int skipped = (int)video_output_get_skipped_frames(obs_get_video());
		if (force || skipped <= 10)
			results.emplace_back(cx, cy, fps_num, fps_den);

		return !cancel;
	};

	if (specificFPSNum && specificFPSDen) {
		count = 5;
		if (!testRes(1.0, 0, 0, false))
			return false;
		if (!testRes(1.5, 0, 0, false))
			return false;
		if (!testRes(1.0 / 0.6, 0, 0, false))
			return false;
		if (!testRes(2.0, 0, 0, false))
			return false;
		if (!testRes(2.25, 0, 0, true))
			return false;
	} else {
		count = 10;
		if (!testRes(1.0, 60, 1, false))
			return false;
		if (!testRes(1.0, 30, 1, false))
			return false;
		if (!testRes(1.5, 60, 1, false))
			return false;
		if (!testRes(1.5, 30, 1, false))
			return false;
		if (!testRes(1.0 / 0.6, 60, 1, false))
			return false;
		if (!testRes(1.0 / 0.6, 30, 1, false))
			return false;
		if (!testRes(2.0, 60, 1, false))
			return false;
		if (!testRes(2.0, 30, 1, false))
			return false;
		if (!testRes(2.25, 60, 1, false))
			return false;
		if (!testRes(2.25, 30, 1, true))
			return false;
	}

	/* -----------------------------------*/
	/* find preferred settings            */

	int minArea = 960 * 540 + 1000;

	if (!specificFPSNum && preferHighFPS && results.size() > 1) {
		Result& result1 = results[0];
		Result& result2 = results[1];

		if (result1.fps_num == 30 && result2.fps_num == 60) {
			int nextArea = result2.cx * result2.cy;
			if (nextArea >= minArea)
				results.erase(results.begin());
		}
	}

	Result result     = results.front();
	idealResolutionCX = result.cx;
	idealResolutionCY = result.cy;

	if (idealResolutionCX * idealResolutionCY > 1280 * 720) {
		idealResolutionCX = 1280;
		idealResolutionCY = 720;
	}

	idealFPSNum = result.fps_num;
	idealFPSDen = result.fps_den;

	long double fUpperBitrate = EstimateUpperBitrate(result.cx, result.cy, result.fps_num, result.fps_den);

	int upperBitrate = int(floor(fUpperBitrate / 50.0l) * 50.0l);

	if (streamingEncoder != Encoder::x264) {
		upperBitrate *= 114;
		upperBitrate /= 100;
	}

	if (idealBitrate > upperBitrate)
		idealBitrate = upperBitrate;

	softwareTested = true;
	return true;
}

void autoConfig::TestStreamEncoderThread()
{
	eventsMutex.lock();
	events.push(AutoConfigInfo("starting_step", "streamingEncoder_test", 0));
	eventsMutex.unlock();

	baseResolutionCX = config_get_int(ConfigManager::getInstance().getBasic(), "Video", "BaseCX");
	baseResolutionCY = config_get_int(ConfigManager::getInstance().getBasic(), "Video", "BaseCY");

	TestHardwareEncoding();

	if (!softwareTested) {
		if (!preferHardware || !hardwareEncodingAvailable) {
			if (!TestSoftwareEncoding()) {
				return;
			}
		}
	}

	if (preferHardware && !softwareTested && hardwareEncodingAvailable)
		FindIdealHardwareResolution();

	if (!softwareTested) {
		if (nvencAvailable)
			streamingEncoder = Encoder::NVENC;
		else if (qsvAvailable)
			streamingEncoder = Encoder::QSV;
		else
			streamingEncoder = Encoder::AMD;
	} else {
		streamingEncoder = Encoder::x264;
	}

	eventsMutex.lock();
	events.push(AutoConfigInfo("stopping_step", "streamingEncoder_test", 100));
	eventsMutex.unlock();
}

void autoConfig::TestRecordingEncoderThread()
{
	eventsMutex.lock();
	events.push(AutoConfigInfo("starting_step", "recordingEncoder_test", 0));
	eventsMutex.unlock();

	TestHardwareEncoding();

	if (!hardwareEncodingAvailable && !softwareTested) {
		if (!TestSoftwareEncoding()) {
			return;
		}
	}

	if (type == Type::Recording && hardwareEncodingAvailable)
		FindIdealHardwareResolution();

	recordingQuality = Quality::High;

	bool recordingOnly = type == Type::Recording;

	if (hardwareEncodingAvailable) {
		if (nvencAvailable)
			recordingEncoder = Encoder::NVENC;
		else if (qsvAvailable)
			recordingEncoder = Encoder::QSV;
		else
			recordingEncoder = Encoder::AMD;
	} else {
		recordingEncoder = Encoder::x264;
	}

	if (recordingEncoder != Encoder::NVENC) {
		if (!recordingOnly) {
			recordingEncoder = Encoder::Stream;
			recordingQuality = Quality::Stream;
		}
	}

	eventsMutex.lock();
	events.push(AutoConfigInfo("stopping_step", "recordingEncoder_test", 100));
	eventsMutex.unlock();
}

inline const char* GetEncoderId(Encoder enc)
{
	switch (enc) {
	case Encoder::NVENC:
		return "ffmpeg_nvenc";
	case Encoder::QSV:
		return "obs_qsv11";
	case Encoder::AMD:
		return "amd_amf_h264";
	default:
		return "ffmpeg_nvenc";
	}
};

inline const char* GetEncoderDisplayName(Encoder enc)
{
	switch (enc) {
	case Encoder::NVENC:
		return SIMPLE_ENCODER_NVENC;
	case Encoder::QSV:
		return SIMPLE_ENCODER_QSV;
	case Encoder::AMD:
		return SIMPLE_ENCODER_AMD;
	default:
		return SIMPLE_ENCODER_X264;
	}
};

bool autoConfig::CheckSettings(void)
{
	OBSData settings = obs_data_create();

	obs_data_set_string(settings, "service", serviceName.c_str());
	obs_data_set_string(settings, "server", server.c_str());

	std::string testKey = key;

	if (serviceName.compare("Twitch") == 0) {
		testKey += "?bandwidthtest";
	}

	obs_data_set_string(settings, "key", testKey.c_str());

	OBSService service = obs_service_create("rtmp_common", "serviceTest", settings, NULL);

	if (!service) {
		eventsMutex.lock();
		events.push(AutoConfigInfo("error", "invalid_service", 100));
		eventsMutex.unlock();
		return false;
	}

	obs_video_info ovi;
	obs_get_video_info(&ovi);

	ovi.output_width  = (uint32_t)idealResolutionCX;
	ovi.output_height = (uint32_t)idealResolutionCY;
	ovi.fps_num       = idealFPSNum;
	ovi.fps_den       = 1;

	obs_reset_video(&ovi);

	OBSEncoder vencoder = obs_video_encoder_create(GetEncoderId(streamingEncoder), "test_encoder", nullptr, nullptr);
	OBSEncoder aencoder = obs_audio_encoder_create("ffmpeg_aac", "test_aac", nullptr, 0, nullptr);
	OBSOutput  output   = obs_output_create("rtmp_output", "test_stream", nullptr, nullptr);
	obs_output_release(output);
	obs_encoder_release(vencoder);
	obs_encoder_release(aencoder);
	obs_service_release(service);

	OBSData service_settings  = obs_data_create();
	OBSData vencoder_settings = obs_data_create();
	OBSData aencoder_settings = obs_data_create();
	OBSData output_settings   = obs_data_create();
	obs_data_release(service_settings);
	obs_data_release(vencoder_settings);
	obs_data_release(aencoder_settings);
	obs_data_release(output_settings);

	obs_data_set_int(vencoder_settings, "bitrate", idealBitrate);
	obs_data_set_string(vencoder_settings, "rate_control", "CBR");
	obs_data_set_string(vencoder_settings, "preset", "veryfast");
	obs_data_set_int(vencoder_settings, "keyint_sec", 2);

	obs_data_set_int(aencoder_settings, "bitrate", 32);

	/* -----------------------------------*/
	/* apply settings                     */

	obs_service_apply_encoder_settings(service, vencoder_settings, aencoder_settings);

	obs_encoder_update(vencoder, vencoder_settings);
	obs_encoder_update(aencoder, aencoder_settings);
	obs_output_update(output, output_settings);

	/* -----------------------------------*/
	/* connect encoders/services/outputs  */

	obs_encoder_set_video(vencoder, obs_get_video());
	obs_encoder_set_audio(aencoder, obs_get_audio());

	obs_output_set_video_encoder(output, vencoder);
	obs_output_set_audio_encoder(output, aencoder, 0);

	obs_output_set_service(output, service);

	obs_output_start(output);

	int  timeout = 8;
	bool success = true;

	while (!obs_output_active(output) && success) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		timeout--;
		if (timeout == 0) {
			success = false;
		}
	}

	if (success)
		obs_output_stop(output);

	return success;
}

void autoConfig::SetDefaultSettings(void)
{
	eventsMutex.lock();
	events.push(AutoConfigInfo("starting_step", "setting_default_settings", 0));
	eventsMutex.unlock();

	idealResolutionCX = 1280;
	idealResolutionCY = 720;
	idealFPSNum       = 30;
	recordingQuality = Quality::High;
	idealBitrate     = 2500;
	streamingEncoder = Encoder::x264;
	recordingEncoder = Encoder::Stream;

	eventsMutex.lock();
	events.push(AutoConfigInfo("stopping_step", "setting_default_settings", 100));
	eventsMutex.unlock();
}

void autoConfig::SaveStreamSettings()
{
	/* ---------------------------------- */
	/* save service                       */

	eventsMutex.lock();
	events.push(AutoConfigInfo("starting_step", "saving_service", 0));
	eventsMutex.unlock();

	const char* service_id = "rtmp_common";

	obs_service_t* oldService = OBS_service::getService();
	OBSData        hotkeyData = obs_hotkeys_save_service(oldService);
	obs_data_release(hotkeyData);

	OBSData settings = obs_data_create();

	if (!customServer)
		obs_data_set_string(settings, "service", serviceName.c_str());
	obs_data_set_string(settings, "server", server.c_str());
	obs_data_set_string(settings, "key", key.c_str());

	OBSService newService = obs_service_create(service_id, "default_service", settings, hotkeyData);

	if (!newService)
		return;

	OBS_service::setService(newService);
	OBS_service::saveService();

	/* ---------------------------------- */
	/* save stream settings               */
	config_set_int(ConfigManager::getInstance().getBasic(), "SimpleOutput", "VBitrate",
			idealBitrate);
	config_set_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "StreamEncoder",
			GetEncoderDisplayName(streamingEncoder));
	config_remove_value(ConfigManager::getInstance().getBasic(), "SimpleOutput", "UseAdvanced");

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);
	
	eventsMutex.lock();
	events.push(AutoConfigInfo("stopping_step", "saving_service", 100));
	eventsMutex.unlock();
}

void autoConfig::SaveSettings()
{
	eventsMutex.lock();
	events.push(AutoConfigInfo("starting_step", "saving_settings", 0));
	eventsMutex.unlock();
	
	if (recordingEncoder != Encoder::Stream)
		config_set_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecEncoder",
				GetEncoderDisplayName(recordingEncoder));

	const char* quality = recordingQuality == Quality::High ? "Small" : "Stream";

	config_set_string(ConfigManager::getInstance().getBasic(), "Output", "Mode", "Simple");
	config_set_string(ConfigManager::getInstance().getBasic(), "SimpleOutput", "RecQuality", quality);
	config_set_int(ConfigManager::getInstance().getBasic(), "Video", "OutputCX", idealResolutionCX);
	config_set_int(ConfigManager::getInstance().getBasic(), "Video", "OutputCY", idealResolutionCY);

	if (fpsType != FPSType::UseCurrent) {
		config_set_uint(ConfigManager::getInstance().getBasic(), "Video", "FPSType", 0);
		config_set_string(ConfigManager::getInstance().getBasic(), "Video", "FPSCommon",
				std::to_string(idealFPSNum).c_str());
		std::string fpsvalue = 
			config_get_string(ConfigManager::getInstance().getBasic(), "Video", "FPSCommon");
	}

	config_save_safe(ConfigManager::getInstance().getBasic(), "tmp", nullptr);

	eventsMutex.lock();
	events.push(AutoConfigInfo("stopping_step", "saving_settings", 100));
	events.push(AutoConfigInfo("done", "", 0));
	eventsMutex.unlock();
}
