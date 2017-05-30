#include "Module.h"
#include "Common.h"

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

    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("file_name"), file_name);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("name"), name);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("author"), author);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("description"), description);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("bin_path"), bin_path);
    Nan::SetAccessor(prototype->InstanceTemplate(), FIELD_NAME("data_path"), data_path);
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
    if (info.Length() != 2) {
        Nan::ThrowError("add_path requires two arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsString()) {
        Nan::ThrowTypeError("Expected string");
        return;
    }

    Nan::Utf8String bin_path(info[0]);
    Nan::Utf8String data_path(info[1]);

    obs::module::add_path(*bin_path, *data_path);
}

NAN_METHOD(Module::load_all)
{
    obs::module::load_all();
}

NAN_METHOD(Module::log_loaded)
{
    obs::module::log_loaded();
}

#define MODULE_UNWRAP \
    Module* object = Nan::ObjectWrap::Unwrap<Module>(info.Holder()); \
    obs::module &handle = object->handle;

/* Instance Methods */
NAN_METHOD(Module::initialize)
{
    MODULE_UNWRAP

    handle.initialize();
}

NAN_GETTER(Module::file_name)
{
    MODULE_UNWRAP

    info.GetReturnValue().Set(Nan::New(handle.file_name()).ToLocalChecked());
}

NAN_GETTER(Module::name)
{
    MODULE_UNWRAP

    info.GetReturnValue().Set(Nan::New(handle.name()).ToLocalChecked());
}

NAN_GETTER(Module::author)
{
    MODULE_UNWRAP

    info.GetReturnValue().Set(Nan::New(handle.author()).ToLocalChecked());
}

NAN_GETTER(Module::description)
{
    MODULE_UNWRAP

    info.GetReturnValue().Set(Nan::New(handle.description()).ToLocalChecked());
}

NAN_GETTER(Module::bin_path)
{
    MODULE_UNWRAP

    info.GetReturnValue().Set(Nan::New(handle.binary_path()).ToLocalChecked());
}

NAN_GETTER(Module::data_path)
{
    MODULE_UNWRAP

    info.GetReturnValue().Set(Nan::New(handle.data_path()).ToLocalChecked());
}

NAN_GETTER(Module::status)
{
    MODULE_UNWRAP

    info.GetReturnValue().Set(Nan::New(handle.status()));
}

}