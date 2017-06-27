#pragma once

#include "obspp/obspp-input.hpp"
#pragma once

#include "ISource.h"
#include "Video.h"
#include "Common.h"

namespace osn {

class Input : public ISource
{
    /* Anything that wants to make a new object from 
     * this class must be a friend. For once, using friends
     * in this case is good since it helps keep track of 
     * functions and objects that interact with this one. */
     friend NAN_METHOD(Video::output);

public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef common::Object<Input, obs::input> Object;
    friend Object;

    obs::input handle;

    Input(obs::input &input);
    Input(std::string id, std::string name, obs_data_t *hotkey, obs_data_t *settings);
    Input(std::string id, std::string name, obs_data_t *settings, bool is_private);

    virtual obs::source *GetHandle();
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);
    static NAN_GETTER(volume);
    static NAN_SETTER(volume);
    static NAN_GETTER(sync_offset);
    static NAN_SETTER(sync_offset);
    static NAN_GETTER(showing);
    static NAN_GETTER(flags);
    static NAN_SETTER(flags);
    static NAN_GETTER(audio_mixers);
    static NAN_SETTER(audio_mixers);
    static NAN_METHOD(add_filter);
    static NAN_METHOD(remove_filter);
};

}