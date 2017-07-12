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

NAN_GETTER(status)
{
    info.GetReturnValue().Set(obs::status());
}

NAN_GETTER(version)
{
    info.GetReturnValue().Set(common::ToValue(obs::version()));
}

}