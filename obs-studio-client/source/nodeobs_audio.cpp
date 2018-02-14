#include <iostream>

#include <nan.h>

#include "nodeobs_common.h"
#include "nodeobs_content.h"

namespace {

obs_fader_t *get_fader(uint32_t id)
{
    return obs::get_handle<obs::object::fader, obs_fader_t*>(id);
}

obs_fader_t *get_fader(v8::Isolate *isolate, v8::Local<v8::Object> object)
{
    return obs::get_handle<obs::object::fader, obs_fader_t*>(isolate, object);
}

obs_volmeter_t *get_volmeter(uint32_t id)
{
    return obs::get_handle<obs::object::volmeter, obs_volmeter_t*>(id);
}

obs_volmeter_t *get_volmeter(v8::Isolate *isolate, v8::Local<v8::Object> object)
{
    return obs::get_handle<obs::object::volmeter, obs_volmeter_t*>(isolate, object);
}

}

void OBS_audio_createFader(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    uint32_t type = args[0]->ToUint32()->Value();
    obs_fader_t *handle = obs_fader_create(static_cast<obs_fader_type>(type));
    obs::object object = { obs::object::fader, handle };
    uint32_t id = g_objectManager.map(object);
    v8::Local<v8::Object> result = v8::Object::New(isolate);
    
    obs::set_id(isolate, result, id);

    args.GetReturnValue().Set(result);

    std::cout << "Creating fader: " << (void*)handle << std::endl;
}

void OBS_audio_destroyFader(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    uint32_t id = obs::get_id(isolate, object);
    obs_fader_t *fader = get_fader(id);
    obs_fader_destroy(fader);
    g_objectManager.unmap(id);

    std::cout << "Destroying fader: " << (void*)fader << std::endl;
}

void OBS_audio_faderSetDb(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Number> db = args[1]->ToNumber();

    obs_fader_t *fader = get_fader(isolate, object);

    obs_fader_set_db(fader, static_cast<float>(db->Value()));
}

void OBS_audio_faderGetDb(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    obs_fader_t *fader = get_fader(isolate, object);

    args.GetReturnValue().Set(obs_fader_get_db(fader));
}

void OBS_audio_faderSetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Number> def = args[1]->ToNumber();

    obs_fader_t *fader = get_fader(isolate, object);

    obs_fader_set_deflection(fader, static_cast<float>(def->Value()));
}

void OBS_audio_faderGetDeflection(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    obs_fader_t *fader = get_fader(isolate, object);

    args.GetReturnValue().Set(obs_fader_get_deflection(fader));
}

void OBS_audio_faderSetMul(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Number> mul = args[1]->ToNumber();

    obs_fader_t *fader = get_fader(isolate, object);

    obs_fader_set_mul(fader, static_cast<float>(mul->Value()));
}

void OBS_audio_faderGetMul(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    obs_fader_t *fader = get_fader(isolate, object);

    args.GetReturnValue().Set(obs_fader_get_mul(fader));
}

/*
 * FIXME - This uses the old method of fetching sources. 
 * When we fix sources to use the ID system, this needs to change. 
 */
void OBS_audio_faderAttachSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::String::Utf8Value sourceName(args[1]->ToString());

    obs_source_t *source = obs_get_source_by_name(*sourceName);
    obs_fader_t *fader = get_fader(isolate, object);
    uint32_t fader_id = obs::get_id(isolate, object);

    if (!source) {
        std::cerr << "Failed to find source for attachment: " 
            << *sourceName << std::endl;
        return;
    }
    
    auto info_it = sourceInfo.find(*sourceName);
    
    if (info_it == sourceInfo.end()) {
        sourceInfo.insert( decltype(sourceInfo)::value_type(*sourceName, new SourceInfo { fader_id, 0 }) );
    }
    else {
        if (info_it->second->fader) {
            /* Make sure it was attached in case it was recreated. */
            obs_fader_attach_source(fader, source);
            obs_source_release(source);
            std::cerr << "Fader already attached to source!" << std::endl;
            return;
        } 
        
        info_it->second->fader = fader_id;
    }

    obs_fader_attach_source(fader, source);
    obs_source_release(source);
}

void OBS_audio_faderDetachSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    obs_fader_t *fader = get_fader(isolate, object);

    obs_fader_detach_source(fader);
}

