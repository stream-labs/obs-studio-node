#include "Service.h"
#include "Properties.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Service::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Service::Service(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey)
    : handle(obs::service(id, name, settings, hotkey))
{
}

Service::Service(std::string id, std::string name, obs_data_t *settings, bool is_private)
    : handle(obs::service(id, name, settings, is_private))
{
}

Service::Service(obs::service service)
 : handle(service)
{
}

NAN_MODULE_INIT(Service::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Service"));
    common::SetObjectTemplateField(locProto, "types", get_types);
    common::SetObjectTemplateField(locProto, "create", create);
    common::SetObjectTemplateField(locProto, "createPrivate", createPrivate);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "properties", get_properties);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "settings", get_settings);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "url", get_url);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "key", get_key);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "username", get_username);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "password", get_password);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "name", get_url);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "id", get_id);
    common::SetObjectField(target, "Service", locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Service::create)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 2);
    
    std::string id, name;
    obs_data_t *settings = nullptr, *hotkeys = nullptr;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], name);

    switch (info.Length()) {
    default:
    case 4:
        ASSERT_GET_VALUE(info[3], hotkeys);
    case 3:
        ASSERT_GET_VALUE(info[2], settings);
    case 2:
        break;
    }

    Service *binding = new Service(id, name, settings, hotkeys);
    auto object = Service::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Service::createPrivate)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 2);
    
    std::string id, name;
    obs_data_t *settings = nullptr;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], name);

    if (info.Length() > 2)
        ASSERT_GET_VALUE(info[2], settings);

    Service *binding = new Service(id, name, settings, true);
    auto object = Service::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}


NAN_METHOD(Service::get_types)
{
    auto type_list = obs::service::types();
    int count = static_cast<int>(type_list.size());

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        Nan::Set(array, i, Nan::New<v8::String>(type_list[i]).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Service::get_url)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->url()));
}

NAN_METHOD(Service::get_key)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->key()));
}

NAN_METHOD(Service::get_username)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->username()));
}

NAN_METHOD(Service::get_password)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->password()));
}

NAN_METHOD(Service::get_settings)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->settings()));
}

NAN_METHOD(Service::get_properties)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    obs::properties props = handle.get()->properties();

    if (props.status() != obs::properties::status_type::okay) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Properties *bindings = new Properties(std::move(props));
    auto object = Properties::Object::GenerateObject(bindings);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Service::get_name)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->name()));
}

NAN_METHOD(Service::get_id)
{
    obs::weak<obs::service> &handle = Service::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->id()));
}


}