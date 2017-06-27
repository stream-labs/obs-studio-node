#pragma once

#include <nan.h>

#include "obspp/obspp-scene.hpp"

#include "ISource.h"
#include "Common.h"

namespace osn {

class Scene;

class Scene : public ISource
{
public:
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    typedef common::Object<Scene, obs::scene> Object;
    friend Object;

    obs::scene handle;

    Scene(std::string name);

    virtual obs::source* GetHandle();
    
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(create);
    
    static NAN_METHOD(add);
};

}