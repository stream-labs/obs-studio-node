#pragma once
#include <nan.h>

#include "obspp/obspp-module.hpp"
#include "Common.h"

namespace osn {


NAN_METHOD(add_path);
NAN_METHOD(load_all);
NAN_METHOD(log_loaded);

class Module : Nan::ObjectWrap {

private:
    static NAN_METHOD(New);

public:
    typedef common::Object<Module, obs::module> Object;
    friend Object;

    obs::module handle;

    Module(std::string path, std::string data_path);
    Module(obs::module &module);
    /* Prototype Scope */
    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(add_path);
    static NAN_METHOD(load_all);
    static NAN_METHOD(log_loaded);

    /* Instance Scope */
    static NAN_METHOD(initialize);
    static NAN_GETTER(fileName);
    static NAN_GETTER(name);
    static NAN_GETTER(author);
    static NAN_GETTER(description);
    static NAN_GETTER(binPath);
    static NAN_GETTER(dataPath);
    static NAN_GETTER(status);
};

}