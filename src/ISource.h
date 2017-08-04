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
    static NAN_GETTER(type);
    static NAN_GETTER(name);
    static NAN_SETTER(name);
    static NAN_GETTER(flags);
    static NAN_SETTER(flags);
    static NAN_METHOD(remove);
    static NAN_METHOD(release);
    static NAN_GETTER(status);
    static NAN_GETTER(id);
    static NAN_GETTER(configurable);
    static NAN_GETTER(properties);
    static NAN_GETTER(settings);
    static NAN_METHOD(update);
    static NAN_GETTER(muted);
    static NAN_SETTER(muted);
    static NAN_GETTER(enabled);
    static NAN_SETTER(enabled);
    static NAN_GETTER(width);
    static NAN_GETTER(height);
};

}