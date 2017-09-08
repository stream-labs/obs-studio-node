#pragma once

#include <nan.h>

#include "obspp/obspp-scene.hpp"

#include "ISource.h"
#include "Common.h"

namespace osn {

class Scene : public ISource
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef obs::weak<obs::scene> weak_handle_t;
    typedef common::Object<Scene, weak_handle_t> Object;
    friend Object;

    weak_handle_t handle;

    Scene(std::string name, bool is_private = false);
    Scene(obs::scene handle);

    virtual obs::source GetHandle();
    
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);
    static NAN_METHOD(createPrivate);
    static NAN_METHOD(fromName);
    static NAN_METHOD(duplicate);
    static NAN_GETTER(source);
    static NAN_METHOD(moveItem);
    static NAN_METHOD(findItem);
    static NAN_METHOD(getItems);
    static NAN_METHOD(getItemAtIdx);
    static NAN_METHOD(add);
    static NAN_METHOD(connect);
};

}