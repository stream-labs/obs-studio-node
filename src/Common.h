#pragma once

#include <nan.h>

#define FIELD_NAME(name) \
    Nan::New(name).ToLocalChecked()


namespace osn {
namespace common{

template <typename BindingType, typename HandleType>
class Object {
public:
    static v8::Local<v8::Object> GenerateObject(BindingType *binding)
    {

        v8::Local<v8::FunctionTemplate> templ =
            Nan::New<v8::FunctionTemplate>(BindingType::prototype);

        v8::Local<v8::Object> object = 
            Nan::NewInstance(templ->InstanceTemplate()).ToLocalChecked();

        binding->Wrap(object);

        return object;
    }

    static HandleType* GetHandle(v8::Local<v8::Object> object)
    {
        BindingType* binding = Nan::ObjectWrap::Unwrap<BindingType>(object);
        return &binding->handle;
    }
};

}
}