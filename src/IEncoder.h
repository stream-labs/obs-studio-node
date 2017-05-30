#pragma once

#include <nan.h>
#include "obspp/obspp-encoder.hpp"

#define UNWRAP_IENCODER(object, name) \

namespace osn {

class IEncoderHandle {
public:
	virtual obs::encoder *GetHandle() = 0;
};

class IEncoder : public IEncoderHandle, public Nan::ObjectWrap
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    static obs::encoder* GetHandle(v8::Local<v8::Object> object);

    static NAN_MODULE_INIT(Init);
    static NAN_GETTER(display_name);
    static NAN_GETTER(name);
    static NAN_SETTER(name);
    static NAN_GETTER(id);
    static NAN_GETTER(caps);
    static NAN_GETTER(type);
    static NAN_GETTER(codec);
    static NAN_METHOD(update);
    static NAN_METHOD(caps_static);
    static NAN_METHOD(type_static);
    static NAN_METHOD(codec_static);
    static NAN_GETTER(types);
    //NAN_GETTER(type_data) /*How to handle this? */
};

}