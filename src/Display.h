#pragma once

#include "obspp/obspp-display.hpp"
#include "Common.h"

#include <nan.h>

namespace osn {

class Display : public Nan::ObjectWrap
{
public:
    typedef common::Object<Display, obs::display> Object;
    friend Object;

    obs::display handle;

    Display(gs_init_data &data);

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);

    /* See RenderPlugin.h for more information
     * on how actual drawing is done (hint: it's
     * not done in javascript) */
    static NAN_GETTER(status);
    static NAN_METHOD(destroy);
    static NAN_METHOD(addDrawer);
    static NAN_METHOD(removeDrawer);
    static NAN_METHOD(resize);
    static NAN_GETTER(enabled);
    static NAN_SETTER(enabled);
    static NAN_SETTER(backgroundColor);
};

};