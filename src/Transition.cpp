#include "Transition.h"
#include "Input.h"
#include "Scene.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Transition::prototype;

Transition::Transition(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey)
 : handle(obs::transition(id, name, settings, hotkey))
{

}

Transition::Transition(std::string id, std::string name, obs_data_t *settings, bool is_private)
 : handle(obs::transition(id, name, settings, is_private))
{

}


Transition::Transition(obs::transition transition) 
 : handle(obs::transition(transition))
{
}

obs::source Transition::GetHandle()
{
    return handle.get().get();
}

NAN_MODULE_INIT(Transition::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Transition"));
    Nan::SetMethod(locProto, "create", create);
    Nan::SetMethod(locProto, "createPrivate", createPrivate);
    Nan::SetMethod(locProto, "types", types);
    Nan::SetMethod(locProto->InstanceTemplate(), "getActiveSource", getActiveSource);
    Nan::SetMethod(locProto->InstanceTemplate(), "start", start);
    Nan::SetMethod(locProto->InstanceTemplate(), "set", set);
    Nan::Set(target, FIELD_NAME("Transition"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Transition::types)
{
    auto type_list = obs::transition::types();
    int count = static_cast<int>(type_list.size());

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        Nan::Set(array, i, Nan::New<v8::String>(type_list[i]).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Transition::create)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 2);
    
    std::string id, name;
    obs_data_t *settings = nullptr, *hotkeys = nullptr;
    bool is_private = false;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], name);

    switch (info.Length()) {
    default:
    case 4:
        ASSERT_GET_VALUE(info[3], hotkeys);
    case 3:
        ASSERT_GET_VALUE(info[2], settings);
    case 2:
        break;
    }

    Transition *binding = new Transition(id, name, settings, hotkeys);
    auto object = Transition::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Transition::createPrivate)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 2);
    
    std::string id, name;
    obs_data_t *settings = nullptr;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], name);

    Transition *binding;

    if (info.Length() > 2)
        ASSERT_GET_VALUE(info[2], settings);

    binding = new Transition(id, name, settings, true);
    auto object = Transition::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Transition::getActiveSource)
{
    obs::weak<obs::transition> &handle = Transition::Object::GetHandle(info.Holder());

    obs::source source = handle.get()->get_active_source();
    
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

    obs_source_release(source.dangerous());
}

NAN_METHOD(Transition::clear)
{
    obs::weak<obs::transition> &handle = Transition::Object::GetHandle(info.Holder());

    handle.get()->clear();
}

NAN_METHOD(Transition::set)
{
    obs::weak<obs::transition> &handle = Transition::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    v8::Local<v8::Object> source_object;

    ASSERT_GET_VALUE(info[0], source_object);

    /* We don't know what type of source this is... so fetch
       the source interface instead. */
    obs::source source = ISource::GetHandle(source_object);
    handle.get()->set(source);
}

NAN_METHOD(Transition::start)
{
    obs::weak<obs::transition> &handle = Transition::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 2);

    int ms;
    v8::Local<v8::Object> source_object;

    ASSERT_GET_VALUE(info[0], ms);
    ASSERT_GET_VALUE(info[1], source_object);
    /* We don't know what type of source this is... so fetch
       the source interface instead. */
    obs::source source = ISource::GetHandle(source_object);

    handle.get()->start(ms, source);
}

}