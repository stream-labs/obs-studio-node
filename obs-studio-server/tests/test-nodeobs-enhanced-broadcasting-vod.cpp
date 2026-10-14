#include <catch2/catch_test_macros.hpp>

#include <obs.hpp>
#include <util/platform.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "nodeobs_configManager.hpp"
#include "nodeobs_service.h"
#include "obs-setup.hpp"
#include "osn-encoders.hpp"
#include "osn-error.hpp"
#include "scoped-helpers.hpp"

namespace {

class LegacyStreamingFixture {
public:
	~LegacyStreamingFixture()
	{
		OBS_service::stopAllOutputs();
		OBS_service::clearOutputObjectsForShutdown();
		OBS_service::setVideoInfo(nullptr, StreamServiceId::Main);
		if (canvas)
			CHECK(obs_remove_video_info(canvas) == OBS_VIDEO_SUCCESS);
		std::error_code ignored;
		std::filesystem::remove(configFile, ignored);
	}

	void initialize()
	{
		canvas = obs_create_video_info();
		REQUIRE(canvas);
#ifdef _WIN32
		canvas->graphics_module = "libobs-d3d11.dll";
#else
		canvas->graphics_module = "libobs-opengl";
#endif
		canvas->base_width = canvas->output_width = 320;
		canvas->base_height = canvas->output_height = 180;
		canvas->fps_num = 30;
		canvas->fps_den = 1;
		canvas->output_format = VIDEO_FORMAT_NV12;
		canvas->colorspace = VIDEO_CS_709;
		canvas->range = VIDEO_RANGE_PARTIAL;
		canvas->scale_type = OBS_SCALE_BILINEAR;
		REQUIRE(obs_reset_video(canvas) == OBS_VIDEO_SUCCESS);
		OBS_service::setVideoInfo(canvas, StreamServiceId::Main);

		configFile = std::filesystem::temp_directory_path() / ("nodeobs-enhanced-vod-config-" + std::to_string(os_gettime_ns()) + ".json");
		std::ofstream file(configFile);
		file << R"({
			"meta": {"service": "IVS", "schema_version": "2025-01-25", "config_id": "local-vod-test"},
			"ingest_endpoints": [{"protocol": "RTMP", "url_template": "rtmp://127.0.0.1:1/live", "authentication": "local-vod-test"}],
			"encoder_configurations": [{"type": "obs_x264", "width": 320, "height": 180, "canvas_index": 0,
				"settings": {"bitrate": 500, "keyint_sec": 2, "preset": "ultrafast", "rate_control": "CBR"}}],
			"audio_configurations": {
				"live": [{"codec": "mp4a", "track_id": 0, "channels": 2, "settings": {"bitrate": 160}}],
				"vod": [{"codec": "mp4a", "track_id": 1, "channels": 2, "settings": {"bitrate": 160}}]
			}
		})";
		file.close();
		REQUIRE(file.good());

		OBSDataAutoRelease settings = obs_data_create();
		obs_data_set_string(settings, "service", "Twitch");
		obs_data_set_string(settings, "server", "auto");
		obs_data_set_string(settings, "key", "local-vod-test");
		OBSServiceAutoRelease service = obs_service_create("rtmp_common", "nodeobs-vod-test", settings, nullptr);
		REQUIRE(service);
		// Set the URL after creation because the common-service plugin fills it from
		// its catalog. The real start handler loads this file without contacting Twitch.
		OBSDataAutoRelease serviceSettings = obs_service_get_settings(service);
		std::string fileUrl = "file://";
		const auto filename = configFile.generic_string();
		if (filename.front() != '/')
			fileUrl += '/';
		for (char character : filename)
			fileUrl += character == ' ' ? "%20" : std::string(1, character);
		obs_data_set_string(serviceSettings, "multitrack_video_configuration_url", fileUrl.c_str());
		OBS_service::setService(obs_service_get_ref(service), StreamServiceId::Main);
	}

