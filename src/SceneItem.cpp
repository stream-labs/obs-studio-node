#include "SceneItem.h"
#include "Input.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> SceneItem::prototype
    = Nan::Persistent<v8::FunctionTemplate>();

SceneItem::SceneItem(obs::scene::item item)
 : handle(item)
{
}

void SceneItem::Init()
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("source"), source);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("scene"), scene);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("selected"), selected, selected);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("position"), position, position);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("rotation"), rotation, rotation);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("scale"), scale, scale);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("alignment"), alignment, alignment);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("boundsAlignment"), boundsAlignment, boundsAlignment);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("bounds"), bounds, bounds);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("transformInfo"), transformInfo, transformInfo);
    // Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("order"), 0, order);
    // Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("orderPosition"), 0, orderPosition);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("boundsType"), boundsType, boundsType);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("crop"), crop, crop);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("scaleFilter"), scaleFilter, scaleFilter);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("id"), id);
    Nan::SetMethod(locProto->PrototypeTemplate(), "remove", remove);
    Nan::SetMethod(locProto->PrototypeTemplate(), "deferUpdateBegin", deferUpdateBegin);
    Nan::SetMethod(locProto->PrototypeTemplate(), "deferUpdateEnd", deferUpdateEnd);
    locProto->SetClassName(FIELD_NAME("SceneItem"));
    prototype.Reset(locProto);
}

NAN_GETTER(SceneItem::source)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    Input *binding = new Input(handle.source());
    auto object = Input::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_GETTER(SceneItem::scene)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    Scene *binding = new Scene(handle.scene());
    auto object = Scene::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(SceneItem::remove)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    handle.remove();
}

NAN_GETTER(SceneItem::id)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    /* FIXME: id() returns uint64_t but JS can't hold that */
    info.GetReturnValue().Set((uint32_t)handle.id());
}

NAN_GETTER(SceneItem::visible)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.visible()));
}

NAN_SETTER(SceneItem::visible)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    bool visible;

    ASSERT_GET_VALUE(value, visible);

    handle.visible(visible);
}

NAN_SETTER(SceneItem::selected)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    bool selected;

    ASSERT_GET_VALUE(value, selected);

    handle.selected(selected);
}

NAN_GETTER(SceneItem::selected)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.selected()));
}

NAN_SETTER(SceneItem::position)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    vec2 position;

    ASSERT_GET_VALUE(value, position);

    handle.position(position);
}

NAN_GETTER(SceneItem::position)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
    vec2 position = handle.position();

    info.GetReturnValue().Set(common::ToValue(position));
}

NAN_SETTER(SceneItem::rotation)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    float rotation;

    ASSERT_GET_VALUE(value, rotation);

    handle.rotation(rotation);
}

NAN_GETTER(SceneItem::rotation)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    float rotation = handle.rotation();

    info.GetReturnValue().Set(common::ToValue(rotation));
}

NAN_SETTER(SceneItem::scale)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    vec2 scale;

    ASSERT_GET_VALUE(value, scale);

    handle.scale(scale);
}

NAN_GETTER(SceneItem::scale)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
    vec2 scale = handle.scale();

    info.GetReturnValue().Set(common::ToValue(scale));
}

NAN_SETTER(SceneItem::alignment)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    uint32_t alignment;

    ASSERT_GET_VALUE(value, alignment);

    handle.alignment(alignment);
}

NAN_GETTER(SceneItem::alignment)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.alignment()));
}

NAN_SETTER(SceneItem::boundsAlignment)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    uint32_t alignment;

    ASSERT_GET_VALUE(value, alignment);

    handle.bounds_alignment(alignment);
}

NAN_GETTER(SceneItem::boundsAlignment)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.bounds_alignment()));
}

NAN_SETTER(SceneItem::bounds)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    vec2 bounds;

    ASSERT_GET_VALUE(value, bounds);

    handle.scale(bounds);
}

NAN_GETTER(SceneItem::bounds)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
    vec2 bounds = handle.bounds();

    info.GetReturnValue().Set(common::ToValue(bounds));
}

NAN_SETTER(SceneItem::transformInfo)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());
}

NAN_GETTER(SceneItem::transformInfo)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::order)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::orderPosition)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::boundsType)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_GETTER(SceneItem::boundsType)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::crop)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_GETTER(SceneItem::crop)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::scaleFilter)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_GETTER(SceneItem::scaleFilter)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

}

NAN_METHOD(SceneItem::deferUpdateBegin)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    handle.defer_update_begin();
}

NAN_METHOD(SceneItem::deferUpdateEnd)
{
    obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

    handle.defer_update_end();
}

}