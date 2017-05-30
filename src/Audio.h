#pragma once

#include <nan.h>
#include "IEncoder.h"
#include "obspp/obspp-audio.hpp"

namespace osn {

class Audio : public Nan::ObjectWrap
{
public:
    obs::audio handle;

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    static NAN_METHOD(reset);
};

class AudioEncoder : public IEncoder
{
public:
    obs::audio_encoder handle;
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    AudioEncoder(std::string id, std::string name);

    virtual obs::encoder *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    static NAN_GETTER(sample_rate);
};

}