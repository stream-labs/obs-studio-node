#pragma once

#include <obs.h>
#include <util/config-file.h>
#include <string>

#include "osn-video-encoder.hpp"

namespace osn::tests {

// Restores the previous in-memory value, including the absence of a user override.
// The configuration must outlive this guard; no configuration files are saved.
class ScopedConfigValue {
public:
	ScopedConfigValue(config_t *config, const char *section, const char *name, const char *value)
		: config(config), section(section), name(name), hadUserValue(config_has_user_value(config, section, name))
	{
		const char *previous = config_get_string(config, section, name);
		previousValue = previous ? previous : "";
		config_set_string(config, section, name, value);
	}

	ScopedConfigValue(const ScopedConfigValue &) = delete;
	ScopedConfigValue &operator=(const ScopedConfigValue &) = delete;

	~ScopedConfigValue()
	{
		if (hadUserValue)
			config_set_string(config, section.c_str(), name.c_str(), previousValue.c_str());
		else
			config_remove_value(config, section.c_str(), name.c_str());
	}

private:
	config_t *config;
	std::string section;
	std::string name;
	bool hadUserValue;
	std::string previousValue;
};

// Owns one existing native reference and its VideoEncoder::Manager registration.
// Neither may be released elsewhere while this guard is alive.
class ScopedEncoder {
public:
	explicit ScopedEncoder(obs_encoder_t *encoder) : encoder(encoder) {}

	ScopedEncoder(const ScopedEncoder &) = delete;
	ScopedEncoder &operator=(const ScopedEncoder &) = delete;

	~ScopedEncoder()
	{
		osn::VideoEncoder::Manager::GetInstance().free(encoder);
		obs_encoder_release(encoder);
	}

private:
	obs_encoder_t *encoder;
};

} // namespace osn::tests
