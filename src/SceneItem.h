#pragma once

#include <nan.h>

#include "Scene.h"

namespace osn {

class SceneItem : public Nan::ObjectWrap
{
    friend Scene;
    static Nan::Persistent<v8::FunctionTemplate> prototype;

    static v8::Local<v8::Object> GenerateObject(obs::scene::item item);

public:
    SceneItem(obs::scene::item item);

    obs::scene::item handle;

    static obs::scene::item *GetHandle(v8::Local<v8::Object> object);

    static void Init();

    static NAN_METHOD(remove);
    static NAN_GETTER(id);

    static NAN_GETTER(selected);
    static NAN_SETTER(selected);

    static NAN_SETTER(position);
    static NAN_GETTER(position);

    static NAN_SETTER(rotation);
    static NAN_GETTER(rotation);

    static NAN_SETTER(scale);
    static NAN_GETTER(scale);

    static NAN_SETTER(alignment);
    static NAN_GETTER(alignment);

    static NAN_SETTER(boundsAlignment);
    static NAN_GETTER(boundsAlignment);

    static NAN_SETTER(bounds);
    static NAN_GETTER(bounds);

    static NAN_SETTER(transformInfo);
    static NAN_GETTER(transformInfo);

    static NAN_SETTER(order);

    static NAN_SETTER(orderPosition);

    static NAN_SETTER(boundsType);
    static NAN_GETTER(boundsType);

    static NAN_SETTER(crop);
    static NAN_GETTER(crop);

    static NAN_SETTER(scaleFilter);
    static NAN_GETTER(scaleFilter);

    static NAN_METHOD(deferUpdateBegin);
    static NAN_METHOD(deferUpdateEnd);
};

}