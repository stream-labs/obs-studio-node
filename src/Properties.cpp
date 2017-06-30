#include "Properties.h"
#include "Data.h"

namespace osn {

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

    if (info.Length() != 2) {
        Nan::ThrowError("Unexpected number of arguments");
        return;
    }

    if (!info[0]->IsString()) {
        Nan::ThrowError("Invalid type passed - expected string");
        return;
    }

    if (!info[1]->IsUint32()) {
        Nan::ThrowError("Invalid type passed - expected integer");
        return;
    }
    
    Nan::Utf8String id(info[0]);
    uint32_t object_type = Nan::To<uint32_t>(info[1]).FromJust();
    Properties *object = new Properties(*id, static_cast<obs::properties::object_type>(object_type));
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Properties::status)
{
    obs::properties *handle = Properties::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->status());
}

NAN_METHOD(Properties::first)
{
    obs::properties *handle = Properties::Object::GetHandle(info.Holder());

    Property *binding = new Property(handle->first());
    auto object = Property::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Properties::count)
{
    obs::properties *handle = Properties::Object::GetHandle(info.Holder());

    obs::property it = handle->first();

    int result = 0;

    while (it.status() == obs::property::status_type::okay) {
        ++result;
    }

    info.GetReturnValue().Set(result);
}

NAN_METHOD(Properties::get)
{
    obs::properties *handle = Properties::Object::GetHandle(info.Holder());

    if (info.Length() != 1 || !info[0]->IsString()) {
        Nan::ThrowError("Unexpected arguments");
        return;
    }

    Nan::Utf8String property_name(info[0]);
    
    Property *binding = new Property(handle->get(*property_name));
    auto object = Property::Object::GenerateObject(binding);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Properties::apply)
{
    /* TODO */
}

Nan::Persistent<v8::FunctionTemplate> Property::prototype =
    Nan::Persistent<v8::FunctionTemplate>();

Property::Property(obs::property &property)
 : handle(property)
{
}

NAN_MODULE_INIT(Property::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Property"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("name"), name);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("type"), type);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("status"), status);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("description"), description);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("longDescription"), longDescription);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("enabled"), enabled);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("visible"), visible);
    Nan::SetMethod(locProto->PrototypeTemplate(), "next", next);

    Nan::Set(target, FIELD_NAME("Property"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_GETTER(Property::status)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->status());
}

NAN_GETTER(Property::name)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle->name().c_str()).ToLocalChecked());
}

NAN_GETTER(Property::description)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle->description().c_str()).ToLocalChecked());
}

NAN_GETTER(Property::longDescription)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle->long_description().c_str()).ToLocalChecked());
}

NAN_GETTER(Property::type)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->type());
}

NAN_GETTER(Property::enabled)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->enabled());
}

NAN_GETTER(Property::visible)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->visible());
}

#if 0

NAN_GETTER(Property::list_type)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->list_type());
}

NAN_GETTER(Property::editable_list_type)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->editable_list_type());
}

NAN_GETTER(Property::path_type)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->path_type());
}

NAN_GETTER(Property::number_type)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->number_type());
}

NAN_GETTER(Property::text_type)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->text_type());
}

#endif

NAN_METHOD(Property::next)
{
    obs::property *handle = Property::Object::GetHandle(info.Holder());

    handle->next();
}

#if 0
NAN_METHOD(Property::getListItems)
{
    obs::property *handle_ = Property::Object::GetHandle(info.Holder());
    obs_property_t *handle = handle_->dangerous();

    int count = static_cast<int>(obs_property_list_item_count(handle));
    auto array = Nan::New<v8::Array>(count);
    obs_combo_type combo_type = obs_property_list_type(handle);

    for (int i = 0; i < count; ++i) {
        const char *name = obs_property_list_item_name(handle, i);
        auto item = Nan::New<v8::Object>();

        switch (combo_type) {
        case OBS_COMBO_FORMAT_INT:
            Nan::Set(item, 
                FIELD_NAME(name), 
                Nan::New<v8::Integer>(static_cast<int32_t>(obs_property_list_item_int(handle, i))));

            break;
        case OBS_COMBO_FORMAT_STRING:
            Nan::Set(item, 
                FIELD_NAME(name), 
                Nan::New<v8::String>(obs_property_list_item_string(handle, i)).ToLocalChecked());
            
            break;

        case OBS_COMBO_FORMAT_FLOAT:
            Nan::Set(item, 
                FIELD_NAME(name), 
                Nan::New<v8::Number>(obs_property_list_item_float(handle, i)));

            break;
        }

        Nan::Set(array, i, item);
    }

    info.GetReturnValue().Set(array);
}
#endif

}