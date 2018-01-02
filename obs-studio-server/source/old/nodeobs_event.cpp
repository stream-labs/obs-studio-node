#include "nodeobs_common.h"
#include "nodeobs_event.h"
#include "nodeobs_service.h"

#include <iostream>

/* 
 * Source Events
 */
class GenericSourceEventWorker : public obs::ForeignWorker
{
    std::string source;

public:
    GenericSourceEventWorker(Nan::Callback *callback, std::string source)
        : obs::ForeignWorker(callback), source(source){ }

    void Execute() {
        v8::Local<v8::Value> args[] = { 
            Nan::New(source.c_str()).ToLocalChecked() 
        };

        Call(1, args);
    }
};

class BoolSourceEventWorker : public obs::ForeignWorker
{
    std::string source;
    bool data;

public:
    BoolSourceEventWorker(Nan::Callback *callback, std::string source, bool data)
        : obs::ForeignWorker(callback),
          source(source), data(data) { }

    void Execute() {

        v8::Local<v8::Value> args[] = { 
            Nan::New(source.c_str()).ToLocalChecked(), 
            Nan::New(data)
        };

        Call(2, args);
    }
};

class GenericEventWorker : public obs::ForeignWorker
{

public:
    GenericEventWorker(Nan::Callback *callback)
        : obs::ForeignWorker(callback) { }

    void Execute() {
        Call();
    }
};

void generic_event_cb(void *data, calldata_t *cd)
{
    uint32_t id = reinterpret_cast<uintptr_t>(data);
    obs::object object = g_objectManager.getObject(id);
    Nan::Callback *cb = static_cast<Nan::Callback*>(object.handle);
    GenericEventWorker *worker = new GenericEventWorker(cb);

    worker->Send();
}

/* For signals that don't have any parameters, this works generically */
static void generic_source_event_cb(void *data, calldata_t *cd)
{
	uint32_t id = reinterpret_cast<uintptr_t>(data);
	obs::object object = g_objectManager.getObject(id);
	Nan::Callback *cb = static_cast<Nan::Callback*>(object.handle);
    obs_source_t *source =
        static_cast<obs_source_t*>(calldata_ptr(cd, "source"));
    GenericSourceEventWorker *worker = new GenericSourceEventWorker(cb, obs_source_get_name(source));

    worker->Send();
}

static void mute_source_event_cb(void *data, calldata_t *cd)
{
    uint32_t id = reinterpret_cast<uint32_t>(data);
    obs_source_t *source = 
        static_cast<obs_source_t*>(calldata_ptr(cd, "source"));
    obs::object object = g_objectManager.getObject(id);
    Nan::Callback *cb = static_cast<Nan::Callback*>(object.handle);
    /* No need for a check, we know the object is valid */

    BoolSourceEventWorker *worker = 
        new BoolSourceEventWorker(cb, obs_source_get_name(source), calldata_bool(cd, "muted"));

    worker->Send();
}

/* FIXME: When we use the ID system for source, we can use the type checking
 * so we have only one function needed for all connect functions. 
 * As it is now, we only recieve a string which doesn't give us enough information. */

/* FIXME: Another case of using name for source identifier. */
static inline uint32_t source_signal_connect(
	const char *source_name,
	const char *event_type,
	v8::Local<v8::Function> js_cb,
	signal_callback_t cb)
{
    Nan::Callback *new_signal = 
        new Nan::Callback(js_cb);

    obs_source_t *source = obs_get_source_by_name(source_name);

    if (!source) {
        std::cerr << "Failed to find source: " << source_name << std::endl;
        return 0;
    }

    signal_handler_t *handler = obs_source_get_signal_handler(source);

	uint32_t id = g_objectManager.map({ obs::object::callback, new_signal });

	/* We use the void * as an integer.
	Not the most elegant but it does work fine here. */
	signal_handler_connect(handler,
		event_type, cb,
		reinterpret_cast<void*>((uintptr_t)id));

	return id;
}

#define SOURCE_SIGNAL_BINDING_IMPL(name, event_type, global_cb)     \
    void name##(const v8::FunctionCallbackInfo<v8::Value> &args)    \
    {                                                               \
		Nan::Utf8String source_name(args[0]);                       \
        uint32_t id = source_signal_connect(*source_name,           \
                event_type, args[1].As<v8::Function>(), global_cb); \
        auto new_object = Nan::New<v8::Object>();                   \
        obs::set_id(v8::Isolate::GetCurrent(), new_object, id);     \
        args.GetReturnValue().Set(new_object);                      \
    }

