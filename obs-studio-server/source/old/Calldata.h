#pragma once

#include "Common.h"

#include <callback/calldata.h>

namespace osn {

/**
 Problems with this approach:
   * Since OBS controls the lifetime of a calldata
     structure, I have to copy the calldata structure 
     in order to make sure it's still alive by the time
     the node event loop process the event. Can't really
     avoid this in a very easy manner at the moment. 
     
   * It's slightly error prone (though the alternatives 
     aren't much better if not more error prone). 
 */

/**
    Very similar to the CALL_PARAM_TYPE_*
    enumeration but this is specific to the OSN
    wrapper. Required since libobs' enumeration
    doesn't describe individual objects so I don't
    know how to treat the data  generically.
    So, in order to make things more generic, I just
    create the maps myself.
 */
enum calldata_type {
    CALLDATA_TYPE_INPUT,
    CALLDATA_TYPE_TRANSITION,
    CALLDATA_TYPE_FILTER,
    CALLDATA_TYPE_SCENE,
    CALLDATA_TYPE_SCENEITEM,
    CALLDATA_TYPE_INT,
    CALLDATA_TYPE_FLOAT,
    CALLDATA_TYPE_BOOL,
    CALLDATA_TYPE_STRING,
    CALLDATA_TYPE_END
};

/**
    A generic structure that describes a single
    element of a calldata structure. 
 */
struct calldata_desc {
    const char *param_name;
    enum calldata_type param_type;
};

/** Generic callback data for signals */
struct callback_data { 
    callback_data(calldata_t data, calldata_desc *desc) 
      : calldata(data), desc(desc) { }
    calldata_t calldata;
    calldata_desc *desc;
    void *param;
};

v8::Local<v8::Value> ValueFromCalldata(callback_data *data);

v8::Local<v8::Value>
ValueFromCalldataParam(calldata_t *cd, const char* name, calldata_type type);

/**
    Copy the calldata structure manually since libobs doesn't provide
    us with such a function. The memory is marshaled and it's up to the
    caller to free the structure passed back.

    Since calldata gives us no fucking way to see what it actually,
    contains, it takes an array of calldata_desc objects that describes
    the parameters you want to copy.
 */
static inline calldata_t
CopyCalldata(calldata_t *data)
{
    calldata_t copy;

    /* This sucks. Most generic way right now. */
    copy.stack = (uint8_t*)bmalloc(data->capacity);
    memcpy(copy.stack, data->stack, data->capacity);
    copy.capacity = data->capacity;
    copy.size = data->size;
    copy.fixed = data->fixed;

    return copy;
}

template <typename BindingType>
static void GenericSignalHandler(void *data, calldata_t *calldata)
{
    BindingType *cb_binding = 
        reinterpret_cast<BindingType*>(data);

    uint32_t signal_type = *((uint32_t*)cb_binding->user_data);

    /* Careful not to use v8 reliant stuff here */
    callback_data *params = 
        new callback_data(CopyCalldata(calldata), (calldata_desc*)cb_binding->user_data);
    
    params->param = cb_binding;

    cb_binding->queue.send(params);
}

template <typename Parent, typename Item, typename Callback>
static void CalldataEventHandler(Parent *scene, Item *item)
{
    /* We're in v8 context here */
    Callback *cb_binding = 
        reinterpret_cast<Callback*>(item->param);

    /** We're in v8 context here */
    v8::Local<v8::Value> result = 
        ValueFromCalldata(item);

    if (cb_binding->stopped) {
        delete item; 
        return;
    }

    v8::Local<v8::Value> args[] = {
        result
    };

    cb_binding->cb.Call(1, args);

    delete item;
}

}