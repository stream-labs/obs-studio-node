#include "obspp-properties.hpp"
#include "obspp.hpp"

namespace obs {

properties::properties(obs_properties_t *properties)
 : m_handle(properties),
   m_status(status_type::okay)
{
    if (!m_handle) {
        m_status = status_type::invalid;
    }
}

properties::properties(std::string id, object_type type)
{
    switch (type) {
    case object_type::source:
        m_handle = obs_get_source_properties(id.c_str());
        break;
    case object_type::encoder:
        obs_get_encoder_properties(id.c_str());
        break;
    case object_type::service:
        obs_get_service_properties(id.c_str());
        break;
    case object_type::output:
        obs_get_output_properties(id.c_str());
        break;
    default:
        m_handle = nullptr;
    }

    if (!m_handle) m_status = status_type::invalid;
}

properties::~properties()
{
    obs_properties_destroy(m_handle);
}

properties::status_type properties::status()
{
    return m_status;
}

properties::property properties::first()
{
    return obs_properties_first(m_handle);
}

properties::property properties::get(std::string property_name)
{
    return obs_properties_get(m_handle, property_name.c_str());
}

obs_properties_t *properties::dangerous()
{
    return m_handle;
}

properties::property::property(obs_property_t *handle)
 : m_handle(handle)
{
    if (!m_handle) m_status = status_type::invalid;
}

properties::property::status_type properties::property::status()
{
    return m_status;
}

obs_property_t *properties::property::dangerous()
{
    return m_handle;
}

std::string properties::property::name()
{
    return obs_property_name(m_handle);
}

std::string properties::property::description()
{
    return obs_property_description(m_handle);
}

std::string properties::property::long_description()
{
    return obs_property_long_description(m_handle);
}

obs_property_type properties::property::type()
{
    return obs_property_get_type(m_handle);
}

bool properties::property::enabled()
{
    return obs_property_enabled(m_handle);
}

bool properties::property::visible()
{
    return obs_property_visible(m_handle);
}

bool properties::property::next()
{
    bool result = obs_property_next(&m_handle);

    if (!m_handle) m_status = status_type::invalid;

    return result;
}


}