namespace {

void fader_main_loop_cb(uv_async_t* handle);

struct FaderData {
    obs::CallbackInfo *cb_info;
    float db;
};

obs::CallbackQueue<FaderData> faderCallbackQueue(fader_main_loop_cb);

void fader_main_loop_cb(uv_async_t* handle)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    /* We're in v8 context here */
    std::unique_lock<std::mutex> lock(faderCallbackQueue.mutex);

    while (!faderCallbackQueue.work_queue.empty()) {
        auto &data = faderCallbackQueue.work_queue.front();

        if (data.cb_info->stopped)
            goto next_element;

        auto result = Nan::New<v8::Object>();

        Nan::Set(result,
            Nan::New("db").ToLocalChecked(),
            Nan::New(data.db));

        v8::Local<v8::Value> params[] = {
            result
        };

        data.cb_info->callback.Call(1, params);

next_element:
        faderCallbackQueue.work_queue.pop_front();
    }
}

void fader_cb_wrapper(void *param, float db)
{
    obs::CallbackInfo *cb = reinterpret_cast<obs::CallbackInfo*>(param);

    {
        std::unique_lock<std::mutex> lock(faderCallbackQueue.mutex);
        faderCallbackQueue.work_queue.push_back({cb, db});
    }

    faderCallbackQueue.Signal();
}

}

void OBS_audio_faderAddCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[1]);

    obs_fader_t *fader = get_fader(isolate, object);

    obs::CallbackInfo *cb_info = new obs::CallbackInfo(callback);
    obs_fader_add_callback(fader, fader_cb_wrapper, cb_info);
    uint32_t id = g_objectManager.map({ obs::object::callback, cb_info });

    v8::Local<v8::Object> result = v8::Object::New(isolate);
    obs::set_id(isolate, result, id);

    args.GetReturnValue().Set(result);
}

void OBS_audio_faderRemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Object> cb_object = args[1]->ToObject();

    obs_fader_t *fader = get_fader(isolate, object);
    
    uint32_t cb_id = obs::get_id(isolate, cb_object);
    obs::CallbackInfo *cb_info =
        obs::get_handle<obs::object::callback, obs::CallbackInfo*>(cb_id);

    g_objectManager.unmap(cb_id);
    obs_fader_remove_callback(fader, fader_cb_wrapper, cb_info);
}

void OBS_audio_createVolmeter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    uint32_t type = args[0]->ToUint32()->Value();
    obs_volmeter_t *handle = obs_volmeter_create(static_cast<obs_fader_type>(type));
    obs::object object = { obs::object::volmeter, handle };
    uint32_t id = g_objectManager.map(object);
    v8::Local<v8::Object> result = v8::Object::New(isolate);
    
    obs::set_id(isolate, result, id);

    args.GetReturnValue().Set(result);

    std::cout << "Creating volmeter: " << (void*)handle << std::endl;
}

void OBS_audio_destroyVolmeter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    uint32_t id = obs::get_id(isolate, object);
    obs_volmeter_t *fader = get_volmeter(id);
    obs_volmeter_destroy(fader);
    g_objectManager.unmap(id);

    std::cout << "Destroying volmeter: " << (void*)fader << std::endl;
}

/*
 * FIXME - This uses the old method of fetching sources. 
 * When we fix sources to use the ID system, this needs to change. 
 */
void OBS_audio_volmeterAttachSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::String::Utf8Value sourceName(args[1]->ToString());

    obs_source_t *source = obs_get_source_by_name(*sourceName);
    obs_volmeter_t *volmeter = get_volmeter(isolate, object);
    uint32_t volmeter_id = obs::get_id(isolate, object);

    if (!source) {
        std::cerr << "Failed to find source for attachment: " 
            << *sourceName << std::endl;
        return;
    }

    auto info_it = sourceInfo.find(*sourceName);
    
    if (info_it == sourceInfo.end()) {
        sourceInfo.insert( decltype(sourceInfo)::value_type(*sourceName, new SourceInfo { 0, volmeter_id }) );
    }
    else {
        if (info_it->second->volmeter) {
            /* Make sure the source is attached in case it was recreated. */
            obs_volmeter_attach_source(volmeter, source);
            obs_source_release(source);
            std::cerr << "Fader already attached to source!" << std::endl;
            return;
        } 
        
        info_it->second->volmeter = volmeter_id;
    }

    obs_volmeter_attach_source(volmeter, source);
    obs_source_release(source);
}

