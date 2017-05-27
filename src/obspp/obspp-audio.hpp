#pragma once

#include <obs.h>

namespace obs {

class audio {
    audio_t *handle;

public:
    enum status {
        okay,
        bad_param,
        invalid
    };

    audio(audio_t *ctx) : handle(ctx) { }
    audio(struct audio_output_info *info);
    ~audio() { }
};

}