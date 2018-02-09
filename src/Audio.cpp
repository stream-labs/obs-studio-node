#include "Audio.h"
#include "Common.h"

namespace osn
{

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
	prototype.Reset(locProto);
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

AudioEncoder::AudioEncoder(std::string id, std::string name,
                           obs_data_t *settings, size_t idx, obs_data_t *hotkeys)
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
	common::SetObjectTemplateField(locProto, "create", create);
	common::SetObjectTemplateField(locProto, "fromName", fromName);
	common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(),
	                                      "sampleRate", getSampleRate);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "getAudio",
	                               getAudio);
	common::SetObjectTemplateField(locProto->InstanceTemplate(), "setAudio",
	                               setAudio);
	common::SetObjectField(target, "AudioEncoder", locProto->GetFunction());
	prototype.Reset(locProto);
}

NAN_METHOD(AudioEncoder::create)
{
	ASSERT_INFO_LENGTH(info, 2);

	std::string id, name;
	uint32_t idx = 0;
	obs_data_t *settings = nullptr, *hotkeys = nullptr;

	ASSERT_GET_VALUE(info[0], id);
	ASSERT_GET_VALUE(info[1], name);

	switch (info.Length()) {
	default:
	case 5:
		ASSERT_GET_VALUE(info[4], hotkeys);
	case 4:
		ASSERT_GET_VALUE(info[3], settings);
	case 3:
		ASSERT_GET_VALUE(info[2], idx);
	case 2:
		break;
	}

	AudioEncoder *binding = new AudioEncoder(id, name, settings, idx, hotkeys);
	auto object = AudioEncoder::Object::GenerateObject(binding);
	info.GetReturnValue().Set(object);
}

NAN_METHOD(AudioEncoder::fromName)
{
	ASSERT_INFO_LENGTH(info, 1);

	std::string name;

	ASSERT_GET_VALUE(info[0], name);

	obs::audio_encoder audio_encoder_ref = obs::audio_encoder::from_name(name);

	if (audio_encoder_ref.status() != obs::audio_encoder::status_type::okay) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	AudioEncoder *binding = new AudioEncoder(audio_encoder_ref);
	auto object = AudioEncoder::Object::GenerateObject(binding);
	info.GetReturnValue().Set(object);
}


NAN_METHOD(AudioEncoder::setAudio)
{
	obs::weak<obs::audio_encoder> &handle = AudioEncoder::Object::GetHandle(
	                                              info.Holder());

	v8::Local<v8::Object> audio_obj;

	ASSERT_GET_VALUE(info[0], audio_obj);

	obs::audio &audio =
	      Audio::Object::GetHandle(audio_obj);

	handle.get()->audio(audio);
}

NAN_METHOD(AudioEncoder::getAudio)
{
	obs::weak<obs::audio_encoder> &handle = AudioEncoder::Object::GetHandle(
	                                              info.Holder());

	Audio *binding = new Audio(handle.get()->audio());
	auto ret_obj = Audio::Object::GenerateObject(binding);

	info.GetReturnValue().Set(ret_obj);
}

NAN_METHOD(AudioEncoder::getSampleRate)
{
	obs::weak<obs::audio_encoder> &handle = AudioEncoder::Object::GetHandle(
	                                              info.Holder());

	info.GetReturnValue().Set(common::ToValue(handle.get()->sample_rate()));
}

}