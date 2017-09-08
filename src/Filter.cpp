#include "Filter.h"

namespace osn {

Nan::Persistent<v8::FunctionTemplate> Filter::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Filter::Filter(std::string id, std::string name, obs_data_t *settings)
 : handle(obs::filter(id, name, settings))
{
}

Filter::Filter(obs::filter filter)
 : handle(obs::filter(filter))
{
}

obs::source Filter::GetHandle()
{
    return handle.get().get();
}

NAN_MODULE_INIT(Filter::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->Inherit(Nan::New(ISource::prototype));
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Filter"));
    Nan::SetMethod(locProto, "types", types);
    Nan::SetMethod(locProto, "create", create);
    Nan::Set(target, FIELD_NAME("Filter"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Filter::types)
{
    auto type_list = obs::filter::types();
    int count = static_cast<int>(type_list.size());

    auto array = Nan::New<v8::Array>(count);

    for (int i = 0; i < count; ++i) {
        Nan::Set(array, i, Nan::New<v8::String>(type_list[i]).ToLocalChecked());
    }

    info.GetReturnValue().Set(array);
}


NAN_METHOD(Filter::create)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 2);
    
    std::string id, name;
    obs_data_t *settings = nullptr;

    ASSERT_GET_VALUE(info[0], id);
    ASSERT_GET_VALUE(info[1], name);

    switch (info.Length()) {
    default:
    case 3:
        ASSERT_GET_VALUE(info[2], settings);
    case 2:
        break;
    }

    Filter *binding = new Filter(id, name, settings);
    auto object = Filter::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

}