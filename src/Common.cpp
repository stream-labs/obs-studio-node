#include "Common.h"

namespace osn {
namespace common {

template <>
const char *GetErrorString<int64_t>()
{ return "Expected signed integer"; }

template <>
const char *GetErrorString<uint32_t>() 
{ return "Expected unsigned integer"; }

template <>
const char *GetErrorString<int32_t>()
{ return "Expected signed integer"; }

template <>
const char *GetErrorString<double>()
{ return "Expected number"; }

template <>
const char *GetErrorString<float>()
{ return GetErrorString<double>(); }

template <>
const char *GetErrorString<bool>()
{ return "Expected boolean"; }

template <>
const char *GetErrorString<std::string>()
{ return "Expected string"; }

template <>
const char *GetErrorString<vec2>()
{ return "Expected vec2 object"; }

template <>
const char *GetErrorString<v8::Local<v8::Object>>()
{ return "Expected JS object"; }

template <>
const char *GetErrorString<obs_data_t*>()
{ return "Expected settings object"; }

template <>
const char *GetErrorString<enum gs_color_format>()
{ return "Expected gs_color_format enum"; }

template <>
const char *GetErrorString<enum gs_zstencil_format>()
{ return "Expected gs_zstencil_format enum"; }

template <>
const char *GetErrorString<enum video_format>()
{ return "Expected video_format enum"; }

template <>
const char *GetErrorString<enum video_colorspace>()
{ return "Expected video_colorspace enum"; }

template <>
const char *GetErrorString<enum video_range_type>()
{ return "Expected video_range_type enum"; }

template <>
const char *GetErrorString<enum obs_scale_type>()
{ return "Expected obs_scale_type enum"; }

template <>
const char *GetErrorString<enum obs_combo_format>()
{ return "Expected obs_combo_format enum"; }

template <>
const char *GetErrorString<enum obs_bounds_type>()
{ return "Expected obs_bounds_type enum"; }

template <>
const char *GetErrorString<enum obs_monitoring_type>()
{ return "Expected obs_monitoring_type enum"; }

template <>
const char *GetErrorString<enum obs_deinterlace_mode>()
{ return "Expected obs_deinterlace_mode enum"; }

template <>
const char *GetErrorString<enum obs_deinterlace_field_order>()
{ return "Expected obs_deinterlace_field_order enum"; }

template <>
const char *GetErrorString<v8::Local<v8::Function>>()
{ return "Expected function"; }

template <>
const char *GetErrorString<Nan::Callback>()
{ return "Expected function"; }

template <>
v8::Local<v8::Value> ToValue(bool value) 
{ return Nan::New<v8::Boolean>(value); }

template <>
v8::Local<v8::Value> ToValue(int value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(long long value)
{
    return Nan::New<v8::Integer>(static_cast<int32_t>(value));
}

template <>
v8::Local<v8::Value> ToValue(unsigned int value)
{ return Nan::New<v8::Uint32>(value); }

template <>
v8::Local<v8::Value> ToValue(float value)
{ return Nan::New<v8::Number>(value); }

template <>
v8::Local<v8::Value> ToValue(double value)
{ return Nan::New<v8::Number>(value); }

template <>
v8::Local<v8::Value> ToValue(std::string value)
{ return Nan::New<v8::String>(value.c_str()).ToLocalChecked(); }

template <>
v8::Local<v8::Value> ToValue<char const*>(char const *value)
{ 
    if (!value) {
        value = "";
    }

    return Nan::New<v8::String>(value).ToLocalChecked(); 
}

/* See comment below. I ended up just making a function per enum... */
template <>
v8::Local<v8::Value> ToValue(enum obs_combo_format value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_path_type value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_text_type value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_number_type value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_editable_list_type value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_bounds_type value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_monitoring_type value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_deinterlace_field_order value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(enum obs_deinterlace_mode value)
{ return Nan::New<v8::Integer>(value); }

template <>
v8::Local<v8::Value> ToValue(v8::Local<v8::Object> value)
{ return value; }

template <>
v8::Local<v8::Value> ToValue(v8::Local<v8::Value> value)
{ return value; }

template <>
v8::Local<v8::Value> ToValue(v8::Local<v8::Array> value)
{ return value; }

template <>
v8::Local<v8::Value> ToValue(vec2 value)
{
    auto object = Nan::New<v8::Object>();

    SetObjectField(object, "x", value.x);
    SetObjectField(object, "y", value.y);

    return object;
}

template <>
v8::Local<v8::Value> ToValue<obs_data_array_t *>(obs_data_array_t *array_data);

template <>
v8::Local<v8::Value> ToValue<obs_data_t*>(obs_data_t *data)
{
    obs_data_item *item_it = obs_data_first(data);
    auto object = Nan::New<v8::Object>();

    while (item_it) {
        const char *name = obs_data_item_get_name(item_it);

        switch (obs_data_item_gettype(item_it)) {
        case OBS_DATA_STRING:
            SetObjectField(object, name, 
                obs_data_item_get_string(item_it));
            break;

        case OBS_DATA_NUMBER: {
            obs_data_number_type number_type = obs_data_item_numtype(item_it);

            switch (number_type) {
            case OBS_DATA_NUM_INT: 
                SetObjectField(object, name,
                    obs_data_item_get_int(item_it));
                break;

            case OBS_DATA_NUM_DOUBLE:
                SetObjectField(object, name,
                    obs_data_item_get_double(item_it));
                break;
            }

            break;
        }
        case OBS_DATA_BOOLEAN:
            SetObjectField(object, name,
                obs_data_item_get_bool(item_it));

            break;

        case OBS_DATA_OBJECT:
            SetObjectField(object, name,
                obs_data_item_get_obj(item_it));

            break;

        case OBS_DATA_ARRAY:
            SetObjectField(object, name,
                obs_data_item_get_array(item_it));
            break;
        }

        obs_data_item_next(&item_it);
    }

    return object;
}

template <>
v8::Local<v8::Value> ToValue<obs_data_array_t *>(obs_data_array_t *array_data)
{
    size_t count = obs_data_array_count(array_data);

    /* Potential bug here */
    auto array = Nan::New<v8::Array>(static_cast<int>(count));

    for (int i = 0; i < count; ++i) {
        obs_data_t *data = obs_data_array_item(array_data, i);
        SetObjectField(array, i, data);
    }

    return array;
}

template <>
bool FromValue<obs_data_t *>(v8::Local<v8::Value> value, obs_data_t * &var)
{
    var = nullptr;
    Nan::JSON NanJSON;

    if (!value->IsObject()) return false;

    auto object = Nan::To<v8::Object>(value).ToLocalChecked();

    Nan::MaybeLocal<v8::String> result = NanJSON.Stringify(object);

    if (result.IsEmpty())
        return false;
    
    Nan::Utf8String string(result.ToLocalChecked());

    var = obs_data_create_from_json(*string);
    return true;
}

template<>
bool FromValue(v8::Local<v8::Value> value, int64_t &var)
{
    /* There are no checks for anything past a 32-bit integer
     * Though, we can just check for a number here.  */
    if (!value->IsNumber()) {
        var = 0;
        return false;
    }

    var = Nan::To<int64_t>(value).FromJust();
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, uint32_t &var)
{
    if (!value->IsUint32()) {
        var = 0;
        return false;
    }

    var = Nan::To<uint32_t>(value).FromJust();
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, int32_t &var)
{
    if (!value->IsInt32()) {
        var = 0;
        return false;
    }

    var = Nan::To<int32_t>(value).FromJust();
    return true;
}

/* Since C++ is a piece of shit, we need to list each and every enum
 * as its own explicit specialization. There are probably better ways
 * but I'd like to finish this before I throw my laptop from our 
 * 4th floor office. */
namespace {

template <typename Type>
bool FromValueToEnum(
    v8::Local<v8::Value> value, 
    Type &var)
{
    if (!value->IsInt32()) {
        return false;
    }

    var = static_cast<Type>(Nan::To<int32_t>(value).FromJust());
    return true;
}

}

template <>
bool FromValue(v8::Local<v8::Value> value, enum gs_color_format &var)
{ return FromValueToEnum<enum gs_color_format>(value, var); }

template <>
bool FromValue(v8::Local<v8::Value> value, enum gs_zstencil_format &var)
{ return FromValueToEnum<enum gs_zstencil_format>(value, var); }

template <>
bool FromValue(v8::Local<v8::Value> value, enum obs_bounds_type &var)
{ return FromValueToEnum<enum obs_bounds_type>(value, var); }

template <>
bool FromValue(v8::Local<v8::Value> value, enum obs_scale_type &var)
{ return FromValueToEnum<enum obs_scale_type>(value, var); }

template <>
bool FromValue(v8::Local<v8::Value> value, enum video_format &var)
{ return FromValueToEnum<enum video_format>(value, var); }

template <>
bool FromValue(v8::Local<v8::Value> value, enum video_colorspace &var)
{ return FromValueToEnum<enum video_colorspace>(value, var); }

template <>
bool FromValue(v8::Local<v8::Value> value, enum video_range_type &var)
{ return FromValueToEnum<enum video_range_type>(value, var); }

template <>
bool FromValue(v8::Local<v8::Value> value, double &var)
{
    if (!value->IsNumber()) {
        var = 0.0;
        return false;
    }

    var = Nan::To<double>(value).FromJust();
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, float &var)
{
    double tmp;
    var = 0.0f;

    if (!FromValue(value, tmp)) return false;

    var = static_cast<float>(tmp);
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, bool &var)
{
    if (!value->IsBoolean()) return false;

    var = Nan::To<bool>(value).FromJust();
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, std::string &var)
{
    var.clear();

    if (!value->IsString()) return false;

    Nan::Utf8String tmp_nan_str(value);
    var = *tmp_nan_str;
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, vec2 &var)
{
    if (!value->IsObject()) return false;

    auto value_obj = Nan::To<v8::Object>(value).ToLocalChecked();

    return
        GetFromObject(value_obj, "x", var.x) &&
        GetFromObject(value_obj, "y", var.y);
}

template <>
bool FromValue(v8::Local<v8::Value> value, v8::Local<v8::Object> &var)
{
    if (!value->IsObject()) return false;

    var = Nan::To<v8::Object>(value).ToLocalChecked();
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, v8::Local<v8::Function> &var)
{
    if (!value->IsFunction()) return false;

    var = value.As<v8::Function>();
    return true;
}

template <>
bool FromValue(v8::Local<v8::Value> value, Nan::Callback &var)
{
    v8::Local<v8::Function> func;

    if (!FromValue(value, func)) {
        var.Reset();
        return false;
    }

    var.Reset(func);
    return true;
}

}
}