#pragma once

/* These are generic functionalities to 
   interchange between libobs obs_data_t
   structures and JS objects. */

#include <nan.h>

#include "Common.h"

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

v8::Local<v8::Object> FromDataToObject(obs_data_t *data);

v8::Local<v8::Array> FromDataToArray(obs_data_array_t *array_data)
{
    size_t count = obs_data_array_count(array_data);

    /* Potential bug here */
    auto array = Nan::New<v8::Array>(static_cast<int>(count));

    for (int i = 0; i < count; ++i) {
        obs_data_t *data = obs_data_array_item(array_data, i);
        Nan::Set(array, i, FromDataToObject(data));
    }

    return array;
}

#if 1

v8::Local<v8::Object> FromDataToObject(obs_data_t *data)
{
    obs_data_item *item_it = obs_data_first(data);
    auto object = Nan::New<v8::Object>();

    while (item_it) {
        const char *name = obs_data_item_get_name(item_it);

        switch (obs_data_item_gettype(item_it)) {
        case OBS_DATA_STRING:
            Nan::Set(object, 
                FIELD_NAME(name), 
                Nan::New<v8::String>(obs_data_item_get_string(item_it)).ToLocalChecked());
            break;
        case OBS_DATA_NUMBER: {
            obs_data_number_type number_type = obs_data_item_numtype(item_it);

            switch (number_type) {
            case OBS_DATA_NUM_INT: 
                Nan::Set(object,
                    FIELD_NAME(name),
                    Nan::New<v8::Integer>(static_cast<int32_t>(obs_data_item_get_int(item_it))));
            case OBS_DATA_NUM_DOUBLE:
                Nan::Set(object,
                    FIELD_NAME(name),
                    Nan::New<v8::Number>(obs_data_item_get_double(item_it)));
            }

            break;
        }
        case OBS_DATA_BOOLEAN:
            Nan::Set(object, 
                FIELD_NAME(name),
                Nan::New<v8::Boolean>(obs_data_item_get_bool(item_it)));

            break;
        case OBS_DATA_OBJECT:
            Nan::Set(object,
                FIELD_NAME(name),
                FromDataToObject(obs_data_item_get_obj(item_it)));

            break;
        case OBS_DATA_ARRAY:
            Nan::Set(object,
                FIELD_NAME(name),
                FromDataToArray(obs_data_item_get_array(item_it)));
            break;
        }
    }

    return object;
}

#else

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

#endif

}}