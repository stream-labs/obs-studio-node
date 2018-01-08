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
    v8::Local<v8::ObjectTemplate> proto_templ = locProto->PrototypeTemplate();

    common::SetObjectTemplateLazyAccessor(proto_templ, "name", get_name, set_name);
    common::SetObjectTemplateLazyAccessor(proto_templ, "type", get_type);
    common::SetObjectTemplateLazyAccessor(proto_templ, "outputFlags", get_outputFlags);
    common::SetObjectTemplateLazyAccessor(proto_templ, "flags", get_flags, set_flags);
    common::SetObjectTemplateLazyAccessor(proto_templ, "name", get_name, set_name);
    common::SetObjectTemplateLazyAccessor(proto_templ, "status", get_status);
    common::SetObjectTemplateLazyAccessor(proto_templ, "id", get_id);
    common::SetObjectTemplateLazyAccessor(proto_templ, "muted", get_muted, set_muted);
    common::SetObjectTemplateLazyAccessor(proto_templ, "enabled", get_enabled, set_enabled);
    common::SetObjectTemplateLazyAccessor(proto_templ, "configurable", get_configurable);
    common::SetObjectTemplateLazyAccessor(proto_templ, "properties", get_properties);
    common::SetObjectTemplateLazyAccessor(proto_templ, "settings", get_settings);

    common::SetObjectTemplateField(proto_templ, "save", save);
    common::SetObjectTemplateField(proto_templ, "release", release);
    common::SetObjectTemplateField(proto_templ, "remove", remove);
    common::SetObjectTemplateField(proto_templ, "update", update);

    common::SetObjectField(target, "Source", locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(ISource::save)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    handle.save();
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

NAN_METHOD(ISource::get_type)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.type());
}

NAN_METHOD(ISource::get_name)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.name()));
}

NAN_METHOD(ISource::set_name)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    std::string name;

    ASSERT_GET_VALUE(info[0], name);

    handle.name(name);
}

NAN_METHOD(ISource::get_outputFlags)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.output_flags());
}

NAN_METHOD(ISource::get_flags)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.flags());
}

NAN_METHOD(ISource::set_flags)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    uint32_t flags;

    ASSERT_GET_VALUE(info[0], flags);

    handle.flags(flags);
}

NAN_METHOD(ISource::get_status)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

NAN_METHOD(ISource::get_id)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.id()));
}

NAN_METHOD(ISource::get_configurable)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.configurable());
}

NAN_METHOD(ISource::get_properties)
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

NAN_METHOD(ISource::get_settings)
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

NAN_METHOD(ISource::get_muted)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.muted()));
}

NAN_METHOD(ISource::set_muted)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    bool muted;

    ASSERT_GET_VALUE(info[0], muted);

    handle.muted(muted);
}

NAN_METHOD(ISource::get_enabled)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.enabled()));
}

NAN_METHOD(ISource::set_enabled)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    bool enabled;

    ASSERT_GET_VALUE(info[0], enabled);

    handle.enabled(enabled);
}

Nan::Persistent<v8::FunctionTemplate> ISource::prototype = Nan::Persistent<v8::FunctionTemplate>();

}