#pragma once
#include <io.h>
#include <iostream>
#include <ipc-server.hpp>
#include <math.h>
#include <obs.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "nodeobs_service.h"
#include <ipc-server.hpp>
#include "nodeobs_configManager.hpp"

using namespace std;

extern std::string g_moduleDirectory;

struct Screen
{
	int width;
	int height;
};

class OBS_API
{
	public:
	OBS_API();
	~OBS_API();

	static void Register(ipc::server&);

	static void OBS_API_initAPI(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_API_destroyOBS_API(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void OBS_API_getPerformanceStatistics(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void SetWorkingDirectory(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void StopCrashHandler(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void
	            QueryHotkeys(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void ProcessHotkeyStatus(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);

	private:
	static void initAPI(void);
	static void destroyOBS_API(void);
	static bool openAllModules(int& video_err);

	static double getCPU_Percentage(void);
	static int    getNumberOfDroppedFrames(void);
	static double getDroppedFramesPercentage(void);
	static double getCurrentBandwidth(void);
	static double getCurrentFrameRate(void);

	static std::vector<std::string> exploreDirectory(std::string directory, std::string typeToReturn);

	public:
	static std::string         getPathConfigDirectory(void);
	static void                setPathConfigDirectory(std::string newPathConfigDirectory);
	static std::string         getOBS_currentProfile(void);
	static void                setOBS_currentProfile(std::string profileName);
	static std::string         getOBS_currentSceneCollection(void);
	static void                setOBS_currentSceneCollection(std::string sceneCollectionName);
	static bool                isOBS_configFilesUsed(void);
	static std::vector<Screen> availableResolutions(void);
	static std::string         getModuleDirectory(void);

	static std::string getGlobalConfigPath(void);
	static std::string getBasicConfigPath(void);
	static std::string getServiceConfigPath(void);
	static std::string getContentConfigPath(void);

	static void setAudioDeviceMonitoring(void);

	static void UpdateProcessPriority(void);
	static void SetProcessPriority(const char* priority);
};
