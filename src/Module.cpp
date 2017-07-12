#include "Module.h"

namespace osn {

Module::Module(std::string path, std::string data_path)
    : handle(path, data_path)
{
}

NAN_METHOD(Module::New)
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

    Nan::Utf8String path(info[0]);
    Nan::Utf8String data_path(info[1]);

    Module *object = new Module(*path, *data_path);
    object->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_MODULE_INIT(Module::Init)
{
    auto module_ns = Nan::New<v8::Object>();

    auto prototype = Nan::New<v8::FunctionTemplate>(New);
    prototype->SetClassName(FIELD_NAME("Module"));
    prototype->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("fileName"), fileName);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("name"), name);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("author"), author);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("description"), description);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("binPath"), binPath);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("dataPath"), dataPath);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("status"), status);

    Nan::SetMethod(module_ns, "load_all", load_all);
    Nan::SetMethod(module_ns, "add_path", add_path);
    Nan::SetMethod(module_ns, "log_loaded", log_loaded);

    Nan::Set(target, FIELD_NAME("Module"), prototype->GetFunction());
    Nan::Set(target, FIELD_NAME("module"), module_ns);
}

/* Prototype Methods */
NAN_METHOD(Module::add_path)
{
    ASSERT_INFO_LENGTH(info, 2);
    
    std::string bin_path, data_path;

    ASSERT_GET_VALUE(info[0], bin_path);
    ASSERT_GET_VALUE(info[1], data_path);

    obs::module::add_path(bin_path, data_path);
}

NAN_METHOD(Module::load_all)
{
    obs::module::load_all();
}

NAN_METHOD(Module::log_loaded)
{
    obs::module::log_loaded();
}

/* Instance Methods */
NAN_METHOD(Module::initialize)
{
    obs::module &handle = Module::Object::GetHandle(info.Holder());

    handle.initialize();
}

NAN_GETTER(Module::fileName)
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