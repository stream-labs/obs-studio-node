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

const std::string a("sdfsdgdgdg4eg423r23r23rgdgd");
const std::string b("ABCDEFGHIJKLMNOP");

static void testThread()
{
	for (int i = 0; i < 1000000; ++i) {
		blog(LOG_INFO, "TEST LOG MESSAGE: %s %d %s", a.data(), rand(), b.data());
	}
}

int main(int argc, char* argv[])
{
	reserveLogMsgStoreSpace();

	for (int i = 0; i < 5; ++i) {
		// OLD
		base_set_log_handler(old_node_obs_log, nullptr);
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
		std::thread(testThread).join();
		std::cout << "old spent: " << (getCounter() / 1000) << " ms" << std::endl;
		resetCounter();
		clearLogMsgStore();
		// NEW
		base_set_log_handler(new_node_obs_log, nullptr);
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
		std::thread(testThread).join();
		std::cout << "new spent: " << (getCounter() / 1000) << " ms" << std::endl;
		resetCounter();
		clearLogMsgStore();
		// LINE BREAK
		std::cout << std::endl;
	}

	return 0;
}
