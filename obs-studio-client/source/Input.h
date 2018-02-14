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

    typedef obs::weak<obs::input> weak_handle_t;
    typedef common::Object<Input, weak_handle_t> Object;
    friend Object;

    weak_handle_t handle;

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
    static NAN_METHOD(get_width);
    static NAN_METHOD(get_height);
    static NAN_METHOD(fromName);
    static NAN_METHOD(get_volume);
    static NAN_METHOD(set_volume);
    static NAN_METHOD(get_syncOffset);
    static NAN_METHOD(set_syncOffset);
    static NAN_METHOD(get_active);
    static NAN_METHOD(get_showing);
    static NAN_METHOD(get_audioMixers);
    static NAN_METHOD(set_audioMixers);
    static NAN_METHOD(copyFilters);
    static NAN_METHOD(get_filters);
    static NAN_METHOD(findFilter);
    static NAN_METHOD(addFilter);
    static NAN_METHOD(removeFilter);
    static NAN_METHOD(get_monitoringType);
    static NAN_METHOD(set_monitoringType);
    static NAN_METHOD(get_deinterlaceFieldOrder);
    static NAN_METHOD(set_deinterlaceFieldOrder);
    static NAN_METHOD(get_deinterlaceMode);
    static NAN_METHOD(set_deinterlaceMode);
    static NAN_METHOD(types);
};

}