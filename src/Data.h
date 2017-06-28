#pragma once

/* These are generic functionalities to 
   interchange between libobs obs_data_t
   structures and JS objects. */

#include <nan.h>

namespace osn {
namespace {
    
obs_data_t *FromObjectToData(v8::Local<v8::Object> object)
{
    Nan::JSON NanJSON;

    Nan::MaybeLocal<v8::String> result = NanJSON.Stringify(object);

    if (result.IsEmpty())
        return nullptr;
    
    Nan::Utf8String string(result.ToLocalChecked());

    return obs_data_create_from_json(*string);
}

v8::Local<v8::Value> FromDataToObject(obs_data_t *data)
{
    Nan::JSON NanJSON;

    const char *string = obs_data_get_json(data);

    Nan::MaybeLocal<v8::Value> result = 
        NanJSON.Parse(Nan::New(string).ToLocalChecked());

    if (result.IsEmpty()) 
        return v8::Local<v8::Value>::Cast(Nan::Null());

    return result.ToLocalChecked();
}

}}