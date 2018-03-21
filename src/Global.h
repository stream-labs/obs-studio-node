#pragma once

#include <nan.h>

namespace osn {

NAN_MODULE_INIT(Init);
NAN_METHOD(startup);
NAN_METHOD(shutdown);
NAN_METHOD(getOutputFlagsFromId);
NAN_METHOD(getOutputSource);
NAN_METHOD(setOutputSource);
NAN_METHOD(get_locale);
NAN_METHOD(set_locale);
NAN_METHOD(initialized);
NAN_METHOD(version);
NAN_METHOD(laggedFrames);
NAN_METHOD(totalFrames);
NAN_METHOD(getProperties);
NAN_METHOD(getActiveFps);
NAN_METHOD(getAudioMonitoringDevices);

}
