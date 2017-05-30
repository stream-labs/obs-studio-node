#include "IEncoder.h"
#include "Common.h"

namespace osn{

obs::encoder* IEncoder::GetHandle(v8::Local<v8::Object> object)
{
    IEncoderHandle* encoder = Nan::ObjectWrap::Unwrap<IEncoder>(object);
    return encoder->GetHandle();
}

NAN_MODULE_INIT(IEncoder::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Encoder"));
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("name"), name, name);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("display_name"), display_name);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("id"), id);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("type"), type);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("caps"), caps);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("codec"), codec);
    Nan::SetMethod(locProto->PrototypeTemplate(), "type", type_static);
    Nan::SetMethod(locProto->PrototypeTemplate(), "caps", caps_static);
    Nan::SetMethod(locProto->PrototypeTemplate(), "codec", codec_static);
    Nan::SetAccessor(locProto->PrototypeTemplate(), FIELD_NAME("types"), types);
    //Nan::SetAccessor(locProto->PrototypeTemplate(), FIELD_NAME("status"), status);

    Nan::Set(target, FIELD_NAME("Encoder"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_GETTER(IEncoder::display_name)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(
        Nan::New(handle->display_name()).ToLocalChecked());
}

NAN_GETTER(IEncoder::name)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle->name()).ToLocalChecked());
}

NAN_SETTER(IEncoder::name)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());
    
    if (!value->IsString()) {
        Nan::ThrowTypeError("Unexpected argument type");
        return;
    }

    Nan::Utf8String name(value);

    handle->name(*name);
}

NAN_GETTER(IEncoder::id)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());
    
    info.GetReturnValue().Set(Nan::New(handle->id()).ToLocalChecked());
}

NAN_GETTER(IEncoder::caps)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->caps());
}

NAN_GETTER(IEncoder::type)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle->type());
}

NAN_GETTER(IEncoder::codec)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());

    info.GetReturnValue().Set(Nan::New(handle->codec()).ToLocalChecked());
}

NAN_METHOD(IEncoder::update)
{
    obs::encoder *handle = IEncoder::GetHandle(info.Holder());

    handle->update();
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

NAN_GETTER(IEncoder::types)
{
    auto types = obs::encoder::types();
    uint32_t array_size = static_cast<uint32_t>(types.size());
    auto array = Nan::New<v8::Array>(array_size);
    
    for (uint32_t i = 0; i < array_size; ++i) {
        Nan::Set(array, i, Nan::New(types[i]).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}

Nan::Persistent<v8::FunctionTemplate> IEncoder::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

}