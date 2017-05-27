#pragma once

#include <obs.h>

namespace obs {

class video {
    video_t *handle;

public:
    video(video_t *video) : handle(video) { }
    ~video() { }
};

}