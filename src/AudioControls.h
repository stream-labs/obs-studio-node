#pragma once

#include "obspp/obspp-audio.hpp"

#include "Async.h"
#include "Common.h"

namespace osn {

class Fader : public obs::fader, public Nan::ObjectWrap {
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;
    obs::fader &handle;

    struct Data {
        void *param;
        float db;
    };

    typedef common::Object<Fader, obs::fader> Object;
    friend Object;

    static void Callback(Fader *fader, Fader::Data *item);

    Fader(obs_fader_type type);

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);

    static NAN_GETTER(db);
    static NAN_SETTER(db);

    static NAN_GETTER(deflection);
    static NAN_SETTER(deflection);

    static NAN_GETTER(mul);
    static NAN_SETTER(mul);

    static NAN_METHOD(attach);
    static NAN_METHOD(detach);

    static NAN_METHOD(addCallback);
    static NAN_METHOD(removeCallback);
};

typedef common::CallbackData<Fader::Data, Fader> FaderCallback;

class Volmeter : public obs::volmeter, public Nan::ObjectWrap {
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;
    obs::volmeter &handle;

    struct Data {
        void *param;
        float level;
        float magnitude;
        float peak;
        bool muted;
    };

    static void Callback(Volmeter *volmeter, Volmeter::Data *item);

    typedef common::Object<Volmeter, obs::volmeter> Object;
    friend Object;

    Volmeter(obs_fader_type type);
    
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);

    static NAN_GETTER(peakHold);
    static NAN_SETTER(peakHold);
    
    static NAN_GETTER(updateInterval);
    static NAN_SETTER(updateInterval);

    static NAN_METHOD(attach);
    static NAN_METHOD(detach);

    static NAN_METHOD(addCallback);
    static NAN_METHOD(removeCallback);
};

typedef common::CallbackData<Volmeter::Data, Volmeter> VolmeterCallback;

}