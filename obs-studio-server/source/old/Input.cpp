#include <nan.h>

#include "Input.h"
#include "Filter.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Input::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Input::Input(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey)
    : handle(obs::input(id, name, settings, hotkey))
{
}

Input::Input(std::string id, std::string name, obs_data_t *settings, bool is_private)
    : handle(obs::input(id, name, settings, is_private))
{
}

Input::Input(obs::input input)
 : handle(input)
{
}

obs::source Input::GetHandle()
{
    return handle.get().get();
}

NAN_MODULE_INIT(Input::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Input"));
    common::SetObjectTemplateField(locProto, "types", types);
    common::SetObjectTemplateField(locProto, "create", create);
    common::SetObjectTemplateField(locProto, "createPrivate", createPrivate);
    common::SetObjectTemplateField(locProto, "fromName", fromName);
    common::SetObjectTemplateField(locProto, "getPublicSources", getPublicSources);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "duplicate", duplicate);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "copyFilters", copyFilters);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "findFilter", findFilter);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "addFilter", addFilter);
    common::SetObjectTemplateField(locProto->InstanceTemplate(), "removeFilter", removeFilter);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "width", get_width);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "height", get_height);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "filters", get_filters);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "volume", get_volume, set_volume);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "syncOffset", get_syncOffset, set_syncOffset);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "showing", get_showing);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "audioMixers", get_audioMixers, set_audioMixers);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "monitoringType", get_monitoringType, set_monitoringType);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "deinterlaceFieldOrder", get_deinterlaceFieldOrder, set_deinterlaceFieldOrder);
    common::SetObjectTemplateLazyAccessor(locProto->InstanceTemplate(), "deinterlaceMode", get_deinterlaceMode, set_deinterlaceMode);
    common::SetObjectField(target, "Input", locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Input::types)
{
    auto type_list = obs::input::types();
    int count = static_cast<int>(type_list.size());

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        Nan::Set(array, i, Nan::New<v8::String>(type_list[i]).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Input::getPublicSources)
{
    auto inputs = obs::input::public_sources();
    int count = static_cast<int>(inputs.size());

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        obs_source_release(inputs[i].dangerous());
        Input *binding = new Input(inputs[i]);
        auto object = Input::Object::GenerateObject(binding);
        Nan::Set(array, i, object);
    }

    info.GetReturnValue().Set(array);
}

NAN_METHOD(Input::duplicate)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    std::string name = "";
    bool is_private = false;
    Input *binding = nullptr;

    switch (info.Length()) {
    default:
    case 2:
        ASSERT_GET_VALUE(info[1], is_private);
    case 1: 
        ASSERT_GET_VALUE(info[0], name);
        binding = new Input(handle.get()->duplicate(name, is_private));
        break;
    case 0:
        handle.get()->addref();
        binding = new Input(handle.get().get());
        break;
    }

    auto object = Input::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Input::create)
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


    Input *binding = new Input(id, name, settings, hotkeys);
    auto object = Input::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Input::createPrivate)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 2);
    
    std::string id, name;
    obs_data_t *settings = nullptr;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], name);

    if (info.Length() > 2)
        ASSERT_GET_VALUE(info[2], settings);

    Input *binding = new Input(id, name, settings, true);
    auto object = Input::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Input::get_width)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.width());
}

NAN_METHOD(Input::get_height)
{
    obs::source handle = ISource::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.height());
}

NAN_METHOD(Input::fromName)
{
    ASSERT_INFO_LENGTH(info, 1);

    std::string name;

    ASSERT_GET_VALUE(info[0], name);

    obs::input input_ref = obs::input::from_name(name);

    if (input_ref.status() != obs::input::status_type::okay) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    Input *binding = new Input(input_ref);
    auto object = Input::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Input::get_volume)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.get()->volume());
}

NAN_METHOD(Input::set_volume)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    float volume;

    ASSERT_GET_VALUE(info[0], volume);

    handle.get()->volume(volume);
}

