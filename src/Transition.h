#pragma once

#include <nan.h>

#include "obspp/obspp-transition.hpp"
#include "ISource.h"
#include "Common.h"

namespace osn {

class Transition : public ISource {
public:
    obs::transition handle;

    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef common::Object<Transition, obs::transition> Object;
    friend Object;

    Transition(std::string id, std::string name, obs_data_t *settings);
    Transition(obs::transition transition);

    virtual obs::source *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);

    static NAN_METHOD(start);
};

}