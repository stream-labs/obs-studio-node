#include <iostream>

#include "AudioControls.h"
#include "ISource.h"

namespace osn {

Fader::Fader(obs_fader_type type)
 : obs::fader(type),
   handle(*this)
{

}

NAN_MODULE_INIT(Fader::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Fader"));
    Nan::SetMethod(locProto, "create", create);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("db"), db, db);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("deflection"), deflection, deflection);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("mul"), mul, mul);
    Nan::SetMethod(locProto->InstanceTemplate(), "attach", attach);
    Nan::SetMethod(locProto->InstanceTemplate(), "detach", detach);
    Nan::SetMethod(locProto->InstanceTemplate(), "addCallback", addCallback);
    Nan::SetMethod(locProto->InstanceTemplate(), "removeCallback", removeCallback);
    Nan::Set(target, FIELD_NAME("Fader"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Fader::create)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 1);
    
    int fader_type;

    ASSERT_GET_VALUE(info[0], fader_type);

    Fader *binding = new Fader(static_cast<obs_fader_type>(fader_type));
    auto object = Fader::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_GETTER(Fader::db)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.db());
}

NAN_SETTER(Fader::db)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    float db;

    ASSERT_GET_VALUE(value, db);

    handle.db(db);
}

NAN_GETTER(Fader::deflection)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.deflection());
}

NAN_SETTER(Fader::deflection)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    float def;

    ASSERT_GET_VALUE(value, def);

    handle.deflection(def);
}

NAN_GETTER(Fader::mul)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.mul());
}

NAN_SETTER(Fader::mul)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    float mul;

    ASSERT_GET_VALUE(value, mul);

    handle.mul(mul);
}

NAN_METHOD(Fader::attach)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);
    
    v8::Local<v8::Object> source_object;

    ASSERT_GET_VALUE(info[0], source_object);

    obs::source source = ISource::GetHandle(source_object);

    handle.attach(source);
}

NAN_METHOD(Fader::detach)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    handle.detach();
}

static void fader_cb_wrapper(
    void *param, float db)
{
    FaderCallback *cb_binding = 
        reinterpret_cast<FaderCallback*>(param);

    /* Careful not to use v8 reliant stuff here */
    Fader::Data *data = new Fader::Data;

    data->param = param;
    data->db = db;

    cb_binding->queue.send(data);
}

void Fader::Callback(Fader *fader, Fader::Data *item)
{
    /* We're in v8 context here */
    FaderCallback *cb_binding = 
        reinterpret_cast<FaderCallback*>(item->param);

    v8::Local<v8::Object> object = Nan::New<v8::Object>();

    common::SetObjectField(object, "db", item->db);

    delete item;

    v8::Local<v8::Value> args[] = {
        object
    };

    cb_binding->cb.Call(1, args);
}

Nan::Persistent<v8::FunctionTemplate> FaderCallback::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

Nan::Persistent<v8::FunctionTemplate> Fader::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

NAN_METHOD(Fader::addCallback)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());
    Fader* binding = Nan::ObjectWrap::Unwrap<Fader>(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    v8::Local<v8::Function> callback;
    ASSERT_GET_VALUE(info[0], callback);

    FaderCallback *cb_binding = 
        new FaderCallback(binding, Fader::Callback, callback);

    handle.add_callback(fader_cb_wrapper, cb_binding);

    auto object = FaderCallback::Object::GenerateObject(cb_binding);
    cb_binding->obj_ref.Reset(object);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Fader::removeCallback)
{
    obs::fader &handle = Fader::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> cb_object;
    ASSERT_GET_VALUE(info[0], cb_object);

    FaderCallback *cb_binding = 
        FaderCallback::Object::GetHandle(cb_object);

    cb_binding->stopped = true;

    handle.remove_callback(fader_cb_wrapper, cb_binding);
    cb_binding->obj_ref.Reset();

    /* What's this? A memory leak? Nope! The GC will decide when
     * and where to destroy the object. */
}


Volmeter::Volmeter(obs_fader_type type)
 : obs::volmeter(type),
   handle(*this)
{
}

Nan::Persistent<v8::FunctionTemplate> Volmeter::prototype =
    Nan::Persistent<v8::FunctionTemplate>();

