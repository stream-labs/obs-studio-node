#include "Transition.h"
#include "Input.h"
#include "Common.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Transition::prototype;

v8::Local<v8::Object> Transition::GenerateObject(obs::transition transition)
{
    Transition *handle = new Transition(transition);

    v8::Local<v8::FunctionTemplate> trans_templ =
        Nan::New<v8::FunctionTemplate>(Transition::prototype);

    v8::Local<v8::Object> object = 
        Nan::NewInstance(trans_templ->InstanceTemplate()).ToLocalChecked();

    handle->Wrap(object);

    return object;
}

obs::transition* Transition::GetHandle(v8::Local<v8::Object> object)
{
    Transition* transition = Nan::ObjectWrap::Unwrap<Transition>(object);
    return &transition->handle;
}

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
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Transition"));
    Nan::SetMethod(locProto->InstanceTemplate(), "start", start);
    Nan::Set(target, FIELD_NAME("Transition"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Transition::New)
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
    Transition *object = new Transition(*id, *name, nullptr);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Transition::start)
{
    obs::transition *handle = Transition::GetHandle(info.Holder());

    if (info.Length() != 2) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }

    if (!info[0]->IsInt32()) {
        Nan::ThrowTypeError("Expected integer");
        return;
    }

    if (!info[1]->IsObject()) {
        Nan::ThrowTypeError("Expected input objectt");
        return;
    }

    int ms = Nan::To<int32_t>(info[0]).FromJust();

    auto source_object = Nan::To<v8::Object>(info[1]).ToLocalChecked();
    obs::input *source = Input::GetHandle(source_object);

    handle->start(ms, *source);
}

}