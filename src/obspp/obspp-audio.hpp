#pragma once

#include <string>
#include <obs.h>

#include "obspp-encoder.hpp"

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

}