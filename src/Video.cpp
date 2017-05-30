#include "Common.h"
#include "Video.h"
#include "Input.h"

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
    auto video_ns = Nan::New<v8::Object>();
    auto prototype = Nan::New<v8::FunctionTemplate>(New);
    prototype->SetClassName(FIELD_NAME("Video"));
    prototype->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetMethod(video_ns, "reset", reset);
    Nan::SetMethod(video_ns, "output", output);

    Nan::Set(target, FIELD_NAME("Video"), prototype->GetFunction());
    Nan::Set(target, FIELD_NAME("video"), video_ns);
}

NAN_METHOD(Video::New)
{
    Nan::ThrowError("video_t is not supported yet!");
}

NAN_METHOD(Video::reset)
{
    v8::Local<v8::Object> vi_object = 
        Nan::To<v8::Object>(info[0]).ToLocalChecked();

    #define VI_OBJECT_FETCH(name) \
        v8::Local<v8::Value> name ## _local = \
            Nan::Get(vi_object, FIELD_NAME(#name)).ToLocalChecked();

    /* Note that the compiler should optimize these away. */
    VI_OBJECT_FETCH(graphics_module);
    VI_OBJECT_FETCH(fps_num);
    VI_OBJECT_FETCH(fps_den);
    VI_OBJECT_FETCH(base_width);
    VI_OBJECT_FETCH(base_height);
    VI_OBJECT_FETCH(output_width);
    VI_OBJECT_FETCH(output_height);
    VI_OBJECT_FETCH(output_format);
    VI_OBJECT_FETCH(adapter);
    VI_OBJECT_FETCH(gpu_conversion);
    VI_OBJECT_FETCH(colorspace);
    VI_OBJECT_FETCH(range);
    VI_OBJECT_FETCH(scale_type);

    #undef VI_OBJECT_FETCH

    Nan::Utf8String graphics_module(graphics_module_local);

    struct obs_video_info vi = {
        *graphics_module,
        Nan::To<uint32_t>(fps_num_local).FromJust(),
        Nan::To<uint32_t>(fps_den_local).FromJust(),
        Nan::To<uint32_t>(base_width_local).FromJust(),
        Nan::To<uint32_t>(base_height_local).FromJust(),
        Nan::To<uint32_t>(output_width_local).FromJust(),
        Nan::To<uint32_t>(output_height_local).FromJust(),
        static_cast<video_format>(Nan::To<int>(output_format_local).FromJust()),
        Nan::To<uint32_t>(adapter_local).FromJust(),
        Nan::To<bool>(gpu_conversion_local).FromJust(),
        static_cast<video_colorspace>(Nan::To<int>(colorspace_local).FromJust()),
        static_cast<video_range_type>(Nan::To<int>(range_local).FromJust()),
        static_cast<obs_scale_type>(Nan::To<int>(scale_type_local).FromJust())
    };

    info.GetReturnValue().Set(obs::video::reset(&vi));
}

NAN_METHOD(Video::output)
{
    uint32_t channel = 
        Nan::To<uint32_t>(info[0]).FromJust();

    switch(info.Length()) {
    case 1: {
        v8::Local<v8::Object> object = 
            Input::GenerateObject(obs::video::output(channel));

        info.GetReturnValue().Set(object);
        break;
    }
    case 2: {
        

        if (info[1]->IsNull()) {
            obs::video::output(channel, obs::input(nullptr));
            break;
        }

        obs::input *handle = 
            Input::GetHandle(info[1]->ToObject());

        obs::video::output(channel, *handle);
        break;
    }
    default: 
        Nan::ThrowError("Unexpected number of arguments");
        break;
    }
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
    VideoEncoder *object = new VideoEncoder(*id, *name);
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