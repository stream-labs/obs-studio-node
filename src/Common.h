#pragma once

#define FIELD_NAME(name) \
    Nan::New(name).ToLocalChecked()
    