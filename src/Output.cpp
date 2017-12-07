#include "Output.h"
#include "Video.h"
#include "Audio.h"
#include "Service.h"
#include "Properties.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Output::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Output::Output(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey)
    : handle(obs::output(id, name, settings, hotkey))
{
}

Output::Output(obs::output output)
 : handle(output)
{
}


NAN_MODULE_INIT(Output::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Output"));
    common::SetObjectTemplateField(locProto, "types", get_types);
    common::SetObjectTemplateField(locProto, "create", create);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "setMedia", setMedia);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "getVideo", getVideo);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "getAudio", getAudio);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "setReconnectOptions", setReconnectOptions);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "setPreferredSize", setPreferredSize);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "start", start);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "stop", stop);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "setDelay", setDelay);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "getDelay", getDelay);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "getActiveDelay", getActiveDelay);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "getAudioEncoder", getAudioEncoder);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "setAudioEncoder", setAudioEncoder);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "getVideoEncoder", getVideoEncoder);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "setVideoEncoder", setVideoEncoder);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "properties", get_properties);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "settings", get_settings);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "name", get_name);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "id", get_id);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "mixer", get_mixer, set_mixer);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "service", get_service, set_service);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "width", get_width);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "height", get_height);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "congestion", get_congestion);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "connectTime", get_connectTime);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "reconnecting", get_reconnecting);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "supportedVideoCodecs", get_supportedVideoCodecs);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "supportedAudioCodecs", get_supportedAudioCodecs);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "framesDropped", get_framesDropped);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "totalFrames", get_totalFrames);
    common::SetObjectField(target, "Output", locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Output::create)
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

    Output *binding = new Output(id, name, settings, hotkeys);
    auto object = Output::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Output::get_types)
{
    auto type_list = obs::output::types();
    int count = static_cast<int>(type_list.size());

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        Nan::Set(array, i, Nan::New<v8::String>(type_list[i]).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Output::get_settings)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->settings()));
}

NAN_METHOD(Output::get_properties)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    obs::properties props = handle.get()->properties();

    if (props.status() != obs::properties::status_type::okay) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Properties *bindings = new Properties(std::move(props));
    auto object = Properties::Object::GenerateObject(bindings);

    info.GetReturnValue().Set(object);
}

NAN_METHOD(Output::get_name)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->name()));
}

NAN_METHOD(Output::get_id)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->id()));
}

NAN_METHOD(Output::setMedia)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> video_obj, audio_obj;

    ASSERT_GET_VALUE(info[0], video_obj);
    ASSERT_GET_VALUE(info[1], audio_obj);

    obs::video &video = 
        Video::Object::GetHandle(video_obj);

    obs::audio &audio = 
        Audio::Object::GetHandle(audio_obj);

    handle.get()->media(video, audio);
}

NAN_METHOD(Output::getVideo)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    Video *binding = new Video(handle.get()->video());
    auto ret_obj = Video::Object::GenerateObject(binding);

    info.GetReturnValue().Set(ret_obj);
}

NAN_METHOD(Output::getAudio)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    Audio *binding = new Audio(handle.get()->audio());
    auto ret_obj = Audio::Object::GenerateObject(binding);

    info.GetReturnValue().Set(ret_obj);
}

NAN_METHOD(Output::get_mixer)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t mixer = handle.get()->mixer();

    info.GetReturnValue().Set(common::ToValue(mixer));
}

NAN_METHOD(Output::set_mixer)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t mixer;

    ASSERT_GET_VALUE(info[0], mixer);

    handle.get()->mixer(mixer);
}

NAN_METHOD(Output::setAudioEncoder)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> audioEncoder_obj;
    uint32_t idx;

    ASSERT_GET_VALUE(info[0], audioEncoder_obj);
    ASSERT_GET_VALUE(info[1], idx);

    obs::weak<obs::audio_encoder> &enc_handle = 
        AudioEncoder::Object::GetHandle(audioEncoder_obj);

    handle.get()->audio_encoder(enc_handle.get().get(), idx);
}

NAN_METHOD(Output::getAudioEncoder)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t idx;

    ASSERT_GET_VALUE(info[0], idx);

    AudioEncoder *binding = new AudioEncoder(handle.get()->audio_encoder(idx));
    auto ret_obj = AudioEncoder::Object::GenerateObject(binding);

    info.GetReturnValue().Set(ret_obj);
}

