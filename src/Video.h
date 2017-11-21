#pragma once

#include <nan.h>
#include "IEncoder.h"
#include "obspp/obspp-video.hpp"
#include "obspp/obspp.hpp"

namespace osn {

class Video : public Nan::ObjectWrap
{
public:
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
    obs::video_encoder handle;
    VideoEncoder(std::string id, std::string name);
    
    virtual obs::encoder *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    static NAN_METHOD(get_height);
    static NAN_METHOD(get_width);
    static NAN_METHOD(get_scaledSize);
    static NAN_METHOD(get_preferredFormat);
    static NAN_METHOD(set_preferredFormat);
};

}