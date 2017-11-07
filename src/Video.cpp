#include "Common.h"
#include "Video.h"

/**
 * A wrapper over video_t and 
 * associated media-io functionality
 */
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
    auto ObsVideo = Nan::New<v8::Object>();
    locProto->SetClassName(FIELD_NAME("Video"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(ObsVideo, "reset", reset);
    Nan::SetAccessor(ObsVideo, FIELD_NAME("skippedFrames"), skippedFrames);
    Nan::SetAccessor(ObsVideo, FIELD_NAME("totalFrames"), totalFrames);

    Nan::Set(target, FIELD_NAME("Video"), ObsVideo);
}

NAN_METHOD(Video::New)
{
    Nan::ThrowError("video_t is not supported yet!");
}

NAN_GETTER(Video::skippedFrames)
{
    info.GetReturnValue().Set(common::ToValue(obs::video::skipped_frames()));
}

NAN_GETTER(Video::totalFrames)
{
    info.GetReturnValue().Set(common::ToValue(obs::video::total_frames()));
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
    //Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("scaledSize"), scaledSize);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("preferredFormat"), preferredFormat, preferredFormat);
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

NAN_SETTER(VideoEncoder::scaledSize)
{

}

NAN_SETTER(VideoEncoder::preferredFormat)
{

}

NAN_GETTER(VideoEncoder::preferredFormat)
{

}

}