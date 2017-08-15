#include "Audio.h"
#include "Common.h"

namespace osn {

/*
 * Audio (audio_t) 
 */
NAN_MODULE_INIT(Audio::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Audio"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(locProto, "reset", reset);

    Nan::Set(target, FIELD_NAME("Audio"), locProto->GetFunction());
}

NAN_METHOD(Audio::reset)
{
    ASSERT_INFO_LENGTH(info, 1);

    uint32_t samples;
    int speakers;

    v8::Local<v8::Object> info_object;

    ASSERT_GET_VALUE(info[0], info_object);
    ASSERT_GET_OBJECT_FIELD(info_object, "samplesPerSec", samples);
    ASSERT_GET_OBJECT_FIELD(info_object, "speakerLayout", speakers);

    obs_audio_info audio_info;

    audio_info.samples_per_sec = samples;
    audio_info.speakers = static_cast<speaker_layout>(speakers);

    info.GetReturnValue().Set(common::ToValue(obs_reset_audio(&audio_info)));
}


/* 
 * Audio Encoder (obs_encoder_t)
 */

AudioEncoder::AudioEncoder(std::string id, std::string name)
 : handle(id, name)
{

}

obs::encoder *AudioEncoder::GetHandle()
{
    return static_cast<obs::encoder*>(&handle);
}

NAN_MODULE_INIT(AudioEncoder::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->Inherit(Nan::New(IEncoder::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("AudioEncoder"));
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("sample_rate"), sample_rate);
    Nan::Set(target, FIELD_NAME("AudioEncoder"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(AudioEncoder::New)
{
    if (!info.IsConstructCall()) {
        Nan::ThrowError("Must be used as a construct call");
        return;
    }

    if (info.Length() < 2) {
        Nan::ThrowError("Too few arguments provided");
        return;
    }
    
    if (!info[0]->IsString() || !info[1]->IsString()) {
        Nan::ThrowError("Invalid type passed");
        return;
    }

    Nan::Utf8String id(info[0]);
    Nan::Utf8String name(info[1]);
    AudioEncoder *object = new AudioEncoder(*id, *name);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_GETTER(AudioEncoder::sample_rate)
{
}

Nan::Persistent<v8::FunctionTemplate> AudioEncoder::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

}