private:
	obs_video_info *canvas = nullptr;
	std::filesystem::path configFile;
};

void checkVodSelection(const char *mode, bool useAdvanced, bool simpleVod, bool advancedVod, int expectedVodMixer)
{
	osn::tests::ObsSetup setupOBS;
	config_t *config = ConfigManager::getInstance().getBasic();
	osn::tests::ScopedConfigValue outputMode(config, "Output", "Mode", mode);
	osn::tests::ScopedConfigValue simpleEncoder(config, "SimpleOutput", "StreamEncoder", SIMPLE_ENCODER_X264);
	osn::tests::ScopedConfigValue advancedEncoder(config, "AdvOut", "Encoder", ADVANCED_ENCODER_X264);
	osn::tests::ScopedConfigValue simpleAdvanced(config, "SimpleOutput", "UseAdvanced", useAdvanced ? "true" : "false");
	osn::tests::ScopedConfigValue simpleEnabled(config, "SimpleOutput", "VodTrackEnabled", simpleVod ? "true" : "false");
	osn::tests::ScopedConfigValue advancedEnabled(config, "AdvOut", "VodTrackEnabled", advancedVod ? "true" : "false");
	osn::tests::ScopedConfigValue advancedTrack(config, "AdvOut", "VodTrackIndex", "6");
	osn::tests::ScopedConfigValue enhanced(config, "EnhancedBroadcasting", "EnableMultitrackVideo", "true");
	LegacyStreamingFixture fixture;
	fixture.initialize();

	std::vector<ipc::value> response;
	OBS_service::OBS_service_startStreaming(nullptr, 0, {ipc::value(int64_t{StreamServiceId::Main})}, response);
	REQUIRE(response.size() == 1);
	REQUIRE((ErrorCode)response[0].value_union.ui64 == ErrorCode::Ok);
	obs_output_t *output = OBS_service::getStreamingOutput(StreamServiceId::Main);
	REQUIRE(output);
	obs_encoder_t *live = obs_output_get_audio_encoder(output, 0);
	REQUIRE(live);
	CHECK(obs_encoder_get_mixer_index(live) == 0);
	obs_encoder_t *vod = obs_output_get_audio_encoder(output, 1);
	if (expectedVodMixer < 0) {
		CHECK_FALSE(vod);
	} else {
		REQUIRE(vod);
		CHECK(obs_encoder_get_mixer_index(vod) == size_t(expectedVodMixer));
	}
	CHECK(config_get_int(config, "AdvOut", "VodTrackIndex") == 6);
}

} // namespace

TEST_CASE("NodeObs Simple Enhanced Broadcasting uses track 2 independently of Advanced VOD", "[nodeobs][enhanced-broadcasting][vod]")
{
	checkVodSelection("Simple", true, true, false, 1);
}

TEST_CASE("NodeObs Simple Enhanced Broadcasting ignores a stale Advanced VOD track", "[nodeobs][enhanced-broadcasting][vod]")
{
	checkVodSelection("Simple", true, true, true, 1);
}

TEST_CASE("NodeObs Simple Enhanced Broadcasting respects disabled Simple VOD", "[nodeobs][enhanced-broadcasting][vod]")
{
	checkVodSelection("Simple", true, false, true, -1);
}

TEST_CASE("NodeObs Simple Enhanced Broadcasting requires advanced encoder settings for VOD", "[nodeobs][enhanced-broadcasting][vod]")
{
	checkVodSelection("Simple", false, true, true, -1);
}

TEST_CASE("NodeObs Advanced Enhanced Broadcasting preserves its configured VOD track", "[nodeobs][enhanced-broadcasting][vod]")
{
	checkVodSelection("Advanced", false, false, true, 5);
}

TEST_CASE("NodeObs Advanced Enhanced Broadcasting ignores the Simple VOD preference", "[nodeobs][enhanced-broadcasting][vod]")
{
	checkVodSelection("Advanced", true, true, false, -1);
}
