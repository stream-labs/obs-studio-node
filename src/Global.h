#pragma once

#include <nan.h>

namespace osn {

NAN_METHOD(startup);
NAN_METHOD(shutdown);
NAN_GETTER(locale);
NAN_SETTER(locale);
NAN_GETTER(status);
NAN_GETTER(version);
NAN_METHOD(reset_video);
NAN_METHOD(load_draw_plugin);

}
