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
    static NAN_GETTER(skippedFrames);
    static NAN_GETTER(totalFrames);
};

class VideoEncoder : public IEncoder
{
public:
    obs::video_encoder handle;
    VideoEncoder(std::string id, std::string name);
    
    virtual obs::encoder *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    static NAN_GETTER(height);
    static NAN_GETTER(width);
    static NAN_SETTER(scaledSize);
    static NAN_SETTER(preferredFormat);
    static NAN_GETTER(preferredFormat);
};

}