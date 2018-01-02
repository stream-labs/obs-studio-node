#pragma once

#include "ISource.h"
#include "obspp/obspp-filter.hpp"
#include "Common.h"

namespace osn {

class Filter : public ISource {
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef obs::weak<obs::filter> weak_handle_t;
    typedef common::Object<Filter, weak_handle_t> Object;
    friend Object;

    weak_handle_t handle;

    Filter(std::string id, std::string name, obs_data_t *settings);
    Filter(obs::filter filter);

    virtual obs::source GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(types);
    static NAN_METHOD(create);
};

}