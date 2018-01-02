#pragma once

#include <obs.h>
#include <string>
#include <vector>

#include "obspp-properties.hpp"
#include "obspp-weak.hpp"

namespace obs {

class service {
public:
    enum status_type {
        okay,
        invalid
    };

private:
    obs_service_t *m_handle;
    status_type m_status;

    service();

public:
    service(std::string &id, std::string &name, obs_data_t *settings = nullptr, obs_data_t *hotkey = nullptr);
    service(std::string &id, std::string &name, obs_data_t *settings, bool is_private = false);

    service(service &copy);
    service(obs_service_t *service);

    template <typename T>
    service(obs::strong<T> &service) : m_handle(service->dangerous())
    {
        if (!m_handle) m_status = status_type::invalid;
    }

    void addref();
    void release();
    ~service();
    bool operator!() const;
    obs_service_t *dangerous();
    status_type status();

    static std::vector<std::string> types();
    static service from_name(std::string name);

    /** Configurable concept */
    bool configurable();
    obs::properties properties();
    obs_data_t *settings();
    void update(obs_data_t *data);

    /** Convenience functions. 
      * These are generally the same as
      * the current setting values. */
    const std::string url();
    const std::string key();
    const std::string username();
    const std::string password();

    const std::string name();
    const std::string id();
};

}