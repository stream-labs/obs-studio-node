#pragma once

#include "ISource.h"
#include "obspp/obspp-filter.hpp"
#include "Common.h"

namespace osn {

class Filter : public ISource {
public:
    obs::filter handle;

    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef common::Object<Filter, obs::filter> Object;
    friend Object;

    Filter(std::string id, std::string name, obs_data_t *settings);
    Filter(obs::filter &filter);

    virtual obs::source *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);
};

}