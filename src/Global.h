#pragma once

#include <nan.h>

namespace osn {

/* obs signal callbacks need to
 * check this. JS should expect
 * no more callbacks once shutdown
 * is called. If true, no JS events
 * should be injected into the libuv
 * event loop anymore. */
bool IsShutdown();

#define CHECK_SHUTDOWN() \
    if (osn::IsShutdown() == true) return

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
NAN_METHOD(setAudioMonitoringDevice);
NAN_METHOD(getAudioMonitoringDevice);

}
