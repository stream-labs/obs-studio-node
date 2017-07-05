#include "Transition.h"
#include "Input.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Transition::prototype;

Transition::Transition(std::string id, std::string name, obs_data_t *settings)
 : handle(id, name, settings)
{
}

Transition::Transition(obs::transition transition) 
 : handle(transition)
{
}

obs::source *Transition::GetHandle()
{
    return static_cast<obs::source*>(&handle);
}

NAN_MODULE_INIT(Transition::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Transition"));
    Nan::SetMethod(locProto, "create", create);
    Nan::SetMethod(locProto, "types", types);
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
    Transition *binding = new Transition(*id, *name, nullptr);
    auto object = Transition::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Transition::set)
{
    obs::transition *handle = Transition::Object::GetHandle(info.Holder());

    if (info.Length() != 1) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }

    if (!info[0]->IsObject()) {
        Nan::ThrowTypeError("Expected input object");
        return;
    }

    auto input_object = Nan::To<v8::Object>(info[0]).ToLocalChecked();
    obs::input *input = Input::Object::GetHandle(input_object);

    handle->set(*input);
}

NAN_METHOD(Transition::start)
{
    obs::transition *handle = Transition::Object::GetHandle(info.Holder());

    if (info.Length() != 2) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }

    if (!info[0]->IsInt32()) {
        Nan::ThrowTypeError("Expected integer");
        return;
    }

    if (!info[1]->IsObject()) {
        Nan::ThrowTypeError("Expected input object");
        return;
    }

    int ms = Nan::To<int32_t>(info[0]).FromJust();

    auto source_object = Nan::To<v8::Object>(info[1]).ToLocalChecked();
    obs::input *source = Input::Object::GetHandle(source_object);

    handle->start(ms, *source);
}

}