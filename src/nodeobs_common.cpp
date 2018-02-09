#include "nodeobs_content.h"
#include "nodeobs_common.h"
#include "nodeobs_obspp_manager.hpp"
#include <map>
#include <iomanip>

/* For sceneitem transform modifications. 
 * We should consider moving this to another module */
#include <graphics/matrix4.h>

vector<std::string> tabScenes;
map<obs_source_t*, obs_properties_t*> propsCache;
vector<const char*> tabTypeInputSources;
map<std::string, SourceInfo*> sourceInfo;
map<std::string, OBS::Display*> displays;
string currentScene;
string currentTransition;
map<string, obs_source_t*> privateSources;
map<string, SourceInfo*> privateSourceInfo;
map<string, obs_source_t*> transitions;
uint32_t transitionDuration = 300;
bool isMuted;
bool isUpdatePropertiesThreadRunning = false;
std::mutex updatePropertiesMutex;
string sourceSelected;

OBS_content::OBS_content()
{

}
OBS_content::~OBS_content()
{

}

#define NODE_OBS_SOURCE(value, variable_name) \
	obs_source_t * variable_name; \
	{ \
		v8::String::Utf8Value source_name(value); \
		variable_name = obs_get_source_by_name(*source_name); \
		\
		if (!source) { \
			cerr << "Failed to find source " << *source_name << std::endl; \
			return; \
		} \
	}

#define NODE_OBS_SOURCE_RELEASE(source) \
	obs_source_release(source);

#define NODE_OBS_SCENE(value, variable_name) \
	obs_scene_t * variable_name; \
	{ \
		v8::String::Utf8Value scene_name(value); \
		obs_source_t *source = obs_get_source_by_name(*scene_name); \
		\
		if (!source) { \
			cerr << "Failed to find source " << *scene_name << std::endl; \
			return; \
		} \
		\
		variable_name = obs_scene_from_source(source); \
		\
		if (!variable_name) { \
			std::cerr << *scene_name << " is not a scene." << std::endl; \
			obs_source_release(source); \
			return; \
		} \
	}

#define NODE_OBS_SCENE_RELEASE(scene) \
	obs_scene_release(scene);

#define NODE_OBS_SCENEITEM(value, scene, variable_name) \
	obs_sceneitem_t * variable_name; \
	{ \
		String::Utf8Value item_name(value); \
		variable_name = obs_scene_find_source(scene, *item_name); \
		\
		if (!variable_name) { \
			cerr << "Failed to find " << *item_name << " in scene " \
				<< obs_source_get_name(obs_scene_get_source(scene)) << std::endl; \
			return; \
		} \
	}

/* Finding a sceneitem doesn't inrease refcount */

#define NODE_OBS_FILTER(value, source, variable_name) \
	obs_source_t *variable_name; \
	{ \
		String::Utf8Value filter_name(value); \
		variable_name = obs_source_get_filter_by_name(source, *filter_name); \
		\
		if (!variable_name) { \
			cerr << "Failed to find filter " << *filter_name << " in source " \
				<< obs_source_get_name(source) << endl; \
			obs_source_release(source); \
			return; \
		} \
	}

#define NODE_OBS_FILTER_RELEASE(filter) \
	obs_source_release(filter);

obs_properties_t *refresh_cached_properties(obs_source_t *source)
{
	auto it = propsCache.find(source);
	obs_properties_t *properties = nullptr;

	if (it != propsCache.end()) {
		properties = obs_source_properties(source);
		obs_properties_destroy(it->second);
		it->second = properties;
	}
	else {
		properties = obs_source_properties(source);
		propsCache[source] = properties;
	}

	return properties;
}

obs_properties_t *get_cached_properties(obs_source_t* source)
{
	auto it = propsCache.find(source);
	obs_properties_t *properties = nullptr;

	if (it != propsCache.end()) {
		properties = it->second;
	} else {
		properties = obs_source_properties(source);
		propsCache[source] = properties;
	}

	return properties;
}

void remove_cached_properties(obs_source_t* source)
{
	auto it = propsCache.find(source);

	if (it != propsCache.end()) {
		obs_properties_destroy(it->second);
		propsCache.erase(it);
	}
}

/* A lot of the sceneitem functionality is a lazy copy-pasta from the Qt UI. */
// https://github.com/jp9000/obs-studio/blob/master/UI/window-basic-main.cpp#L4888
static void GetItemBox(obs_sceneitem_t *item, vec3 &tl, vec3 &br)
{
	matrix4 boxTransform;
	obs_sceneitem_get_box_transform(item, &boxTransform);

	vec3_set(&tl, M_INFINITE, M_INFINITE, 0.0f);
	vec3_set(&br, -M_INFINITE, -M_INFINITE, 0.0f);

	auto GetMinPos = [&] (float x, float y)
	{
		vec3 pos;
		vec3_set(&pos, x, y, 0.0f);
		vec3_transform(&pos, &pos, &boxTransform);
		vec3_min(&tl, &tl, &pos);
		vec3_max(&br, &br, &pos);
	};

	GetMinPos(0.0f, 0.0f);
	GetMinPos(1.0f, 0.0f);
	GetMinPos(0.0f, 1.0f);
	GetMinPos(1.0f, 1.0f);
}

static vec3 GetItemTL(obs_sceneitem_t *item)
{
	vec3 tl, br;
	GetItemBox(item, tl, br);
	return tl;
}

static void SetItemTL(obs_sceneitem_t *item, const vec3 &tl)
{
	vec3 newTL;
	vec2 pos;

	obs_sceneitem_get_pos(item, &pos);
	newTL = GetItemTL(item);
	pos.x += tl.x - newTL.x;
	pos.y += tl.y - newTL.y;
	obs_sceneitem_set_pos(item, &pos);
}

static bool CenterAlignSelectedItems(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param)
{
	obs_bounds_type boundsType = *reinterpret_cast<obs_bounds_type*>(param);

	if (!obs_sceneitem_selected(item))
		return true;

	obs_video_info ovi;
	obs_get_video_info(&ovi);

	obs_transform_info itemInfo;
	vec2_set(&itemInfo.pos, 0.0f, 0.0f);
	vec2_set(&itemInfo.scale, 1.0f, 1.0f);
	itemInfo.alignment = OBS_ALIGN_LEFT | OBS_ALIGN_TOP;
	itemInfo.rot = 0.0f;

	vec2_set(&itemInfo.bounds,
			float(ovi.base_width), float(ovi.base_height));
	itemInfo.bounds_type = boundsType;
	itemInfo.bounds_alignment = OBS_ALIGN_CENTER;

	obs_sceneitem_set_info(item, &itemInfo);

	UNUSED_PARAMETER(scene);
	return true;
}


static bool MultiplySelectedItemScale(obs_scene_t *scene, obs_sceneitem_t *item,
		void *param)
{
	vec2 &mul = *reinterpret_cast<vec2*>(param);

	if (!obs_sceneitem_selected(item))
		return true;

	vec3 tl = GetItemTL(item);

	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);
	vec2_mul(&scale, &scale, &mul);
	obs_sceneitem_set_scale(item, &scale);

	SetItemTL(item, tl);

	UNUSED_PARAMETER(scene);
	return true;
}

/* Perhaps some of these should be in a separate utility library for slobs. 
 * Otherwise, these are usually handled by the frontend. We're doing them here,
 * since it requires vector math and it would be unwise to do it in JS. */
