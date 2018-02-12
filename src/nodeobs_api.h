#pragma once
#include <node.h>
#include <obs.h>
#include <string>
#include <vector>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nodeobs_service.h"



using namespace std;
using namespace v8;

struct Screen {
	int width;
	int height;
};

class OBS_API
{
public:
	OBS_API();
	~OBS_API();

	static void OBS_API_initAPI(const FunctionCallbackInfo<Value> &args);
	/**
	 * Initializes OBS
	 *
	 * @param  locale              The locale to use for modules
	 * @param  module_config_path  Path to module config storage directory
	 *                             (or NULL if none)
	 * @param  store               The profiler name store for OBS to use or NULL
	 */
	static void OBS_API_initOBS_API(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_destroyOBS_API(const FunctionCallbackInfo<Value> &args);

	static void OBS_API_getPerformanceStatistics(const FunctionCallbackInfo<Value> &args);

	static void OBS_API_getPathConfigDirectory(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_setPathConfigDirectory(const FunctionCallbackInfo<Value> &args);

	static void OBS_API_getOBS_existingProfiles(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_getOBS_existingSceneCollections(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_getOBS_currentProfile(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_setOBS_currentProfile(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_getOBS_currentSceneCollection(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_setOBS_currentSceneCollection(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_isOBS_installed(const FunctionCallbackInfo<Value> &args);
	static void OBS_API_useOBS_config(const FunctionCallbackInfo<Value> &args);

private:
	static void initAPI(void);
	static void destroyOBS_API(void);
	static void openAllModules(void);
	static Local<Object> getPerformanceStatistics(void);

	static double 	getCPU_Percentage(void);
	static int 	 	getNumberOfDroppedFrames(void);
	static double 	getDroppedFramesPercentage(void);
	static double	getCurrentBandwidth(void);
	static double	getCurrentFrameRate(void);
	static bool		isOBS_installed(void);

public:
	static std::string 			getPathConfigDirectory(void);
	static void 				setPathConfigDirectory(std::string newPathConfigDirectory);
	static Local<Array> 		getOBS_existingProfiles(void);
	static Local<Array> 		getOBS_existingSceneCollections(void);
	static std::string 			getOBS_currentProfile(void);
	static void 				setOBS_currentProfile(std::string profileName);
	static std::string 			getOBS_currentSceneCollection(void);
	static void 				setOBS_currentSceneCollection(std::string sceneCollectionName);
	static bool 				isOBS_configFilesUsed(void);
	static std::vector<Screen> 	availableResolutions(void);


	static std::string getGlobalConfigPath(void);
	static std::string getBasicConfigPath(void);
	static std::string getServiceConfigPath(void);
	static std::string getContentConfigPath(void);

	static void setAudioDeviceMonitoring(void);

	// Encoders
	static std::string getStreamingEncoderConfigPath(void);
	static std::string getRecordingEncoderConfigPath(void);

	static config_t *openConfigFile(std::string configFile);

	static void UpdateProcessPriority(void);
	static void SetProcessPriority(const char *priority);
};
