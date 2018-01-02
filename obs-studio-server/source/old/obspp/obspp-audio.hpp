#pragma once

#include <string>
#include <obs.h>

#include "obspp-encoder.hpp"
#include "obspp-source.hpp"

namespace obs {

class audio {
    audio_t *m_handle;

public:
    enum status {
        okay,
        bad_param,
        invalid
    };

    audio(audio_t *ctx);
    ~audio();

    audio_t *dangerous();

    static obs::audio global();
};

class audio_encoder : public encoder {
    obs_encoder_t *m_handle;

public:
    audio_encoder(obs_encoder_t *encoder);
    audio_encoder(std::string id, std::string name, obs_data_t *settings = NULL, size_t idx = 0, obs_data_t *hotkeys = NULL);

    static std::vector<std::string> types();

    obs_encoder_t *dangerous();
    uint32_t sample_rate();

    void audio(obs::audio audio);
    obs::audio audio();
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