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

    static int reset(struct obs_video_info *info);
    static void output(int channel, input &source);
    static input output(int channel);
};

class video_encoder : public encoder {
    obs_encoder_t *handle;

public:
    video_encoder(std::string id, std::string name);
};

}