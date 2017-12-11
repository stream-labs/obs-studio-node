#include "obspp-encoder.hpp"

#include <vector>

namespace obs {

obs_encoder_t *encoder::dangerous()
{
    return m_handle;
}

void encoder::release()
{
    obs_encoder_release(m_handle);
}

std::string encoder::display_name()
{
    return obs_encoder_get_display_name(id().c_str());
}

void encoder::name(std::string name)
{
    obs_encoder_set_name(m_handle, name.c_str());
}

std::string encoder::name()
{
    return obs_encoder_get_name(m_handle);
}

std::string encoder::id()
{
    return obs_encoder_get_id(m_handle);
}

obs_encoder_type encoder::type()
{
    return obs_encoder_get_type(m_handle);
}

uint32_t encoder::caps()
{
    return obs_get_encoder_caps(id().c_str());
}

std::string encoder::codec()
{
    return obs_encoder_get_codec(m_handle);
}

void *encoder::type_data()
{
    return obs_encoder_get_type_data(m_handle);
}

void encoder::update(obs_data_t *data)
{
    obs_encoder_update(m_handle, data);
}

obs_data_t *encoder::settings()
{
    return obs_encoder_get_settings(m_handle);
}

obs::properties encoder::properties()
{
    return obs_encoder_properties(m_handle);
}

bool encoder::active()
{
    return obs_encoder_active(m_handle);
}

std::string encoder::codec(std::string id)
{
    return obs_get_encoder_codec(id.c_str());
}

obs_encoder_type encoder::type(std::string id)
{
    return obs_get_encoder_type(id.c_str());
}

uint32_t encoder::caps(std::string id)
{
    return obs_get_encoder_caps(id.c_str());
}

}