NAN_MODULE_INIT(Volmeter::Init)
{
    auto locProto = Nan::New<v8::FunctionTemplate>();
    locProto->InstanceTemplate()->SetInternalFieldCount(1);
    locProto->SetClassName(FIELD_NAME("Volmeter"));
    Nan::SetMethod(locProto, "create", create);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("peakHold"), peakHold, peakHold);
    Nan::SetAccessor(locProto->InstanceTemplate(), FIELD_NAME("updateInterval"), updateInterval, updateInterval);
    Nan::SetMethod(locProto->InstanceTemplate(), "attach", attach);
    Nan::SetMethod(locProto->InstanceTemplate(), "detach", detach);
    Nan::SetMethod(locProto->InstanceTemplate(), "addCallback", addCallback);
    Nan::SetMethod(locProto->InstanceTemplate(), "removeCallback", removeCallback);
    Nan::Set(target, FIELD_NAME("Volmeter"), locProto->GetFunction());
    prototype.Reset(locProto);
}

NAN_METHOD(Volmeter::create)
{
    ASSERT_INFO_LENGTH_AT_LEAST(info, 1);
    
    int fader_type;

    ASSERT_GET_VALUE(info[0], fader_type);

    Volmeter *binding = new Volmeter(static_cast<obs_fader_type>(fader_type));
    auto object = Volmeter::Object::GenerateObject(binding);
    info.GetReturnValue().Set(object);
}

NAN_GETTER(Volmeter::peakHold)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.peak_hold());
}

NAN_SETTER(Volmeter::peakHold)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

    int peak_hold;

    ASSERT_GET_VALUE(value, peak_hold);

    handle.peak_hold(peak_hold);
}

NAN_GETTER(Volmeter::updateInterval)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

    info.GetReturnValue().Set(handle.interval());
}

NAN_SETTER(Volmeter::updateInterval)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

    int ms;

    ASSERT_GET_VALUE(value, ms);

    handle.interval(ms);
}

NAN_METHOD(Volmeter::attach)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    v8::Local<v8::Object> source_object;

    ASSERT_GET_VALUE(info[0], source_object);

    obs::source source = ISource::GetHandle(source_object);

    handle.attach(source);
}

NAN_METHOD(Volmeter::detach)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

    handle.detach();
}

static void volmeter_cb_wrapper(
    void *param, float level,
    float magnitude, float peak, float muted)
{
    VolmeterCallback *cb_binding = 
        reinterpret_cast<VolmeterCallback*>(param);

    /* Careful not to use v8 reliant stuff here */
    Volmeter::Data *data = new Volmeter::Data;

    data->param = param;
    data->level = level;
    data->magnitude = magnitude;
    data->peak = peak;
    data->muted = muted;

    cb_binding->queue.send(data);
}

void Volmeter::Callback(Volmeter *volmeter, Volmeter::Data *item)
{
    /* We're in v8 context here */
    VolmeterCallback *cb_binding = 
        reinterpret_cast<VolmeterCallback*>(item->param);

    v8::Local<v8::Object> object = Nan::New<v8::Object>();

    if (cb_binding->stopped) {
        delete item; 
        return;
    }

    common::SetObjectField(object, "level", item->level);
    common::SetObjectField(object, "magnitude", item->magnitude);
    common::SetObjectField(object, "peak", item->peak);
    common::SetObjectField(object, "muted", static_cast<bool>(item->muted));

    delete item;

    v8::Local<v8::Value> args[] = {
        object
    };

    cb_binding->cb.Call(1, args);
}

NAN_METHOD(Volmeter::addCallback)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());
    Volmeter* binding = Nan::ObjectWrap::Unwrap<Volmeter>(info.Holder());

    ASSERT_INFO_LENGTH(info, 1);

    v8::Local<v8::Function> callback;
    ASSERT_GET_VALUE(info[0], callback);

    VolmeterCallback *cb_binding = 
        new VolmeterCallback(binding, Volmeter::Callback, callback);

    handle.add_callback(volmeter_cb_wrapper, cb_binding);

    auto object = VolmeterCallback::Object::GenerateObject(cb_binding);
    cb_binding->obj_ref.Reset(object);
    info.GetReturnValue().Set(object);
}

NAN_METHOD(Volmeter::removeCallback)
{
    obs::volmeter &handle = Volmeter::Object::GetHandle(info.Holder());

    v8::Local<v8::Object> cb_object;
    ASSERT_GET_VALUE(info[0], cb_object);

    VolmeterCallback *cb_binding = 
        VolmeterCallback::Object::GetHandle(cb_object);

    cb_binding->stopped = true;
    cb_binding->obj_ref.Reset();

    handle.remove_callback(volmeter_cb_wrapper, cb_binding);

    /* What's this? A memory leak? Nope! The GC will automagically
     * destroy the CallbackData structure when it becomes weak. We
     * just need to make sure its in an unusable state. */
}

Nan::Persistent<v8::FunctionTemplate> VolmeterCallback::prototype = 
    Nan::Persistent<v8::FunctionTemplate>();

}