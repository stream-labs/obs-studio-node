#include "Module.h"

namespace osn {

Module::Module(std::string path, std::string data_path)
    : handle(path, data_path)
{
}

NAN_METHOD(Module::create)
{
    ASSERT_INFO_LENGTH(info, 2);
    
    std::string path, data_path;

    ASSERT_GET_VALUE(info[0], path);
    ASSERT_GET_VALUE(info[1], data_path);

    Module *object = new Module(path, data_path);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_MODULE_INIT(Module::Init)
{
    auto prototype = Nan::New<v8::FunctionTemplate>();
    prototype->SetClassName(FIELD_NAME("Module"));
    prototype->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("filename"), filename);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("name"), name);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("author"), author);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("description"), description);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("binPath"), binPath);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("dataPath"), dataPath);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("status"), status);

    Nan::SetMethod(prototype, "loadAll", loadAll);
    Nan::SetMethod(prototype, "addPath", addPath);
    Nan::SetMethod(prototype, "logLoaded", logLoaded);

    Nan::Set(target, FIELD_NAME("Module"), prototype->GetFunction());
}

/* Prototype Methods */
NAN_METHOD(Module::addPath)
{
    ASSERT_INFO_LENGTH(info, 2);
    
    std::string bin_path, data_path;

    ASSERT_GET_VALUE(info[0], bin_path);
    ASSERT_GET_VALUE(info[1], data_path);

    obs::module::add_path(bin_path, data_path);
}

NAN_METHOD(Module::loadAll)
{
    obs::module::load_all();
}

NAN_METHOD(Module::logLoaded)
{
    obs::module::log_loaded();
}

/* Instance Methods */
NAN_METHOD(Module::initialize)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    handle.initialize();
}

NAN_GETTER(Module::filename)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.file_name()));
}

NAN_GETTER(Module::name)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.name()));
}

NAN_GETTER(Module::author)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.author()));
}

NAN_GETTER(Module::description)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.description()));
}

NAN_GETTER(Module::binPath)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.binary_path()));
}

NAN_GETTER(Module::dataPath)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(common::ToValue(handle.data_path()));
}

NAN_GETTER(Module::status)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.status());
}

}