void OBS_content::OBS_content_flipHorzSceneItems(const FunctionCallbackInfo<Value>& args)
{
	NODE_OBS_SCENE(args[0], scene)

	vec2 scale;
	vec2_set(&scale, -1.0f, 1.0f);
	obs_scene_enum_items(scene, MultiplySelectedItemScale,
			&scale);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_flipVertSceneItems(const FunctionCallbackInfo<Value>& args)
{
	NODE_OBS_SCENE(args[0], scene)

	vec2 scale;
	vec2_set(&scale, 1.0f, -1.0f);
	obs_scene_enum_items(scene, MultiplySelectedItemScale,
			&scale);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_resetSceneItems(const FunctionCallbackInfo<Value>& args)
{
	NODE_OBS_SCENE(args[0], scene);

	auto func = [] (obs_scene_t *scene, obs_sceneitem_t *item, void *param)
	{
		if (!obs_sceneitem_selected(item))
			return true;

		obs_sceneitem_defer_update_begin(item);

		obs_transform_info info;
		vec2_set(&info.pos, 0.0f, 0.0f);
		vec2_set(&info.scale, 1.0f, 1.0f);
		info.rot = 0.0f;
		info.alignment = OBS_ALIGN_TOP | OBS_ALIGN_LEFT;
		info.bounds_type = OBS_BOUNDS_NONE;
		info.bounds_alignment = OBS_ALIGN_CENTER;
		vec2_set(&info.bounds, 0.0f, 0.0f);
		obs_sceneitem_set_info(item, &info);

		obs_sceneitem_crop crop = {};
		obs_sceneitem_set_crop(item, &crop);

		obs_sceneitem_defer_update_end(item);

		UNUSED_PARAMETER(scene);
		UNUSED_PARAMETER(param);
		return true;
	};

	obs_scene_enum_items(scene, func, nullptr);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_stretchSceneItems(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	NODE_OBS_SCENE(args[0], scene);

	obs_bounds_type boundsType = OBS_BOUNDS_STRETCH;
	obs_scene_enum_items(scene, CenterAlignSelectedItems,
			&boundsType);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_fitSceneItems(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	NODE_OBS_SCENE(args[0], scene)

	obs_bounds_type boundsType = OBS_BOUNDS_SCALE_INNER;
	obs_scene_enum_items(scene, CenterAlignSelectedItems,
			&boundsType);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_centerSceneItems(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	NODE_OBS_SCENE(args[0], scene)

	auto func = [] (obs_scene_t *scene, obs_sceneitem_t *item, void *param)
	{
		vec3 tl, br, itemCenter, screenCenter, offset;
		obs_video_info ovi;

		if (!obs_sceneitem_selected(item))
			return true;

		obs_get_video_info(&ovi);

		vec3_set(&screenCenter, float(ovi.base_width),
				float(ovi.base_height), 0.0f);
		vec3_mulf(&screenCenter, &screenCenter, 0.5f);

		GetItemBox(item, tl, br);

		vec3_sub(&itemCenter, &br, &tl);
		vec3_mulf(&itemCenter, &itemCenter, 0.5f);
		vec3_add(&itemCenter, &itemCenter, &tl);

		vec3_sub(&offset, &screenCenter, &itemCenter);
		vec3_add(&tl, &tl, &offset);

		SetItemTL(item, tl);

		UNUSED_PARAMETER(scene);
		UNUSED_PARAMETER(param);
		return true;
	};

	obs_scene_enum_items(scene, func, nullptr);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_getSceneItemCrop(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	NODE_OBS_SCENE(args[0], scene)
	NODE_OBS_SCENEITEM(args[1], scene, item);	

	obs_sceneitem_crop crop;
	obs_sceneitem_get_crop(item, &crop);

	Local<Object> result = Object::New(isolate);
	result->Set(String::NewFromUtf8(isolate, "left"), Uint32::New(isolate, crop.left));
	result->Set(String::NewFromUtf8(isolate, "top"), Uint32::New(isolate, crop.top));
	result->Set(String::NewFromUtf8(isolate, "right"), Uint32::New(isolate, crop.right));
	result->Set(String::NewFromUtf8(isolate, "bottom"), Uint32::New(isolate, crop.bottom));
	args.GetReturnValue().Set(result);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_getSceneItemRot(const FunctionCallbackInfo<Value>& args)
{
	String::Utf8Value itemName(args[1]);

	NODE_OBS_SCENE(args[0], scene)
	NODE_OBS_SCENEITEM(args[1], scene, item)

	double rotation = obs_sceneitem_get_rot(item);
	args.GetReturnValue().Set(rotation);

	NODE_OBS_SCENE_RELEASE(scene)
}

void OBS_content::OBS_content_setSceneItemRot(const FunctionCallbackInfo<Value>& args)
{
	String::Utf8Value itemName(args[1]);
	float rotation = (float)args[2].As<Number>()->Value();

	NODE_OBS_SCENE(args[0], scene);
	NODE_OBS_SCENEITEM(args[1], scene, item);

	obs_sceneitem_set_rot(item, rotation);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_setSceneItemCrop(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	Local<Object> cropObject = args[2].As<Object>();

	NODE_OBS_SCENE(args[0], scene)
	NODE_OBS_SCENEITEM(args[1], scene, item);

	const obs_sceneitem_crop crop = {
		cropObject->Get(String::NewFromUtf8(isolate, "left")).As<Uint32>()->Value(),
		cropObject->Get(String::NewFromUtf8(isolate, "top")).As<Uint32>()->Value(),
		cropObject->Get(String::NewFromUtf8(isolate, "right")).As<Uint32>()->Value(),
		cropObject->Get(String::NewFromUtf8(isolate, "bottom")).As<Uint32>()->Value()
	};

	obs_sceneitem_set_crop(item, &crop);

	NODE_OBS_SCENE_RELEASE(scene);
}

void OBS_content::OBS_content_getListCurrentScenes(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	vector<std::string> listCurrentScenes = getListCurrentScenes();

	Handle<Array> result = Array::New(isolate, (int)listCurrentScenes.size());
	for (size_t i = 0; i < listCurrentScenes.size(); i++) {
		result->Set((uint32_t)i, String::NewFromUtf8(isolate, listCurrentScenes.at(i).c_str()));
	}

	args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getListCurrentSourcesFromScene(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string name = std::string(*param1);

	vector<const char*> listSources = getListCurrentSourcesFromScene(name.c_str());

	Handle<Array> result = Array::New(isolate, (int)listSources.size());
	for (size_t i = 0; i < listSources.size(); i++) {
		result->Set((uint32_t)i, String::NewFromUtf8(isolate, listSources.at(i)));
	}

	args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getListInputSources(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	vector<const char*> testListInputSource = getListInputSources();

	Handle<Array> result = Array::New(isolate, (int)testListInputSource.size());
	for (size_t i = 0; i < testListInputSource.size(); i++) {
		result->Set((uint32_t)i, String::NewFromUtf8(isolate, testListInputSource.at(i)));
	}

	args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getListFilters(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	vector<const char*> listNames = getListFilterNames();
    vector<const char*> listTypes = getListFilterTypes();

	Handle<Array> result = Array::New(isolate, (int)listNames.size());
	for (size_t i = 0; i < listNames.size(); i++) {
		uint32_t caps = obs_get_source_output_flags(listTypes[i]);
        Local<Object> entry = Object::New(isolate);
		entry->Set(v8::String::NewFromUtf8(isolate, "audio"), Nan::New((caps & OBS_SOURCE_AUDIO) != 0));
		entry->Set(v8::String::NewFromUtf8(isolate, "video"), Nan::New((caps & OBS_SOURCE_VIDEO) != 0));
		entry->Set(v8::String::NewFromUtf8(isolate, "async"), Nan::New((caps & OBS_SOURCE_ASYNC) != 0));
        entry->Set(v8::String::NewFromUtf8(isolate, "type"), v8::String::NewFromUtf8(isolate, listTypes.at(i)));
        entry->Set(v8::String::NewFromUtf8(isolate, "description"), v8::String::NewFromUtf8(isolate, listNames.at(i)));
        result->Set((uint32_t)i, entry);
	}

	args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getListSourceFilters(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();

    vector<const char*> source_filters;

	NODE_OBS_SOURCE(args[0], source);

    auto cb =
    [](obs_source_t *parent, obs_source_t *child, void *param) {
        vector<const char*> *source_filters =
            reinterpret_cast<vector<const char*> *>(param);

        source_filters->push_back(obs_source_get_name(child));
    };

    obs_source_enum_filters(source, cb, &source_filters);

    Handle<Array> result = Array::New(isolate, (int)source_filters.size());
    for (int i = 0; i < source_filters.size(); ++i) {
        result->Set((uint32_t)i, String::NewFromUtf8(isolate, source_filters.at(i)));
    }

    NODE_OBS_SOURCE_RELEASE(source);

    args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getCurrentTransition(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();

    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, currentTransition.c_str()));
}

void OBS_content::OBS_content_getListCurrentTransitions(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    Handle<Array> result = Array::New(isolate, (int)transitions.size());
    for (decltype(transitions)::iterator it = transitions.begin(); it != transitions.end(); it++) {
        result->Set(std::distance(transitions.begin(), it), String::NewFromUtf8(isolate, it->first.c_str()));
    }

    args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getListTransitions(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	vector<const char*> listTransitions = getListTransitionNames();
    vector<const char*> listTypeTransitions = getListTransitionTypes();

	Handle<Array> result = Array::New(isolate, (int)listTransitions.size());

	for (size_t i = 0; i < listTransitions.size(); i++) {
        Local<Object> entry = Object::New(isolate);
        entry->Set(v8::String::NewFromUtf8(isolate, "type"), v8::String::NewFromUtf8(isolate, listTypeTransitions.at(i)));
        entry->Set(v8::String::NewFromUtf8(isolate, "description"), v8::String::NewFromUtf8(isolate, listTransitions.at(i)));
		result->Set((uint32_t)i, entry);
	}

	args.GetReturnValue().Set(result);
}


void OBS_content::OBS_content_createScene(const FunctionCallbackInfo<Value>& args)
{
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string name = std::string(*param1);

	//createScene(name.c_str());
	//char* foo = name.c_str();

	char* nameScene = new char[name.size() + 1];
    std::memcpy(nameScene, name.data(), name.size());
	nameScene[name.size()] = '\0';

	createScene(nameScene);
}

void OBS_content::OBS_content_removeScene(const FunctionCallbackInfo<Value>& args)
{
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string sourceName = std::string(*param1);

	removeScene(sourceName.c_str());
}

void OBS_content::OBS_content_addSource(const FunctionCallbackInfo<Value>& args)
{
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string sourceType = std::string(*param1);

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string sourceName = std::string(*param2);

	v8::String::Utf8Value param4(args[4]->ToString());
	std::string sceneName = std::string(*param4);


	addSource(sourceType.c_str(), sourceName.c_str(), nullptr, nullptr, sceneName.c_str());
}

void OBS_content::OBS_content_removeSource(const FunctionCallbackInfo<Value>& args)
{
	v8::String::Utf8Value param1(args[0]->ToString());
	std::string sourceName = std::string(*param1);

	removeSource(sourceName.c_str());
}

void OBS_content::OBS_content_getSourceFrame(const FunctionCallbackInfo<Value>& args)
{
	// Isolate* isolate = args.GetIsolate();
    
	// v8::String::Utf8Value param1(args[0]->ToString());
	// std::string sourceName = std::string(*param1);

 //    Local<Object> frame = getSourceFrame(sourceName.c_str());

 //    args.GetReturnValue().Set(frame);
}

void OBS_content::OBS_content_getSourceProperties(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	NODE_OBS_SOURCE(args[0], source);

	vector<std::string> listSourceProperties = getSourceProperties(source);

	Handle<Array> result = Array::New(isolate, (int)listSourceProperties.size()/10);
	// Handle<Array> result = Array::New(isolate, (int)listSourceProperties.size()/6);
	for (int i = 0; i < listSourceProperties.size(); i++) {
		Local<Object> obj = Object::New(isolate);

  		obj->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, listSourceProperties.at(i).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, listSourceProperties.at(i+1).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "long_description"), String::NewFromUtf8(isolate, listSourceProperties.at(i+2).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, listSourceProperties.at(i+3).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "visible"), String::NewFromUtf8(isolate, listSourceProperties.at(i+4).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "enabled"), String::NewFromUtf8(isolate, listSourceProperties.at(i+5).c_str()));

  		obj->Set(String::NewFromUtf8(isolate, "subType"), String::NewFromUtf8(isolate, listSourceProperties.at(i+6).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "minVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+7).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "maxVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+8).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "stepVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+9).c_str()));
		
		result->Set(i/10, obj);
		i+=9;
	}

    NODE_OBS_SOURCE_RELEASE(source);

	args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getSourcePropertiesSubParameters(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	NODE_OBS_SOURCE(args[0], source);

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string propertyNameSelected = std::string(*param2);

	vector<string> listSourcePropertiesSubParameters = getSourcePropertiesSubParameters(source,
																				propertyNameSelected);
	
	Handle<Array> result = Array::New(isolate, (int)listSourcePropertiesSubParameters.size()/2);
	for (int i = 0; i < listSourcePropertiesSubParameters.size(); i++) {
		Local<Object> obj = Object::New(isolate);
  		obj->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, listSourcePropertiesSubParameters.at(i).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, listSourcePropertiesSubParameters.at(i+1).c_str()));

		result->Set(i/2, obj);
		i++;
	}
	
    NODE_OBS_SOURCE_RELEASE(source);

	args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getSourcePropertyCurrentValue(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	NODE_OBS_SOURCE(args[0], source)

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string propertyNameSelected = std::string(*param2);

	Local<Object> object = getSourcePropertyCurrentValue(source, propertyNameSelected.c_str());
	
	/*v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
	std::string valueString = std::string(*value);
	const char* result = valueString.c_str();*/

    NODE_OBS_SOURCE_RELEASE(source);

	args.GetReturnValue().Set(object);
}

void OBS_content::OBS_content_setProperty(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	NODE_OBS_SOURCE(args[0], source);

	v8::String::Utf8Value param2(args[1]->ToString());
	std::string propertyNameSelected = std::string(*param2);

	bool needsToBeRefresh = setProperty(source, propertyNameSelected.c_str(), Local<Object>::Cast(args[2]));

	const char* result = needsToBeRefresh ? "true" : "false";
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result));

	NODE_OBS_SOURCE_RELEASE(source);
}

void OBS_content::OBS_content_setCurrentScene(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string sceneName = std::string(*param1);

	setCurrentScene(sceneName.c_str());
}

void OBS_content::OBS_content_setSourcePosition(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sceneName = std::string(*param0);

	v8::String::Utf8Value param1(args[1]->ToString());
	std::string sourceName = std::string(*param1);

	v8::String::Utf8Value param2(args[2]->ToString());
	std::string x = std::string(*param2);

	v8::String::Utf8Value param3(args[3]->ToString());
	std::string y = std::string(*param3);

	setSourcePosition(sceneName.c_str(), sourceName.c_str(), (float)atof(x.c_str()), (float)atof(y.c_str()));
}
	
void OBS_content::OBS_content_setSourceScaling(const FunctionCallbackInfo<Value>& args)
{	
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sceneName = std::string(*param0);

	v8::String::Utf8Value param1(args[1]->ToString());
	std::string sourceName = std::string(*param1);

	v8::String::Utf8Value param2(args[2]->ToString());
	std::string x = std::string(*param2);

	v8::String::Utf8Value param3(args[3]->ToString());
	std::string y = std::string(*param3);

	setSourceScaling(sceneName.c_str(), sourceName.c_str(), (float)atof(x.c_str()), (float)atof(y.c_str()));
}
	
void OBS_content::OBS_content_getSourcePosition(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sceneName = std::string(*param0);

	v8::String::Utf8Value param1(args[1]->ToString());
	std::string sourceName = std::string(*param1);

	Local<Object> position = getSourcePosition(sceneName, sourceName);
	args.GetReturnValue().Set(position);
}

void OBS_content::OBS_content_getSourceScaling(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sceneName = std::string(*param0);

	v8::String::Utf8Value param1(args[1]->ToString());
	std::string sourceName = std::string(*param1);

	Local<Object> scaling = getSourceScaling(sceneName, sourceName);
	args.GetReturnValue().Set(scaling);
}

void OBS_content::OBS_content_getSourceSize(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string sourceName = std::string(*param1);

	Local<Object> size = getSourceSize(sourceName);
	args.GetReturnValue().Set(size);
}

void OBS_content::OBS_content_setSourceOrder(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param1(args[0]->ToString());
	std::string sourceName = std::string(*param1);
	v8::String::Utf8Value param2(args[1]->ToString());
	std::string order = std::string(*param2);

	setSourceOrder(sourceName, order);
}

void OBS_content::OBS_content_updateSourceProperties(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	NODE_OBS_SOURCE(args[0], source);

    obs_data_t* settings = obs_source_get_settings(source);

    updateSourceProperties(source, settings);

    NODE_OBS_SOURCE_RELEASE(source);
}

void OBS_content::OBS_content_createDisplay(const FunctionCallbackInfo<Value>& args) 
{
	Isolate* isolate = args.GetIsolate();
    v8::EscapableHandleScope scope(isolate);

	v8::Local<v8::Object> bufferObj = args[0].As<v8::Object>(); 
	unsigned char* bufferData = (unsigned char*)node::Buffer::Data(bufferObj);
	uint64_t windowHandle = *reinterpret_cast<uint64_t*>(bufferData);

    v8::String::Utf8Value key(args[1]);

	createDisplay(std::string(*key), windowHandle);
}

void OBS_content::OBS_content_destroyDisplay(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    
    v8::String::Utf8Value key(args[0]);

    destroyDisplay(std::string(*key));
}

void OBS_content::OBS_content_createSourcePreviewDisplay(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	v8::EscapableHandleScope scope(isolate);

	v8::Local<v8::Object> bufferObj = args[0].As<v8::Object>(); 
	unsigned char* bufferData = (unsigned char*)node::Buffer::Data(bufferObj);
	uint64_t windowHandle = *reinterpret_cast<uint64_t*>(bufferData);

	v8::String::Utf8Value param1(args[1]->ToString());
	std::string sourceName = std::string(*param1);

 	v8::String::Utf8Value key(args[2]);

	createSourcePreviewDisplay(std::string(*key), windowHandle, sourceName);
}

void OBS_content::OBS_content_resizeDisplay(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();

    v8::String::Utf8Value key(args[0]);

    auto value = displays.find(*key);
    if (value == displays.end()) {
        std::cout << "Invalid key provided to resizeDisplay: " << *key << std::endl;
        return;
    }

    OBS::Display *display = value->second;

    int width = args[1]->ToUint32()->Value();
    int height = args[2]->ToUint32()->Value();

    display->SetSize(width, height);
}

void OBS_content::OBS_content_moveDisplay(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();

    v8::String::Utf8Value key(args[0]);
    
    auto value = displays.find(*key);
    if (value == displays.end()) {
        std::cout << "Invalid key provided to moveDisplay: " << *key << std::endl;
        return;
    }

    OBS::Display *display = value->second;

    int x = args[1]->ToUint32()->Value();
    int y = args[2]->ToUint32()->Value();
    
    display->SetPosition(x, y);
}

void OBS_content::OBS_content_setPaddingSize(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setPaddingSize(displayKey<string>, size<number>)")
				)
			);
			return;
		case 1:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}
	
	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{size} is not a <number>!")
			)
		);
		return;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetPaddingSize(args[1]->Int32Value());
	return;
}

void OBS_content::OBS_content_setPaddingColor(const FunctionCallbackInfo<Value>& args) {
	union {
		uint32_t rgba;
		uint8_t c[4];
	} color;
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setPaddingColor(displayKey<string>, red<number>{0.0, 255.0}, green<number>{0.0, 255.0}, blue<number>{0.0, 255.0}[, alpha<number>{0.0, 1.0}])")
				)
			);
			return;
		case 1:
		case 2:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{red} is not a <number>!")
			)
		);
		return;
	}
	if (args[2]->IsUndefined() && !args[2]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{green} is not a <number>!")
			)
		);
		return;
	}
	if (args[3]->IsUndefined() && !args[3]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{blue} is not a <number>!")
			)
		);
		return;
	}

	// Assign Color
	color.c[0] = (uint8_t)(args[1]->NumberValue());
	color.c[1] = (uint8_t)(args[2]->NumberValue());
	color.c[2] = (uint8_t)(args[3]->NumberValue());
	if (!args[4]->IsUndefined() && args[4]->IsNumber()) {
		color.c[3] = (uint8_t)(args[4]->NumberValue() * 255.0);
	} else {
		color.c[3] = 255;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetPaddingColor(color.c[0], color.c[1], color.c[2], color.c[3]);
	return;
}

void OBS_content::OBS_content_setBackgroundColor(const FunctionCallbackInfo<Value>& args) {
	union {
		uint32_t rgba;
		uint8_t c[4];
	} color;
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setBackgroundColor(displayKey<string>, red<number>{0.0, 255.0}, green<number>{0.0, 255.0}, blue<number>{0.0, 255.0}[, alpha<number>{0.0, 1.0}])")
				)
			);
			return;
		case 1:
		case 2:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{red} is not a <number>!")
			)
		);
		return;
	}
	if (args[2]->IsUndefined() && !args[2]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{green} is not a <number>!")
			)
		);
		return;
	}
	if (args[3]->IsUndefined() && !args[3]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{blue} is not a <number>!")
			)
		);
		return;
	}

	// Assign Color
	color.c[0] = (uint8_t)(args[1]->NumberValue());
	color.c[1] = (uint8_t)(args[2]->NumberValue());
	color.c[2] = (uint8_t)(args[3]->NumberValue());
	if (!args[4]->IsUndefined() && args[4]->IsNumber()) {
		color.c[3] = (uint8_t)(args[4]->NumberValue() * 255.0);
	} else {
		color.c[3] = 255;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetBackgroundColor(color.c[0], color.c[1], color.c[2], color.c[3]);
	return;
}

void OBS_content::OBS_content_setOutlineColor(const FunctionCallbackInfo<Value>& args) {
	union {
		uint32_t rgba;
		uint8_t c[4];
	} color;
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setOutlineColor(displayKey<string>, red<number>{0.0, 255.0}, green<number>{0.0, 255.0}, blue<number>{0.0, 255.0}[, alpha<number>{0.0, 1.0}])")
				)
			);
			return;
		case 1:
		case 2:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{red} is not a <number>!")
			)
		);
		return;
	}
	if (args[2]->IsUndefined() && !args[2]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{green} is not a <number>!")
			)
		);
		return;
	}
	if (args[3]->IsUndefined() && !args[3]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{blue} is not a <number>!")
			)
		);
		return;
	}

	// Assign Color
	color.c[0] = (uint8_t)(args[1]->NumberValue());
	color.c[1] = (uint8_t)(args[2]->NumberValue());
	color.c[2] = (uint8_t)(args[3]->NumberValue());
	if (!args[4]->IsUndefined() && args[4]->IsNumber()) {
		color.c[3] = (uint8_t)(args[4]->NumberValue() * 255.0);
	} else {
		color.c[3] = 255;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetOutlineColor(color.c[0], color.c[1], color.c[2], color.c[3]);
	return;
}

void OBS_content::OBS_content_setGuidelineColor(const FunctionCallbackInfo<Value>& args) {
	union {
		uint32_t rgba;
		uint8_t c[4];
	} color;
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setGuidelineColor(displayKey<string>, red<number>{0.0, 255.0}, green<number>{0.0, 255.0}, blue<number>{0.0, 255.0}[, alpha<number>{0.0, 1.0}])")
				)
			);
			return;
		case 1:
		case 2:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{red} is not a <number>!")
			)
		);
		return;
	}
	if (args[2]->IsUndefined() && !args[2]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{green} is not a <number>!")
			)
		);
		return;
	}
	if (args[3]->IsUndefined() && !args[3]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{blue} is not a <number>!")
			)
		);
		return;
	}

	// Assign Color
	color.c[0] = (uint8_t)(args[1]->NumberValue());
	color.c[1] = (uint8_t)(args[2]->NumberValue());
	color.c[2] = (uint8_t)(args[3]->NumberValue());
	if (!args[4]->IsUndefined() && args[4]->IsNumber()) {
		color.c[3] = (uint8_t)(args[4]->NumberValue() * 255.0);
	} else {
		color.c[3] = 255;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetGuidelineColor(color.c[0], color.c[1], color.c[2], color.c[3]);
	return;
}

void OBS_content::OBS_content_setResizeBoxOuterColor(const FunctionCallbackInfo<Value>& args) {
	union {
		uint32_t rgba;
		uint8_t c[4];
	} color;
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setResizeBoxOuterColor(displayKey<string>, red<number>{0.0, 255.0}, green<number>{0.0, 255.0}, blue<number>{0.0, 255.0}[, alpha<number>{0.0, 1.0}])")
				)
			);
			return;
		case 1:
		case 2:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{red} is not a <number>!")
			)
		);
		return;
	}
	if (args[2]->IsUndefined() && !args[2]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{green} is not a <number>!")
			)
		);
		return;
	}
	if (args[3]->IsUndefined() && !args[3]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{blue} is not a <number>!")
			)
		);
		return;
	}

	// Assign Color
	color.c[0] = (uint8_t)(args[1]->NumberValue());
	color.c[1] = (uint8_t)(args[2]->NumberValue());
	color.c[2] = (uint8_t)(args[3]->NumberValue());
	if (!args[4]->IsUndefined() && args[4]->IsNumber()) {
		color.c[3] = (uint8_t)(args[4]->NumberValue() * 255.0);
	} else {
		color.c[3] = 255;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetResizeBoxOuterColor(color.c[0], color.c[1], color.c[2], color.c[3]);
	return;
}

