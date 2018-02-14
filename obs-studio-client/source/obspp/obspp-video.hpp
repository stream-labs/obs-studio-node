#pragma once

#include <obs.h>
#include "obspp-input.hpp"
#include "obspp-encoder.hpp"

namespace obs {

class video {
public:
    enum status_type {
        Success = 0,
        InvalidParam = -1,
        GenericFailure = -2
    };

private:
    video_t *m_handle;
    status_type m_status;

public:
    video(video_t *video);
    video(video_output_info *info);
    ~video();

    void close();
    bool active();
    void stop();
    bool stopped();
    enum video_format format();
    uint32_t height();
    uint32_t width();
    uint32_t frame_rate();
    uint32_t skipped_frames();
    uint32_t total_frames();
    static int reset(struct obs_video_info *info);

    video_t *dangerous();

    static obs::video global();
};

class video_encoder : public encoder {
public:
    video_encoder(obs_encoder_t *encoder);
    video_encoder(std::string id, std::string name, obs_data_t *settings = nullptr, obs_data_t *hotkeys = nullptr);

    static video_encoder from_name(std::string name);
    static std::vector<std::string> types();

    obs_encoder_t *dangerous();

    obs::video video();
    void video(obs::video video);

    void preferred_format(video_format format);
    video_format preferred_format();

    void scaled_size(uint32_t width, uint32_t height);
    uint32_t height();
    uint32_t width();
};

}