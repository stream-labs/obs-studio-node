#include "obspp-service.hpp"

namespace obs {

std::vector<std::string> service::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    for (int i = 0; obs_enum_service_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}

service::service()
 : m_handle(nullptr), m_status(status_type::invalid)
{
}

service::service(std::string &id, std::string &name, obs_data_t *settings, obs_data_t *hotkey)
    : m_handle(obs_service_create(id.c_str(), name.c_str(), settings, hotkey)),
    m_status(status_type::okay)
{
    /* obs_service_create implicitly creates a reference. */
    if (!m_handle)
        m_status = status_type::invalid;
}

service::service(std::string &id, std::string &name, obs_data_t *settings, bool is_private)
{
    if (is_private)
        m_handle = obs_service_create_private(id.c_str(), name.c_str(), settings);
    else
        m_handle = obs_service_create(id.c_str(), name.c_str(), settings, NULL);

    if (!m_handle)
        m_status = status_type::invalid;
}

service::service(obs_service_t *service)
    : m_handle(service),
    m_status(status_type::okay)
{
    if (!m_handle)
        m_status = status_type::invalid;
}

service::service(service &copy)
    : m_handle(copy.m_handle),
    m_status(copy.m_status)
{
}

void service::addref()
{
    obs_service_addref(m_handle);
}

void service::release()
{
    obs_service_release(m_handle);
}

service::~service()
{
}

bool service::operator!() const
{
    return m_status != status_type::okay;
}

obs_service_t *service::dangerous()
{
    return m_handle;
}

service::status_type service::status()
{
    return m_status;
}

service service::from_name(std::string name)
{
    obs_service_t *handle = obs_get_service_by_name(name.c_str());
    obs_service_release(handle);
    return service(handle);
}

bool service::configurable()
{
    return true;
}

obs::properties service::properties()
{
    return std::move(obs::properties(obs_service_properties(m_handle)));
}

obs_data_t *service::settings()
{
    return obs_service_get_settings(m_handle);
}

void service::update(obs_data_t *data)
{
    obs_service_update(m_handle, data);
}


/** Convenience functions. 
  * These are generally the same as
  * the current setting values. */
const std::string service::url()
{
    return obs_service_get_url(m_handle);
}

const std::string service::key()
{
    return obs_service_get_key(m_handle);
}

const std::string service::username()
{
    return obs_service_get_username(m_handle);
}

const std::string service::password()
{
   return obs_service_get_password(m_handle);
}

const std::string service::name()
{
    return obs_service_get_name(m_handle);
}

const std::string service::id()
{
    return obs_service_get_id(m_handle);
}


}