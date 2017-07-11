#pragma once

#include <nan.h>

#include "obspp/obspp-transition.hpp"
#include "ISource.h"
#include "Common.h"

namespace osn {

class Transition : public ISource {
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef obs::weak<obs::transition> weak_handle_t;
    typedef common::Object<Transition, weak_handle_t> Object;
    friend Object;

    weak_handle_t handle;

    Transition(std::string id, std::string name, obs_data_t *settings);
    Transition(obs::transition transition);

    virtual obs::source GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(types);
    static NAN_METHOD(create);
    static NAN_METHOD(set);

    static NAN_METHOD(start);
};

}