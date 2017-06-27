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
    Nan::SetMethod(locProto->InstanceTemplate(), "start", start);
    Nan::Set(target, FIELD_NAME("Transition"), locProto->GetFunction());
    prototype.Reset(locProto);
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
        Nan::ThrowTypeError("Expected input objectt");
        return;
    }

    int ms = Nan::To<int32_t>(info[0]).FromJust();

    auto source_object = Nan::To<v8::Object>(info[1]).ToLocalChecked();
    obs::input *source = Input::Object::GetHandle(source_object);

    handle->start(ms, *source);
}

}