void OBS_content::OBS_content_setResizeBoxInnerColor(const FunctionCallbackInfo<Value>& args) {
	union {
		uint32_t rgba;
		uint8_t c[4];
	} color;
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setResizeBoxInnerColor(displayKey<string>, red<number>{0.0, 255.0}, green<number>{0.0, 255.0}, blue<number>{0.0, 255.0}[, alpha<number>{0.0, 1.0}])")
				)
			);
			return;
		case 1:
		case 2:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{red} is not a <number>!")
			)
		);
		return;
	}
	if (args[2]->IsUndefined() && !args[2]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{green} is not a <number>!")
			)
		);
		return;
	}
	if (args[3]->IsUndefined() && !args[3]->IsNumber()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{blue} is not a <number>!")
			)
		);
		return;
	}

	// Assign Color
	color.c[0] = (uint8_t)(args[1]->NumberValue());
	color.c[1] = (uint8_t)(args[2]->NumberValue());
	color.c[2] = (uint8_t)(args[3]->NumberValue());
	if (!args[4]->IsUndefined() && args[4]->IsNumber()) {
		color.c[3] = (uint8_t)(args[4]->NumberValue() * 255.0);
	} else {
		color.c[3] = 255;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetResizeBoxInnerColor(color.c[0], color.c[1], color.c[2], color.c[3]);
	return;
}

void OBS_content::OBS_content_setShouldDrawUI(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
		case 0:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Usage: OBS_content_setShouldDrawUI(displayKey<string>, value<boolean>)")
				)
			);
			return;
		case 1:
			isolate->ThrowException(
				v8::Exception::SyntaxError(
					v8::String::NewFromUtf8(isolate, "Not enough Parameters")
				)
			);
			return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}
	if (args[1]->IsUndefined() && !args[1]->IsBoolean()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{value} is not a <boolean>!")
			)
		);
		return;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetDrawUI(args[1]->BooleanValue());
}

void OBS_content::OBS_content_getDisplayPreviewOffset(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    v8::EscapableHandleScope handle(isolate);

    v8::String::Utf8Value key(args[0]);

    auto value = displays.find(*key);
    if (value == displays.end()) {
        std::cout << "Invalid key provided to moveDisplay: " << *key << std::endl;
        return;
    }

    OBS::Display *display = value->second;

    auto offset = display->GetPreviewOffset();

    v8::Local<v8::Object> object = v8::Object::New(isolate);
    object->Set(v8::String::NewFromUtf8(isolate, "x"), v8::Int32::New(isolate, offset.first));
    object->Set(v8::String::NewFromUtf8(isolate, "y"), v8::Int32::New(isolate, offset.second));

    args.GetReturnValue().Set(handle.Escape(object));
}

void OBS_content::OBS_content_getDisplayPreviewSize(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    v8::EscapableHandleScope handle(isolate);

    v8::String::Utf8Value key(args[0]);

    auto value = displays.find(*key);
    if (value == displays.end()) {
        std::cout << "Invalid key provided to moveDisplay: " << *key << std::endl;
        return;
    }

    OBS::Display *display = value->second;

    auto size = display->GetPreviewSize();

    v8::Local<v8::Object> object = v8::Object::New(isolate);
    object->Set(v8::String::NewFromUtf8(isolate, "width"), v8::Int32::New(isolate, size.first));
    object->Set(v8::String::NewFromUtf8(isolate, "height"), v8::Int32::New(isolate, size.second));

    args.GetReturnValue().Set(handle.Escape(object));
}

void OBS_content::OBS_content_selectSource(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	selectSource(args[0]->NumberValue(), args[1]->NumberValue());
}

void OBS_content::OBS_content_selectSources(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(args[0]);
	selectSources(array);
}

void OBS_content::OBS_content_dragSelectedSource(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	dragSelectedSource(args[0]->NumberValue(), args[1]->NumberValue());
}

void OBS_content::OBS_content_loadConfigFile(const FunctionCallbackInfo<Value>& args)
{
	loadConfigFile();
}

void OBS_content::OBS_content_saveIntoConfigFile(const FunctionCallbackInfo<Value>& args)
{
	saveIntoConfigFile();
}
	
void OBS_content::OBS_content_getSourceFlags(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sourceName = std::string(*param0);

	args.GetReturnValue().Set(getSourceFlags(sourceName));
}

void OBS_content::OBS_content_getSourceFilterVisibility(const FunctionCallbackInfo<Value> &args)
{
	NODE_OBS_SOURCE(args[0], source);
	NODE_OBS_FILTER(args[1], source, filter);

	args.GetReturnValue().Set(obs_source_enabled(filter));	

	NODE_OBS_SOURCE_RELEASE(source);
	NODE_OBS_FILTER_RELEASE(filter);
}

void OBS_content::OBS_content_setSourceFilterVisibility(const FunctionCallbackInfo<Value> &args)
{
	NODE_OBS_SOURCE(args[0], source);
	NODE_OBS_FILTER(args[1], source, filter);

	bool enabled = args[2]->ToBoolean()->Value();
	obs_source_set_enabled(filter, enabled);

	NODE_OBS_SOURCE_RELEASE(source);
	NODE_OBS_FILTER_RELEASE(filter);
}

void OBS_content::OBS_content_sourceSetMuted(const FunctionCallbackInfo<Value>& args) 
{
	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sourceName = std::string(*param0);	

	bool muted = args[1]->IntegerValue();

	sourceSetMuted(sourceName, muted);
}

void OBS_content::OBS_content_isSourceMuted(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sourceName = std::string(*param0);	

	args.GetReturnValue().Set(v8::Boolean::New(isolate, isSourceMuted(sourceName)));
}

void OBS_content::OBS_content_getSourceVisibility(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sceneName = std::string(*param0);

	v8::String::Utf8Value param1(args[1]->ToString());
	std::string sourceName = std::string(*param1);

	args.GetReturnValue().Set(v8::Boolean::New(isolate, getSourceVisibility(sceneName, sourceName)));
}

void OBS_content::OBS_content_setSourceVisibility(const FunctionCallbackInfo<Value>& args)
{
	v8::String::Utf8Value param0(args[0]->ToString());
	std::string sceneName = std::string(*param0);

	v8::String::Utf8Value param1(args[1]->ToString());
	std::string sourceName = std::string(*param1);

	setSourceVisibility(sceneName, sourceName, args[2]->IntegerValue());
}

void OBS_content::OBS_content_fillTabScenes(const FunctionCallbackInfo<Value>& args)
{
    v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(args[0]);
    fillTabScenes(array);
}

