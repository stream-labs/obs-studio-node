#pragma once
#include <node.h>
#include <v8.h>
#include <obs.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <thread>
#include <mutex>
#ifndef _WIN32
	#include <unistd.h>
#endif
#include <ctime>
#include <fstream>
#include <map>
#include <nan.h>
#include "nodeobs_display.h"
#include "nodeobs_api.h"
using namespace std;
using namespace v8;

struct SourceInfo {
	uint32_t fader;
	uint32_t volmeter;
};

extern map<std::string, SourceInfo*> sourceInfo;
extern vector<std::string> tabScenes;
extern string currentTransition;
extern map<string, obs_source_t*> transitions;

class OBS_content
{
public:
	OBS_content();
	~OBS_content();

	static void OBS_content_getSourceFilterVisibility(const FunctionCallbackInfo<Value> &args);

	static void OBS_content_setSourceFilterVisibility(const FunctionCallbackInfo<Value> &args);

	static void OBS_content_flipHorzSceneItems(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_flipVertSceneItems(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_resetSceneItems(const FunctionCallbackInfo<Value>& args);
	
	static void OBS_content_stretchSceneItems(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_fitSceneItems(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_centerSceneItems(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_getSceneItemRot(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_getSceneItemCrop(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_setSceneItemRot(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_setSceneItemCrop(const FunctionCallbackInfo<Value>& args);
	/**
	 * @return Return a list of strings containing current scenes
	*/	
	static void OBS_content_getListCurrentScenes(const FunctionCallbackInfo<Value>& args);
	
	/**
	 * @param name: String of the name of the scene.
	 
	 * @return Return a list of strings containing current sources from a scene
	*/	
	static void OBS_content_getListCurrentSourcesFromScene(const FunctionCallbackInfo<Value>& args);
	
	/**
	 * @return Return a list of strings containing all available input sources
	*/	
	static void OBS_content_getListInputSources(const FunctionCallbackInfo<Value>& args);
	
	/**
	 * @return Return a list of strings containing all available filters
	*/	
	static void OBS_content_getListFilters(const FunctionCallbackInfo<Value>& args);
	
    /**
     * @return Return a list of strings containing already created transitions.
     */

    static void OBS_content_getListCurrentTransitions(const FunctionCallbackInfo<Value>& args);

	/**
	 * @return Return a list of strings containing all available transitions
	*/	
	static void OBS_content_getListTransitions(const FunctionCallbackInfo<Value>& args);
	
	/**
	 * Create a new scene and add it to the list of current scenes

	 * @param name: String of the name of the scene.
	*/	
	static void OBS_content_createScene(const FunctionCallbackInfo<Value>& args);
	
	/**
	 * Remove a scene from the current context.

	 * @param name: String of the name of the scene.
	*/	
	static void OBS_content_removeScene(const FunctionCallbackInfo<Value>& args);
	

	static void OBS_content_getSourceFader(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_getSourceVolmeter(const FunctionCallbackInfo<Value>& args);
	/**
	 * Add a new source and add it to the list of sources for a specific scene

	 * @param type source: String of the type of the desired source.
	 * @param name: String of the name of the source.
	 * @param settings: Settings to apply to the source.
	 * @param hotkey_data: Hotkeys associated to the source.
	 * @param scene_Name: Name of the scene to add to the source.
	*/	
	static void OBS_content_addSource(const FunctionCallbackInfo<Value>& args);
	
	/**
	 * Remove a scene from the current context.
	 
	 * @param name: String of the name of the source.
	*/	
	static void OBS_content_removeSource(const FunctionCallbackInfo<Value>& args);
	
	/**
	 * Return the frame from a given source.

	 * @param name: String of the name of the source.
	*/	
	static void OBS_content_getSourceFrame(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_getSourceProperties(const FunctionCallbackInfo<Value>& args);
	
	static void OBS_content_getSourcePropertiesSubParameters(const FunctionCallbackInfo<Value>& args);
	
	static void OBS_content_getSourcePropertyCurrentValue(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_setProperty(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_setCurrentScene(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_renameTransition(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_renameSourceFilter(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_renameSource(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_renameScene(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_addSourceFilter(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_removeSourceFilter(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getSourceFilterProperties(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_updateSourceFilterProperties(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getListSourceFilters(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getCurrentTransition(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getSourceFilterPropertyCurrentValue(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_setSourceFilterProperty(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getSourceFilterPropertiesSubParameters(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getTransitionPropertiesSubParameters(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_setTransitionProperty(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_setTransitionDuration(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getTransitionDuration(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_addTransition(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_removeTransition(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_setTransition(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_getTransitionPropertyCurrentValue(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_getTransitionProperties(const FunctionCallbackInfo<Value>& args);

    static void OBS_content_updateTransitionProperties(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_getSourceFrameSettings(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_setSourcePosition(const FunctionCallbackInfo<Value>& args);
	
	static void OBS_content_setSourceScaling(const FunctionCallbackInfo<Value>& args);	

    static void OBS_content_getSourcePosition(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_getSourceScaling(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_getSourceSize(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_setSourceOrder(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_updateSourceProperties(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_createDisplay(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_destroyDisplay(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_getDisplayPreviewOffset(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_getDisplayPreviewSize(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_createSourcePreviewDisplay(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_resizeDisplay(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_moveDisplay(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setPaddingSize(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setPaddingColor(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setBackgroundColor(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setOutlineColor(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setGuidelineColor(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setResizeBoxOuterColor(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setResizeBoxInnerColor(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setShouldDrawUI(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_selectSource(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_selectSources(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_dragSelectedSource(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_loadConfigFile(const FunctionCallbackInfo<Value>& args);
    static void OBS_content_saveIntoConfigFile(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_getSourceFlags(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_sourceSetMuted(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_isSourceMuted(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_getSourceVisibility(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setSourceVisibility(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_fillTabScenes(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_getDrawGuideLines(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_setDrawGuideLines(const FunctionCallbackInfo<Value>& args);

	static void OBS_content_test_getListCurrentScenes(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getListCurrentSourcesFromScene(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getListInputSources(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getListFilters(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getListTransitions(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_createScene(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_removeScene(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_addSource(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_removeSource(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getSourceProperties(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getSourcePropertiesSubParameters(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getSourcePropertyCurrentValue(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getSourcePropertyCurrentValue_boolType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_colorType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_intType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_floatType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_textType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_fontType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_pathType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_buttonType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_editableListType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_listType_intFormat(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getSourcePropertyCurrentValue_listType_floatFormat(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getSourcePropertyCurrentValue_listType_stringFormat(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_getSourcePropertyCurrentValue_frameRateType(const FunctionCallbackInfo<Value>& args);	
	static void OBS_content_test_setProperty(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_getSourceFrameSettings(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_setSourcePosition(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_setSourceScaling(const FunctionCallbackInfo<Value>& args);
	static void OBS_content_test_setSourceOrder(const FunctionCallbackInfo<Value>& args);


    void DisplayCallback(OBS::Display* dp, uint32_t cx, uint32_t cy);

	static vector<std::string> getListCurrentScenes(void);
	static vector<const char*> getListCurrentSourcesFromScene(const std::string &name);
	static vector<const char*> getListInputSources(void);
    static vector<const char*> getListFilterNames(void);
    static vector<const char*> getListFilterTypes(void);
	static vector<const char*> getListTransitionNames(void);
    static vector<const char*> getListTransitionTypes(void);
	static bool createScene(const std::string &name);
	static bool removeScene(const std::string &name);
	static bool addSource(const std::string &typeSource, const std::string &name,
										obs_data_t* settings, obs_data_t *hotkey_data,
										const char* sceneName);
	static bool removeSource(const std::string &name);
	static uint8_t* convert_UYVY422_to_I420(uint8_t* source, int width, int height);
	static uint8_t* convert_YUY2_to_I420(uint8_t* source, int width, int height);
	
	static vector<std::string> getSourceProperties(obs_source_t* source);
	static vector<string> getSourcePropertiesSubParameters(obs_source_t* source, const std::string &propertyNameSelected);
	static Local<Object> getSourcePropertyCurrentValue(obs_source_t* source, const std::string &propertyNameSelected);
	static bool setProperty(obs_source_t* source, const std::string &propertyNameSelected, Local<Object> objectValue);
	static const char* pathTypeToString(obs_path_type type);
	static void setCurrentScene(const std::string &name);
    static obs_source_t *setTransition(const std::string &id, const std::string &name, obs_data_t *settings);
    static void addTransition(const std::string &id, const std::string &name, obs_data_t *settings);
    static void removeTransition(const std::string &name);
    static void setTransitionDuration(const uint32_t duration);
    static obs_source_t* setTransition(const std::string &name);
	static void setSourcePosition(const std::string &sceneName, const std::string &sourceName, float x, float y);
	static void setSourceScaling(const std::string &sceneName, const std::string &sourceName, float x, float y);
	static Local<Object> getSourcePosition(const std::string &sceneName, const std::string &sourceName);
	static Local<Object> getSourceScaling(const std::string &sceneName, const std::string &sourceName);
	static Local<Object> getSourceSize(const std::string &name);

	static void setSourceOrder(const std::string &name, const std::string &order);
    static void updateSourceProperties(obs_source_t* source, obs_data_t* settings);

	static void createDisplay(const std::string key, uint64_t windowHandle);
    static void destroyDisplay(const std::string key);
	static void createSourcePreviewDisplay(const std::string key, uint64_t windowHandle, const std::string sourceName);
	static void selectSource(int x, int y);
	static void selectSources(Local<Array> sources);
	static void dragSelectedSource(int x, int y);
	static void SourceLoaded(void* data, obs_source_t* source);
	static void LoadSceneListOrder(obs_data_array_t *array);
    static void LoadTransitions(obs_data_array_t *);
	static void loadConfigFile();

	static obs_data_array_t* saveSceneListOrder(void);
    static obs_data_array_t* saveTransitions();
	static void SaveAudioDevice(const char *name, int channel, obs_data_t *parent, vector<obs_source_t*> &audioSources);
	static obs_data_array_t* saveSource(obs_data_t* saveData);
	static void saveIntoConfigFile();
	static Local<Object> getSourceFlags(std::string sourceName);
	static void sourceSetMuted(std::string sourceName, bool muted);
	static bool isSourceMuted(std::string sourceName);
	static bool getSourceVisibility(std::string sceneName, std::string sourceName);
	static void setSourceVisibility(std::string sceneName, std::string sourceName, bool isVisible);
	static void fillTabScenes(v8::Local<v8::Array> arrayScenes);
};
