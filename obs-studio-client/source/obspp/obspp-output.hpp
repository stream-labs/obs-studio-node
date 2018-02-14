#pragma once

#include <vector>
#include <obs.h>

#include "obspp-weak.hpp"
#include "obspp-video.hpp"
#include "obspp-audio.hpp"
#include "obspp-service.hpp"

namespace obs {

class output {
public:
    enum status_type {
        okay,
        invalid
    };

private:
    obs_output_t *m_handle;
    status_type   m_status;

    output();

public:
    output(std::string &id, std::string &name, obs_data_t *settings = nullptr, obs_data_t *hotkey = nullptr);

    output(output &copy);
    output(obs_output_t *output);

    void release();
    status_type status();
    obs_output_t *dangerous();

    std::string name();
    std::string id();

    template <typename T>
    output(obs::strong<T> &output) : m_handle(output->dangerous())
    {
        if (!m_handle) m_status = status_type::invalid;
    }

    static std::vector<std::string> types();
    static obs_data_t *defaults(std::string id);
    static output from_name(std::string name);

    /* Association functions */
    void media(obs::video video, obs::audio audio);
    obs::video video();
    obs::audio audio();

    /* Sets track for audio output. */
    void mixer(size_t idx);
    size_t mixer();

    void video_encoder(obs::video_encoder encoder);
    void audio_encoder(obs::audio_encoder audio, size_t idx);

    obs::video_encoder video_encoder();
    obs::audio_encoder audio_encoder(size_t idx);

    void service(obs::service service);
    obs::service service();

    void reconnect_options(int retry_count, int retry_sec);
    void preferred_size(uint32_t width, uint32_t height);

    uint32_t width();
    uint32_t height();

    float congestion();
    int connect_time();
    bool reconnecting();

    std::string supported_video_codecs();
    std::string supported_audio_codecs();

    int frames_dropped();
    int total_frames();

    void start();
    void stop();
    void delay(uint32_t ms, uint32_t flags);
    uint32_t delay();
    uint32_t active_delay();

    /** Configurable concept */
    bool configurable();
    obs::properties properties();
    obs_data_t *settings();
    void update(obs_data_t *data);
};

}