void OBS_content::OBS_content_test_getListCurrentScenes(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("MyScene for test");

	vector<std::string> listCurrentScenes = getListCurrentScenes();

	if(listCurrentScenes.size() == 2 &&
		listCurrentScenes.at(0).compare("MyScene") == 0 &&
		listCurrentScenes.at(1).compare("MyScene for test") == 0)
	{
    	result = "SUCCESS";
    	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
    	return;
	}

	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getListCurrentSourcesFromScene(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
    string result;

    createScene("New Scene");

    bool resultAddingFirstSource = addSource("Video Capture Device", "First Source", nullptr, nullptr, "New Scene");
    bool resultAddingSecondSource = addSource("Audio Input Capture", "Second Source", nullptr, nullptr, "New Scene");

    if(!resultAddingFirstSource && !resultAddingSecondSource) {
		result = "FAILURE";
    	return;
    }

    vector<const char*> listSources = getListCurrentSourcesFromScene("New Scene");

    if(listSources.size() == 2) {
    	result = "SUCCESS";
    }
    else {
		result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getListInputSources(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "SUCCESS";

	std::vector<std::pair<std::string, bool>> requiredList,
		optionalList,
		additionalList;
	vector<const char*> testList = getListInputSources();

	// Define Sources
	requiredList.push_back(std::make_pair("Audio Input Capture", false));
	requiredList.push_back(std::make_pair("Audio Output Capture", false));
	requiredList.push_back(std::make_pair("Display Capture", false));
	requiredList.push_back(std::make_pair("Window Capture", false));
	requiredList.push_back(std::make_pair("Image", false));
	requiredList.push_back(std::make_pair("Image Slide Show", false));
	requiredList.push_back(std::make_pair("Media Source", false));
	requiredList.push_back(std::make_pair("Text (FreeType 2)", false));
	optionalList.push_back(std::make_pair("Video Capture Device", false));
	optionalList.push_back(std::make_pair("Browser Source", false));

	#ifdef _WIN32
	{
		requiredList.push_back(std::make_pair("Text (GDI+)", false));
		requiredList.push_back(std::make_pair("Game Capture", false));
		optionalList.push_back(std::make_pair("Color Source", false)); // Non-standard source.
	} 
	#else
	{
		requiredList.push_back(std::make_pair("Game Capture (Syphon)", false));
	}
	#endif

	size_t requiredCount = 0,
		optionalCount = 0;
	for (const char* centry : testList) {
		// ^ C++11 iterator, very useful :D
		bool matched = false;

		for (std::pair<std::string, bool>& entry : requiredList) {
			if (entry.first == centry) {
				entry.second = true;
				requiredCount++;
				matched = true;
				break;
			}
		}
		for (std::pair<std::string, bool>& entry : optionalList) {
			if (entry.first == centry) {
				entry.second = true;
				optionalCount++;
				matched = true;
				break;
			}
		}
		if (!matched)
			additionalList.push_back(std::make_pair(centry, true));
	}

	blog(LOG_INFO, "Required: %lld/%lld",
		(unsigned long long)requiredCount,
		(unsigned long long)requiredList.size());
	for (std::pair<std::string, bool>& vplugin : requiredList) {
		blog(LOG_INFO, "  %s: %s",
			vplugin.first.data(),
			vplugin.second ? "Found" : "Not Found");
	}
	blog(LOG_INFO, "Optional: %lld/%lld",
		(unsigned long long)optionalCount,
		(unsigned long long)optionalList.size());
	for (std::pair<std::string, bool>& vplugin : optionalList) {
		blog(LOG_INFO, "  %s: %s",
			vplugin.first.data(),
			vplugin.second ? "Found" : "Not Found");
	}
	blog(LOG_INFO, "Additional: %lld",
		(unsigned long long)additionalList.size());
	for (std::pair<std::string, bool>& vplugin : additionalList) {
		blog(LOG_INFO, "  %s: Found",
			vplugin.first.data());
	}

	if (requiredCount < requiredList.size())
		result = "FAILURE";
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getListFilters(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "SUCCESS";

	std::vector<std::pair<std::string, bool>> requiredList,
		optionalList,
		additionalList;
	vector<const char*> testList = getListFilterNames();
	
	// Video
	requiredList.push_back(std::make_pair("Image Mask/Blend", false));
	requiredList.push_back(std::make_pair("Crop/Pad", false));
	requiredList.push_back(std::make_pair("Color Correction", false));
	requiredList.push_back(std::make_pair("Scaling/Aspect Ratio", false));
	requiredList.push_back(std::make_pair("Scroll", false));
	requiredList.push_back(std::make_pair("Color Key", false));
	requiredList.push_back(std::make_pair("Apply LUT", false));
	requiredList.push_back(std::make_pair("Sharpen", false));
	requiredList.push_back(std::make_pair("Chroma Key", false));
	requiredList.push_back(std::make_pair("Video Delay (Async)", false));

	// Audio
	requiredList.push_back(std::make_pair("Gain", false));
	requiredList.push_back(std::make_pair("Noise Gate", false));

	#ifdef _WIN32 
	{
		optionalList.push_back(std::make_pair("Noise Suppression", false));
		optionalList.push_back(std::make_pair("Compressor", false)); // Non-standard
		optionalList.push_back(std::make_pair("VST 2.x Plug-in", false));
	} 
	#else
	{
		optionalList.push_back(std::make_pair("VST 2.x Plug-in", false));
	} 
	#endif


	size_t requiredCount = 0,
		optionalCount = 0;
	for (const char* centry : testList) {
		// ^ C++11 iterator, very useful :D
		bool matched = false;

		for (std::pair<std::string, bool>& entry : requiredList) {
			if (entry.first == centry) {
				entry.second = true;
				requiredCount++;
				matched = true;
				break;
			}
		}
		for (std::pair<std::string, bool>& entry : optionalList) {
			if (entry.first == centry) {
				entry.second = true;
				optionalCount++;
				matched = true;
				break;
			}
		}
		if (!matched)
			additionalList.push_back(std::make_pair(centry, true));
	}

	blog(LOG_INFO, "Required: %lld/%lld",
		(unsigned long long)requiredCount,
		(unsigned long long)requiredList.size());
	for (std::pair<std::string, bool>& vplugin : requiredList) {
		blog(LOG_INFO, "  %s: %s",
			vplugin.first.data(),
			vplugin.second ? "Found" : "Not Found");
	}
	blog(LOG_INFO, "Optional: %lld/%lld",
		(unsigned long long)optionalCount,
		(unsigned long long)optionalList.size());
	for (std::pair<std::string, bool>& vplugin : optionalList) {
		blog(LOG_INFO, "  %s: %s",
			vplugin.first.data(),
			vplugin.second ? "Found" : "Not Found");
	}
	blog(LOG_INFO, "Additional: %lld",
		(unsigned long long)additionalList.size());
	for (std::pair<std::string, bool>& vplugin : additionalList) {
		blog(LOG_INFO, "  %s: Found",
			vplugin.first.data());
	}

	if (requiredCount < requiredList.size())
		result = "FAILURE";
	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getListTransitions(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	vector<const char*> mokeListTransitions;
	vector<const char*> testListTransitions = getListTransitionNames();

	mokeListTransitions.push_back("Cut");
	mokeListTransitions.push_back("Fade");
	mokeListTransitions.push_back("Swipe");
	mokeListTransitions.push_back("Slide");
	mokeListTransitions.push_back("Fade to Color");
	mokeListTransitions.push_back("Luma Wipe");

	string result = "SUCCESS";

	if(testListTransitions.size() != mokeListTransitions.size()) {
		result = "FAILURE";
		blog(LOG_INFO, "Failure because size is different");
    	args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
    	return;
	}

	for(int i=0;i<testListTransitions.size();i++) {
		if(strcmp(testListTransitions.at(i), mokeListTransitions.at(i)) != 0) {
			result = "FAILURE";

			blog(LOG_INFO, "Failure because content is different %d", i);
			blog(LOG_INFO, "testListTransitions : %s", testListTransitions.at(i));
			blog(LOG_INFO, "mokeListTransitions : %s", mokeListTransitions.at(i));
    		
    		args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
			return;
		} 
	}
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}


void OBS_content::OBS_content_test_createScene(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();
    string result;
    
    bool resultCreatingSuccess = createScene("MyScene");

    if(resultCreatingSuccess && tabScenes.at(0).compare("MyScene") == 0 ) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_removeScene(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result;
	
	createScene("MyScene");

	int sizeTabScenesBeforeRemovingScene = (int)tabScenes.size();
	bool resultRemove = removeScene("MyScene");
	int sizeTabScenesAfterRemovingScene = (int)tabScenes.size();

    if(resultRemove && 
    	sizeTabScenesBeforeRemovingScene == sizeTabScenesAfterRemovingScene +1) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_addSource(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
    string result;

    if(tabScenes.size() == 0) {
    	createScene("Scene");
    }
    
    bool resultAddingSource = addSource("Video Capture Device", "Mysource", nullptr, nullptr, tabScenes.at(0).c_str());

    if(resultAddingSource) {
        result = "SUCCESS"; 
    } else {
    	result = "FAILURE"; 
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_removeSource(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result;

	addSource("Image", "Mysource", nullptr, nullptr, "MyScene");
    
    if(removeSource("Mysource")) {
        result = "SUCCESS";
    } else {
        result = "FAILURE";
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getSourceProperties(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	addSource("Video Capture Device", "VideoCaptureDeviceSource", nullptr, nullptr, "MyScene");

    obs_source_t *source = obs_get_source_by_name("VideoCaptureDeviceSource");

	vector<std::string> listProperties = getSourceProperties(source);

	#ifdef _WIN32
	{
		if(listProperties.at(0).compare("video_device_id") == 0 &&
			listProperties.at(1).compare("Device") == 0 &&
			listProperties.at(2).compare("") == 0 &&
			listProperties.at(3).compare("OBS_PROPERTY_LIST") == 0 &&
			listProperties.at(4).compare("true") == 0 &&
			listProperties.at(5).compare("true") == 0 &&
			listProperties.at(6).compare("activate") == 0 &&
			listProperties.at(7).compare("Deactivate") == 0 &&
			listProperties.at(8).compare("") == 0 &&
			listProperties.at(9).compare("OBS_PROPERTY_BUTTON") == 0 &&
			listProperties.at(10).compare("true") == 0 &&
			listProperties.at(11).compare("true") == 0 &&
			listProperties.at(12).compare("video_config") == 0 &&
			listProperties.at(13).compare("Configure Video") == 0 &&
			listProperties.at(14).compare("") == 0 &&
			listProperties.at(15).compare("OBS_PROPERTY_BUTTON") == 0 &&
			listProperties.at(16).compare("true") == 0 &&
			listProperties.at(17).compare("true") == 0 &&
			listProperties.at(18).compare("xbar_config") == 0 &&
			listProperties.at(19).compare("Configure Crossbar") == 0 &&
			listProperties.at(20).compare("") == 0 &&
			listProperties.at(21).compare("OBS_PROPERTY_BUTTON") == 0 &&
			listProperties.at(22).compare("true") == 0 &&
			listProperties.at(23).compare("true") == 0 &&
			listProperties.at(24).compare("deactivate_when_not_showing") == 0 &&
			listProperties.at(25).compare("Deactivate when not showing") == 0 &&
			listProperties.at(26).compare("") == 0 &&
			listProperties.at(27).compare("OBS_PROPERTY_BOOL") == 0 &&
			listProperties.at(28).compare("true") == 0 &&
			listProperties.at(29).compare("true") == 0 &&
			listProperties.at(30).compare("res_type") == 0 &&
			listProperties.at(31).compare("Resolution/FPS Type") == 0 &&
			listProperties.at(32).compare("") == 0 && 
			listProperties.at(33).compare("OBS_PROPERTY_LIST") == 0 &&
			listProperties.at(34).compare("true") == 0 &&
			listProperties.at(35).compare("true") == 0 &&
			listProperties.at(36).compare("resolution") == 0 &&
			listProperties.at(37).compare("Resolution") == 0 &&
			listProperties.at(38).compare("") == 0 &&
			listProperties.at(39).compare("OBS_PROPERTY_LIST") == 0 &&
			listProperties.at(40).compare("true") == 0 &&
			listProperties.at(41).compare("false") == 0 &&
			listProperties.at(42).compare("frame_interval") == 0 &&
			listProperties.at(43).compare("FPS") == 0 &&
			listProperties.at(44).compare("") == 0 &&
			listProperties.at(45).compare("OBS_PROPERTY_LIST") == 0 &&
			listProperties.at(46).compare("true") == 0 &&
			listProperties.at(47).compare("false") == 0 &&
			listProperties.at(48).compare("video_format") == 0 &&
			listProperties.at(49).compare("Video Format") == 0 &&
			listProperties.at(50).compare("") == 0 &&
			listProperties.at(51).compare("OBS_PROPERTY_LIST") == 0 &&
			listProperties.at(52).compare("true") == 0 &&
			listProperties.at(53).compare("false") == 0)
			{ 
			result = "SUCCESS";
		} else {
			result = "FAILURE";
		}
	}
	// #else
	{
	if(listProperties.at(0).compare("device") == 0 &&
		listProperties.at(1).compare("Device") == 0 &&
		listProperties.at(2).compare("") == 0 &&
		listProperties.at(3).compare("OBS_PROPERTY_LIST") == 0 &&
		listProperties.at(4).compare("true") == 0 &&
		listProperties.at(5).compare("true") == 0 &&
		listProperties.at(6).compare("use_preset") == 0 &&
		listProperties.at(7).compare("Use Preset") == 0 &&
		listProperties.at(8).compare("") == 0 &&
		listProperties.at(9).compare("OBS_PROPERTY_BOOL") == 0 &&
		listProperties.at(10).compare("true") == 0 &&
		listProperties.at(11).compare("true") == 0 &&
		listProperties.at(12).compare("preset") == 0 &&
		listProperties.at(13).compare("Preset") == 0 &&
		listProperties.at(14).compare("") == 0 &&
		listProperties.at(15).compare("OBS_PROPERTY_LIST") == 0 &&
		listProperties.at(16).compare("true") == 0 &&
		listProperties.at(17).compare("true") == 0 &&
		listProperties.at(18).compare("resolution") == 0 &&
		listProperties.at(19).compare("Resolution") == 0 &&
		listProperties.at(20).compare("") == 0 &&
		listProperties.at(21).compare("OBS_PROPERTY_LIST") == 0 &&
		listProperties.at(22).compare("false") == 0 &&
		listProperties.at(23).compare("false") == 0 &&
		listProperties.at(24).compare("frame_rate") == 0 &&
		listProperties.at(25).compare("Frame rate") == 0 &&
		listProperties.at(26).compare("") == 0 &&
		listProperties.at(27).compare("OBS_PROPERTY_FRAME_RATE") == 0 &&
		listProperties.at(28).compare("false") == 0 &&
		listProperties.at(29).compare("false") == 0 &&
		listProperties.at(30).compare("input_format") == 0 &&
		listProperties.at(31).compare("Input format") == 0 &&
		listProperties.at(32).compare("") == 0 && 
		listProperties.at(33).compare("OBS_PROPERTY_LIST") == 0 &&
		listProperties.at(34).compare("false") == 0 &&
		listProperties.at(35).compare("false") == 0 &&
		listProperties.at(36).compare("color_space") == 0 &&
		listProperties.at(37).compare("Color space") == 0 &&
		listProperties.at(38).compare("") == 0 &&
		listProperties.at(39).compare("OBS_PROPERTY_LIST") == 0 &&
		listProperties.at(40).compare("false") == 0 &&
		listProperties.at(41).compare("false") == 0 &&
		listProperties.at(42).compare("video_range") == 0 &&
		listProperties.at(43).compare("Video range") == 0 &&
		listProperties.at(44).compare("") == 0 &&
		listProperties.at(45).compare("OBS_PROPERTY_LIST") == 0 &&
		listProperties.at(46).compare("false") == 0 &&
		listProperties.at(47).compare("false") == 0 &&
		listProperties.at(48).compare("buffering") == 0 &&
		listProperties.at(49).compare("Use Buffering") == 0 &&
		listProperties.at(50).compare("") == 0 &&
		listProperties.at(51).compare("OBS_PROPERTY_BOOL") == 0 &&
		listProperties.at(52).compare("true") == 0 &&
		listProperties.at(53).compare("true") == 0)
		{ 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}
	}
	#endif

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getSourcePropertiesSubParameters(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	addSource("Video Capture Device", "Video", nullptr, nullptr, "MyScene");

    obs_source_t *source = obs_get_source_by_name("Video");

	vector<string> listSourcePropertiesSubParameters = getSourcePropertiesSubParameters(source, "device");

	if(listSourcePropertiesSubParameters.at(0).compare("") == 0 &&
		listSourcePropertiesSubParameters.at(1).compare("") == 0 &&
		listSourcePropertiesSubParameters.at(2).compare("FaceTime HD Camera") == 0 &&
		listSourcePropertiesSubParameters.at(3).compare("CC25243BJN1G1HNAN") == 0) { 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result;

	addSource("Media Source", "Source", nullptr, nullptr, "MyScene");
    obs_source_t *source = obs_get_source_by_name("Source");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;
	//v8::String::Utf8Value value;
	std::string valueString;

	tabProperties.push_back("input_format");
	tabProperties.push_back("looping");
	tabProperties.push_back("force_scale");
	tabProperties.push_back("clear_on_media_end");
	tabProperties.push_back("restart_on_activate");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));
		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		valueString = std::string(*value);
		testValues.push_back(valueString.c_str());
	}
	
	if(testValues.at(0).compare("") == 0 &&
		testValues.at(1).compare("false") == 0 &&
		testValues.at(2).compare("true") == 0 &&
		testValues.at(3).compare("true") == 0 &&
		testValues.at(4).compare("true") == 0) { 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_boolType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result;

	createScene("My scene for testing");
	addSource("Media Source", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	tabProperties.push_back("looping");
	tabProperties.push_back("force_scale");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));
		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		testValues.push_back(std::string(*value));
	}
	
	if(testValues.at(0).compare("false") == 0 &&
		testValues.at(1).compare("true") == 0) { 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	
	
void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_colorType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Text (FreeType 2)", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	tabProperties.push_back("color1");
	tabProperties.push_back("color2");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));
		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		testValues.push_back(std::string(*value));
	}
	
	if(testValues.at(0).compare("ffffffff") == 0 &&
		testValues.at(1).compare("ffffffff") == 0) { 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	
	
void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_intType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Text (FreeType 2)", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	tabProperties.push_back("custom_width");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));
		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		testValues.push_back(std::string(*value));
	}
	
	if(testValues.at(0).compare("0") == 0 ) { 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_floatType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "SUCCESS";

	#ifdef __APPLE__
	{
		createScene("My scene for testing");
		addSource("Game Capture (Syphon)", "SourceTest", nullptr, nullptr, "My scene for testing");

		vector<string> testValues;
		vector<const char*> tabProperties;
		Local<Object> object;

		tabProperties.push_back("crop.origin.x");
		tabProperties.push_back("crop.origin.y");

		for(int i=0;i<tabProperties.size();i++) {
			object = getSourcePropertyCurrentValue("SourceTest", tabProperties.at(i));
			v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
			testValues.push_back(std::string(*value));
		}
		
		if(testValues.at(0).compare("0.000000") == 0 &&
			testValues.at(1).compare("0.000000") == 0) { 
			result = "SUCCESS";
		} else {
			result = "FAILURE";
		}

		removeScene("My scene for testing");
		removeSource("SourceTest");
	}
	#endif

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	
	
void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_textType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Text (FreeType 2)", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	tabProperties.push_back("text");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));
		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		testValues.push_back(std::string(*value));
	}
	
	if(testValues.at(0).compare("") == 0) { 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_fontType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Text (FreeType 2)", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	tabProperties.push_back("font");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));

		v8::String::Utf8Value faceValue(object->Get(String::NewFromUtf8(isolate, "face"))->ToString());
		testValues.push_back(std::string(*faceValue));
		v8::String::Utf8Value styleValue(object->Get(String::NewFromUtf8(isolate, "style"))->ToString());
		testValues.push_back(std::string(*styleValue));
		v8::String::Utf8Value sizeValue(object->Get(String::NewFromUtf8(isolate, "size"))->ToString());
		testValues.push_back(std::string(*sizeValue));
	}
	
	#ifdef _WIN32
	{
		if(testValues.at(0).compare("Arial") == 0 &&
			testValues.at(1).compare("") == 0 &&
			testValues.at(2).compare("") == 0) 
		{
			result = "SUCCESS";
		} 
		else 
		{
			result = "FAILURE";
		}
	}
	#else
	{
		if(testValues.at(0).compare("Helvetica") == 0 &&
			testValues.at(1).compare("") == 0 &&
			testValues.at(2).compare("32") == 0) 
		{ 
			result = "SUCCESS";
		} 
		else 
		{
			result = "FAILURE";
		}
	}
	#endif

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_pathType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Text (FreeType 2)", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	tabProperties.push_back("text_file");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));

		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		testValues.push_back(std::string(*value));
		v8::String::Utf8Value descriptionValue(object->Get(String::NewFromUtf8(isolate, "description"))->ToString());
		testValues.push_back(std::string(*descriptionValue));
		v8::String::Utf8Value typeValue(object->Get(String::NewFromUtf8(isolate, "type"))->ToString());
		testValues.push_back(std::string(*typeValue));
		v8::String::Utf8Value filterValue(object->Get(String::NewFromUtf8(isolate, "filter"))->ToString());
		testValues.push_back(std::string(*filterValue));
		v8::String::Utf8Value defaultPathValue(object->Get(String::NewFromUtf8(isolate, "default_path"))->ToString());
		testValues.push_back(std::string(*defaultPathValue));
	}
	
	if(testValues.at(0).compare("") == 0 &&
		testValues.at(1).compare("Text File (UTF-8 or UTF-16)") == 0 &&
		testValues.at(2).compare("OBS_PATH_FILE") == 0 &&
		testValues.at(3).compare("Text Files (*.txt);;") == 0 &&
		testValues.at(4).compare("") == 0) { 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_buttonType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "SUCCESS";

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	
	
void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_editableListType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Image Slide Show", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;
	Handle<Array> arrayRanges;

	tabProperties.push_back("files");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));

		arrayRanges = Local<Array>::Cast(object->Get(String::NewFromUtf8(isolate, "valuesArray")));
		v8::String::Utf8Value typeValue(object->Get(String::NewFromUtf8(isolate, "type"))->ToString());
		testValues.push_back(std::string(*typeValue));
		v8::String::Utf8Value filterValue(object->Get(String::NewFromUtf8(isolate, "filter"))->ToString());
		testValues.push_back(std::string(*filterValue));
		v8::String::Utf8Value defaultPathValue(object->Get(String::NewFromUtf8(isolate, "default_path"))->ToString());
		testValues.push_back(std::string(*defaultPathValue));
	}

	if(arrayRanges->Length() == 0 &&
		testValues.at(0).compare("OBS_EDITABLE_LIST_TYPE_FILES") == 0 &&
		testValues.at(1).compare("Image files (*.bmp *.tga *.png *.jpeg *.jpg *.gif)") == 0 &&
		testValues.at(2).compare("") == 0) 
	{ 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	
	
void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_listType_intFormat(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	#ifdef _WIN32
	{
		addSource("Video Capture Device", "SourceTest", nullptr, nullptr, "My scene for testing");
		tabProperties.push_back("res_type");
	}
	#else
	{
		addSource("Display Capture", "SourceTest", nullptr, nullptr, "My scene for testing");
		tabProperties.push_back("display");
	}
	#endif
    
    obs_source_t *source = obs_get_source_by_name("SourceTest");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));
		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		testValues.push_back(std::string(*value));
	}
	
	if(testValues.at(0).compare("0") == 0) 
	{ 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_listType_floatFormat(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();

	//No list with float type to test
	string result = "SUCCESS";

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	

void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_listType_stringFormat(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Image Slide Show", "SourceTest", nullptr, nullptr, "My scene for testing");

    obs_source_t *source = obs_get_source_by_name("SourceTest");

	vector<string> testValues;
	vector<const char*> tabProperties;
	Local<Object> object;

	tabProperties.push_back("transition");

	for(int i=0;i<tabProperties.size();i++) {
		object = getSourcePropertyCurrentValue(source, tabProperties.at(i));
		v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
		testValues.push_back(std::string(*value));
	}
	
	if(testValues.at(0).compare("fade") == 0) 
	{ 
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	
	
void OBS_content::OBS_content_test_getSourcePropertyCurrentValue_frameRateType(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Video Capture Device", "SourceTest", nullptr, nullptr, "My scene for testing");

	#ifdef UNIX
	{
		vector<string> testValues;
		vector<const char*> tabProperties;
		Local<Object> object = Object::New(isolate);;

		object->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, "CC25243BJN1G1HNAN"));
		setProperty("SourceTest", "device", object);
		object->Delete(String::NewFromUtf8(isolate, "value"));

		object->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, "{\\n    \\\"height\\\": 720,\\n    \\\"width\\\": 1280\\n}"));
		setProperty("SourceTest", "resolution", object);
		object->Delete(String::NewFromUtf8(isolate, "value"));

		object->Set(String::NewFromUtf8(isolate, "numerator"), String::NewFromUtf8(isolate, "30"));
		object->Set(String::NewFromUtf8(isolate, "denominator"), String::NewFromUtf8(isolate, "1"));
		setProperty("SourceTest", "frame_rate", object);
		object->Delete(String::NewFromUtf8(isolate, "numerator"));
		object->Delete(String::NewFromUtf8(isolate, "denominator"));
		
		tabProperties.push_back("frame_rate");

		object = getSourcePropertyCurrentValue("SourceTest", tabProperties.at(0));

		Local<Array> arrayRanges = Local<Array>::Cast(object->Get(String::NewFromUtf8(isolate, "ranges")));

		for (int i = 0; i < arrayRanges->Length(); i++) {
			Local<Object> currentRange = v8::Handle<v8::Object>::Cast(arrayRanges->Get(i));

			Local<Object> minObject = v8::Handle<v8::Object>::Cast(currentRange->Get(String::NewFromUtf8(isolate, "min")));
			v8::String::Utf8Value minNumeratorValue(minObject->Get(String::NewFromUtf8(isolate, "numerator"))->ToString());
			testValues.push_back(std::string(*minNumeratorValue));
			v8::String::Utf8Value minDenominatorValue(minObject->Get(String::NewFromUtf8(isolate, "denominator"))->ToString());
			testValues.push_back(std::string(*minDenominatorValue));

			Local<Object> maxObject = v8::Handle<v8::Object>::Cast(currentRange->Get(String::NewFromUtf8(isolate, "max")));
			v8::String::Utf8Value maxNumeratorValue(maxObject->Get(String::NewFromUtf8(isolate, "numerator"))->ToString());
			testValues.push_back(std::string(*maxNumeratorValue));
			v8::String::Utf8Value maxDenominatorValue(maxObject->Get(String::NewFromUtf8(isolate, "denominator"))->ToString());
			testValues.push_back(std::string(*maxDenominatorValue));
		}

		v8::String::Utf8Value numeratorValue(object->Get(String::NewFromUtf8(isolate, "numerator"))->ToString());
		testValues.push_back(std::string(*numeratorValue));
		v8::String::Utf8Value denominatorValue(object->Get(String::NewFromUtf8(isolate, "denominator"))->ToString());
		testValues.push_back(std::string(*denominatorValue));

		if(testValues.at(0).compare("30") == 0 &&
			testValues.at(1).compare("1") == 0) 
		{ 
			result = "SUCCESS";
		} else {
			result = "FAILURE";
		}
	}
	#else 
	{
		result = "SUCCESS";
	}
	#endif

	removeScene("My scene for testing");
	removeSource("SourceTest");

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}	

void OBS_content::OBS_content_test_setProperty(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result;

	addSource("Text (FreeType 2)", "TextFreeType", nullptr, nullptr, "MyScene");
	
    obs_source_t *source = obs_get_source_by_name("TextFreeType");

	Local<Object> object = getSourcePropertyCurrentValue(source, "custom_width");
	v8::String::Utf8Value oldValueStringV8(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
	std::string oldValueString = std::string(*oldValueStringV8);
	double oldValue = atof(oldValueString.c_str());

	object = Object::New(isolate);
	object->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, "12"));

	bool needsToBeRefresh = setProperty(source, "custom_width", object);


	object = getSourcePropertyCurrentValue(source, "custom_width");
	v8::String::Utf8Value newValueStringV8(object->Get(String::NewFromUtf8(isolate, "value"))->ToString());
	std::string newValueString = std::string(*newValueStringV8);
	double newValue = atof(newValueString.c_str());

	if(oldValue == 0 && newValue == 12 && !needsToBeRefresh) {
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

    obs_source_release(source);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_setSourcePosition(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Text (FreeType 2)", "SourceTest", nullptr, nullptr, "My scene for testing");

	setSourcePosition("My scene for testing", "SourceTest", 123, 456);

	obs_source_t* sourceScene = obs_get_source_by_name(currentScene.c_str());
	obs_scene_t* scene = obs_scene_from_source(sourceScene);

	obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, "SourceTest");

	struct vec2 position;

	obs_sceneitem_get_pos(sourceItem, &position);

	if(position.x == 123 &&
		position.y == 456 ) {
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_setSourceScaling(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

	createScene("My scene for testing");
	addSource("Text (FreeType 2)", "SourceTest", nullptr, nullptr, "My scene for testing");

	setSourceScaling("My scene for testing", "SourceTest", 1, 2);

	obs_source_t* sourceScene = obs_get_source_by_name(currentScene.c_str());
	obs_scene_t* scene = obs_scene_from_source(sourceScene);

	obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, "SourceTest");

	struct vec2 scale;

	obs_sceneitem_get_scale(sourceItem, &scale);


	if(scale.x == 1 &&
		scale.y == 2 ) 
	{
		result = "SUCCESS";
	} else {
		result = "FAILURE";
	}

	removeScene("My scene for testing");
	removeSource("SourceTest");

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));
}

void OBS_content::OBS_content_test_setSourceOrder(const FunctionCallbackInfo<Value>& args)
{
	Isolate* isolate = args.GetIsolate();
	string result = "FAILURE";

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result.c_str()));	
}

vector<std::string> OBS_content::getListCurrentScenes(void)
{
    return tabScenes; 
}

vector<const char*> OBS_content::getListCurrentSourcesFromScene(const std::string &name)
{
	vector<const char*> listSourcesName;

	obs_source_t* source = obs_get_source_by_name(name.c_str());
	obs_scene_t* scene = obs_scene_from_source(source);

	auto func = [] (obs_scene_t *, obs_sceneitem_t *item, void *param)
    {
        vector<const char*> &listSourcesName = *reinterpret_cast<vector<const char*>*>(param);

        obs_source_t *source = obs_sceneitem_get_source(item);
        listSourcesName.push_back(obs_source_get_name(source));
        obs_source_release(source);
        return true;
    };

    obs_scene_enum_items(scene, func, &listSourcesName); 
    obs_source_release(source);
    return listSourcesName;
}

vector<const char*>  OBS_content::getListInputSources(void)
{
	vector<const char*> listInputSource;

	if(tabTypeInputSources.size() != 0) {
		tabTypeInputSources.clear();
	}

	size_t index = 0;
	const char* type;
	while(obs_enum_input_types(index++, &type)) {
		tabTypeInputSources.push_back(type);
		listInputSource.push_back(obs_source_get_display_name(type));	
	}
	
	
	return listInputSource;
}

vector<const char*> OBS_content::getListFilterNames(void)
{
	vector<const char*> listFilters;
	size_t index = 0;
	const char* type;
	while(obs_enum_filter_types(index++, &type)) {
		listFilters.push_back(obs_source_get_display_name(type));	
	}

	return listFilters;
}

vector<const char*>  OBS_content::getListFilterTypes(void)
{
    vector<const char*> listFilters;
    size_t index = 0;
    const char* type;
    while (obs_enum_filter_types(index++, &type)) {
        listFilters.push_back(type);
    }

    return listFilters;
}

vector<const char*>  OBS_content::getListTransitionNames(void)
{
	vector<const char*> listTransitions;
	size_t index = 0;
	const char* type;
	while(obs_enum_transition_types(index++, &type)) {
		listTransitions.push_back(obs_source_get_display_name(type));
	}

	return listTransitions;
}

vector<const char*>  OBS_content::getListTransitionTypes(void)
{
    vector<const char*> listTransitions;
    size_t index = 0;
    const char* type;
    while (obs_enum_transition_types(index++, &type)) {
        listTransitions.push_back(type);
    }

    return listTransitions;
}

bool OBS_content::createScene(const std::string &name)
{
	bool success;

	obs_scene_t* scene = obs_scene_create(name.c_str());
	
	if(scene != NULL) {
		obs_source_t* source = obs_scene_get_source(scene);

		tabScenes.push_back(name.c_str());
		success = true;
		setCurrentScene(name.c_str());
	}
	else {
		success = false;
	}

	return success;
}

bool OBS_content::removeScene(const std::string &name)
{
	obs_source_t* source = obs_get_source_by_name(name.c_str());
    obs_scene_t* scene = obs_scene_from_source(source);

    if (!source)
        return false;

    std::vector<obs_sceneitem_t*> items;

    auto cb = 
    [](obs_scene_t*, obs_sceneitem_t* item, void* data) -> bool {
        std::vector<obs_sceneitem_t*> *items =
            reinterpret_cast<std::vector<obs_sceneitem_t*>*>(data);

        items->push_back(item);
        return true;
    };

    /* Destroy all sceneitems within the scene. */
    obs_scene_enum_items(scene, cb, &items);

    for (int i = 0; i < items.size(); ++i) {
        obs_source_t *source = obs_sceneitem_get_source(items[i]);

        obs_source_release(source);
    }

    for (int i = 0; i < tabScenes.size(); ++i) {
        if (tabScenes[i] == name) {
            tabScenes.erase(tabScenes.begin() + i);
            break;
        }
    }
	obs_source_remove(source);
	obs_source_release(source); /* obs_get_source_by_name */
	obs_scene_release(scene); /* obs_create_scene */
	return obs_source_removed(source);
}

uint32_t createFader(obs_source_t *source)
{
	obs_fader_t *handle = obs_fader_create(OBS_FADER_CUBIC);
	obs::object object = { obs::object::fader, handle };
	uint32_t id = g_objectManager.map(object);
	obs_fader_attach_source(handle, source);

	return id;
}

uint32_t createVolmeter(obs_source_t *source)
{
	obs_volmeter_t *handle = obs_volmeter_create(OBS_FADER_LOG);
	obs::object object = { obs::object::volmeter, handle };
	uint32_t id = g_objectManager.map(object);
	obs_volmeter_attach_source(handle, source);

	return id;
}

void destroyFader(uint32_t id)
{
	obs_fader_t *handle = 
		obs::get_handle<obs::object::fader, obs_fader_t*>(id);
	obs_fader_destroy(handle);
	g_objectManager.unmap(id);
}

void destroyVolmeter(uint32_t id)
{
	obs_volmeter_t *handle = 
		obs::get_handle<obs::object::volmeter, obs_volmeter_t*>(id);
	obs_volmeter_destroy(handle);
	g_objectManager.unmap(id);
}

void OBS_content::OBS_content_getSourceFader(const FunctionCallbackInfo<Value>& args)
{
	v8::String::Utf8Value source_name(args[0]);

	obs_source_t *source = nullptr;
	SourceInfo *source_info = nullptr;

	source = obs_get_source_by_name(*source_name);

	if (!source) {
		std::cerr << "Failed to find source " << *source_name << std::endl;
		return;
	}

	auto info_it = sourceInfo.find(*source_name);

	source_info = info_it->second;

	/* Create new object with id */
	auto object = Nan::New<v8::Object>();
	obs::set_id(Isolate::GetCurrent(), object, source_info->fader);
	obs_fader_t *handle = 
		obs::get_handle<obs::object::fader, obs_fader_t*>(source_info->fader);
	
	/* Set some initial data. */
	Nan::Set(object, 
		Nan::New("db").ToLocalChecked(),
		Nan::New(obs_fader_get_db(handle)));

	Nan::Set(object,
		Nan::New("deflection").ToLocalChecked(),
		Nan::New(obs_fader_get_deflection(handle)));

	obs_source_release(source);

	args.GetReturnValue().Set(object);
}

void OBS_content::OBS_content_getSourceVolmeter(const FunctionCallbackInfo<Value>& args)
{
	v8::String::Utf8Value source_name(args[0]);

	obs_source_t *source = nullptr;
	SourceInfo *source_info = nullptr;

	source = obs_get_source_by_name(*source_name);

	if (!source) {
		std::cerr << "Failed to find source " << *source_name << std::endl;
		return;
	}

	auto info_it = sourceInfo.find(*source_name);
	source_info = info_it->second;

	/* Create new object with id */
	auto object = Nan::New<v8::Object>();
	obs::set_id(Isolate::GetCurrent(), object, source_info->volmeter);

	obs_source_release(source);
	args.GetReturnValue().Set(object);
}

bool OBS_content::addSource(const std::string &typeSource, const std::string &name,
							obs_data_t* settings, obs_data_t *hotkey_data, 
							const char* sceneName)
{
	bool success;
	const char* sourceType = "";
	int index = 0;

	if(strcmp(sourceType, "") != 0){
		obs_source_t* newSource = obs_source_create(sourceType, name.c_str(), settings, hotkey_data);

		if(newSource != NULL) {
			obs_source_t* source = obs_get_source_by_name(sceneName);
			obs_scene_t* scene = obs_scene_from_source(source);

			obs_scene_add(scene, newSource);

			obs_source_release(source);
			/* Create Source Info and associate it with the source */
			uint32_t fader = createFader(newSource);
			uint32_t volmeter = createVolmeter(newSource);
			SourceInfo * info = new SourceInfo { fader, volmeter };
			sourceInfo[name] = info;

			success = true;
		}
		else {
			success = false;
		}

	}
	else {
		success = false;
	}

	return success;
}

bool OBS_content::removeSource(const std::string &name)
{	
	obs_source_t* source = obs_get_source_by_name(name.c_str());

	obs_source_t* sourceScene = obs_get_source_by_name(currentScene.c_str());
	obs_scene_t* scene = obs_scene_from_source(sourceScene);

	obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, name.c_str());

	obs_sceneitem_remove(sourceItem);

    obs_source_release(sourceScene);

	remove_cached_properties(source);
	obs_source_release(source);
    obs_source_release(source);

	auto info_it = sourceInfo.find(name);
	SourceInfo *info = info_it->second;
	destroyFader(info->fader);
	destroyVolmeter(info->volmeter);
	delete info;
	sourceInfo.erase(info_it);

	return obs_source_removed(source);
}

