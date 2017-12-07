#pragma once

#include <nan.h>
#include "IEncoder.h"
#include "obspp/obspp-video.hpp"
#include "obspp/obspp.hpp"

namespace osn {

class Video : public Nan::ObjectWrap
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    Video(obs::video video);
    Video(video_output_info *info);

    typedef common::Object<Video, obs::video> Object;
    friend Object;

    obs::video handle;

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    static NAN_METHOD(reset);
    static NAN_METHOD(get_skippedFrames);
    static NAN_METHOD(get_totalFrames);
};

class VideoEncoder : public IEncoder
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef obs::weak<obs::video_encoder> weak_handle_t;
    typedef common::Object<VideoEncoder, weak_handle_t> Object;
    friend Object;

    weak_handle_t handle;

    VideoEncoder(obs::video_encoder encoder);
    VideoEncoder(std::string id, std::string name, obs_data_t *settings = nullptr, obs_data_t *hotkeys = nullptr);
    
    virtual obs::encoder GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);
    static NAN_METHOD(get_height);
    static NAN_METHOD(get_width);
    static NAN_METHOD(get_scaledSize);
    static NAN_METHOD(get_preferredFormat);
    static NAN_METHOD(set_preferredFormat);
};

}