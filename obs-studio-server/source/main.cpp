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

#include <chrono>
#include <inttypes.h>
#include <iostream>
#include <ipc-class.hpp>
#include <ipc.hpp>
#include <ipc-function.hpp>
#include <ipc-server.hpp>
#include <memory>
#include <thread>
#include <vector>
#include "error.hpp"
#include "nodeobs_api.h"
#include "nodeobs_autoconfig.h"
#include "nodeobs_content.h"
#include "nodeobs_service.h"
#include "nodeobs_settings.h"
#include "osn-fader.hpp"
#include "osn-filter.hpp"
#include "osn-global.hpp"
#include "osn-input.hpp"
#include "osn-module.hpp"
#include "osn-properties.hpp"
#include "osn-scene.hpp"
#include "osn-sceneitem.hpp"
#include "osn-source.hpp"
#include "osn-transition.hpp"
#include "osn-video.hpp"
#include "osn-volmeter.hpp"
#include "callback-manager.h"
#include "osn-service.hpp"

#include "util-crashmanager.h"
#include "shared.hpp"

#ifndef OSN_VERSION
#define OSN_VERSION "DEVMODE_VERSION"
#endif

#define GET_OSN_VERSION \
[]() { \
const char *__CHECK_EMPTY = OSN_VERSION; \
if (strlen(__CHECK_EMPTY) < 3) { \
    return "DEVMODE_VERSION"; \
}  \
return OSN_VERSION; \
}()


#ifdef __APPLE__
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if defined(_WIN32)
#include "Shlobj.h"

// Checks ForceGPUAsRenderDevice setting
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = [] {
	LPWSTR       roamingPath;
	std::wstring filePath;
	std::string  line;
	std::fstream file;
	bool         settingValue = true; // Default value (NvOptimusEnablement = 1)

	if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roamingPath))) {
		// Couldn't find roaming app data folder path, assume default value
		return settingValue;
	} else {
		filePath.assign(roamingPath);
		filePath.append(L"\\slobs-client\\basic.ini");
		CoTaskMemFree(roamingPath);
	}

	file.open(filePath);

	if (file.is_open()) {
		while (std::getline(file, line)) {
			if (line.find("ForceGPUAsRenderDevice", 0) != std::string::npos) {
				if (line.substr(line.find('=') + 1) == "false") {
					settingValue = false;
					file.close();
					break;
				}
			}
		}
	} else {
		//Couldn't open config file, assume default value
		return settingValue;
	}

	// Return setting value
	return settingValue;
}();
#endif

#define BUFFSIZE 512

struct ServerData
{
	std::mutex                                     mtx;
	std::chrono::high_resolution_clock::time_point last_connect, last_disconnect;
	size_t                                         count_connected = 0;
};

bool ServerConnectHandler(void* data, int64_t)
{
	ServerData*                  sd = reinterpret_cast<ServerData*>(data);
	std::unique_lock<std::mutex> ulock(sd->mtx);
	sd->last_connect = std::chrono::high_resolution_clock::now();
	sd->count_connected++;
	return true;
}

void ServerDisconnectHandler(void* data, int64_t)
{
	ServerData*                  sd = reinterpret_cast<ServerData*>(data);
	std::unique_lock<std::mutex> ulock(sd->mtx);
	sd->last_disconnect = std::chrono::high_resolution_clock::now();
	sd->count_connected--;
}

namespace System
{
	static void
	    Shutdown(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval)
	{
		bool* shutdown = (bool*)data;
		*shutdown      = true;
#ifdef __APPLE__
		g_util_osx->stopApplication();
#endif
		rval.push_back(ipc::value((uint64_t)ErrorCode::Ok));
		return;
	}
} // namespace System

std::atomic_bool run = false;

const std::string a1("streamlabs1");
const std::string b1("logitech1");

static void testThread1()
{
	std::uint64_t k = 0;
	while (!run) {
		++k;
	}

	for (int i = 0; i < 50000; ++i) {
		blog(LOG_INFO, "THREAD1 LOG MESSAGE: %I64u %d %s\n%d %s", k, rand(), a1.data(), rand(), b1.data());
	}
}

const std::string a2("streamlabs2");
const std::string b2("logitech2");

static void testThread2()
{
	std::uint64_t k = 0;
	while (!run) {
		++k;
	}

	for (int i = 0; i < 50000; ++i) {
		blog(LOG_INFO, "THREAD2 LOG MESSAGE: %I64u %d %s\n%d %s", k, rand(), a2.data(), rand(), b2.data());
	}
}

int main(int argc, char* argv[])
{
	auto logParam = std::make_unique<NodeOBSLogParam>();	
	logParam->enableDebugLogs = true;
	logParam->logStream = std::fstream(L"C:\\Temp\\test-log-stab.txt", std::ios_base::out | std::ios_base::trunc);

	base_set_log_handler(node_obs_log, logParam.release());

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::thread thread1(testThread1);
	std::thread thread2(testThread2);

	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	run = true;

	thread1.join();
	thread2.join();

	return 0;
}
