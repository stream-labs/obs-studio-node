#include <nan.h>

#include "Input.h"
#include "Filter.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Input::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Input::Input(std::string id, std::string name, obs_data_t *hotkey, obs_data_t *settings)
    : handle(id, name, hotkey, settings)
{
}

Input::Input(std::string id, std::string name, obs_data_t *settings, bool is_private)
    : handle(id, name, settings, is_private)
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
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Input"));
    Nan::SetMethod(locProto->PrototypeTemplate(), "types", types);
    Nan::SetMethod(locProto->PrototypeTemplate(), "create", create);
    Nan::SetMethod(locProto->PrototypeTemplate(), "fromName", fromName);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("volume"), volume, volume);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("syncOffset"), syncOffset, syncOffset);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("showing"), showing);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("audioMixers"), audioMixers, audioMixers);
    Nan::Set(target, FIELD_NAME("Input"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Input::types)
{
    int count = 0;
    const char *id; 

    while (obs_enum_input_types(count, &id)) {
        ++count;
    }

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        Nan::Set(array, i, Nan::New<v8::String>(id).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Input::create)
{
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

    Input *binding;

    switch (info.Length()) {
    case 2: 
        binding = new Input(*id, *name, nullptr, nullptr);
        break;
    case 3:
        if (info[2]->IsBoolean()) {
            bool is_private = Nan::To<bool>(info[2]).FromJust();

            binding = new Input(*id, *name, nullptr, is_private);
        }
        else if (info[2]->IsObject()) {
            Nan::ThrowError("Unfinished implementation");
            return;
        }
        else {
            Nan::ThrowError("Unexpected argument");
            return;
        }

        break;
    case 4:
        Nan::ThrowError("Unfinished implementation");
        return;
    }

    auto object = Input::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Input::fromName)
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
    Input *binding = new Input(obs::input::from_name(*name));
    auto object = Input::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_GETTER(Input::volume)
{
    obs::input *handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->volume());
}

NAN_SETTER(Input::volume)
{
    obs::input *handle = Input::Object::GetHandle(info.Holder());

    if (!value->IsNumber()) {
        Nan::ThrowTypeError("Expected number");
        return;
    }

    float volume = static_cast<float>(Nan::To<double>(value).FromJust());
    handle->volume(volume);
}

NAN_GETTER(Input::syncOffset)
{
    /* TODO Needs a 64-bit offset */
}

NAN_SETTER(Input::syncOffset)
{
    /* TODO Needs a 64-bit offset */
}

NAN_GETTER(Input::showing)
{
    obs::input *handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->showing());
}

NAN_GETTER(Input::audioMixers)
{
    obs::input *handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->audio_mixers());
}

NAN_SETTER(Input::audioMixers)
{
    obs::input *handle = Input::Object::GetHandle(info.Holder());

    if (!value->IsUint32()) {
        Nan::ThrowTypeError("Expected unsigned 32-bit integer");
        return;
    }

    uint32_t flags = Nan::To<uint32_t>(value).FromJust();
    handle->audio_mixers(flags);
}

NAN_METHOD(Input::addFilter)
{
    obs::input *handle = Input::Object::GetHandle(info.Holder());

    if (!info[0]->IsObject()) {
        Nan::ThrowError("Expected object");
        return;
    }

    auto filter_object = Nan::To<v8::Object>(info[0]).ToLocalChecked();

    obs::filter *filter = Filter::Object::GetHandle(filter_object);

    handle->add_filter(*filter);
}

NAN_METHOD(Input::removeFilter)
{
    obs::input *handle = Input::Object::GetHandle(info.Holder());

    if (!info[0]->IsObject()) {
        Nan::ThrowError("Expected object");
        return;
    }

    auto filter_object = Nan::To<v8::Object>(info[0]).ToLocalChecked();

    obs::filter *filter = Filter::Object::GetHandle(filter_object);

    handle->remove_filter(*filter);
}

}