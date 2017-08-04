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
    Nan::SetMethod(locProto, "types", types);
    Nan::SetMethod(locProto, "create", create);
    Nan::SetMethod(locProto, "createPrivate", createPrivate);
    Nan::SetMethod(locProto, "fromName", fromName);
    Nan::SetMethod(locProto, "getPublicSources", getPublicSources);
    Nan::SetMethod(locProto->InstanceTemplate(), "duplicate", duplicate);
    Nan::SetMethod(locProto->InstanceTemplate(), "findFilter", findFilter);
    Nan::SetMethod(locProto->InstanceTemplate(), "addFilter", addFilter);
    Nan::SetMethod(locProto->InstanceTemplate(), "removeFilter", removeFilter);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("filters"), filters);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("volume"), volume, volume);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("syncOffset"), syncOffset, syncOffset);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("showing"), showing);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("audioMixers"), audioMixers, audioMixers);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("monitoringType"), monitoringType, monitoringType);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("deinterlaceFieldOrder"), deinterlaceFieldOrder, deinterlaceFieldOrder);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("deinterlaceMode"), deinterlaceMode, deinterlaceMode);
    Nan::Set(target, FIELD_NAME("Input"), locProto->GetFunction());
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
    bool is_private = false;

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

    Input *binding;

    if (info.Length() > 2)
        ASSERT_GET_VALUE(info[2], settings);


    binding = new Input(id, name, settings, true);
    auto object = Input::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
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

NAN_GETTER(Input::volume)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.get()->volume());
}

NAN_SETTER(Input::volume)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    float volume;

    ASSERT_GET_VALUE(value, volume);

    handle.get()->volume(volume);
}

NAN_GETTER(Input::syncOffset)
{
    /* TODO Needs a 64-bit offset */
}

NAN_SETTER(Input::syncOffset)
{
    /* TODO Needs a 64-bit offset */
}

NAN_GETTER(Input::showing)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.get()->showing());
}

NAN_GETTER(Input::audioMixers)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.get()->audio_mixers());
}

NAN_SETTER(Input::audioMixers)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    uint32_t flags;

    ASSERT_GET_VALUE(value, flags);

    handle.get()->audio_mixers(flags);
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

NAN_GETTER(Input::monitoringType)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->monitoring_type()));
}

NAN_SETTER(Input::monitoringType)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    int type;

    ASSERT_GET_VALUE(value, type);

    handle.get()->monitoring_type(static_cast<obs_monitoring_type>(type));
}

NAN_GETTER(Input::deinterlaceFieldOrder)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->deinterlace_field_order()));
}

NAN_SETTER(Input::deinterlaceFieldOrder)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    int order;

    ASSERT_GET_VALUE(value, order);

    handle.get()->deinterlace_field_order(static_cast<obs_deinterlace_field_order>(order));
}

NAN_GETTER(Input::deinterlaceMode)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.get()->deinterlace_mode()));
}

NAN_SETTER(Input::deinterlaceMode)
{
    obs::weak<obs::input> &handle = Input::Object::GetHandle(info.Holder());

    int mode;

    ASSERT_GET_VALUE(value, mode);

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

NAN_GETTER(Input::filters)
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