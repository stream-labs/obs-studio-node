#include "Scene.h"
#include "SceneItem.h"
#include "Input.h"

namespace osn {

obs::source *Scene::GetHandle()
{
    return static_cast<obs::source*>(&handle);
}

Scene::Scene(std::string name)
 : handle(name)
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

    Nan::SetMethod(locProto->PrototypeTemplate(), "add", add);
    Nan::SetMethod(locProto, "create", create);

    Nan::Set(target, FIELD_NAME("Scene"), locProto->GetFunction());
    prototype.Reset(locProto);
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

NAN_METHOD(Scene::add)
{
    obs::scene *scene = Scene::Object::GetHandle(info.Holder());

    if (info.Length() != 1) {
        Nan::ThrowError("Expected one argument");
        return;
    }

    obs::input *input = Input::Object::GetHandle(info[0]->ToObject());

    auto item_object = SceneItem::GenerateObject(scene->add(*input));

    info.GetReturnValue().Set(item_object);
}

}