void pushValue(vector<std::string>* array, std::string value) {
	if(!value.empty()) {
		array->push_back(value);
	} else {
		array->push_back("");
	}
}

void OBS_content::updateSourceProperties(obs_source_t* source, obs_data_t* settings) 
{
	obs_source_update_properties(source); 
	obs_source_update(source, settings);
}

vector<std::string> OBS_content::getSourceProperties(obs_source_t *source)
{
	vector<std::string> listProperties;
	
	std::string nameProperty;
	std::string descriptionProperty;
	std::string longDescriptionProperty;
	std::string visible;
	std::string enabled;

	obs_property_type typeProperty;
	std::string typePropertyName;

	obs_properties_t* properties = refresh_cached_properties(source);
	obs_property_t* property = obs_properties_first(properties);

	obs_data_t* settings = obs_source_get_settings(source);
	obs_properties_apply_settings(properties, settings);

	while(property) {
		nameProperty = obs_property_name(property);
		descriptionProperty = obs_property_description(property);
		typeProperty = obs_property_get_type(property);

		if(obs_property_long_description(property) != NULL) {
			longDescriptionProperty = obs_property_long_description(property);
		}

		visible = obs_property_visible(property) ? "true" : "false";
		enabled = obs_property_enabled(property) ? "true" : "false";

		pushValue(&listProperties, nameProperty);
		pushValue(&listProperties, descriptionProperty);
		pushValue(&listProperties, longDescriptionProperty);

		std::string subType;
		double minVal = 0;
		double maxVal = 0;
		double stepVal = 0;

		switch(typeProperty) {
			case OBS_PROPERTY_INVALID: 
				typePropertyName = "OBS_PROPERTY_INVALID";
				break;
			case OBS_PROPERTY_BOOL: 
				typePropertyName = "OBS_PROPERTY_BOOL";
				break;
			case OBS_PROPERTY_INT: 
			{
				typePropertyName = "OBS_PROPERTY_INT";

				//Getting subtype
				obs_number_type type = obs_property_int_type(property);
				if(type == OBS_NUMBER_SLIDER) {
					subType = "OBS_NUMBER_SLIDER";

					//Getting min, max and step values
					minVal = obs_property_int_min(property);
					maxVal = obs_property_int_max(property);
					stepVal = obs_property_int_step(property);
				} 

				break;
			}
			case OBS_PROPERTY_FLOAT: 
			{
				typePropertyName = "OBS_PROPERTY_FLOAT";

				//Getting subtype
				obs_number_type type = obs_property_float_type(property);
				if(type == OBS_NUMBER_SLIDER) {
					subType = "OBS_NUMBER_SLIDER";

					//Getting min, max and step values
					minVal = obs_property_float_min(property);
					maxVal = obs_property_float_max(property);
					stepVal = obs_property_float_step(property);
				} 

				break;
			}
			case OBS_PROPERTY_TEXT: 
				typePropertyName = "OBS_PROPERTY_TEXT";
				break;
			case OBS_PROPERTY_PATH: 
				typePropertyName = "OBS_PROPERTY_PATH";
				break;
			case OBS_PROPERTY_LIST: 
				typePropertyName = "OBS_PROPERTY_LIST";
				break;
			case OBS_PROPERTY_COLOR: 
				typePropertyName = "OBS_PROPERTY_COLOR";
				break;
			case OBS_PROPERTY_BUTTON: 
				typePropertyName = "OBS_PROPERTY_BUTTON";
				break;
			case OBS_PROPERTY_FONT: 
				typePropertyName = "OBS_PROPERTY_FONT";
				break;
			case OBS_PROPERTY_EDITABLE_LIST: 
				typePropertyName = "OBS_PROPERTY_EDITABLE_LIST";
				break;
			case OBS_PROPERTY_FRAME_RATE: 
				typePropertyName = "OBS_PROPERTY_FRAME_RATE";
				break;
		}
		pushValue(&listProperties, typePropertyName);
		pushValue(&listProperties, visible);
		pushValue(&listProperties, enabled);
		pushValue(&listProperties, subType);
		pushValue(&listProperties, to_string(minVal));
		pushValue(&listProperties, to_string(maxVal));
		pushValue(&listProperties, to_string(stepVal));

		obs_property_next(&property);
	}

	return listProperties;
}

vector<string> OBS_content::getSourcePropertiesSubParameters(obs_source_t *source, const std::string &propertyNameSelected)
{
	vector<string> listPropertiesSubParameters;
	
	obs_properties_t* properties = get_cached_properties(source);
	obs_property_t* property = obs_properties_first(properties);
	obs_combo_format format;
	//const char* valueString;

	bool propertyFound = false;

	vector<string> testStringVector;

	while(property && !propertyFound) {
		std::string nameProperty(obs_property_name(property));

		if(nameProperty == propertyNameSelected) {
			int count = (int)obs_property_list_item_count(property);
			for(int i=0;i<count;i++) {
				//Name
				listPropertiesSubParameters.push_back(obs_property_list_item_name(property, i));
				
				//Value
				format = obs_property_list_format(property);

				if (format == OBS_COMBO_FORMAT_INT) 
				{
					long long val = obs_property_list_item_int(property, i);
					string valueString = std::to_string(val);
					listPropertiesSubParameters.push_back(valueString);
				} 
				else if (format == OBS_COMBO_FORMAT_FLOAT) 
				{
					double val = obs_property_list_item_float(property, i);
					string valueString = to_string(val).c_str();
					listPropertiesSubParameters.push_back(valueString);
				} 
				else if (format == OBS_COMBO_FORMAT_STRING) 
				{
					string valueString(obs_property_list_item_string(property, i));
					listPropertiesSubParameters.push_back(valueString);
				}
			}
			propertyFound = true;
		}
		obs_property_next(&property);
	}

	int index = 1;
	for(int i=0;i<testStringVector.size();i++) {
		listPropertiesSubParameters.insert(listPropertiesSubParameters.begin() + index, testStringVector.at(i).c_str());
		index += 2;
	}

	return listPropertiesSubParameters;
}

