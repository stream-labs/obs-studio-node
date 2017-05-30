#pragma once

#include <nan.h>

#include "obspp/obspp-scene.hpp"

#include "ISource.h"

namespace osn {

class Scene : public ISource
{
public:
    obs::scene handle;

    Scene(std::string name);

    virtual obs::source* GetHandle();
    static obs::scene *GetHandle(v8::Local<v8::Object> object);
    
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);
    
    static NAN_METHOD(add);
};

}