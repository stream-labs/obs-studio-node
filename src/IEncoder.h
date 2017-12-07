#pragma once

#include <nan.h>
#include "obspp/obspp-encoder.hpp"

#define UNWRAP_IENCODER(object, name) \

namespace osn {

class IEncoderHandle {
public:
	virtual obs::encoder GetHandle() = 0;
};

class IEncoder : public IEncoderHandle, public Nan::ObjectWrap
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    static obs::encoder GetHandle(v8::Local<v8::Object> object);

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(get_displayName);
    static NAN_METHOD(get_name);
    static NAN_METHOD(set_name);
    static NAN_METHOD(get_id);
    static NAN_METHOD(get_caps);
    static NAN_METHOD(get_type);
    static NAN_METHOD(get_codec);
    static NAN_METHOD(update);
    static NAN_METHOD(caps_static);
    static NAN_METHOD(type_static);
    static NAN_METHOD(codec_static);
    //NAN_GETTER(type_data) /*How to handle this? */
};

}