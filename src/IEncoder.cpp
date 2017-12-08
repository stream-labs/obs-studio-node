#include "IEncoder.h"
#include "Common.h"

namespace osn{

obs::encoder IEncoder::GetHandle(v8::Local<v8::Object> object)
{
    IEncoderHandle* encoder = Nan::ObjectWrap::Unwrap<IEncoder>(object);
    return encoder->GetHandle();
}

NAN_MODULE_INIT(IEncoder::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Encoder"));
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "name", get_name, set_name);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "id", get_id);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "type", get_type);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "caps", get_caps);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "codec", get_codec);
    common::SetObjectTemplateField(locProto->PrototypeTemplate(), "type", type_static);
    common::SetObjectTemplateField(locProto->PrototypeTemplate(), "caps", caps_static);
    common::SetObjectTemplateField(locProto->PrototypeTemplate(), "codec", codec_static);
    //Nan::SetAccessor(locProto->PrototypeTemplate(), FIELD_NAME("status"), status);

    Nan::Set(target, FIELD_NAME("Encoder"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(IEncoder::get_name)
{
    obs::encoder handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle.name()).ToLocalChecked());
}

NAN_METHOD(IEncoder::set_name)
{
    obs::encoder handle = IEncoder::GetHandle(info.Holder());

    std::string name;

    ASSERT_GET_VALUE(info[0], name);

    handle.name(name);
}

NAN_METHOD(IEncoder::get_id)
{
    obs::encoder handle = IEncoder::GetHandle(info.Holder());
    
    info.GetReturnValue().Set(Nan::New(handle.id()).ToLocalChecked());
}

NAN_METHOD(IEncoder::get_caps)
{
    obs::encoder handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.caps());
}

NAN_METHOD(IEncoder::get_type)
{
    obs::encoder handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.type());
}

NAN_METHOD(IEncoder::get_codec)
{
    obs::encoder handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle.codec()).ToLocalChecked());
}

NAN_METHOD(IEncoder::update)
{
    obs::encoder handle = IEncoder::GetHandle(info.Holder());

    obs_data_t *settings;

    ASSERT_GET_VALUE(info[0], settings);

    handle.update(settings);
}

NAN_METHOD(IEncoder::caps_static)
{
    if (!info[0]->IsString()) {
        Nan::ThrowTypeError("Unexpected argument type");
        return;
    }

    Nan::Utf8String id(info[0]);

    info.GetReturnValue().Set(obs::encoder::caps(*id));
}

NAN_METHOD(IEncoder::type_static)
{
    if (!info[0]->IsString()) {
        Nan::ThrowTypeError("Unexpected argument type");
        return;
    }

    Nan::Utf8String id(info[0]);

    info.GetReturnValue().Set(obs::encoder::type(*id));
}

NAN_METHOD(IEncoder::codec_static)
{
    if (!info[0]->IsString()) {
        Nan::ThrowTypeError("Unexpected argument type");
        return;
    }

    Nan::Utf8String id(info[0]);

    info.GetReturnValue().Set(
        Nan::New(obs::encoder::codec(*id)).ToLocalChecked());
}

Nan::Persistent<v8::FunctionTemplate> IEncoder::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

}