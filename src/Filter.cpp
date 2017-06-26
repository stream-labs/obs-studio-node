#include "Filter.h"
#include "Common.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Filter::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

v8::Local<v8::Object> Filter::GenerateObject(obs::filter filter)
{
    Filter *output_source = new Filter(filter);

    v8::Local<v8::FunctionTemplate> input_templ =
        Nan::New<v8::FunctionTemplate>(Filter::prototype);

    v8::Local<v8::Object> object = 
        Nan::NewInstance(input_templ->InstanceTemplate()).ToLocalChecked();

    output_source->Wrap(object);

    return object;
}

obs::filter* Filter::GetHandle(v8::Local<v8::Object> object)
{
    Filter* filter = Nan::ObjectWrap::Unwrap<Filter>(object);
    return &filter->handle;
}

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
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Filter"));
    Nan::Set(target, FIELD_NAME("Filter"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Filter::New)
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
    Filter *object = new Filter(*id, *name, nullptr);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

}