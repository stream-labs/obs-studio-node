#pragma once
#include <obs.h>
#include <string>
#include <vector>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nodeobs_service.h"
#include <ipc-server.hpp>
#include "nodeobs_configManager.hpp"

using namespace std;

extern std::string g_moduleDirectory;
extern std::string appdata;

struct Screen {
	int width;
	int height;
};

class OBS_API
{
public:
	OBS_API();
	~OBS_API();

	static void Register(ipc::server&);

	static void OBS_API_initAPI(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_API_destroyOBS_API(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void OBS_API_getPerformanceStatistics(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void SetWorkingDirectory(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);

private:
    static void destroyOBS_API(void);
	static void openAllModules(void);

	static double 	getCPU_Percentage(void);
	static int 	 	getNumberOfDroppedFrames(void);
	static double 	getDroppedFramesPercentage(void);
	static double	getCurrentBandwidth(void);
	static double	getCurrentFrameRate(void);
	
public:
	static std::vector<Screen> 	availableResolutions(void); 
	
	static void setAudioDeviceMonitoring(void);

	static void UpdateProcessPriority(void);
	static void SetProcessPriority(const char *priority);
};
