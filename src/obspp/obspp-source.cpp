#include "obspp-source.hpp"

namespace obs {

source::source()
 : m_handle(nullptr), m_status(status_type::invalid)
{
}

source::source(std::string &id, std::string &name, obs_data_t *settings, obs_data_t *hotkey)
    : m_handle(obs_source_create(id.c_str(), name.c_str(), settings, hotkey)),
    m_status(status_type::okay)
{
    /* obs_source_create implicitly creates a reference. */
    if (!m_handle)
        m_status = status_type::invalid;
}

source::source(std::string &id, std::string &name, obs_data_t *settings, bool is_private)
{
    if (is_private)
        m_handle = obs_source_create_private(id.c_str(), name.c_str(), settings);
    else
        m_handle = obs_source_create(id.c_str(), name.c_str(), settings, NULL);

    if (!m_handle)
        m_status = status_type::invalid;
}

source::source(obs_source_t *source)
    : m_handle(source),
    m_status(status_type::okay)
{
    if (!m_handle)
        m_status = status_type::invalid;
}

source::source(source &copy)
    : m_handle(copy.m_handle),
    m_status(copy.m_status)
{
}

void source::flags(uint32_t flags)
{
    obs_source_set_flags(m_handle, flags);
}

uint32_t source::flags()
{
    return obs_source_get_flags(m_handle);
}

void source::release() 
{
    obs_source_release(m_handle);
    m_handle = nullptr;
}

void source::remove() 
{
    obs_source_remove(m_handle);
}

source::~source() 
{
}

void source::check_type(obs_source_t * source, obs_source_type type)
{
    obs_source_type source_type = obs_source_get_type(source);

    if (source_type == OBS_SOURCE_TYPE_SCENE &&
        type == OBS_SOURCE_TYPE_INPUT) 
    {
        return;
    }

    if (source_type == type)
        return;

    throw std::runtime_error("Incorrect source type used");
}

bool source::operator!() const
{
    return m_status != status_type::okay;
}

obs_source_t *source::dangerous()
{
    return m_handle;
}

source::status_type source::status()
{
    return m_status;
}

const std::string source::name()
{
    return obs_source_get_name(m_handle);
}

void source::name(std::string name)
{
    obs_source_set_name(m_handle, name.c_str());
}

const std::string source::id()
{
    return obs_source_get_id(m_handle);
}

bool source::configurable()
{
    return obs_source_configurable(m_handle);
}

obs::properties source::properties()
{
    return std::move(obs::properties(obs_source_properties(m_handle)));
}

void source::update(obs_data_t *data)
{
    obs_source_update(m_handle, data);
}

uint32_t source::height()
{
    return obs_source_get_height(m_handle);
}

uint32_t source::width()
{
    return obs_source_get_width(m_handle);
}

}