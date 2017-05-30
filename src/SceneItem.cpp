#include "SceneItem.h"
#include "Common.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> SceneItem::prototype
    = Nan::Persistent<v8::FunctionTemplate>();

SceneItem::SceneItem(obs::scene::item item)
 : handle(item)
{
}

v8::Local<v8::Object> SceneItem::GenerateObject(obs::scene::item input)
{
    SceneItem *item = new SceneItem(input);
    v8::Local<v8::FunctionTemplate> input_templ =
        Nan::New<v8::FunctionTemplate>(SceneItem::prototype);

    v8::Local<v8::Object> object = 
        Nan::NewInstance(input_templ->InstanceTemplate()).ToLocalChecked();

    item->Wrap(object);
    return object;
}

obs::scene::item *SceneItem::GetHandle(v8::Local<v8::Object> object)
{
    SceneItem* item = Nan::ObjectWrap::Unwrap<SceneItem>(object);
    return &item->handle;
}

void SceneItem::Init()
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("selected"), selected, selected);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("position"), position, position);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("rotation"), rotation, rotation);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("scale"), scale, scale);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("alignment"), alignment, alignment);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("bounds_alignment"), bounds_alignment, bounds_alignment);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("bounds"), bounds, bounds);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("transform_info"), transform_info, transform_info);
    // Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("order"), 0, order);
    // Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("order_position"), 0, order_position);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("bounds_type"), bounds_type, bounds_type);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("crop"), crop, crop);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("scale_filter"), scale_filter, scale_filter);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("id"), id);
    Nan::SetMethod(locProto->PrototypeTemplate(), "remove", remove);
    Nan::SetMethod(locProto->PrototypeTemplate(), "defer_update_begin", defer_update_begin);
    Nan::SetMethod(locProto->PrototypeTemplate(), "defer_update_end", defer_update_end);
    locProto->SetClassName(FIELD_NAME("SceneItem"));
    prototype.Reset(locProto);
}

NAN_METHOD(SceneItem::remove)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    handle->remove();
}

NAN_GETTER(SceneItem::id)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    /* FIXME: id() returns uint64_t but JS can't hold that */
    info.GetReturnValue().Set((uint32_t)handle->id());
}

NAN_SETTER(SceneItem::selected)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    if (!value->IsBoolean()) {
        Nan::ThrowError("Expected boolean");
        return;
    }

    bool selected = value->ToBoolean()->Value();

    handle->selected(selected);
}

NAN_GETTER(SceneItem::selected)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->selected());
}

/* Note that we use typed arrays here!
 * I chose to do this because of the following:
 * 1. Most IPCs handle it correctly. 
 * 2. It's typed to make sure we aren't being passed weird things. 
 * 3. It's faster than most alternatives. 
 
 * We use V8 directly here since Nan abstraction for this sucks
 * and uses node::Buffer, except for where we can use 
 * Nan::TypedArrayContents. 
 
 * That said, NOTE: ArrayBuffers can be tricky. They take byte counts,
 * not element counts. You must make sure that anytime you're making
 * a new typed array, that the underlying arraybuffer has enough
 * room to accomdate. SERIOUSLY. PAY ATTENTION PLEASE.
 * Also... apparently, Nan::New doesn't work with ArrayBuffer. */

NAN_SETTER(SceneItem::position)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    if (!value->IsArray()) {
        Nan::ThrowError("Expected array");
        return;
    }

    Nan::TypedArrayContents<float> data(value);

    if (data.length() != 2) {
        Nan::ThrowError("Array of unexpected size (expected size of 2)");
        return;
    }

    vec2 position = {
        (*data)[0],
        (*data)[1]
    };

    handle->position(position);
}

NAN_GETTER(SceneItem::position)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    auto buffer = 
        v8::ArrayBuffer::New(isolate, sizeof(float) * 2);

    auto float_array = 
        v8::Float32Array::New(buffer, 0, 2);

    float *data = reinterpret_cast<float*>(buffer->GetContents().Data());

    vec2 position = handle->position();

    data[0] = position.x;
    data[1] = position.y;

    info.GetReturnValue().Set(float_array);
}

NAN_SETTER(SceneItem::rotation)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    if (!value->IsNumber()) {
        Nan::ThrowError("Expected float");
        return;
    }

    float rotation = static_cast<float>(Nan::To<double>(value).FromJust());

    handle->rotation(rotation);
}

NAN_GETTER(SceneItem::rotation)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    float rotation = handle->rotation();

    info.GetReturnValue().Set(rotation);
}

NAN_SETTER(SceneItem::scale)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    if (!value->IsArray()) {
        Nan::ThrowError("Expected array");
        return;
    }

    Nan::TypedArrayContents<float> data(value);

    if (data.length() != 2) {
        Nan::ThrowError("Array of unexpected size (expected size of 2)");
        return;
    }

    vec2 scale = {
        (*data)[0],
        (*data)[1]
    };

    handle->scale(scale);
}

NAN_GETTER(SceneItem::scale)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    auto buffer = 
        v8::ArrayBuffer::New(isolate, sizeof(float) * 2);

    auto float_array = 
        v8::Float32Array::New(buffer, 0, 2);

    float *data = reinterpret_cast<float*>(buffer->GetContents().Data());

    vec2 scale = handle->scale();

    data[0] = scale.x;
    data[1] = scale.y;

    info.GetReturnValue().Set(float_array);
}

NAN_SETTER(SceneItem::alignment)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    if (!value->IsUint32()) {
        Nan::ThrowError("Expected unsigned integer");
        return;
    }

    uint32_t alignment = Nan::To<uint32_t>(value).FromJust();
    handle->alignment(alignment);
}

NAN_GETTER(SceneItem::alignment)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->alignment());
}

NAN_SETTER(SceneItem::bounds_alignment)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    if (!value->IsUint32()) {
        Nan::ThrowError("Expected unsigned integer");
        return;
    }

    uint32_t alignment = Nan::To<uint32_t>(value).FromJust();
    handle->bounds_alignment(alignment);
}

NAN_GETTER(SceneItem::bounds_alignment)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->bounds_alignment());
}

NAN_SETTER(SceneItem::bounds)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    if (!value->IsArray()) {
        Nan::ThrowError("Expected array");
        return;
    }

    Nan::TypedArrayContents<float> data(value);

    if (data.length() != 2) {
        Nan::ThrowError("Array of unexpected size (expected size of 2)");
        return;
    }

    vec2 bounds = {
        (*data)[0],
        (*data)[1]
    };

    handle->bounds(bounds);
}

NAN_GETTER(SceneItem::bounds)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    auto buffer = 
        v8::ArrayBuffer::New(isolate, sizeof(float) * 2);

    auto float_array = 
        v8::Float32Array::New(buffer, 0, 2);

    float *data = reinterpret_cast<float*>(buffer->GetContents().Data());

    vec2 bounds = handle->bounds();

    data[0] = bounds.x;
    data[1] = bounds.y;

    info.GetReturnValue().Set(float_array);
}

NAN_SETTER(SceneItem::transform_info)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_GETTER(SceneItem::transform_info)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::order)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::order_position)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::bounds_type)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_GETTER(SceneItem::bounds_type)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::crop)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_GETTER(SceneItem::crop)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_SETTER(SceneItem::scale_filter)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_GETTER(SceneItem::scale_filter)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

}

NAN_METHOD(SceneItem::defer_update_begin)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    handle->defer_update_begin();
}

NAN_METHOD(SceneItem::defer_update_end)
{
    obs::scene::item *handle = SceneItem::GetHandle(info.Holder());

    handle->defer_update_end();
}


}