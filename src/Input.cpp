#include <nan.h>

#include "Input.h"
#include "Common.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Input::prototype = Nan::Persistent<v8::FunctionTemplate>();

v8::Local<v8::Object> Input::GenerateObject(obs::input input)
{
    Input *output_source = new Input(input);
    v8::Local<v8::FunctionTemplate> input_templ =
        Nan::New<v8::FunctionTemplate>(Input::prototype);
    v8::Local<v8::Object> object = 
        Nan::NewInstance(input_templ->InstanceTemplate()).ToLocalChecked();
    output_source->Wrap(object);
    return object;
}

obs::input* Input::GetHandle(v8::Local<v8::Object> object)
{
    Input* input = Nan::ObjectWrap::Unwrap<Input>(object);
    return &input->handle;
}

Input::Input(std::string id, std::string name, obs_data_t *settings)
    : handle(id, name, settings)
{
}

Input::Input(obs::input &input)
 : handle(input)
{
}

obs::source *Input::GetHandle()
{
    return static_cast<obs::source*>(&handle);
}

NAN_MODULE_INIT(Input::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Input"));
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("volume"), volume, volume);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("sync_offset"), sync_offset, sync_offset);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("showing"), showing);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("flags"), flags, flags);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("audio_mixers"), audio_mixers, audio_mixers);
    Nan::Set(target, FIELD_NAME("Input"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Input::New)
{
    if (!info.IsConstructCall()) {
        Nan::ThrowError("Must be used as a construct call");
        return;
    }
    if (info.Length() < 2) {
        Nan::ThrowError("Too few arguments provided");
        return;
    }
    if (!info[0]->IsString() || !info[1]->IsString()) {
        Nan::ThrowError("Invalid type passed");
        return;
    }
    
    Nan::Utf8String id(info[0]);
    Nan::Utf8String name(info[1]);
    Input *object = new Input(*id, *name, nullptr);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Input::volume)
{
    obs::input *handle = Input::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->volume());
}

NAN_SETTER(Input::volume)
{
    obs::input *handle = Input::GetHandle(info.Holder());

    if (!value->IsNumber()) {
        Nan::ThrowTypeError("Expected number");
        return;
    }

    float volume = static_cast<float>(Nan::To<double>(value).FromJust());
    handle->volume(volume);
}

NAN_GETTER(Input::sync_offset)
{
    /* TODO Needs a 64-bit offset */
}

NAN_SETTER(Input::sync_offset)
{
    /* TODO Needs a 64-bit offset */
}

NAN_GETTER(Input::showing)
{
    obs::input *handle = Input::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->showing());
}

NAN_GETTER(Input::flags)
{
    obs::input *handle = Input::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->flags());
}

NAN_SETTER(Input::flags)
{
    obs::input *handle = Input::GetHandle(info.Holder());

    if (!value->IsUint32()) {
        Nan::ThrowTypeError("Expected unsigned 32-bit integer");
        return;
    }

    uint32_t flags = Nan::To<uint32_t>(value).FromJust();
    handle->flags(flags);
}

NAN_GETTER(Input::audio_mixers)
{
    obs::input *handle = Input::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->audio_mixers());
}

NAN_SETTER(Input::audio_mixers)
{
    obs::input *handle = Input::GetHandle(info.Holder());

    if (!value->IsUint32()) {
        Nan::ThrowTypeError("Expected unsigned 32-bit integer");
        return;
    }

    uint32_t flags = Nan::To<uint32_t>(value).FromJust();
    handle->audio_mixers(flags);
}


}