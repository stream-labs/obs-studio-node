#include "obspp/obspp.hpp"

#include "Global.h"
#include "Common.h"

#include "Input.h"
#include "Scene.h"
#include "Transition.h"

namespace osn {

#define OBS_VALID \
    if (!obs::initialized()) \
        return;

NAN_MODULE_INIT(Init)
{
    auto ObsGlobal = Nan::New<v8::Object>();

    Nan::SetMethod(ObsGlobal, "startup", startup);
    Nan::SetMethod(ObsGlobal, "shutdown", shutdown);
    Nan::SetMethod(ObsGlobal, "getOutputSource", getOutputSource);
    Nan::SetMethod(ObsGlobal, "setOutputSource", setOutputSource);
    Nan::SetMethod(ObsGlobal, "getOutputFlagsFromId", getOutputFlagsFromId);
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

NAN_METHOD(getOutputFlagsFromId)
{
    std::string id;

    ASSERT_GET_VALUE(info[0], id);

    uint32_t flags = obs_get_source_output_flags(id.c_str());

    info.GetReturnValue().Set(flags);
}

NAN_METHOD(setOutputSource)
{
    uint32_t channel;
    v8::Local<v8::Object> source_object;

    ASSERT_INFO_LENGTH(info, 2);
    ASSERT_GET_VALUE(info[0], channel);
    
    if (info[1]->IsNull()) {
        obs::output(channel, obs::source(nullptr));
        return;
    }
    
    ASSERT_GET_VALUE(info[1], source_object);

    obs::source source = ISource::GetHandle(source_object);
    obs::output(channel, source);
}

NAN_METHOD(getOutputSource)
{
    ASSERT_INFO_LENGTH(info, 1);

    uint32_t channel;

    ASSERT_GET_VALUE(info[0], channel);

    obs::source source = obs::output(channel);

    if (source.type() == OBS_SOURCE_TYPE_INPUT) {
        Input *binding = new Input(source.dangerous());

        v8::Local<v8::Object> object = 
            Input::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }
    else if (source.type() == OBS_SOURCE_TYPE_SCENE) {
        Scene *binding = new Scene(source.dangerous());

        v8::Local<v8::Object> object = 
            Scene::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }
    else if (source.type() == OBS_SOURCE_TYPE_TRANSITION) {
        Transition *binding = new Transition(source.dangerous());

        v8::Local<v8::Object> object = 
            Transition::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }

    obs_source_release(source.dangerous());
}

}