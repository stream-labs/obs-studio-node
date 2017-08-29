#include "Properties.h"
#include "ISource.h"

namespace osn {

using namespace osn::common;

Nan::Persistent<v8::FunctionTemplate> Properties::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Properties::Properties(obs::properties &&handle)
 : handle(std::move(handle))
{
}

Properties::Properties(std::string id, obs::properties::object_type type)
 : handle(id, type)
{
}

NAN_MODULE_INIT(Properties::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->SetClassName(FIELD_NAME("Properties"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(locProto->PrototypeTemplate(), "first", first);
    Nan::SetMethod(locProto->PrototypeTemplate(), "count", count);
    Nan::SetMethod(locProto->PrototypeTemplate(), "get", get);

    Nan::Set(target, FIELD_NAME("Properties"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Properties::New)
{
    if (!info.IsConstructCall()) {
        Nan::ThrowError("Must be used as a construct call");
        return;
    }

    ASSERT_INFO_LENGTH(info, 2);

    std::string id;
    uint32_t object_type;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], object_type);

    Properties *object = new Properties(id, static_cast<obs::properties::object_type>(object_type));
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Properties::status)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

NAN_METHOD(Properties::first)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    Property *binding = new Property(info.Holder(), handle.first());
    auto object = Property::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Properties::count)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    obs::property it = handle.first();

    int result = 0;

    while (it.status() == obs::property::status_type::okay) {
        ++result;
    }

    info.GetReturnValue().Set(result);
}

NAN_METHOD(Properties::get)
{
    obs::properties &handle = Properties::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    std::string property_name;

    ASSERT_GET_VALUE(info[0], property_name);
    
    Property *binding = new Property(info.Holder(), handle.get(property_name));
    auto object = Property::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Properties::apply)
{
    /* TODO */
}

Nan::Persistent<v8::FunctionTemplate> Property::prototype =
    Nan::Persistent<v8::FunctionTemplate>();

Property::Property(v8::Local<v8::Object> parent, obs::property &property)
 :  parent(parent),
    handle(property)
{
}

NAN_MODULE_INIT(Property::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Property"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("value"), value);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("name"), name);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("type"), type);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("status"), status);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("description"), description);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("longDescription"), longDescription);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("details"), details);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("enabled"), enabled);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("visible"), visible);
    Nan::SetMethod(locProto->PrototypeTemplate(), "buttonClicked", buttonClicked);
    Nan::SetMethod(locProto->PrototypeTemplate(), "next", next);

    Nan::Set(target, FIELD_NAME("Property"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_GETTER(Property::status)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

NAN_GETTER(Property::name)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(ToValue(handle.name()));
}

NAN_GETTER(Property::description)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(ToValue(handle.description().c_str()));
}

NAN_GETTER(Property::longDescription)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(ToValue(handle.long_description().c_str()));
}

NAN_GETTER(Property::type)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.type());
}

NAN_GETTER(Property::enabled)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.enabled());
}

NAN_GETTER(Property::visible)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.visible());
}

NAN_METHOD(Property::modified)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    obs_data_t *settings;

    ASSERT_GET_VALUE(info[0], settings);

    bool refresh = handle.modified(settings);

    info.GetReturnValue().Set(refresh);
}

NAN_METHOD(Property::buttonClicked)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> source_object;
    ASSERT_GET_VALUE(info[0], source_object);

    obs::source source = ISource::GetHandle(source_object);
    obs::button_property button_prop = handle.button_property();

    button_prop.clicked(source.dangerous());
}

NAN_GETTER(Property::value)
{
    info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Property::next)
{
    Property* this_binding = Nan::ObjectWrap::Unwrap<Property>(info.Holder());
    obs::property &handle = this_binding->handle;
    obs::property next(handle.next());

    if (next.status() == obs::property::status_type::invalid) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Property *binding = new Property(this_binding->parent, next);
    auto object = Property::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

namespace {

v8::Local<v8::Array> GetListItems(obs::list_property &property)
{
    int count = static_cast<int>(property.count());
    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        auto object = Nan::New<v8::Object>();

        SetObjectField(object, "name", property.get_name(i));

        switch (property.format()) {
        case OBS_COMBO_FORMAT_INT:
            SetObjectField(object, "value", 
                static_cast<int32_t>(property.get_integer(i)));
            break;
        case OBS_COMBO_FORMAT_STRING:
            SetObjectField(object, "value", property.get_string(i));
            break;
        case OBS_COMBO_FORMAT_FLOAT:
            SetObjectField(object, "value", property.get_float(i));
            break;
        }

        SetObjectField(array, i, object);
    }

    return array;
}

}

NAN_GETTER(Property::details)
{
    obs::property &handle = Property::Object::GetHandle(info.Holder());

    /* Some types have extra data. Associated with them. Construct objects
       and pass them with the property since you're more likely to need them
       than not. */
    auto object = Nan::New<v8::Object>();

    switch (handle.type()) {
    case OBS_PROPERTY_LIST: {
        obs::list_property list_prop = 
            handle.list_property();

        SetObjectField(object, "format", list_prop.format());

        SetObjectField(object, "items", GetListItems(list_prop));
        break;
    }
    case OBS_PROPERTY_EDITABLE_LIST: {
        obs::editable_list_property edit_list_prop = 
            handle.editable_list_property();

        SetObjectField(object, "type", edit_list_prop.type());
        SetObjectField(object, "format", edit_list_prop.format());
        SetObjectField(object, "items", 
            GetListItems(static_cast<obs::list_property>(edit_list_prop)));

        SetObjectField(object, "filter", edit_list_prop.filter());
        SetObjectField(object, "defaultPath", edit_list_prop.default_path());
        break;
    }
    case OBS_PROPERTY_TEXT:  {
        obs::text_property text_prop = handle.text_property();

        SetObjectField(object, "type", text_prop.type());
        break;
    }
    case OBS_PROPERTY_PATH: {
        obs::path_property path_prop = handle.path_property();

        SetObjectField(object, "type", path_prop.type());
        break;
    }
    case OBS_PROPERTY_FLOAT: {
        obs::float_property float_prop = 
            handle.float_property();

        SetObjectField(object, "type", float_prop.type());
        SetObjectField(object, "min", float_prop.min());
        SetObjectField(object, "max", float_prop.max());
        SetObjectField(object, "step", float_prop.step());
        break;
    }
    case OBS_PROPERTY_INT: {
        obs::integer_property int_prop = 
            handle.integer_property();

        SetObjectField(object, "type", int_prop.type());
        SetObjectField(object, "min", int_prop.min());
        SetObjectField(object, "max", int_prop.max());
        SetObjectField(object, "step", int_prop.step());
        break;
    }
    }

    info.GetReturnValue().Set(object);
}

}