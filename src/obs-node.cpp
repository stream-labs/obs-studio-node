#include "obspp/obspp.hpp"
#include <nan.h>

#define NAME(name) \
    Nan::New(#name).ToLocalChecked()

namespace osn {
namespace global {
/* Please note that setters/getters have required defaults. 
 * This means you should expect a getter to be called without
 * any context what so ever. */

#define OBS_VALID \
	if (obs::status() != obs::status_type::okay) \
		return; \

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

NAN_METHOD(startup)
{
    Nan::Utf8String locale(info[0]);
    Nan::Utf8String path(info[1]);

    auto ReturnValue = info.GetReturnValue();

    switch (info.Length()) {
    case 0:
        Nan::ThrowError("Locale must be specified");
        break;
    case 1:
        ReturnValue.Set(obs::startup(*locale));
        break;
    case 2:
        ReturnValue.Set(obs::startup(*locale, *path));
        break;
    default:
        Nan::ThrowError("Incorrect number of arguments");
    }
}

NAN_METHOD(shutdown)
{
    obs::shutdown();
}

NAN_GETTER(locale)
{
    OBS_VALID

    info.GetReturnValue().Set(
        Nan::New(obs::locale()).ToLocalChecked());
}

NAN_SETTER(locale)
{
    OBS_VALID

    Nan::Utf8String locale(value);

    obs::locale(*locale);
}

NAN_GETTER(status)
{
    info.GetReturnValue().Set(obs::status());
}

NAN_GETTER(version)
{
    info.GetReturnValue().Set(obs::version());
}

}

namespace module {

NAN_METHOD(add_path)
{
    if (info.Length() != 2) {
        Nan::ThrowError("add_path requires two arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsString()) {
        Nan::ThrowTypeError("add_path requires two strings");
        return;
    }

    Nan::Utf8String bin_path(info[0]);
    Nan::Utf8String data_path(info[1]);

    obs::module::add_path(*bin_path, *data_path);
}

NAN_METHOD(load_all)
{
    obs::module::load_all();
}

NAN_METHOD(log_loaded)
{
    obs::module::log_loaded();
}

}

namespace output {

NAN_METHOD(output_source)
{
    auto ReturnValue = info.GetReturnValue();

    switch (info.Length()) {
    case 1:
        uint32_t channel
            = Nan::To<uint32_t>(info[0]).FromJust();

        return;
    }
}

}

namespace source {

/* FIXME:
    We can use a prototype that encompasses most of
    the source functionality that the individual 
    source types can "inherit" from. Right now, each
    object has its own prototype which is pretty 
    useless. . 
 */

class Input : public Nan::ObjectWrap
{
public:
    obs::input handle;

    Input(std::string id, std::string name, obs_data_t *settings)
        : handle(id, name, settings)
    {

    }

    static NAN_MODULE_INIT(Init)
    {
        auto prototype = Nan::New<v8::FunctionTemplate>(New);

        /* Used internally by the Wrap function */
        prototype->InstanceTemplate()->SetInternalFieldCount(1);

        /* Name of Prototype class */
        prototype->SetClassName(NAME(Input));
        
        Nan::SetAccessor(prototype->InstanceTemplate(), NAME(name), name);
        Nan::SetPrototypeMethod(prototype, "release", release);

        Nan::Set(target, NAME(Input), prototype->GetFunction());
    }

    static NAN_METHOD(New)
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

        Input *object = new Input(*id, *name, nullptr);

        object->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }

    static NAN_GETTER(name)
    {
        Input* object = ObjectWrap::Unwrap<Input>(info.Holder());

        info.GetReturnValue().Set(Nan::New(object->handle.name()).ToLocalChecked());
    }

    static NAN_METHOD(release)
    {
        Input* object = ObjectWrap::Unwrap<Input>(info.Holder());

        object->handle.release();
        object->MakeWeak();

        /* Any use of the object passed this point is undefined! 
         * The wrapper will clean up the allocated object whenever
         * the garbage collector is called. */
    }
};

}

}

NAN_MODULE_INIT(node_initialize)
{
    using namespace osn;

    /* obs (global) namespace */
    auto global = Nan::New<v8::Object>();

    Nan::SetAccessor(global, NAME(status), global::status);
    Nan::SetAccessor(global, NAME(locale), global::locale, global::locale);
    Nan::SetAccessor(global, NAME(version), global::version);
    Nan::SetMethod(global, "startup", global::startup);
    Nan::SetMethod(global, "shutdown", global::shutdown);

    /* module object */
    auto module = Nan::New<v8::Object>();
    Nan::SetMethod(module, "add_path", module::add_path);
    Nan::SetMethod(module, "load_all", module::load_all);
    Nan::SetMethod(module, "log_loaded", module::log_loaded);

    /* source object */
    osn::source::Input::Init(global);

    Nan::Set(global, NAME(module), module);

    Nan::Set(target, NAME(obs), global);
}

NODE_MODULE(osn, node_initialize)