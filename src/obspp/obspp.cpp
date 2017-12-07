#include "obspp.hpp"

namespace obs {

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

bool initialized()
{
    return obs_initialized();
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

uint32_t output_flags_from_id(std::string id)
{
    return obs_get_source_output_flags(id.c_str());
}

void output_source(int channel, obs::source source)
{
    obs_set_output_source(channel, source.dangerous());
}

obs::source output_source(int channel)
{
    obs_source_t * source = obs_get_output_source(channel);
    return source;
}

uint32_t lagged_frames()
{
    return obs_get_lagged_frames();
}

uint32_t total_frames()
{
    return obs_get_total_frames();
}

}