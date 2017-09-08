#include "Calldata.h"
#include "Scene.h"
#include "Input.h"
#include "Filter.h"
#include "Transition.h"
#include "SceneItem.h"

namespace osn {

v8::Local<v8::Value>
ValueFromCalldataParam(calldata_t *cd, const char* name, calldata_type type)
{
    switch (type) {
   case CALLDATA_TYPE_INPUT: {
       Input *binding = new Input((obs_source_t*)calldata_ptr(cd, name));
       return Input::Object::GenerateObject(binding);
   }
   case CALLDATA_TYPE_FILTER: {
       Filter *binding = new Filter((obs_source_t*)calldata_ptr(cd, name));
       return Filter::Object::GenerateObject(binding);
   }
   case CALLDATA_TYPE_SCENEITEM: {
       SceneItem *binding = new SceneItem((obs_sceneitem_t*)calldata_ptr(cd, name));
       return SceneItem::Object::GenerateObject(binding);
   }
   case CALLDATA_TYPE_TRANSITION: {
       Transition *binding = new Transition((obs_source_t*)calldata_ptr(cd, name));
       return Transition::Object::GenerateObject(binding);
   }
    case CALLDATA_TYPE_SCENE: {
        Scene *binding = new Scene((obs_scene_t*)calldata_ptr(cd, name));
        return Scene::Object::GenerateObject(binding);
    }
    case CALLDATA_TYPE_INT:
        return common::ToValue(calldata_int(cd, name));
    case CALLDATA_TYPE_FLOAT: 
        return common::ToValue(calldata_float(cd, name));
    case CALLDATA_TYPE_BOOL:
        return common::ToValue(calldata_bool(cd, name));
    case CALLDATA_TYPE_STRING:
        return common::ToValue(calldata_string(cd, name));
    case CALLDATA_TYPE_END:
        return Nan::Null();
    }
}

v8::Local<v8::Value> ValueFromCalldata(callback_data *data)
{
    v8::Local<v8::Object> result = Nan::New<v8::Object>();

    if (!data) { 
        return Nan::Null();
    }

    for (int i = 0; data->desc[i].param_type != CALLDATA_TYPE_END; ++i) {
        const char *name = data->desc[i].param_name;
        calldata_type type = data->desc[i].param_type;
        calldata_t *calldata = &data->calldata;

        v8::Local<v8::Value> value = 
            ValueFromCalldataParam(calldata, name, type);
        
        common::SetObjectField(result, name, value);
    }

    return result;
}

}