NAN_METHOD(Input::get_syncOffset)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());
    uint64_t offset = handle.get()->sync_offset();
    timespec tv;

    tv.tv_sec = offset / 1000000000;
    tv.tv_nsec = offset % 1000000000;

    info.GetReturnValue().Set(common::ToValue(tv));
}

NAN_METHOD(Input::set_syncOffset)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());
    uint64_t offset;
    timespec tv;
    
    ASSERT_GET_VALUE(info[0], tv);

    offset = (tv.tv_sec * 1000000000) + tv.tv_nsec;
    
    handle.get()->sync_offset(offset);
}

NAN_METHOD(Input::get_active)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->active()));
}

NAN_METHOD(Input::get_showing)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.get()->showing());
}

NAN_METHOD(Input::get_audioMixers)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.get()->audio_mixers());
}

NAN_METHOD(Input::set_audioMixers)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    uint32_t flags;

    ASSERT_GET_VALUE(info[0], flags);

    handle.get()->audio_mixers(flags);
}

NAN_METHOD(Input::copyFilters)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> source_src_obj;

    ASSERT_GET_VALUE(info[0], source_src_obj);

    obs::weak<obs::input> &source_src = Input::Object::GetHandle(source_src_obj);

    handle.get()->copy_filters(source_src.get().get());
}

NAN_METHOD(Input::findFilter)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());
    
    ASSERT_INFO_LENGTH(info, 1);

    std::string filter_name;

    ASSERT_GET_VALUE(info[0], filter_name);

    obs::filter found = handle.get()->find_filter(filter_name.c_str());

    if (found.status() != obs::source::status_type::okay) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }

    obs_source_release(found.dangerous());
    Filter *binding = new Filter(found);
    auto object = Filter::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Input::get_monitoringType)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->monitoring_type()));
}

NAN_METHOD(Input::set_monitoringType)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    int type;

    ASSERT_GET_VALUE(info[0], type);

    handle.get()->monitoring_type(static_cast<obs_monitoring_type>(type));
}

NAN_METHOD(Input::get_deinterlaceFieldOrder)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->deinterlace_field_order()));
}

NAN_METHOD(Input::set_deinterlaceFieldOrder)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    int order;

    ASSERT_GET_VALUE(info[0], order);

    handle.get()->deinterlace_field_order(static_cast<obs_deinterlace_field_order>(order));
}

NAN_METHOD(Input::get_deinterlaceMode)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->deinterlace_mode()));
}

NAN_METHOD(Input::set_deinterlaceMode)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    int mode;

    ASSERT_GET_VALUE(info[0], mode);

    handle.get()->deinterlace_mode(static_cast<obs_deinterlace_mode>(mode));
}

NAN_METHOD(Input::addFilter)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    v8::Local<v8::Object> filter_obj;

    ASSERT_GET_VALUE(info[0], filter_obj);

    obs::weak<obs::filter> &filter = Filter::Object::GetHandle(filter_obj);

    handle.get()->add_filter(filter.get().get());
}

NAN_METHOD(Input::removeFilter)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    v8::Local<v8::Object> filter_obj;

    ASSERT_GET_VALUE(info[0], filter_obj);

    obs::weak<obs::filter> &filter = Filter::Object::GetHandle(filter_obj);

    handle.get()->remove_filter(filter.get().get());
}

NAN_METHOD(Input::get_filters)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    std::vector<obs::filter> filters = handle.get()->filters();

    int size = static_cast<int>(filters.size());
    v8::Local<v8::Array> array = Nan::New<v8::Array>(size);

    for (int i = 0; i < size; ++i) {
        Filter *binding = new Filter(filters[i]);
        filters[i].release();
        auto object = Filter::Object::GenerateObject(binding);

        common::SetObjectField(array, i, object);
    }

    info.GetReturnValue().Set(array);
}

}