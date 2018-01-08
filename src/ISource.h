#pragma once

#include <nan.h>
#include "obspp/obspp-source.hpp"
#include "obspp/obspp-weak.hpp"
#include "Common.h"

namespace osn {

class ISourceHandle {
public:
	virtual obs::source GetHandle() = 0;
};

class ISource : public ISourceHandle, public Nan::ObjectWrap
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    static obs::source GetHandle(v8::Local<v8::Object> object);

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(get_type);
    static NAN_METHOD(get_name);
    static NAN_METHOD(set_name);
    static NAN_METHOD(get_outputFlags);
    static NAN_METHOD(get_flags);
    static NAN_METHOD(set_flags);
    static NAN_METHOD(remove);
    static NAN_METHOD(release);
    static NAN_METHOD(save);
    static NAN_METHOD(get_status);
    static NAN_METHOD(get_id);
    static NAN_METHOD(get_configurable);
    static NAN_METHOD(get_properties);
    static NAN_METHOD(get_settings);
    static NAN_METHOD(update);
    static NAN_METHOD(get_muted);
    static NAN_METHOD(set_muted);
    static NAN_METHOD(get_enabled);
    static NAN_METHOD(set_enabled);
};

}