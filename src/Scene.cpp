#include "Scene.h"
#include "SceneItem.h"
#include "Input.h"
#include "Common.h"

namespace osn {

obs::scene *Scene::GetHandle(v8::Local<v8::Object> object)
{
    Scene* scene = Nan::ObjectWrap::Unwrap<Scene>(object);
    return &scene->handle;
}

obs::source *Scene::GetHandle()
{
    return static_cast<obs::source*>(&handle);
}

Scene::Scene(std::string name)
 : handle(name)
{
}

NAN_MODULE_INIT(Scene::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->SetClassName(FIELD_NAME("Scene"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(locProto->PrototypeTemplate(), "add", add);

    Nan::Set(target, FIELD_NAME("Scene"), locProto->GetFunction());
}

NAN_METHOD(Scene::New)
{
    if (!info.IsConstructCall()) {
        Nan::ThrowError("Must be used as a construct call");
        return;
    }
    if (info.Length() != 1) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }
    if (!info[0]->IsString()) {
        Nan::ThrowError("Invalid type passed");
        return;
    }

    Nan::Utf8String name(info[0]);

    Scene *object = new Scene(*name);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Scene::add)
{
    obs::scene *scene = Scene::GetHandle(info.Holder());

    if (info.Length() != 1) {
        Nan::ThrowError("Expected one argument");
        return;
    }

    obs::input *input = Input::GetHandle(info[0]->ToObject());

    auto item_object = SceneItem::GenerateObject(scene->add(*input));

    info.GetReturnValue().Set(item_object);
}

}