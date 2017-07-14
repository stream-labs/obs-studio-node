#include "obspp/obspp.hpp"

#include "Global.h"
#include "Common.h"

namespace osn {

#define OBS_VALID \
    if (!obs::initialized()) \
        return;

NAN_MODULE_INIT(Init)
{
    auto ObsGlobal = Nan::New<v8::Object>();

    Nan::SetMethod(ObsGlobal, "startup", startup);
    Nan::SetMethod(ObsGlobal, "shutdown", shutdown);
    Nan::SetAccessor(ObsGlobal, FIELD_NAME("initialized"), initialized);
    Nan::SetAccessor(ObsGlobal, FIELD_NAME("locale"), locale);

    Nan::Set(target, FIELD_NAME("Global"), ObsGlobal);
}

/* Technically, we can control logs from JS. This will require a bit of
   engineering since logging might be executed in a secondary thread. 
   For now, just pick a spot and keep it there.  */

NAN_METHOD(startup)
{
    std::string locale, path;

    auto ReturnValue = info.GetReturnValue();

    ASSERT_INFO_LENGTH_AT_LEAST(info, 1);
    ASSERT_GET_VALUE(info[0], locale);

    switch (info.Length()) {
    case 1:
        ReturnValue.Set(common::ToValue(obs::startup(locale)));
        break;
    case 2:
        ASSERT_GET_VALUE(info[1], path);
        ReturnValue.Set(common::ToValue(obs::startup(locale, path)));
        break;
    }
}

NAN_METHOD(shutdown)
{
    obs::shutdown();
}

NAN_GETTER(locale)
{
    OBS_VALID

    info.GetReturnValue().Set(common::ToValue(obs::locale()));
}

NAN_SETTER(locale)
{
    OBS_VALID

    std::string locale;

    ASSERT_GET_VALUE(value, locale);

    obs::locale(locale);
}

NAN_GETTER(initialized)
{
    info.GetReturnValue().Set(common::ToValue(obs::initialized()));
}

NAN_GETTER(version)
{
    info.GetReturnValue().Set(common::ToValue(obs::version()));
}

}