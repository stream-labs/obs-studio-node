#pragma once

#include "obspp/obspp-input.hpp"
#pragma once

#include "ISource.h"
#include "Video.h"
#include "Common.h"

namespace osn {

class Input : public ISource
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef common::Object<Input, obs::weak<obs::input>> Object;
    friend Object;

    obs::weak<obs::input> handle;
    bool released;

    Input(obs::input input);
    Input(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey);
    Input(std::string id, std::string name, obs_data_t *settings, bool is_private);

    virtual obs::source GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(getPublicSources);
    static NAN_METHOD(duplicate);
    static NAN_METHOD(create);
    static NAN_METHOD(createPrivate);
    static NAN_METHOD(fromName);
    static NAN_GETTER(volume);
    static NAN_SETTER(volume);
    static NAN_GETTER(syncOffset);
    static NAN_SETTER(syncOffset);
    static NAN_GETTER(showing);
    static NAN_GETTER(audioMixers);
    static NAN_SETTER(audioMixers);
    static NAN_GETTER(filters);
    static NAN_METHOD(findFilter);
    static NAN_METHOD(addFilter);
    static NAN_METHOD(removeFilter);
    static NAN_GETTER(monitoringType);
    static NAN_SETTER(monitoringType);
    static NAN_GETTER(deinterlaceFieldOrder);
    static NAN_SETTER(deinterlaceFieldOrder);
    static NAN_GETTER(deinterlaceMode);
    static NAN_SETTER(deinterlaceMode);
    static NAN_METHOD(types);
};

}