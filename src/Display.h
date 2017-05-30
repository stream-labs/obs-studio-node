#pragma once

#include "obspp/obspp-display.hpp"

#include <nan.h>

namespace osn {

class Display : public Nan::ObjectWrap
{
public:
    obs::display handle;

    Display(gs_init_data &data);

    static NAN_MODULE_INIT(Init);
    static NAN_METHOD(New);

    /* See RenderPlugin.h for more information
     * on how actual drawing is done (hint: it's
     * not done in javascript) */
    static NAN_GETTER(status);
    static NAN_METHOD(destroy);
    static NAN_METHOD(add_drawer);
    static NAN_METHOD(remove_drawer);
    static NAN_METHOD(resize);
    static NAN_GETTER(enabled);
    static NAN_SETTER(enabled);
    static NAN_SETTER(background_color);
};

};