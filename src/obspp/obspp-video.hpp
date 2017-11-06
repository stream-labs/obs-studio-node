#pragma once

#include <obs.h>
#include "obspp-input.hpp"
#include "obspp-encoder.hpp"

namespace obs {

class video {
    video_t *handle;

public:
    video(video_t *video);
    ~video();

    static bool active();
    static void stop();
    static bool stopped();
    static enum video_format format();
    static uint32_t height();
    static uint32_t width();
    static uint32_t frame_rate();
    static uint32_t skipped_frames();
    static uint32_t total_frames();
    static int reset(struct obs_video_info *info);
};

class video_encoder : public encoder {
    obs_encoder_t *handle;

public:
    video_encoder(std::string id, std::string name);
};

}