void OBS_audio_volmeterDetachSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    obs_volmeter_t *volmeter = get_volmeter(isolate, object);

    obs_volmeter_detach_source(volmeter);
}

void OBS_audio_volmeterSetUpdateInterval(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Uint32> ms = args[1]->ToUint32();

    obs_volmeter_t *volmeter = get_volmeter(isolate, object);

    obs_volmeter_set_update_interval(volmeter, ms->Value());
}

void OBS_audio_volmeterGetUpdateInterval(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    obs_volmeter_t *volmeter = get_volmeter(isolate, object);

    args.GetReturnValue().Set(obs_volmeter_get_update_interval(volmeter));
}

void OBS_audio_volmeterSetPeakHold(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Uint32> ms = args[1]->ToUint32();

    obs_volmeter_t *volmeter = get_volmeter(isolate, object);

    obs_volmeter_set_peak_hold(volmeter, ms->Value());
}

void OBS_audio_volmeterGetPeakHold(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();

    obs_volmeter_t *volmeter = get_volmeter(isolate, object);

    args.GetReturnValue().Set(obs_volmeter_get_peak_hold(volmeter));
}

namespace {

void volmeter_main_loop_cb(uv_async_t* handle);

struct VolmeterData {
    obs::CallbackInfo *cb_info;
    float level;
    float magnitude;
    float peak;
    float muted;
};

obs::CallbackQueue<VolmeterData> vmCallbackQueue(volmeter_main_loop_cb);

void volmeter_main_loop_cb(uv_async_t* handle)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    /* We're in v8 context here */
    std::unique_lock<std::mutex> lock(vmCallbackQueue.mutex);

    while (!vmCallbackQueue.work_queue.empty()) {
        auto &data = vmCallbackQueue.work_queue.front();

        if (data.cb_info->stopped)
            goto next_element;

        auto result = Nan::New<v8::Object>();

        Nan::Set(result,
            Nan::New("level").ToLocalChecked(),
            Nan::New(data.level));

        Nan::Set(result,
            Nan::New("magnitude").ToLocalChecked(),
            Nan::New(data.magnitude));

        Nan::Set(result,
            Nan::New("peak").ToLocalChecked(),
            Nan::New(data.peak));

        Nan::Set(result,
            Nan::New("muted").ToLocalChecked(),
            Nan::New(data.muted));

        v8::Local<v8::Value> params[] = {
            result
        };

        data.cb_info->callback.Call(1, params);

next_element:
        vmCallbackQueue.work_queue.pop_front();
    }
}

void volmeter_cb_wrapper(void *param, float level,
    float magnitude, float peak, float muted)
{
    obs::CallbackInfo *cb = reinterpret_cast<obs::CallbackInfo*>(param);

    {
        std::unique_lock<std::mutex> lock(vmCallbackQueue.mutex);
        vmCallbackQueue.work_queue.push_back({cb, level, magnitude, peak, muted});
    }

    vmCallbackQueue.Signal();
}

}

void OBS_audio_volmeterAddCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Function> callback = args[1].As<v8::Function>();

    obs_volmeter_t *volmeter = get_volmeter(isolate, object);

    obs::CallbackInfo *cb = new obs::CallbackInfo(callback);
    obs_volmeter_add_callback(volmeter, volmeter_cb_wrapper, cb);

    uint32_t id = g_objectManager.map({ obs::object::callback, cb });

    v8::Local<v8::Object> result = v8::Object::New(isolate);
    obs::set_id(isolate, result, id);

    args.GetReturnValue().Set(result);
}

void OBS_audio_volmeterRemoveCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate *isolate = args.GetIsolate();
    v8::Local<v8::Object> object = args[0]->ToObject();
    v8::Local<v8::Object> cb_object = args[1]->ToObject();

    obs_volmeter_t *volmeter = get_volmeter(isolate, object);

    uint32_t cb_id = obs::get_id(isolate, cb_object);
    obs::CallbackInfo *cb =
        obs::get_handle<obs::object::callback, obs::CallbackInfo*>(cb_id);
    g_objectManager.unmap(cb_id);

    cb->stopped = true;
    obs_volmeter_remove_callback(volmeter, volmeter_cb_wrapper, cb);

    /* We have no to tell if there's an event still left in the queue. 
     * We also can't take advantage of garbage collection here. 
     * Unfortunately, let it leak for now.  */
}