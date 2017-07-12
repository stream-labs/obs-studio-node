#pragma once

#include <nan.h>
#include <obs.h>

#define FIELD_NAME(name) \
    Nan::New(name).ToLocalChecked()

#define ASSERT_INFO_LENGTH_AT_LEAST(info, length) \
    if ((info).Length() < length) { \
        Nan::ThrowError("Unexpected number of arguments"); \
        return; \
    }

#define ASSERT_INFO_LENGTH(info, length) \
    if ((info).Length() != (length)) { \
        Nan::ThrowError("Unexpected number of arguments"); \
        return; \
    }

#define ASSERT_GET_OBJECT_FIELD(object, field, var) \
    if (!osn::common::GetFromObject((object), (field), (var))) { \
        Nan::ThrowTypeError(osn::common::GetErrorString<decltype(var)>()); \
        return; \
    }

#define ASSERT_GET_VALUE(value, var) \
    if (!osn::common::FromValue((value), (var))) { \
        Nan::ThrowTypeError(osn::common::GetErrorString<decltype(var)>()); \
        return; \
    }

namespace osn {
namespace common {

template<typename Type>
const char *GetErrorString();

template <typename Type>
v8::Local<v8::Value> ToValue(Type value);

template <typename Type>
void SetObjectField(v8::Local<v8::Object> object, const char *field, Type value)
{
    Nan::Set(object, ToValue(field), ToValue(value));
}

template <typename Type>
void SetObjectField(v8::Local<v8::Object> object, const int field, Type value)
{
    Nan::Set(object, field, ToValue(value));
}

template <typename Type>
bool FromValue(v8::Local<v8::Value> value, Type &var);

template <typename Type>
bool GetFromObject(v8::Local<v8::Object> object, const char *field, Type &var)
{
    auto field_value = Nan::Get(object, FIELD_NAME(field)).ToLocalChecked();
    
    return FromValue(field_value, var);
}

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

    static HandleType &GetHandle(v8::Local<v8::Object> object)
    {
        BindingType* binding = Nan::ObjectWrap::Unwrap<BindingType>(object);
        return binding->handle;
    }
};

}
}