void color_from_int(string& out, long long val)
{
    out.resize(8);

    for (int i = 0; i < 4; ++i) {
        sprintf(&out[i * 2], "%02x", (val >> i * 8) & 0xFF);
    }
}

long long color_to_int(const char* color)
{
	int numberOfColors = 4;
	string colorsString;
	vector<long long> arrayValues;
	long long value = 0;

	for(int i=0;i<numberOfColors*2;i+=2) {
		//Extract hex color value to a string
		string colorString(1, color[i]);
		colorString += color[i+1];

		//Convert hex string to int
		int colorInt = strtoul(colorString.substr(0, 2).c_str(), NULL, 16);

		//Store int to final color int value
		arrayValues.push_back(colorInt);
	}

	value = ((arrayValues.at(0) & 0xff) << 0) |
			((arrayValues.at(1) & 0xff) << 8) |
			((arrayValues.at(2) & 0xff) << 16) |
			((arrayValues.at(3) & 0xff) << 24);

	return value;
}

Local<Object> OBS_content::getSourcePropertyCurrentValue(obs_source_t *source, const std::string &propertyNameSelected)
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Object> objectValue = Object::New(isolate);

	const char* propertyDefaultValue = "initial";
	
	obs_property_type typeProperty;

	obs_properties_t* properties = get_cached_properties(source);
	obs_property_t* property = obs_properties_first(properties);
	
	obs_data_t* settings = obs_source_get_settings(source);

	bool propertyFound = false;

	bool updateProperties = false;

	while(property && !propertyFound) {
		std::string nameProperty(obs_property_name(property));
		std::string descriptionProperty(obs_property_description(property));

		if(nameProperty == propertyNameSelected) {
			typeProperty = obs_property_get_type(property);
			
			switch(typeProperty) {
				case OBS_PROPERTY_INVALID: {
					break;
				}
				case OBS_PROPERTY_BOOL: 
				{
					bool boolValue = obs_data_get_bool(settings, nameProperty.c_str());
					
					objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, boolValue ? "true" : "false"));
					const char* test = boolValue ? "true" : "false";
					break;
				}	
				case OBS_PROPERTY_COLOR:
				{
					std::string colorString;
					int intValue = (int)obs_data_get_int(settings, nameProperty.c_str());
					color_from_int(colorString, intValue);
					objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, colorString.c_str()));
					break;
				}
				case OBS_PROPERTY_INT: 
				{
					int intValue = (int)obs_data_get_int(settings, nameProperty.c_str());
					objectValue->Set(String::NewFromUtf8(isolate, "value"), v8::Integer::New(isolate, intValue));
					break;
				}
				case OBS_PROPERTY_FLOAT: 
				{
					double floatValue = (float)obs_data_get_double(settings, nameProperty.c_str());
					floatValue = round( floatValue * 100.0 ) / 100.0;
					objectValue->Set(String::NewFromUtf8(isolate, "value"), v8::Number::New(isolate, floatValue));
					break;
				}
				case OBS_PROPERTY_FONT: 
				{
					obs_data_t* fontObject  = obs_data_get_obj(settings, nameProperty.c_str());
					std::string face        = obs_data_get_string(fontObject, "face");
					std::string style       = obs_data_get_string(fontObject, "style");
					std::string size 		= to_string(obs_data_get_int(fontObject, "size")).c_str();
					int32_t flags           = static_cast<int32_t>(obs_data_get_int(fontObject, "flags"));
					std::string description;

					if(obs_property_long_description(property) != NULL) {
						description = obs_property_long_description(property);
					}

					objectValue->Set(String::NewFromUtf8(isolate, "face"), String::NewFromUtf8(isolate, face.c_str()));
					objectValue->Set(String::NewFromUtf8(isolate, "style"), String::NewFromUtf8(isolate, style.c_str()));
					objectValue->Set(String::NewFromUtf8(isolate, "size"), String::NewFromUtf8(isolate, size.c_str()));
					Nan::Set(objectValue, Nan::New<v8::String>("flags").ToLocalChecked(), Nan::New<v8::Integer>(flags));

					if(!description.empty()) {
						objectValue->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, description.c_str()));
					}

					obs_data_release(fontObject);
					break;
				}
				case OBS_PROPERTY_PATH: 
				{
					const char* pathValue = obs_data_get_string(settings, nameProperty.c_str());
					const char* description = obs_property_description(property);
					obs_path_type type = obs_property_path_type(property);
					const char    *filter       = obs_property_path_filter(property);
					const char    *default_path = obs_property_path_default_path(property);
					
					if(default_path == NULL ) {
						default_path = "";
					}

					const char* typeString = "";

					objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, pathValue));

					if(description != NULL) {					
						objectValue->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, description));
					}
		
					objectValue->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, pathTypeToString(type)));
					objectValue->Set(String::NewFromUtf8(isolate, "filter"), String::NewFromUtf8(isolate, filter));
					objectValue->Set(String::NewFromUtf8(isolate, "default_path"), String::NewFromUtf8(isolate, default_path));
					
					break;
				}
				case OBS_PROPERTY_BUTTON: 
				{
					const char* description = obs_property_description(property);

					if(description != NULL) {
						objectValue->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, description));
					} else {
						objectValue->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, ""));
					}

					break;
				}
				case OBS_PROPERTY_EDITABLE_LIST: 
				{
					obs_data_array_t *array = obs_data_get_array(settings, nameProperty.c_str());
					size_t           count  = obs_data_array_count(array);
					Handle<Array> 	 arrayValues = Array::New(isolate, (int)count);
					enum obs_editable_list_type type = obs_property_editable_list_type(property);
					const char* typeString = "";
					const char* filter = obs_property_editable_list_filter(property);
					const char* default_path = obs_property_editable_list_default_path(property);


					for (size_t i = 0; i < count; i++) {
						obs_data_t *item = obs_data_array_item(array, i);
						arrayValues->Set((uint32_t)i, String::NewFromUtf8(isolate, obs_data_get_string(item, "value")));
						obs_data_release(item);
					}
				
					objectValue->Set(String::NewFromUtf8(isolate, "valuesArray"), arrayValues);

					switch(type) 
					{
						case OBS_EDITABLE_LIST_TYPE_STRINGS:
						{
							typeString = "OBS_EDITABLE_LIST_TYPE_STRINGS";
							break;
						}
						case OBS_EDITABLE_LIST_TYPE_FILES:
						{
							typeString = "OBS_EDITABLE_LIST_TYPE_FILES";
							break;
						}
						case OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS:
						{
							typeString = "OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS";
							break;
						}
					}

					objectValue->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, typeString));
					objectValue->Set(String::NewFromUtf8(isolate, "filter"), String::NewFromUtf8(isolate, filter));
					if(default_path == NULL) {
						default_path = "";
					}
					objectValue->Set(String::NewFromUtf8(isolate, "default_path"), String::NewFromUtf8(isolate, default_path));
					break;
				}
				case OBS_PROPERTY_TEXT:  
				{
					std::string textValue = obs_data_get_string(settings, nameProperty.c_str());
					objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, textValue.c_str()));
					
					break;
				}
				case OBS_PROPERTY_LIST:  
				{
					obs_combo_format format = obs_property_list_format(property);

					switch(format) {
						case OBS_COMBO_FORMAT_INVALID:
							break;
						case OBS_COMBO_FORMAT_INT:
						{
							long long value = obs_data_get_int(settings, nameProperty.c_str());
							string valueString = to_string(value);
							objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, valueString.c_str()));

							break;
						}
						case OBS_COMBO_FORMAT_FLOAT: 
						{
							double value = obs_data_get_double(settings, nameProperty.c_str());
							string valueString = to_string(value);
							objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, valueString.c_str()));

							break;
						}
						case OBS_COMBO_FORMAT_STRING: 
						{
							const char* textValue = obs_data_get_string(settings, nameProperty.c_str());
							if(strcmp(textValue, "") == 0) {
								const char* valueItem = obs_property_list_item_string(property, 0);
								if (valueItem != NULL) {
									string valueString(valueItem);
									obs_data_set_string(settings, nameProperty.c_str(), valueString.c_str());
									objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, valueString.c_str()));
									updateProperties = true;
								}
							} else {
								objectValue->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, textValue));
							}
							break;
						}
					}
					break;
				}
				case OBS_PROPERTY_FRAME_RATE: 
				{
					int rangeCount = (int)obs_property_frame_rate_fps_ranges_count(property);
					
					Local<Array> arrayRanges = Array::New(isolate, rangeCount);

					stringstream stream;
					string stringValue;

					for(int i=0;i<rangeCount;i++) {
						Local<Object> range = Object::New(isolate);
						
						media_frames_per_second minRange = obs_property_frame_rate_fps_range_min(property, i);
						media_frames_per_second maxRange = obs_property_frame_rate_fps_range_max(property, i);

						Local<Object> min = Object::New(isolate);
						min->Set(String::NewFromUtf8(isolate, "numerator"), String::NewFromUtf8(isolate, to_string(minRange.numerator).c_str()));
						min->Set(String::NewFromUtf8(isolate, "denominator"), String::NewFromUtf8(isolate, to_string(minRange.denominator).c_str()));

						Local<Object> max = Object::New(isolate);
						max->Set(String::NewFromUtf8(isolate, "numerator"), String::NewFromUtf8(isolate, to_string(maxRange.numerator).c_str()));
						max->Set(String::NewFromUtf8(isolate, "denominator"), String::NewFromUtf8(isolate, to_string(maxRange.denominator).c_str()));

						range->Set(String::NewFromUtf8(isolate, "min"), min);
						range->Set(String::NewFromUtf8(isolate, "max"), max);

						arrayRanges->Set(i, range);
					}
					obs_data_item_t* dataItem = obs_data_item_byname(settings, nameProperty.c_str());
					media_frames_per_second fps{};
					obs_data_item_get_frames_per_second(dataItem, &fps, nullptr);

					blog(LOG_INFO, "Numerator : %d", fps.numerator);
					blog(LOG_INFO, "Denominator : %d", fps.denominator);

					objectValue->Set(String::NewFromUtf8(isolate, "ranges"), arrayRanges);
					objectValue->Set(String::NewFromUtf8(isolate, "numerator"), String::NewFromUtf8(isolate,to_string(fps.numerator).c_str()));
					objectValue->Set(String::NewFromUtf8(isolate, "denominator"), String::NewFromUtf8(isolate,to_string(fps.denominator).c_str()));
					break;
				}
			}
			propertyFound = true;
		}

		obs_property_next(&property);
	}

	if(!propertyFound)
	{
		blog(LOG_ERROR, "Property not found");
	}

	return objectValue;
}

bool OBS_content::setProperty(obs_source_t *source, const std::string &propertyNameSelected, Local<Object> objectValue)
{
	obs_property_type typeProperty;

	Isolate *isolate = v8::Isolate::GetCurrent();
	v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, "value");
	//Local<Object> objectValue = Object::New(isolate);

	obs_properties_t* properties = get_cached_properties(source);
	obs_property_t* property = obs_properties_first(properties);
	
	obs_data_t* settings = obs_source_get_settings(source);

	bool propertyFound = false;
	bool arePropertiesNeedToBeRefresh;

	while(property && !propertyFound) {
		std::string nameProperty(obs_property_name(property));
        std::string descriptionProperty(obs_property_description(property));

		if(nameProperty == propertyNameSelected) {
			typeProperty = obs_property_get_type(property);

			switch(typeProperty) {
				case OBS_PROPERTY_INVALID: 
					break;
				case OBS_PROPERTY_BOOL: 
				{
					bool boolValue;

					v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
					string valueString = std::string(*value);

					if(valueString == "true") {
						boolValue = true;
					} else {
						boolValue = false;
					}

					obs_data_set_bool(settings, nameProperty.c_str(), boolValue);

					break;
				}	
				case OBS_PROPERTY_COLOR:
				{
					v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
					string valueString = std::string(*value);

					obs_data_set_int(settings, nameProperty.c_str(), color_to_int(valueString.c_str()));
					break;
				}
				case OBS_PROPERTY_INT: 
				{
					v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
					string valueString = std::string(*value);

					int intValue = atoi(valueString.c_str());
					obs_data_set_int(settings, nameProperty.c_str(), intValue);
					break;
				}
				case OBS_PROPERTY_FLOAT: 
				{
					v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
					string valueString = std::string(*value);

					double floatValue = (double)atof(valueString.c_str());
					obs_data_set_double(settings, nameProperty.c_str(), floatValue);
					break;
				}
				case OBS_PROPERTY_TEXT: 
				{
					obs_text_type type = obs_proprety_text_type(property);
					v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
					string valueString = std::string(*value);
					obs_data_set_string(settings, nameProperty.c_str(), valueString.c_str());
					break;
				}
				case OBS_PROPERTY_FONT: 
				{
					obs_data_t* fontObject  = obs_data_get_obj(settings, nameProperty.c_str());

					v8::String::Utf8Value valueFace(objectValue->Get(String::NewFromUtf8(isolate, "face"))->ToString());
					string valueFaceString = std::string(*valueFace);
					obs_data_set_string(fontObject, "face", valueFaceString.c_str());

					v8::String::Utf8Value valueStyle(objectValue->Get(String::NewFromUtf8(isolate, "style"))->ToString());
					string valueStyleString = std::string(*valueStyle);
					obs_data_set_string(fontObject, "style", valueStyleString.c_str());

					v8::String::Utf8Value valueSize(objectValue->Get(String::NewFromUtf8(isolate, "size"))->ToString());
					string valueSizeString = std::string(*valueSize);
					obs_data_set_int(fontObject, "size", atoi(valueSizeString.c_str()));

					auto valueFlags = Nan::Get(objectValue, Nan::New<v8::String>("flags").ToLocalChecked()).ToLocalChecked();
					obs_data_set_int(fontObject, "flags", Nan::To<int32_t>(valueFlags).FromJust());

					obs_data_set_obj(settings, nameProperty.c_str(), fontObject);
					obs_data_release(fontObject);
					break;
				}
				case OBS_PROPERTY_PATH:  
				{
					v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
					string valueString = std::string(*value);

					obs_data_set_string(settings, nameProperty.c_str(), valueString.c_str());
					break;
				}
				case OBS_PROPERTY_BUTTON: 
				{
					bool result = obs_property_button_clicked(property, source);
					break;
				}
				case OBS_PROPERTY_EDITABLE_LIST: 
				{
					obs_data_array *array = obs_data_array_create();
					Handle<Array> arrayRanges = Local<Array>::Cast(objectValue->Get(String::NewFromUtf8(isolate, "valuesArray")));
					
					for (uint32_t i = 0; i < arrayRanges->Length(); i++) {
						obs_data_t *arrayItem = obs_data_create();

						v8::String::Utf8Value value(arrayRanges->Get(i)->ToString());
						string valueString = std::string(*value);

						obs_data_set_string(arrayItem, "value", valueString.c_str());

						obs_data_array_push_back(array, arrayItem);
						obs_data_release(arrayItem);
					}
					obs_data_set_array(settings, nameProperty.c_str(), array);
					obs_data_array_release(array);
					
					break;
				}
				case OBS_PROPERTY_LIST: 
				{
					obs_combo_format format = obs_property_list_format(property);
					obs_combo_type type = obs_property_list_type(property);

					switch(format) {
						case OBS_COMBO_FORMAT_INVALID:
							return false;
						case OBS_COMBO_FORMAT_INT:
						{
							v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
							string valueString = std::string(*value);

							int intValue = atoi(valueString.c_str());
							obs_data_set_int(settings, nameProperty.c_str(), intValue);
							break;
						}
						case OBS_COMBO_FORMAT_FLOAT: 
						{
							v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
							string valueString = std::string(*value);

							float floatValue = (float)atof(valueString.c_str());
							obs_data_set_double(settings, nameProperty.c_str(), floatValue);
							break;
						}
						case OBS_COMBO_FORMAT_STRING: 
						{
							v8::String::Utf8Value value(objectValue->Get(String::NewFromUtf8(isolate, "value"))->ToString());
							string valueString = std::string(*value);

							obs_data_set_string(settings, nameProperty.c_str(), valueString.c_str());
							break;
						}
					}

					break;
				}
				case OBS_PROPERTY_FRAME_RATE: 
				{
					if (!obs_data_has_user_value(settings, nameProperty.c_str())) {
						obs_data_set_obj(settings, nameProperty.c_str(), nullptr);
					}
					
					obs_data_item_t* dataItem = obs_data_item_byname(settings, nameProperty.c_str());

					media_frames_per_second fps{};
					
					v8::String::Utf8Value valueNumerator(objectValue->Get(String::NewFromUtf8(isolate, "numerator"))->ToString());
					v8::String::Utf8Value valueDenominator(objectValue->Get(String::NewFromUtf8(isolate, "denominator"))->ToString());

					string numeratorString = std::string(*valueNumerator);
					string denominatorString = std::string(*valueDenominator);

					fps.numerator = atoi(numeratorString.c_str());
					fps.denominator = atoi(denominatorString.c_str());
					
					obs_data_item_set_frames_per_second(&dataItem, fps, nullptr);

					break;
				}
			}
			arePropertiesNeedToBeRefresh = obs_property_modified(property, settings);
			propertyFound = true;
		}

		obs_property_next(&property);
	}
	
    updateSourceProperties(source, settings);
	
	if(!propertyFound)
	{
		blog(LOG_ERROR, "Property not found");
		return false;
	}
	else {
		return arePropertiesNeedToBeRefresh;
	}
}

