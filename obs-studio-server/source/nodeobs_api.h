/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

******************************************************************************/

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
#include <queue>
#include "nodeobs_configManager.hpp"
#include "nodeobs_service.h"

extern std::string g_moduleDirectory;

struct Screen
{
	int width;
	int height;
};

namespace util
{
	class CrashManager;
}

class OBS_API
{
	friend util::CrashManager;

    public:
    struct LogReport
	{
		static const int MaximumGeneralMessages = 150;

		void push(std::string message, int logLevel)
		{
			general.push(message);
			if (general.size() >= MaximumGeneralMessages) {
				general.pop();
			}

			if (logLevel == LOG_ERROR) {
				errors.push_back(message);
			}

			if (logLevel == LOG_WARNING) {
				warnings.push_back(message);
			}
		}

		std::vector<std::string> errors;
		std::vector<std::string> warnings;
		std::queue<std::string>  general;
	};

    struct OutputStats
    {
        double kbitsPerSec = 0;
        double dataOutput= 0;
        uint64_t lastBytesSent = 0;
        uint64_t lastBytesSentTime = 0;

        void getCurrentStats(bool recording = false);
    };

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
	static void InformCrashHandler(const int crash_id);
	static void
	            QueryHotkeys(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	static void ProcessHotkeyStatus(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);
	static void SetUsername(
	    void*                          data,
	    const int64_t                  id,
	    const std::vector<ipc::value>& args,
	    std::vector<ipc::value>&       rval);

	protected:
	static void initAPI(void);
	static bool openAllModules(int& video_err);

	static double getCPU_Percentage(void);
	static int    getNumberOfDroppedFrames(void);
	static double getDroppedFramesPercentage(void);
	static double getCurrentFrameRate(void);
	static double getAverageTimeToRenderFrame();
	static std::string getDiskSpaceAvailable();
	static double getMemoryUsage();


    static const std::vector<std::string>& getOBSLogErrors();
	static const std::vector<std::string>& getOBSLogWarnings();
	static std::queue<std::string>&        getOBSLogGeneral();

	static std::string getCurrentVersion();
	static std::string getUsername();

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
	static void destroyOBS_API(void);
};

class outdated_driver_error 
{
	static outdated_driver_error * inst;
	std::string line_1 = ""; 
	std::string line_2 = "";
	int lookup_enabled = 0;

public:
	static outdated_driver_error * instance();
	void set_active( bool state); 
	std::string get_error(const char* base_error);

	void catch_error(const char* msg);
};