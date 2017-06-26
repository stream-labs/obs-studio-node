#pragma once

#include "ISource.h"
#include "obspp/obspp-filter.hpp"

namespace osn {

class Filter : public ISource {
public:
    obs::filter handle;

    static Nan::Persistent<v8::FunctionTemplate> prototype;

    static v8::Local<v8::Object> GenerateObject(obs::filter filter);
    static obs::filter* GetHandle(v8::Local<v8::Object> object);

    Filter(std::string id, std::string name, obs_data_t *settings);
    Filter(obs::filter &filter);

    virtual obs::source *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
};

}