const char* OBS_content::pathTypeToString(obs_path_type type) 
{
	const char* typeString = "";

	switch(type) 
	{
		case OBS_PATH_DIRECTORY: 
		{
			typeString = "OBS_PATH_DIRECTORY";
			break;
		}
		case OBS_PATH_FILE: 
		{
			typeString = "OBS_PATH_FILE";
			break;
		}
		case OBS_PATH_FILE_SAVE: 
		{
			typeString = "OBS_PATH_FILE_SAVE";
			break;
		}
	}
	return typeString;
}

void OBS_content::setCurrentScene(const std::string &name)
{
    obs_source_t *transition;
    obs_source_t *destination;

    transition = obs_get_output_source(0);
    destination = obs_get_source_by_name(name.c_str());

    currentScene = name;

    obs_transition_start(transition, OBS_TRANSITION_MODE_AUTO, transitionDuration, destination);

    obs_source_release(transition);
    obs_source_release(destination);
}

void OBS_content::OBS_content_renameSourceFilter(const FunctionCallbackInfo<Value>& args)
{
    NODE_OBS_SOURCE(args[0], source);
	NODE_OBS_FILTER(args[1], source, filter);

	v8::String::Utf8Value newName(args[2]->ToString());
	obs_source_set_name(filter, *newName);

    NODE_OBS_SOURCE_RELEASE(source);
    NODE_OBS_FILTER_RELEASE(filter);
}

void OBS_content::OBS_content_renameTransition(const FunctionCallbackInfo<Value>& args)
{
    v8::String::Utf8Value transitionName(args[0]->ToString());
    v8::String::Utf8Value newName(args[1]->ToString());

    auto found = transitions.find(*transitionName);

    if (found == transitions.end()) {
        std::cerr << "Failed to find transition for renaming!" << std::endl;
        return;
    }

    transitions[*newName] = found->second;
    transitions.erase(found);

    if (currentTransition == *transitionName) {
        currentTransition = *newName;
    }
}

void OBS_content::OBS_content_renameScene(const FunctionCallbackInfo<Value>& args)
{
    NODE_OBS_SOURCE(args[0], source);
    v8::String::Utf8Value newName(args[1]->ToString());

    obs_source_set_name(source, *newName);

    if (currentScene == obs_source_get_name(source)) {
        currentScene = *newName;
    }

    NODE_OBS_SOURCE_RELEASE(source);

}

void OBS_content::OBS_content_renameSource(const FunctionCallbackInfo<Value>& args)
{
    NODE_OBS_SOURCE(args[0], source);
    v8::String::Utf8Value newName(args[1]->ToString());

    obs_source_set_name(source, *newName);
    
    NODE_OBS_SOURCE_RELEASE(source);
}

void OBS_content::OBS_content_addSourceFilter(const FunctionCallbackInfo<Value>& args)
{
    v8::String::Utf8Value filterId(args[1]->ToString());
    v8::String::Utf8Value filterName(args[2]->ToString());

    obs_source_t *filter = obs_source_create_private(*filterId, *filterName, nullptr);

    if (!filter) {
        std::cout << "Failed to create filter: " << *filterId << std::endl;
        return;
    }

    NODE_OBS_SOURCE(args[0], source);

    obs_source_filter_add(source, filter);

    NODE_OBS_SOURCE_RELEASE(source);
}

void OBS_content::OBS_content_removeSourceFilter(const FunctionCallbackInfo<Value>& args)
{
	NODE_OBS_SOURCE(args[0], source);
    NODE_OBS_FILTER(args[1], source, filter);

    obs_source_filter_remove(source, filter);
    obs_source_release(filter);

    NODE_OBS_SOURCE_RELEASE(source);
	NODE_OBS_FILTER_RELEASE(filter);
}

void OBS_content::OBS_content_updateSourceFilterProperties(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    NODE_OBS_SOURCE(args[0], source);
	NODE_OBS_FILTER(args[1], source, filter);

    obs_data_t* settings = obs_source_get_settings(filter);
    updateSourceProperties(filter, settings);

    NODE_OBS_SOURCE_RELEASE(source);
    NODE_OBS_FILTER_RELEASE(filter);
}

void OBS_content::OBS_content_getSourceFilterProperties(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    NODE_OBS_SOURCE(args[0], source);
	NODE_OBS_FILTER(args[1], source, filter);

    vector<std::string> listSourceProperties = getSourceProperties(filter);

	Handle<Array> result = Array::New(isolate, (int)listSourceProperties.size()/10);
    for (int i = 0; i < listSourceProperties.size(); i++) {
        Local<Object> obj = Object::New(isolate);

        obj->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, listSourceProperties.at(i).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 1).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "long_description"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 2).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 3).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "visible"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 4).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "enabled"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 5).c_str()));

  		obj->Set(String::NewFromUtf8(isolate, "subType"), String::NewFromUtf8(isolate, listSourceProperties.at(i+6).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "minVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+7).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "maxVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+8).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "stepVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+9).c_str()));

        result->Set(i / 10, obj);
        i += 9;
    }

    args.GetReturnValue().Set(result);

	NODE_OBS_SOURCE_RELEASE(source);
	NODE_OBS_FILTER_RELEASE(filter);
}


void OBS_content::OBS_content_getSourceFilterPropertyCurrentValue(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    NODE_OBS_SOURCE(args[0], source);
	NODE_OBS_FILTER(args[1], source, filter);

    v8::String::Utf8Value propertyName(args[2]->ToString());
    args.GetReturnValue().Set(getSourcePropertyCurrentValue(filter, *propertyName));

    NODE_OBS_SOURCE_RELEASE(source);
    NODE_OBS_FILTER_RELEASE(filter);
}

void OBS_content::OBS_content_setSourceFilterProperty(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    NODE_OBS_SOURCE(args[0], source);
    NODE_OBS_FILTER(args[1], source, filter);
    v8::String::Utf8Value propertyName(args[2]->ToString());

    bool needsToBeRefresh = setProperty(filter, *propertyName, Local<Object>::Cast(args[3]));

    const char* result = needsToBeRefresh ? "true" : "false";
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result));

    NODE_OBS_SOURCE_RELEASE(source);
    NODE_OBS_FILTER_RELEASE(filter);
}

void OBS_content::OBS_content_getSourceFilterPropertiesSubParameters(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    NODE_OBS_SOURCE(args[0], source);
    NODE_OBS_FILTER(args[1], source, filter);
    v8::String::Utf8Value propertyName(args[2]->ToString());

    vector<string> listSourcePropertiesSubParameters =
        getSourcePropertiesSubParameters(filter, *propertyName);

    Handle<Array> result = Array::New(isolate, (int)listSourcePropertiesSubParameters.size() / 2);
    for (int i = 0; i < listSourcePropertiesSubParameters.size(); i++) {
        Local<Object> obj = Object::New(isolate);
        obj->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, listSourcePropertiesSubParameters.at(i).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, listSourcePropertiesSubParameters.at(i + 1).c_str()));
        result->Set(i / 2, obj);
        i++;
    }

    NODE_OBS_SOURCE_RELEASE(source);
    NODE_OBS_FILTER_RELEASE(filter);

    args.GetReturnValue().Set(result);
}

void OBS_content::setTransitionDuration(const uint32_t duration)
{
    transitionDuration = duration;
}

void OBS_content::OBS_content_setTransitionDuration(const FunctionCallbackInfo<Value>& args)
{
    uint32_t duration = args[0]->ToUint32()->Value();

    setTransitionDuration(duration);
}

void OBS_content::OBS_content_getTransitionDuration(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    Local<v8::Integer> result(v8::Integer::New(isolate, transitionDuration));

    args.GetReturnValue().Set(result);
}

void OBS_content::addTransition(const std::string &id, const std::string &name, obs_data_t *settings)
{
    auto found = transitions.find(name);

    /* If found, it's a duplicate */
    if (found != transitions.end()) {
        std::cerr << "Duplicate key provided to addTransition!" << std::endl;
        return;
    }

    obs_source_t *transition = obs_source_create_private(id.c_str(), name.c_str(), settings);

    if (!transition) {
        std::cerr << "Failed to create transition: " << id << std::endl;
        return;
    }

    transitions[name] = transition;
}

void OBS_content::OBS_content_addTransition(const FunctionCallbackInfo<Value>& args)
{
    v8::String::Utf8Value param1(args[0]->ToString());
    v8::String::Utf8Value param2(args[1]->ToString());

    addTransition(*param1, *param2, nullptr);
}

void OBS_content::removeTransition(const std::string &name)
{
    auto found = transitions.find(name);

    if (found == transitions.end()) {
        std::cerr << "Failed to find transition in removeTransition!" << std::endl;
        return;
    }

    obs_source_t *transition = found->second;
    transitions.erase(found);

    obs_source_remove(transition);
    obs_source_release(transition);
}

void OBS_content::OBS_content_removeTransition(const FunctionCallbackInfo<Value>& args)
{
    v8::String::Utf8Value param1(args[0]->ToString());

    removeTransition(*param1);
}

obs_source_t *OBS_content::setTransition(const std::string &name)
{
    auto found = transitions.find(name);

    if (found == transitions.end()) {
        std::cerr << "Failed to find transition!" << std::endl;
        return nullptr;
    }

    obs_source_t *transition = found->second;
    obs_source_t *prev_transition = obs_get_output_source(0);

    obs_set_output_source(0, transition);

    obs_source_t *scene = obs_get_source_by_name(currentScene.c_str());
    obs_transition_set(transition, scene);
    obs_source_release(scene);

    if (prev_transition) {
        obs_source_release(prev_transition); /* obs_get_output_source */
    }

    currentTransition = name;

    return transition;
}

void OBS_content::OBS_content_setTransition(const FunctionCallbackInfo<Value>& args)
{
    v8::String::Utf8Value param1(args[0]->ToString());

    setTransition(*param1);
}

void OBS_content::OBS_content_updateTransitionProperties(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    v8::String::Utf8Value param1(args[0]->ToString());

    auto found = transitions.find(*param1);

    if (found == transitions.end()) {
        std::cerr << "Failed to find filter in addSourceFilter!" << std::endl;
        return;
    }

    obs_source_t *source = found->second;

    obs_data_t* settings = obs_source_get_settings(source);

    updateSourceProperties(source, settings);
}

void OBS_content::OBS_content_getTransitionPropertyCurrentValue(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    v8::String::Utf8Value sourceName(args[0]->ToString());
    v8::String::Utf8Value propertyName(args[1]->ToString());

    auto found = transitions.find(*sourceName);

    if (found == transitions.end()) {
        std::cerr << "Failed to find filter in addSourceFilter!" << std::endl;
        return;
    }

    args.GetReturnValue().Set(getSourcePropertyCurrentValue(found->second, *propertyName));
}

void OBS_content::OBS_content_setTransitionProperty(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    v8::String::Utf8Value sourceName(args[0]->ToString());
    v8::String::Utf8Value propertyName(args[1]->ToString());

    auto found = transitions.find(*sourceName);

    if (found == transitions.end()) {
        std::cerr << "Failed to find filter in addSourceFilter!" << std::endl;
        return;
    }

    bool needsToBeRefresh = setProperty(found->second, *propertyName, Local<Object>::Cast(args[2]));

    const char* result = needsToBeRefresh ? "true" : "false";
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
}

void OBS_content::OBS_content_getTransitionPropertiesSubParameters(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    v8::String::Utf8Value sourceName(args[0]->ToString());

    auto found = transitions.find(*sourceName);

    if (found == transitions.end()) {
        std::cerr << "Failed to find filter in addSourceFilter!" << std::endl;
        return;
    }

    v8::String::Utf8Value propertyName(args[1]->ToString());

    vector<string> listSourcePropertiesSubParameters = 
        getSourcePropertiesSubParameters(found->second, *propertyName);

    Handle<Array> result = Array::New(isolate, (int)listSourcePropertiesSubParameters.size() / 2);
    for (int i = 0; i < listSourcePropertiesSubParameters.size(); i++) {
        Local<Object> obj = Object::New(isolate);
        obj->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, listSourcePropertiesSubParameters.at(i).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "value"), String::NewFromUtf8(isolate, listSourcePropertiesSubParameters.at(i + 1).c_str()));
        result->Set(i / 2, obj);
        i++;
    }

    args.GetReturnValue().Set(result);
}

void OBS_content::OBS_content_getTransitionProperties(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    v8::String::Utf8Value param1(args[0]->ToString());

    auto found = transitions.find(*param1);

    if (found == transitions.end()) {
        std::cerr << "Failed to find filter in addSourceFilter!" << std::endl;
        return;
    }

    obs_source_t *source = found->second;

    vector<std::string> listSourceProperties = getSourceProperties(source);

    Handle<Array> result = Array::New(isolate, (int)listSourceProperties.size() / 10);
    for (int i = 0; i < listSourceProperties.size(); i++) {
        Local<Object> obj = Object::New(isolate);

        obj->Set(String::NewFromUtf8(isolate, "name"), String::NewFromUtf8(isolate, listSourceProperties.at(i).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "description"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 1).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "long_description"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 2).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "type"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 3).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "visible"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 4).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "enabled"), String::NewFromUtf8(isolate, listSourceProperties.at(i + 5).c_str()));
        obj->Set(String::NewFromUtf8(isolate, "currentValue"), getSourcePropertyCurrentValue(source, listSourceProperties.at(i).c_str()));

  		obj->Set(String::NewFromUtf8(isolate, "subType"), String::NewFromUtf8(isolate, listSourceProperties.at(i+6).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "minVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+7).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "maxVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+8).c_str()));
  		obj->Set(String::NewFromUtf8(isolate, "stepVal"), String::NewFromUtf8(isolate, listSourceProperties.at(i+9).c_str()));

        result->Set(i / 10, obj);
        i += 9;
    }

    args.GetReturnValue().Set(result);
}

void OBS_content::setSourcePosition(const std::string &sceneName, const std::string &sourceName, float x, float y)
{
    obs_source_t* sourceScene = obs_get_source_by_name(sceneName.c_str());
    obs_scene_t* scene = obs_scene_from_source(sourceScene);

    obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, sourceName.c_str());

    struct vec2 position;
    position.x = x;
    position.y = y;

    obs_sceneitem_set_pos(sourceItem, &position);

    obs_source_release(sourceScene);
}

void OBS_content::setSourceScaling(const std::string &sceneName, const std::string &sourceName, float x, float y)
{
    obs_source_t* sourceScene = obs_get_source_by_name(sceneName.c_str());
    obs_scene_t* scene = obs_scene_from_source(sourceScene);

    obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, sourceName.c_str());

    struct vec2 scale;
    scale.x = x;
    scale.y = y;
    obs_sceneitem_set_scale(sourceItem, &scale);

    obs_source_release(sourceScene);
}

Local<Object> OBS_content::getSourcePosition(const std::string &sceneName, const std::string &sourceName)
{
    obs_source_t* sourceScene = obs_get_source_by_name(sceneName.c_str());
    obs_scene_t* scene = obs_scene_from_source(sourceScene);

	obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, sourceName.c_str());

	struct vec2 position;
    obs_sceneitem_get_pos(sourceItem, &position);

	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Object> currentPosition = Object::New(isolate);

	currentPosition->Set(String::NewFromUtf8(isolate, "x"), Number::New(isolate, position.x));
	currentPosition->Set(String::NewFromUtf8(isolate, "y"), Number::New(isolate, position.y));

    obs_source_release(sourceScene);

    return currentPosition;
}

Local<Object> OBS_content::getSourceScaling(const std::string &sceneName, const std::string &sourceName)
{
    obs_source_t* sourceScene = obs_get_source_by_name(sceneName.c_str());
    obs_scene_t* scene = obs_scene_from_source(sourceScene);

	obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, sourceName.c_str());

    struct vec2 scale;
    obs_sceneitem_get_scale(sourceItem, &scale);

	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Object> currentScaling = Object::New(isolate);

	currentScaling->Set(String::NewFromUtf8(isolate, "x"), Number::New(isolate, scale.x));
	currentScaling->Set(String::NewFromUtf8(isolate, "y"), Number::New(isolate, scale.y));

    obs_source_release(sourceScene);

    return currentScaling;
}

