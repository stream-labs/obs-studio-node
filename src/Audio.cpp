#include "Audio.h"
#include "Common.h"

namespace osn {

/*
 * Audio (audio_t) 
 */
Nan::Persistent<v8::FunctionTemplate> Audio::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Audio::Audio(audio_t *audio)
 : handle(audio)
{

}

Audio::Audio(obs::audio audio)
 : handle(audio)
{
}


NAN_MODULE_INIT(Audio::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Audio"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    common::SetObjectTemplateField(locProto, "reset", reset);
    common::SetObjectTemplateField(locProto, "getGlobal", getGlobal);

    common::SetObjectField(target, "Audio", locProto->GetFunction());
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

NAN_METHOD(Audio::getGlobal)
{
    Audio *binding = new Audio(obs::audio::global());
    auto object = Audio::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}


/* 
 * Audio Encoder (obs_encoder_t)
 */
Nan::Persistent<v8::FunctionTemplate> AudioEncoder::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

AudioEncoder::AudioEncoder(std::string id, std::string name, obs_data_t *settings, size_t idx, obs_data_t *hotkeys)
 : handle(obs::audio_encoder(id, name, settings, idx, hotkeys))
{
}

AudioEncoder::AudioEncoder(obs::audio_encoder encoder)
 : handle(encoder)
{
}

obs::encoder AudioEncoder::GetHandle()
{
    return handle.get().get();
}

NAN_MODULE_INIT(AudioEncoder::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(IEncoder::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("AudioEncoder"));
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "sampleRate", get_sampleRate);
    common::SetObjectField(target, "AudioEncoder", locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(AudioEncoder::create)
{
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

NAN_METHOD(AudioEncoder::get_sampleRate)
{
}

}