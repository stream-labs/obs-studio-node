#include <catch2/catch_test_macros.hpp>

#include <obs.hpp>
#include <string>
#include <vector>

#include "nodeobs_configManager.hpp"
#include "nodeobs_settings.h"
#include "obs-setup.hpp"
#include "osn-error.hpp"
#include "osn-video-encoder.hpp"

namespace {

class ScopedConfigValue {
public:
	ScopedConfigValue(config_t *config, const char *section, const char *name, const char *value)
		: config(config), section(section), name(name), hadUserValue(config_has_user_value(config, section, name))
	{
		const char *previous = config_get_string(config, section, name);
		previousValue = previous ? previous : "";
		config_set_string(config, section, name, value);
	}

	~ScopedConfigValue()
	{
		if (hadUserValue)
			config_set_string(config, section, name, previousValue.c_str());
		else
			config_remove_value(config, section, name);
	}

private:
	config_t *config;
	const char *section;
	const char *name;
	bool hadUserValue;
	std::string previousValue;
};

class ScopedEncoder {
public:
	explicit ScopedEncoder(obs_encoder_t *encoder) : encoder(encoder) {}
	~ScopedEncoder()
	{
		osn::VideoEncoder::Manager::GetInstance().free(encoder);
		obs_encoder_release(encoder);
	}

private:
	obs_encoder_t *encoder;
};

std::string readSimpleStreamingSettings()
{
	std::vector<ipc::value> args = {ipc::value("obs_x264"), ipc::value("streaming"), ipc::value("Simple")};
	std::vector<ipc::value> response;
	OBS_settings::OBS_settings_getEncoderSettings(nullptr, 0, args, response);
	REQUIRE(response.size() == 2);
	REQUIRE((ErrorCode)response[0].value_union.ui64 == ErrorCode::Ok);
	return response[1].value_str;
}

} // namespace

TEST_CASE("Encoder settings preserve native defaults during creation and replacement", "[encoder-settings]")
{
	osn::tests::ObsSetup setupOBS;
	config_t *config = ConfigManager::getInstance().getBasic();
	ScopedConfigValue mode(config, "Output", "Mode", "Simple");
	ScopedConfigValue selectedEncoder(config, "SimpleOutput", "StreamEncoder", "x264");
	ScopedConfigValue advanced(config, "SimpleOutput", "UseAdvanced", "false");
	ScopedConfigValue preset(config, "SimpleOutput", "Preset", "fast");
	ScopedConfigValue customSettings(config, "SimpleOutput", "x264Settings", "scenecut=0");

	for (bool explicitPreset : {false, true}) {
		INFO("UseAdvanced = " << explicitPreset);
		config_set_bool(config, "SimpleOutput", "UseAdvanced", explicitPreset);

		const std::string settingsJson = readSimpleStreamingSettings();
		OBSDataAutoRelease snapshot = obs_data_create_from_json(settingsJson.c_str());
		REQUIRE(snapshot);
		CHECK(obs_data_has_user_value(snapshot, "preset") == explicitPreset);
		CHECK_FALSE(obs_data_has_user_value(snapshot, "keyint_sec"));

		std::vector<ipc::value> createArgs = {ipc::value("obs_x264"), ipc::value("native-defaults-test"), ipc::value(settingsJson)};
		std::vector<ipc::value> response;
		osn::VideoEncoder::Create(nullptr, 0, createArgs, response);
		REQUIRE(response.size() == 2);
		REQUIRE((ErrorCode)response[0].value_union.ui64 == ErrorCode::Ok);
		const uint64_t encoderId = response[1].value_union.ui64;
		obs_encoder_t *encoder = osn::VideoEncoder::Manager::GetInstance().find(encoderId);
		ScopedEncoder encoderCleanup(encoder);
		REQUIRE(encoder);
		OBSDataAutoRelease settings = obs_encoder_get_settings(encoder);
		REQUIRE(settings);

		// Encoders can refine defaults after detecting hardware capabilities. A saved
		// override must win, while a default serialized by OSN must not pin the value.
		obs_data_set_default_string(settings, "preset", "medium");
		CHECK(obs_data_has_user_value(settings, "preset") == explicitPreset);
		CHECK(std::string(obs_data_get_string(settings, "preset")) == (explicitPreset ? "fast" : "medium"));

		std::vector<ipc::value> updateArgs = {ipc::value(encoderId), ipc::value("{\"keyint_sec\":2}"), ipc::value(uint32_t{0})};
		response.clear();
		osn::VideoEncoder::Update(nullptr, 0, updateArgs, response);
		REQUIRE(response.size() == 1);
		REQUIRE((ErrorCode)response[0].value_union.ui64 == ErrorCode::Ok);
		CHECK(obs_data_has_user_value(settings, "keyint_sec"));
		CHECK(obs_data_get_int(settings, "keyint_sec") == 2);
		CHECK(std::string(obs_data_get_string(settings, "preset")) == (explicitPreset ? "fast" : "medium"));

		config_set_bool(config, "SimpleOutput", "UseAdvanced", false);
		updateArgs = {ipc::value(encoderId), ipc::value(readSimpleStreamingSettings()), ipc::value(uint32_t{1})};
		response.clear();
		osn::VideoEncoder::Update(nullptr, 0, updateArgs, response);
		REQUIRE(response.size() == 1);
		REQUIRE((ErrorCode)response[0].value_union.ui64 == ErrorCode::Ok);
		CHECK_FALSE(obs_data_has_user_value(settings, "preset"));
		CHECK_FALSE(obs_data_has_user_value(settings, "x264opts"));
		CHECK_FALSE(obs_data_has_user_value(settings, "keyint_sec"));
		CHECK(std::string(obs_data_get_string(settings, "preset")) == "medium");
		CHECK(obs_data_get_int(settings, "keyint_sec") == 0);
	}
}
