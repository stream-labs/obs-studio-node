#pragma once

#include <nan.h>

#include "obspp/obspp-transition.hpp"
#include "ISource.h"

namespace osn {

class Transition : public ISource {
public:
    obs::transition handle;

    static Nan::Persistent<v8::FunctionTemplate> prototype;

    static v8::Local<v8::Object> GenerateObject(obs::transition transition);
    static obs::transition* GetHandle(v8::Local<v8::Object> object);

    Transition(std::string id, std::string name, obs_data_t *settings);
    Transition(obs::transition transition);

    virtual obs::source *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);

    static NAN_METHOD(start);
};

}