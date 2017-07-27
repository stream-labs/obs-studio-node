#include "Common.h"
#include "Video.h"
#include "Input.h"
#include "Scene.h"
#include "Transition.h"

namespace osn {

VideoEncoder::VideoEncoder(std::string id, std::string name)
 : handle(id, name)
{

}

/*
 * Video (video_t) and associated functions
 */
NAN_MODULE_INIT(Video::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->SetClassName(FIELD_NAME("Video"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(locProto, "reset", reset);
    Nan::SetMethod(locProto, "getOutputSource", getOutputSource);
    Nan::SetMethod(locProto, "setOutputSource", setOutputSource);

    Nan::Set(target, FIELD_NAME("Video"), locProto->GetFunction());
}

NAN_METHOD(Video::New)
{
    Nan::ThrowError("video_t is not supported yet!");
}

NAN_METHOD(Video::reset)
{
    v8::Local<v8::Object> vi_object;

    ASSERT_GET_VALUE(info[0], vi_object);

    obs_video_info vi;
    std::string graphics_module;

    ASSERT_GET_OBJECT_FIELD(vi_object, "graphicsModule", graphics_module);
    ASSERT_GET_OBJECT_FIELD(vi_object, "fpsNum", vi.fps_num);
    ASSERT_GET_OBJECT_FIELD(vi_object, "fpsDen", vi.fps_den);
    ASSERT_GET_OBJECT_FIELD(vi_object, "baseWidth", vi.base_width);
    ASSERT_GET_OBJECT_FIELD(vi_object, "baseHeight", vi.base_height);
    ASSERT_GET_OBJECT_FIELD(vi_object, "outputWidth", vi.output_width);
    ASSERT_GET_OBJECT_FIELD(vi_object, "outputHeight", vi.output_height);
    ASSERT_GET_OBJECT_FIELD(vi_object, "outputFormat", vi.output_format);
    ASSERT_GET_OBJECT_FIELD(vi_object, "adapter", vi.adapter);
    ASSERT_GET_OBJECT_FIELD(vi_object, "gpuConversion", vi.gpu_conversion);
    ASSERT_GET_OBJECT_FIELD(vi_object, "colorspace", vi.colorspace);
    ASSERT_GET_OBJECT_FIELD(vi_object, "range", vi.range);
    ASSERT_GET_OBJECT_FIELD(vi_object, "scaleType", vi.scale_type);

    vi.graphics_module = graphics_module.c_str();

    info.GetReturnValue().Set(obs::video::reset(&vi));
}

NAN_METHOD(Video::setOutputSource)
{
    uint32_t channel;
    v8::Local<v8::Object> source_object;

    ASSERT_INFO_LENGTH(info, 2);
    ASSERT_GET_VALUE(info[0], channel);
    
    if (info[1]->IsNull()) {
        obs::video::output(channel, obs::source(nullptr));
        return;
    }
    
    ASSERT_GET_VALUE(info[1], source_object);

    obs::source source = ISource::GetHandle(source_object);
    obs::video::output(channel, source);
}

NAN_METHOD(Video::getOutputSource)
{
    ASSERT_INFO_LENGTH(info, 1);

    uint32_t channel;

    ASSERT_GET_VALUE(info[0], channel);

    obs::source source = obs::video::output(channel);

    if (source.type() == OBS_SOURCE_TYPE_INPUT) {
        Input *binding = new Input(source.dangerous());

        v8::Local<v8::Object> object = 
            Input::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }
    else if (source.type() == OBS_SOURCE_TYPE_SCENE) {
        Scene *binding = new Scene(source.dangerous());

        v8::Local<v8::Object> object = 
            Scene::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }
    else if (source.type() == OBS_SOURCE_TYPE_TRANSITION) {
        Transition *binding = new Transition(source.dangerous());

        v8::Local<v8::Object> object = 
            Transition::Object::GenerateObject(binding);

        info.GetReturnValue().Set(object);
    }

    obs_source_release(source.dangerous());
}

/* 
 * VideoEncoder
 */

obs::encoder *VideoEncoder::GetHandle()
{
    return static_cast<obs::encoder*>(&handle);
}

NAN_MODULE_INIT(VideoEncoder::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>(New);
    locProto->Inherit(Nan::New(IEncoder::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("VideoEncoder"));
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("height"), height);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("width"), width);
    //Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("scaled_size"), scaled_size);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("preferred_format"), preferred_format, preferred_format);
    Nan::Set(target, FIELD_NAME("VideoEncoder"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(VideoEncoder::New)
{
    if (!info.IsConstructCall()) {
        Nan::ThrowError("Must be used as a construct call");
        return;
    }

    ASSERT_INFO_LENGTH(info, 2);
    
    std::string id, name;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], name);

    VideoEncoder *object = new VideoEncoder(id, name);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_GETTER(VideoEncoder::height)
{

}

NAN_GETTER(VideoEncoder::width)
{

}

NAN_SETTER(VideoEncoder::scaled_size)
{

}

NAN_SETTER(VideoEncoder::preferred_format)
{

}

NAN_GETTER(VideoEncoder::preferred_format)
{

}

}