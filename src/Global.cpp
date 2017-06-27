#include "obspp/obspp.hpp"

#include "Global.h"
#include "Common.h"

namespace osn {

#define OBS_VALID \
    if (obs::status() != obs::status_type::okay) \
        return;

NAN_MODULE_INIT(Init)
{
    auto ObsGlobal = Nan::New<v8::Object>();

    Nan::SetMethod(ObsGlobal, "startup", startup);
    Nan::SetMethod(ObsGlobal, "shutdown", shutdown);
    Nan::SetAccessor(ObsGlobal, FIELD_NAME("status"), status);
    Nan::SetAccessor(ObsGlobal, FIELD_NAME("locale"), locale);

    Nan::Set(target, FIELD_NAME("Global"), ObsGlobal);
}

NAN_METHOD(startup)
{
    Nan::Utf8String locale(info[0]);
    Nan::Utf8String path(info[1]);

    auto ReturnValue = info.GetReturnValue();

    switch (info.Length()) {
    case 0:
        Nan::ThrowError("Locale must be specified");
        break;
    case 1:
        ReturnValue.Set(obs::startup(*locale));
        break;
    case 2:
        ReturnValue.Set(obs::startup(*locale, *path));
        break;
    default:
        Nan::ThrowError("Incorrect number of arguments");
    }
}

NAN_METHOD(shutdown)
{
    obs::shutdown();
}

NAN_GETTER(locale)
{
    OBS_VALID

    info.GetReturnValue().Set(
        Nan::New(obs::locale()).ToLocalChecked());
}

NAN_SETTER(locale)
{
    OBS_VALID

    Nan::Utf8String locale(value);

    obs::locale(*locale);
}

NAN_GETTER(status)
{
    info.GetReturnValue().Set(obs::status());
}

NAN_GETTER(version)
{
    info.GetReturnValue().Set(obs::version());
}

}