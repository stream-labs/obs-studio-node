#include "Filter.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Filter::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Filter::Filter(std::string id, std::string name, obs_data_t *settings)
 : handle(id, name, settings)
{
}

Filter::Filter(obs::filter &filter)
 : handle(filter)
{
}

obs::source* Filter::GetHandle()
{
    return static_cast<obs::source*>(&handle);
}

NAN_MODULE_INIT(Filter::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Filter"));
    Nan::SetMethod(locProto->PrototypeTemplate(), "types", types);
    Nan::SetMethod(locProto->PrototypeTemplate(), "create", create);
    Nan::Set(target, FIELD_NAME("Filter"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Filter::types)
{
    int count = 0;
    const char *id; 

    while (obs_enum_filter_types(count, &id)) {
        ++count;
    }

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        Nan::Set(array, i, Nan::New<v8::String>(id).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Filter::create)
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
    Filter *binding = new Filter(*id, *name, nullptr);
    auto object = Filter::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

}