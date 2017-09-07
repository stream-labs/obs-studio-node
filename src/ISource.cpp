#include "ISource.h"
#include "Properties.h"
#include "Common.h"

/* FIXME or NOTE:
	Need to make sure there's no side effects of passing
	back an interface like this.
 */

namespace osn {

obs::source ISource::GetHandle(v8::Local<v8::Object> object)
{
    ISourceHandle* source = Nan::ObjectWrap::Unwrap<ISource>(object);
    return source->GetHandle();
}

NAN_MODULE_INIT(ISource::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Source"));
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("name"), name, name);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("type"), type);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("outputFlags"), outputFlags);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("flags"), flags, flags);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("name"), name, name);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("status"), status);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("id"), id);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("muted"), muted, muted);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("enabled"), enabled, enabled);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("configurable"), configurable);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("properties"), properties);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("settings"), settings);

    Nan::SetMethod(locProto->PrototypeTemplate(), "release", release);
    Nan::SetMethod(locProto->PrototypeTemplate(), "remove", remove);
    Nan::SetMethod(locProto->PrototypeTemplate(), "update", update);

    Nan::Set(target, FIELD_NAME("Source"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(ISource::release)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    handle.release();
    /* Invalidate said handle to help prevent abuse. 
     * Note that this can still be abused if you try! */
    handle = nullptr;
}

NAN_METHOD(ISource::remove)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    handle.remove();
}

NAN_GETTER(ISource::type)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.type());
}

NAN_GETTER(ISource::name)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.name()));
}

NAN_SETTER(ISource::name)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    std::string name;

    ASSERT_GET_VALUE(value, name);

    handle.name(name);
}

NAN_GETTER(ISource::outputFlags)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.output_flags());
}

NAN_GETTER(ISource::flags)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.flags());
}

NAN_SETTER(ISource::flags)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    uint32_t flags;

    ASSERT_GET_VALUE(value, flags);

    handle.flags(flags);
}

NAN_GETTER(ISource::status)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

NAN_GETTER(ISource::id)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.id()));
}

NAN_GETTER(ISource::configurable)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.configurable());
}

NAN_GETTER(ISource::properties)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    obs::properties props = handle.properties();

    if (props.status() != obs::properties::status_type::okay) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Properties *bindings = new Properties(std::move(props));
    auto object = Properties::Object::GenerateObject(bindings);

    info.GetReturnValue().Set(object);
}

NAN_GETTER(ISource::settings)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    obs_data_t *data = handle.settings();

    info.GetReturnValue().Set(common::ToValue(data));
}

NAN_METHOD(ISource::update)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    obs_data_t *data;
    
    ASSERT_GET_VALUE(info[0], data);

    handle.update(data);
    obs_data_release(data);
}

NAN_GETTER(ISource::muted)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.muted()));
}

NAN_SETTER(ISource::muted)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    bool muted;

    ASSERT_GET_VALUE(value, muted);

    handle.muted(muted);
}

NAN_GETTER(ISource::enabled)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.enabled()));
}

NAN_SETTER(ISource::enabled)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    bool enabled;

    ASSERT_GET_VALUE(value, enabled);

    handle.enabled(enabled);
}

Nan::Persistent<v8::FunctionTemplate> ISource::prototype = Nan::Persistent<v8::FunctionTemplate>();

}