NAN_METHOD(Output::setVideoEncoder)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> videoEncoder_obj;

    ASSERT_GET_VALUE(info[0], videoEncoder_obj);

    obs::weak<obs::video_encoder> &enc_handle = 
        VideoEncoder::Object::GetHandle(videoEncoder_obj);

    handle.get()->video_encoder(enc_handle.get().get());
}

NAN_METHOD(Output::getVideoEncoder)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    VideoEncoder *binding = new VideoEncoder(handle.get()->video_encoder());
    auto ret_obj = VideoEncoder::Object::GenerateObject(binding);

    info.GetReturnValue().Set(ret_obj);
}

NAN_METHOD(Output::get_service)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    Service *binding = new Service(handle.get()->service());
    auto ret_obj = Service::Object::GenerateObject(binding);

    info.GetReturnValue().Set(ret_obj);
}

NAN_METHOD(Output::set_service)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> service_obj;

    ASSERT_GET_VALUE(info[0], service_obj);

    obs::weak<obs::service> &service_handle =
        Service::Object::GetHandle(service_obj);

    handle.get()->service(service_handle.get());
}

NAN_METHOD(Output::setReconnectOptions)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    int retry_amount, retry_delay;

    ASSERT_GET_VALUE(info[0], retry_amount);
    ASSERT_GET_VALUE(info[1], retry_delay);

    handle.get()->reconnect_options(retry_amount, retry_delay);
}

NAN_METHOD(Output::setPreferredSize)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t width, height;

    ASSERT_GET_VALUE(info[0], width);
    ASSERT_GET_VALUE(info[1], height);

    handle.get()->preferred_size(width, height);
}

NAN_METHOD(Output::get_width)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t width = handle.get()->width();

    info.GetReturnValue().Set(common::ToValue(width));
}

NAN_METHOD(Output::get_height)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t height = handle.get()->height();

    info.GetReturnValue().Set(common::ToValue(height));
}

NAN_METHOD(Output::get_congestion)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    float congestion = handle.get()->congestion();

    info.GetReturnValue().Set(common::ToValue(congestion));
}

NAN_METHOD(Output::get_connectTime)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    int connect_time = handle.get()->connect_time();

    info.GetReturnValue().Set(common::ToValue(connect_time));
}

NAN_METHOD(Output::get_reconnecting)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    bool reconnecting = handle.get()->reconnecting();

    info.GetReturnValue().Set(common::ToValue(reconnecting));
}

NAN_METHOD(Output::get_supportedVideoCodecs)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    std::string supported_video_codecs = handle.get()->supported_video_codecs();

    info.GetReturnValue().Set(common::ToValue(supported_video_codecs));
}

NAN_METHOD(Output::get_supportedAudioCodecs)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    std::string supported_audio_codecs = handle.get()->supported_audio_codecs();

    info.GetReturnValue().Set(common::ToValue(supported_audio_codecs));
}

NAN_METHOD(Output::get_framesDropped)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    int frames_dropped = handle.get()->frames_dropped();

    info.GetReturnValue().Set(common::ToValue(frames_dropped));
}

NAN_METHOD(Output::get_totalFrames)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    int total_frames = handle.get()->total_frames();

    info.GetReturnValue().Set(common::ToValue(total_frames));
}

NAN_METHOD(Output::start)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    handle.get()->start();
}

NAN_METHOD(Output::stop)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    handle.get()->stop();
}

NAN_METHOD(Output::setDelay)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t delay_amount;
    int flags;

    ASSERT_GET_VALUE(info[0], delay_amount);
    ASSERT_GET_VALUE(info[1], flags);

    handle.get()->delay(delay_amount, flags);
}

NAN_METHOD(Output::getDelay)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t delay_amount = handle.get()->delay();

    info.GetReturnValue().Set(delay_amount);
}

NAN_METHOD(Output::getActiveDelay)
{
    obs::weak<obs::output> &handle = Output::Object::GetHandle(info.Holder());

    uint32_t delay_amount = handle.get()->delay();

    info.GetReturnValue().Set(delay_amount);
}

}