/* Source Signal Implementations */
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceRemoved, "remove", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceDestroyed, "destroy", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceSaved, "save", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceLoaded, "load", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceActivated, "activate", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceDeactivated, "deactivate", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceShown, "show", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceHidden, "hide", generic_source_event_cb);
SOURCE_SIGNAL_BINDING_IMPL(OBS_signal_sourceMuted, "mute", mute_source_event_cb);

/* 
 * Global Events 
 */


static inline uint32_t global_signal_connect(
        const char *event_type, 
        v8::Local<v8::Function> js_cb,
        signal_callback_t cb)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    signal_handler_t *handler = obs_get_signal_handler();

    Nan::Callback *new_signal =
        new Nan::Callback(js_cb);

    uint32_t id = g_objectManager.map({ obs::object::callback, new_signal });

    /* We use the void * as an integer.
    Not the most elegant but it does work fine here. */
    signal_handler_connect(handler,
        event_type, cb,
        reinterpret_cast<void*>((uintptr_t)id));

    return id;
}

#define SIGNAL_BINDING_IMPL(name, event_type, global_cb)            \
    void name##(const v8::FunctionCallbackInfo<v8::Value> &args)    \
    {                                                               \
        uint32_t id = global_signal_connect(                        \
                event_type, args[0].As<v8::Function>(), global_cb);\
        auto new_object = Nan::New<v8::Object>();                   \
        obs::set_id(v8::Isolate::GetCurrent(), new_object, id);     \
        args.GetReturnValue().Set(new_object);                      \
    }

SIGNAL_BINDING_IMPL(OBS_signal_createdSource, "source_create", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_destroyedSource, "source_destroy", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_removedSource, "source_remove", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_savedSource, "source_save", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_loadedSource, "source_load", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_activatedSource, "source_activate", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_deactivatedSource, "source_deactivate", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_showedSource, "source_show", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_hidSource, "source_hidden", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_startTransition, "source_transition_start", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_stopTransitionVideo, "source_transition_video_stop", generic_source_event_cb);
SIGNAL_BINDING_IMPL(OBS_signal_stopTransition, "source_transition_stop", generic_source_event_cb);

/*
 * Output Signals 
 */

/* FIXME: Output signals are just weird right now.
 * We don't pass anything back from callbacks since
 * I don't know how to work with this current setup.
 */
static inline uint32_t output_signal_connect(
        const char *event_type, 
        v8::Local<v8::Function> js_cb,
        signal_callback_t cb)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    signal_handler_t *handler = 
        obs_output_get_signal_handler(OBS_service::getStreamingOutput());

    Nan::Callback *new_signal =
        new Nan::Callback(js_cb);

    uint32_t id = g_objectManager.map({ obs::object::callback, new_signal });

    /* We use the void * as an integer.
    Not the most elegant but it does work fine here. */
    signal_handler_connect(handler,
        event_type, cb,
        reinterpret_cast<void*>((uintptr_t)id));

    return id;
}

#define OUTPUT_SIGNAL_BINDING_IMPL(name, event_type, global_cb)     \
    void name##(const v8::FunctionCallbackInfo<v8::Value> &args)    \
    {                                                               \
        uint32_t id = output_signal_connect(                        \
                event_type, args[0].As<v8::Function>(), global_cb); \
        auto new_object = Nan::New<v8::Object>();                   \
        obs::set_id(v8::Isolate::GetCurrent(), new_object, id);     \
        args.GetReturnValue().Set(new_object);                      \
    }

OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputStarted, "start", generic_event_cb);
OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputStopped, "stop", generic_event_cb);
OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputStarting, "starting", generic_event_cb);
OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputStopping, "stopping", generic_event_cb);
OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputActivated, "activate", generic_event_cb);
OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputDeactivated, "deactivate", generic_event_cb);
OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputReconnecting, "reconnect", generic_event_cb);
OUTPUT_SIGNAL_BINDING_IMPL(OBS_signal_outputReconnected, "reconnect_success", generic_event_cb);