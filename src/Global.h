#pragma once

#include <nan.h>

namespace osn {

NAN_MODULE_INIT(Init);
NAN_METHOD(startup);
NAN_METHOD(shutdown);
NAN_METHOD(getOutputFlagsFromId);
NAN_METHOD(getOutputSource);
NAN_METHOD(setOutputSource);
NAN_GETTER(locale);
NAN_SETTER(locale);
NAN_GETTER(initialized);
NAN_GETTER(version);

}
