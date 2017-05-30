#pragma once

#include <nan.h>
#include "obspp/obspp-source.hpp"

namespace osn {

class ISourceHandle {
public:
	virtual obs::source *GetHandle() = 0;
};

class ISource : public ISourceHandle, public Nan::ObjectWrap
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    static obs::source* GetHandle(v8::Local<v8::Object> object);

    static NAN_MODULE_INIT(Init);
    static NAN_GETTER(name);
    static NAN_SETTER(name);
    static NAN_METHOD(remove);
    static NAN_METHOD(release);
    static NAN_GETTER(status);
    static NAN_GETTER(id);
    static NAN_GETTER(configurable);
    static NAN_GETTER(width);
    static NAN_GETTER(height);
};

}