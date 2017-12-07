#include "obspp/obspp-output.hpp"
#include "Common.h"

#include <nan.h>

namespace osn {

class Output : public Nan::ObjectWrap {
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef obs::weak<obs::output> weak_handle_t;
    typedef common::Object<Output, weak_handle_t> Object;
    friend Object;

    weak_handle_t handle;

    Output(obs::output output);
    Output(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey);

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(get_types);
    static NAN_METHOD(create);
    static NAN_METHOD(get_settings);
    static NAN_METHOD(get_properties);
    static NAN_METHOD(get_name);
    static NAN_METHOD(get_id);

    static NAN_METHOD(release);
    static NAN_METHOD(setMedia);
    static NAN_METHOD(getVideo);
    static NAN_METHOD(getAudio);
    static NAN_METHOD(get_mixer);
    static NAN_METHOD(set_mixer);
    static NAN_METHOD(setAudioEncoder);
    static NAN_METHOD(getAudioEncoder);
    static NAN_METHOD(setVideoEncoder);
    static NAN_METHOD(getVideoEncoder);
    static NAN_METHOD(get_service);
    static NAN_METHOD(set_service);
    static NAN_METHOD(setReconnectOptions);
    static NAN_METHOD(setPreferredSize);
    static NAN_METHOD(get_width);
    static NAN_METHOD(get_height);
    static NAN_METHOD(get_congestion);
    static NAN_METHOD(get_connectTime);
    static NAN_METHOD(get_reconnecting);
    static NAN_METHOD(get_supportedVideoCodecs);
    static NAN_METHOD(get_supportedAudioCodecs);
    static NAN_METHOD(get_framesDropped);
    static NAN_METHOD(get_totalFrames);
    static NAN_METHOD(start);
    static NAN_METHOD(stop);
    static NAN_METHOD(setDelay);
    static NAN_METHOD(getDelay);
    static NAN_METHOD(getActiveDelay);
};

}