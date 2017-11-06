#pragma once

#include "obspp-video.hpp"

namespace obs {

video::video(video_t *video)
 : handle(video)
{

}

video::~video()
{

}

int video::reset(struct obs_video_info *info)
{
    return obs_reset_video(info);
}

bool video::active()
{
    return video_output_active(obs_get_video());
}

void video::stop()
{
    video_output_stop(obs_get_video());
}

bool video::stopped()
{
    return video_output_stopped(obs_get_video());
}

enum video_format video::format()
{
    return video_output_get_format(obs_get_video());
}

uint32_t video::height()
{
    return video_output_get_height(obs_get_video());
}

uint32_t video::width()
{
    return video_output_get_width(obs_get_video());
}

uint32_t video::frame_rate()
{
    return video_output_get_frame_rate(obs_get_video());
}

uint32_t video::skipped_frames()
{
    return video_output_get_skipped_frames(obs_get_video());
}

uint32_t video::total_frames()
{
    return video_output_get_total_frames(obs_get_video());
}

/* Video Encoder */
video_encoder::video_encoder(std::string id, std::string name)
 : handle(obs_video_encoder_create(id.c_str(), name.c_str(), NULL, NULL))
{

}

}