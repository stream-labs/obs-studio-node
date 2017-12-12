#pragma once

#include <obs.h>
#include <string>
#include <vector>

#include "obspp-properties.hpp"

namespace obs {

class encoder {
public:
    enum status_type {
        okay,
        invalid
    };

protected:
    obs_encoder_t *m_handle;
    status_type    m_status;

    static void check_type(obs_encoder_t *encoder, obs_encoder_type type);

public:
    obs_encoder_t *dangerous();
    void release();
    std::string display_name();
    void name(std::string name);
    std::string name();
    std::string id();
    obs_encoder_type type();
    uint32_t caps();
    std::string codec();
    void *type_data();
    void update(obs_data_t *settings);
    bool active();

    void update();
    obs_data_t *settings();
    obs::properties properties();

    static std::string display_name(std::string id);
    static std::string codec(std::string id);
    static obs_encoder_type type(std::string id);
    static uint32_t caps(std::string id);
};

}