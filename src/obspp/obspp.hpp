#pragma once

#include <obs.h>
#include "obspp-input.hpp"
#include "obspp-output.hpp"
#include "obspp-scene.hpp"
#include "obspp-filter.hpp"
#include "obspp-transition.hpp"
#include "obspp-module.hpp"

#include <string>
#include <exception>

namespace obs {

enum status_type {
    okay,
    invalid
};

bool startup(std::string locale, std::string path)
{
    if (obs_initialized())
        throw std::logic_error("obs is already initialized");

    return obs_startup(locale.c_str(), path.c_str(), nullptr);
}

bool startup(std::string locale)
{
    return obs_startup(locale.c_str(), nullptr, nullptr);
}

void shutdown()
{
    obs_shutdown();
}

unsigned char status()
{
    return static_cast<obs::status_type>(!obs_initialized());
}

uint32_t version()
{
    return obs_get_version();
}

std::string locale()
{
    return obs_get_locale();
}

void locale(std::string locale)
{
    obs_set_locale(locale.c_str());
}

/* OBS namespace functionality */

void log_handler(log_handler_t handler)
{
}

log_handler_t log_handler()
{
    return nullptr;
}

template <typename T>
void output_source(int channel, T &source)
{
    obs_set_output_source(channel, source.dangerous());
}

void output_source(int channel, obs_source_t *source)
{
    obs_set_output_source(channel, source);
}

template <typename T>
T&& output_source(int channel)
{
    return std::move(obs_get_output_source(channel));
}

}