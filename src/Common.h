#pragma once

#include <nan.h>

#define FIELD_NAME(name) \
    Nan::New(name).ToLocalChecked()