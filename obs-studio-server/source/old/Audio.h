#pragma once

#include <nan.h>

#include "obspp/obspp-audio.hpp"

#include "Common.h"
#include "IEncoder.h"

namespace osn {

class Audio : public Nan::ObjectWrap
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef common::Object<Audio, obs::audio> Object;
    friend Object;

    obs::audio handle;

    Audio(audio_t *audio);
    Audio(obs::audio audio);

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    static NAN_METHOD(reset);
    static NAN_METHOD(getGlobal);
};

class AudioEncoder : public IEncoder
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef obs::weak<obs::audio_encoder> weak_handle_t;
    typedef common::Object<AudioEncoder, weak_handle_t> Object;
    friend Object;

    weak_handle_t handle;

    AudioEncoder(std::string id, std::string name, obs_data_t *settings = NULL, size_t idx = 0, obs_data_t *hotkeys = NULL);
    AudioEncoder(obs::audio_encoder encoder);

    virtual obs::encoder GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);
    static NAN_METHOD(getAudio);
    static NAN_METHOD(setAudio);
    static NAN_METHOD(getSampleRate);
};

}