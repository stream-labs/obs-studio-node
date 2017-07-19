#pragma once

#include <string>
#include <obs.h>

#include "obspp-encoder.hpp"
#include "obspp-source.hpp"

namespace obs {

class audio {
    audio_t *handle;

public:
    enum status {
        okay,
        bad_param,
        invalid
    };

    audio(audio_t *ctx);
    audio(struct audio_output_info *info);
    ~audio();
};

class audio_encoder : public encoder {
    obs_encoder_t *handle;

public:
    audio_encoder(std::string id, std::string name);
};

class fader {
    obs_fader_t *m_handle;

public:
    fader(obs_fader_type type);
    ~fader();

    void db(const float ms);
    float db();

    void deflection(const float def);
    float deflection();

    void mul(const float mul);
    float mul();

    void attach(obs::source &source);
    void detach();

    void add_callback(obs_fader_changed_t cb, void *data);
    void remove_callback(obs_fader_changed_t cb, void *data);
};

class volmeter {
    obs_volmeter_t *m_handle;

public:
    volmeter(obs_fader_type type);
    ~volmeter();

    void attach(obs::source &source);
    void detach();

    void interval(int ms);
    int interval();

    void peak_hold(int hold_time);
    int peak_hold();

    void add_callback(obs_volmeter_updated_t cb, void *data);
    void remove_callback(obs_volmeter_updated_t cb, void *data);
};

}