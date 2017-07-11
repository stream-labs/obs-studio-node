#include "Scene.h"
#include "SceneItem.h"
#include "Input.h"

namespace osn {

obs::source Scene::GetHandle()
{
    return handle.get();
}

Scene::Scene(std::string name)
 : handle(obs::scene(name))
{
}

Scene::Scene(obs::scene handle)
 : handle(obs::scene(handle))
{
}

Nan::Persistent<v8::FunctionTemplate> Scene::prototype =
    Nan::Persistent<v8::FunctionTemplate>();

NAN_MODULE_INIT(Scene::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->SetClassName(FIELD_NAME("Scene"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(locProto->PrototypeTemplate(), "getItems", getItems);
    Nan::SetMethod(locProto->PrototypeTemplate(), "add", add);
    Nan::SetMethod(locProto, "getCurrentListDeprecated", getCurrentListDeprecated);
    Nan::SetMethod(locProto, "create", create);
    Nan::SetMethod(locProto, "fromName", fromName);

    Nan::Set(target, FIELD_NAME("Scene"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Scene::getCurrentListDeprecated)
{
    std::vector<obs_source_t *> sources;

    auto cb = [] (void *data, obs_source_t *source) -> bool {
        std::vector<obs_source_t *> *sources = 
            reinterpret_cast<std::vector<obs_source_t *>*>(data);

        if (obs_source_get_type(source) == OBS_SOURCE_TYPE_SCENE)
            sources->push_back(obs_source_get_ref(source));

        return true;
    };

    obs_enum_sources(cb, &sources);
    auto array = Nan::New<v8::Array>(static_cast<int>(sources.size()));

    for (int i = 0; i < sources.size(); ++i) {
        Scene *binding = new Scene(obs::scene(sources[i]));
        auto object = Scene::Object::GenerateObject(binding);
        Nan::Set(array, i, object);
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Scene::create)
{
    if (info.Length() != 1) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }
    if (!info[0]->IsString()) {
        Nan::ThrowError("Invalid type passed");
        return;
    }

    Nan::Utf8String name(info[0]);

    Scene *binding = new Scene(*name);
    auto object = Scene::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}


NAN_METHOD(Scene::fromName)
{
    if (info.Length() != 1) {
        Nan::ThrowError("Unexpected arguments");
        return;
    }

    if (!info[0]->IsString()) {
        Nan::ThrowError("Expected string");
        return;
    }

    Nan::Utf8String name(info[0]);
    Scene *binding = new Scene(obs::scene::from_name(*name));
    auto object = Scene::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Scene::getItems)
{
    obs::weak<obs::scene> &scene = Scene::Object::GetHandle(info.Holder());
    std::vector<obs::scene::item> items = scene.get()->items();
    int size = static_cast<int>(items.size());
    auto array = Nan::New<v8::Array>(size);

    for (int i = 0; i < size; ++i) {
        SceneItem *binding = new SceneItem(items[i]);
        auto object = SceneItem::Object::GenerateObject(binding);
        Nan::Set(array, i, object);
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Scene::add)
{
    obs::weak<obs::scene> &scene = Scene::Object::GetHandle(info.Holder());

    if (info.Length() != 1) {
        Nan::ThrowError("Expected one argument");
        return;
    }

    if (!info[0]->IsObject()) {
        Nan::ThrowError("Expected object");
        return;
    }

    obs::weak<obs::input> &input = 
        Input::Object::GetHandle(
            Nan::To<v8::Object>(info[0]).ToLocalChecked());

    SceneItem *binding = new SceneItem(scene.get()->add(input.get().get()));
    auto object = SceneItem::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

}