Local<Object> OBS_content::getSourceSize(const std::string &name)
{
	obs_source_t* source = obs_get_source_by_name(name.c_str());
	int width = obs_source_get_width(source);
	int height = obs_source_get_height(source);

	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Object> currentSize = Object::New(isolate);

	currentSize->Set(String::NewFromUtf8(isolate, "width"), Number::New(isolate, width));
	currentSize->Set(String::NewFromUtf8(isolate, "height"), Number::New(isolate, height));

    obs_source_release(source);

    return currentSize;
}

void OBS_content::setSourceOrder(const std::string &name, const std::string &order)
{
    obs_source_t* sourceScene = obs_get_source_by_name(currentScene.c_str());
    obs_scene_t* scene = obs_scene_from_source(sourceScene);

    obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, name.c_str());

    obs_order_movement movement;

    if (order.compare("move_up") == 0)
    {
        movement = OBS_ORDER_MOVE_UP;
    }
    else if (order.compare("move_down") == 0)
    {
        movement = OBS_ORDER_MOVE_DOWN;
    }
    else if (order.compare("move_top") == 0)
    {
        movement = OBS_ORDER_MOVE_TOP;
    }
    else if (order.compare("move_bottom") == 0)
    {
        movement = OBS_ORDER_MOVE_BOTTOM;
    }

    obs_source_release(sourceScene);
    obs_sceneitem_set_order(sourceItem, movement);
}

void OBS_content::createDisplay(const std::string key, uint64_t windowHandle)
{
    auto found = displays.find(key);

    /* If found, do nothing since it would
       be a memory leak otherwise. */
    if (found != displays.end()) {
        std::cerr << "Duplicate key provided to createDisplay: " << key << std::endl;
        return;
    }

	displays[key] = new OBS::Display(windowHandle);
}

void OBS_content::createSourcePreviewDisplay(const std::string key, uint64_t windowHandle, std::string sourceName)
{
    auto found = displays.find(key);

    /* If found, do nothing since it would
       be a memory leak otherwise. */
    if (found != displays.end()) {
        std::cout << "Duplicate key provided to createDisplay!" << std::endl;
        return;
    }
	displays[key] = new OBS::Display(windowHandle, sourceName);
}

void OBS_content::destroyDisplay(const std::string key)
{
    auto found = displays.find(key);

    if (found == displays.end()) {
        std::cerr << "Failed to find key for destruction: " << key << std::endl;
        return;
    }

    delete found->second;
    displays.erase(found);
}

void OBS_content::selectSource(int x, int y)
{
	obs_source_t* sourceScene = obs_get_source_by_name(currentScene.c_str());
	obs_scene_t* scene = obs_scene_from_source(sourceScene);

	auto function = [] (obs_scene_t *, obs_sceneitem_t *item, void *listSceneItems)
	{
		vector<obs_sceneitem_t* > &items = *reinterpret_cast<vector<obs_sceneitem_t*>*>(listSceneItems);

		items.push_back(item);
		return true;
	};

	vector<obs_sceneitem_t*> listSceneItems;
	obs_scene_enum_items(scene, function, &listSceneItems);

	bool sourceFound = false;
	int index = listSceneItems.size()-1;
	while(!sourceFound && index>=0) {
		obs_sceneitem_t* item = listSceneItems.at(index);
		obs_source_t* source = obs_sceneitem_get_source(item);
		const char* sourceName = obs_source_get_name(source);
		
		struct vec2 position;
		obs_sceneitem_get_pos(item, &position);

		int positionX = position.x;
		int positionY = position.y;

		int width = obs_source_get_width(source);
		int height = obs_source_get_height(source);

		if(x >= positionX && x <= width + positionX &&
			y >= positionY && y < height + positionY) {
			sourceSelected = sourceName;
			sourceFound = true;
		}
		index--;
	}
	if(!sourceFound) {
		sourceSelected = "";
		cout << "source not found !!!!" << endl;
	}

	obs_source_release(sourceScene);
}

bool selectItems(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	vector<std::string> &sources = *reinterpret_cast<vector<std::string>*>(param);

	obs_source_t* source = obs_sceneitem_get_source(item);
	std::string name = obs_source_get_name(source);

	if(std::find(sources.begin(), sources.end(), name) != sources.end()) {
		obs_sceneitem_select(item, true);
	} else {
		obs_sceneitem_select(item, false);
	}
	return true;
}

void OBS_content::selectSources(Local<Array> sources)
{	
	Isolate *isolate = v8::Isolate::GetCurrent();

	obs_source_t *source = obs_get_source_by_name(currentScene.c_str());
    obs_scene_t *scene = obs_scene_from_source(source);

    vector<std::string> tabSources;

    // if(sources->Length() > 0)
    {
	    for(int i=0;i<sources->Length();i++) {
	    	v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(sources->Get(i));

	    	v8::String::Utf8Value value(object->Get(String::NewFromUtf8(isolate, "name")));

			std::string sourceName = std::string(*value);

	    	tabSources.push_back(sourceName);
	    }

	    if (scene) {
	        obs_scene_enum_items(scene, selectItems, &tabSources);
	    }
    }

    obs_source_release(source);
}

void OBS_content::dragSelectedSource(int x, int y)
{
	if(sourceSelected.compare("") !=0) {
		if(x<0) {
			x = 0;
		}
		if(y<0) {
			y = 0;
		}
		obs_source_t* sourceScene = obs_get_source_by_name(currentScene.c_str());
		obs_scene_t* scene = obs_scene_from_source(sourceScene);

		obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, sourceSelected.c_str());

		struct vec2 position;
		obs_sceneitem_get_pos(sourceItem, &position);

		setSourcePosition(currentScene, sourceSelected, x, y);

        obs_source_release(sourceScene);
	}
}

void OBS_content::LoadSceneListOrder(obs_data_array_t *data_array)
{
	size_t num = obs_data_array_count(data_array);

	for (size_t i = 0; i < num; i++) {
		obs_data_t *data = obs_data_array_item(data_array, i);
		const char *name = obs_data_get_string(data, "name");

		if(i==0) {
    		setCurrentScene(name);
		}

		tabScenes.push_back(name);

		obs_data_release(data);
	}
}

void OBS_content::LoadTransitions(obs_data_array_t *data_array)
{
    size_t size = obs_data_array_count(data_array);

    for (int i = 0; i < size; ++i) {
        obs_data_t *data = obs_data_array_item(data_array, i);

        const char *name = obs_data_get_string(data, "name");
        const char *id = obs_data_get_string(data, "id");
        obs_data_t* settings = obs_data_get_obj(data, "settings");

        transitions[name] = obs_source_create_private(id, name, settings);

        obs_data_release(data);
        obs_data_release(settings);
    }
}

void OBS_content::SourceLoaded(void* data, obs_source_t* source) {
    obs_source_addref(source);

	int type = obs_source_get_type(source);

	if (type == OBS_SOURCE_TYPE_INPUT) {
		/* Create Source Info and associate it with the source */
		uint32_t fader = createFader(source);
		uint32_t volmeter = createVolmeter(source);
		SourceInfo * info = new SourceInfo { fader, volmeter };

		sourceInfo[obs_source_get_name(source)] = info;
	}
}

void TransitionLoaded(void *data, obs_source_t *source)
{
	const char *name = obs_source_get_name(source);

	if (transitions.find(name) == transitions.end())
		transitions[name] = source;
}

void OBS_content::loadConfigFile()
{
	std::string contentConfigFile = OBS_API::getContentConfigPath();
	
	//Reading to config scenes file to load the last user's configuration
    obs_data_t *data = obs_data_create_from_json_file_safe(contentConfigFile.c_str(), "bak");

	if (!data) {
		blog(LOG_ERROR, "Failed to load '%s', creating default scene", contentConfigFile.c_str());
        addTransition("cut_transition", "Cut", nullptr);
        addTransition("fade_transition", "Fade", nullptr);
        setTransition("Cut");
		saveIntoConfigFile();
		return;
	}

    //Serializing scenes / sources / transitions from the config file
    obs_data_array_t *sceneOrder = obs_data_get_array(data, "scene_order");
    obs_data_array_t *sources = obs_data_get_array(data, "sources");
    obs_data_array_t *transitions_data = obs_data_get_array(data, "transitions");
    const char *current_transition = obs_data_get_string(data, "current_transition");
    int transition_duration = obs_data_get_int(data, "transition_duration");

    //Loading all the sources that we previously created
    obs_load_sources(sources, SourceLoaded, nullptr);
    //Loading all the transitions that we previously created
    obs_load_sources(transitions_data, TransitionLoaded, nullptr);

    //Loading all the scenes that we previously created
    LoadTransitions(transitions_data);

    auto found_transition = transitions.find(current_transition);

    if (found_transition == transitions.end()) {
        std::cout << "Failed to find previously set transition! Using a default." << std::endl;

        auto fade = transitions.find("Fade");

        if (fade == transitions.end()) {
            std::cout << "Failed to find fade transition... what happened last session...?" << std::endl;
            addTransition("fade_transition", "Fade", nullptr);
        }

        setTransition("Fade");
        currentTransition = "Fade";
    }
    else {
        setTransition(current_transition);
    }

    if (transition_duration)
        transitionDuration = transition_duration;

    LoadSceneListOrder(sceneOrder);
}

obs_data_array_t* OBS_content::saveSceneListOrder(void)
{
	obs_data_array_t *sceneOrder = obs_data_array_create();

	for (int i = 0; i < tabScenes.size(); i++) {
		obs_data_t *data = obs_data_create();
		obs_data_set_string(data, "name", tabScenes.at(i).c_str());
		obs_data_array_push_back(sceneOrder, data);
		obs_data_release(data);
	}

	return sceneOrder;
}

void OBS_content::SaveAudioDevice(const char *name, int channel, obs_data_t *parent, vector<obs_source_t*> &audioSources)
{
	obs_source_t *source = obs_get_output_source(channel);
	if (!source)
		return;

	audioSources.push_back(source);

	obs_data_t *data = obs_save_source(source);

	obs_data_set_obj(parent, name, data);

	obs_data_release(data);
	obs_source_release(source);
}

obs_data_array_t* OBS_content::saveSource(obs_data_t* saveData)
{
	vector<obs_source_t*> audioSources;
	audioSources.reserve(5);

	SaveAudioDevice("DesktopAudioDevice1", 1, saveData, audioSources);
	SaveAudioDevice("DesktopAudioDevice2", 2, saveData, audioSources);
	SaveAudioDevice("AuxAudioDevice1",     3, saveData, audioSources);
	SaveAudioDevice("AuxAudioDevice2",     4, saveData, audioSources);
	SaveAudioDevice("AuxAudioDevice3",     5, saveData, audioSources);


	auto FilterAudioSources = [&](obs_source_t *source)
	{
		return find(begin(audioSources), end(audioSources), source) ==
				end(audioSources);
	};
	using FilterAudioSources_t = decltype(FilterAudioSources);

	obs_data_array_t *sourcesArray = obs_save_sources_filtered(
			[](void *data, obs_source_t *source)
	{
		return (*static_cast<FilterAudioSources_t*>(data))(source);
	}, static_cast<void*>(&FilterAudioSources));

	return sourcesArray;
}

obs_data_array_t *OBS_content::saveTransitions()
{
    obs_data_array_t *data_array = obs_data_array_create();

    for (decltype(transitions)::iterator it = transitions.begin(); 
         it != transitions.end(); it++) 
    {
        obs_data_t *data = obs_save_source(it->second);
        obs_data_array_push_back(data_array, data);
        obs_data_release(data);
    }

    return data_array;
}

void OBS_content::saveIntoConfigFile()
{
	obs_data_t *saveData = obs_data_create();

	obs_data_array_t* sceneOrder = saveSceneListOrder();
	obs_data_array_t* sources = saveSource(saveData);
    obs_data_array_t* transitions_data = saveTransitions();

	obs_data_set_array(saveData, "scene_order", sceneOrder);
	obs_data_set_array(saveData, "sources", sources);
    obs_data_set_array(saveData, "transitions", transitions_data);
    obs_data_set_string(saveData, "current_scene", currentScene.c_str());
    obs_data_set_string(saveData, "current_transition", currentTransition.c_str());
    obs_data_set_int(saveData, "transition_duration", transitionDuration);

    std::string contentConfigFile = OBS_API::getContentConfigPath();

	if (!obs_data_save_json_safe(saveData, contentConfigFile.c_str(), "tmp", "bak")) {
		blog(LOG_ERROR, "Could not save scene data to %s", contentConfigFile.c_str());
	}
}

Local<Object> OBS_content::getSourceFlags(std::string sourceName) 
{
	Isolate *isolate = v8::Isolate::GetCurrent();
	Local<Object> sourceFlags = Object::New(isolate);

	obs_source_t* source = obs_get_source_by_name(sourceName.c_str());

	if(source) {
		uint32_t flags = obs_source_get_output_flags(source);

		bool audio = (flags & OBS_SOURCE_AUDIO) != 0;
		bool video = (flags & OBS_SOURCE_VIDEO) != 0;

		sourceFlags->Set(String::NewFromUtf8(isolate, "audio"), Integer::New(isolate, audio));
		sourceFlags->Set(String::NewFromUtf8(isolate, "video"), Integer::New(isolate, video));	
	} else {
		sourceFlags->Set(String::NewFromUtf8(isolate, "audio"), Integer::New(isolate, false));
		sourceFlags->Set(String::NewFromUtf8(isolate, "video"), Integer::New(isolate, false));	
	}
	obs_source_release(source);
	return sourceFlags;
}

void OBS_content::sourceSetMuted(std::string sourceName, bool muted)
{
	obs_source_t* source = obs_get_source_by_name(sourceName.c_str());
 
	if(source != NULL) {
 		obs_source_set_muted(source, muted);
		obs_source_release(source);
 	}
}
 
bool OBS_content::isSourceMuted(std::string sourceName)
{
 	obs_source_t* source = obs_get_source_by_name(sourceName.c_str());
 
	if(source != NULL) {
		bool isMuted = obs_source_muted(source);
		obs_source_release(source);
		return isMuted;
	} else {
		return false;
 	}		  
} 

bool OBS_content::getSourceVisibility(std::string sceneName, std::string sourceName)
{
    obs_source_t* sourceScene = obs_get_source_by_name(sceneName.c_str());

    if(!sourceScene) {
    	cout << "Failed to load source !" << endl;
    	return 0;
    }
    obs_scene_t* scene = obs_scene_from_source(sourceScene);

	obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, sourceName.c_str());

	bool isVisible = obs_sceneitem_visible(sourceItem);

    obs_source_release(sourceScene);

    return isVisible;
}

void OBS_content::setSourceVisibility(std::string sceneName, std::string sourceName, bool isVisible)
{
    obs_source_t* sourceScene = obs_get_source_by_name(sceneName.c_str());
    obs_scene_t* scene = obs_scene_from_source(sourceScene);

	obs_sceneitem_t* sourceItem = obs_scene_find_source(scene, sourceName.c_str());

	obs_sceneitem_set_visible(sourceItem, isVisible);

    obs_source_release(sourceScene);
}

void OBS_content::fillTabScenes(v8::Local<v8::Array> arrayScenes)
{
	tabScenes.clear();
	for(int i=0;i<arrayScenes->Length();i++) {		
		v8::String::Utf8Value sceneName(arrayScenes->Get(i)->ToString());
		tabScenes.push_back(std::string(*sceneName));
	}
}

void OBS_content::OBS_content_getDrawGuideLines(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();
	
	// Validate Arguments
	/// Amount
	switch (args.Length()) {
	case 0:
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "Usage: OBS_content_getDrawGuideLines(displayKey<string>)")
			)
		);
		return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	args.GetReturnValue().Set(Boolean::New(isolate, it->second->GetDrawGuideLines()));
}

void OBS_content::OBS_content_setDrawGuideLines(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();

	// Validate Arguments
	/// Amount
	switch (args.Length()) {
	case 0:
	case 1:
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "Usage: OBS_content_getDrawGuideLines(displayKey<string>, drawGuideLines<boolean>)")
			)
		);
		return;
	}

	/// Types
	if (args[0]->IsUndefined() && !args[0]->IsString()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not a <string>!")
			)
		);
		return;
	}

	if (args[1]->IsUndefined() && !args[1]->IsBoolean()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{size} is not a <boolean>!")
			)
		);
		return;
	}

	// Find Display
	v8::String::Utf8Value key(args[0]->ToString());
	auto it = displays.find(*key);
	if (it == displays.end()) {
		isolate->ThrowException(
			v8::Exception::SyntaxError(
				v8::String::NewFromUtf8(isolate, "{displayKey} is not valid!")
			)
		);
		return;
	}

	it->second->SetDrawGuideLines(args[1]->BooleanValue());
}
