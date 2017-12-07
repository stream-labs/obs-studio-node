#include "Common.h"
#include "Video.h"

/**
 * A wrapper over video_t and 
 * associated media-io functionality
 */
namespace osn {

/*
 * Video (video_t) and associated functions
 */
Nan::Persistent<v8::FunctionTemplate> Video::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Video::Video(obs::video video)
 : handle(video)
{
}

NAN_MODULE_INIT(Video::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->SetClassName(FIELD_NAME("Audio"));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);

    common::SetObjectTemplateField(locProto, "reset", reset);
    common::SetObjectTemplateField(locProto, "getGlobal", getGlobal);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "skippedFrames", get_skippedFrames);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "totalFrames", get_totalFrames);

    common::SetObjectField(target, "Video", locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Video::get_skippedFrames)
{
    info.GetReturnValue().Set(common::ToValue(obs::video::global().skipped_frames()));
}

NAN_METHOD(Video::get_totalFrames)
{
    info.GetReturnValue().Set(common::ToValue(obs::video::global().total_frames()));
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

NAN_METHOD(Video::getGlobal)
{
    Video *binding = new Video(obs::video::global());
    auto object = Video::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

/* 
 * VideoEncoder
 */
Nan::Persistent<v8::FunctionTemplate> VideoEncoder::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

VideoEncoder::VideoEncoder(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkeys)
 : handle(obs::video_encoder(id, name, settings, hotkeys))
{
}

VideoEncoder::VideoEncoder(obs::video_encoder encoder)
 : handle(encoder)
{
}

obs::encoder VideoEncoder::GetHandle()
{
    return handle.get().get();
}

NAN_MODULE_INIT(VideoEncoder::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(IEncoder::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("VideoEncoder"));
    common::SetObjectTemplateField(locProto, "create", create);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "height", get_height);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "width", get_width);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "scaledSize", get_scaledSize);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "preferredFormat", get_preferredFormat, set_preferredFormat);
    common::SetObjectField(target, "VideoEncoder", locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(VideoEncoder::create)
{
    ASSERT_INFO_LENGTH(info, 2);
    
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

    VideoEncoder *binding = new VideoEncoder(id, name, settings, hotkeys);
    auto object = VideoEncoder::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(VideoEncoder::get_height)
{
    
}

NAN_METHOD(VideoEncoder::get_width)
{

}

NAN_METHOD(VideoEncoder::get_scaledSize)
{

}

NAN_METHOD(VideoEncoder::get_preferredFormat)
{

}

NAN_METHOD(VideoEncoder::set_preferredFormat)
{

}

}