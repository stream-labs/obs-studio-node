// Server program for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "obs-main.hpp"
#include <ipc-class.hpp>
// OBS
#include "obs.h"
#include "util/platform.h"

#pragma region Singleton
std::shared_ptr<OBS::Main> OBS::Main::GetInstance() {
	static std::shared_ptr<OBS::Main> inst;
	static std::mutex inst_mutex;
	if (inst == nullptr) {
		std::unique_lock<std::mutex> ulock(inst_mutex);
		if (inst == nullptr) {
			inst = std::make_shared<OBS::Main>();
		}
	}
	return inst;
}

OBS::Main::Main() {}

OBS::Main::~Main() {}
#pragma endregion Singleton

void OBS::Main::Register(IPC::Server& server) {
	IPC::Class cls("OBS");
	cls.RegisterFunction(std::make_shared<IPC::Function>("Initialize", std::vector<IPC::Type>{IPC::Type::String},
		[](int64_t, void*, std::vector<IPC::Value> vals) {
		return IPC::Value(OBS::Main::GetInstance()->Initialize(vals[0].value_str, ""));
	}, nullptr));
	cls.RegisterFunction(std::make_shared<IPC::Function>("Initialize", std::vector<IPC::Type>{IPC::Type::String, IPC::Type::String},
		[](int64_t, void*, std::vector<IPC::Value> vals) {
		return IPC::Value(OBS::Main::GetInstance()->Initialize(vals[0].value_str, vals[1].value_str));
	}, nullptr));
	cls.RegisterFunction(std::make_shared<IPC::Function>("Initialize",
		[](int64_t, void*, std::vector<IPC::Value> vals) {
		return IPC::Value(OBS::Main::GetInstance()->Finalize());
	}, nullptr));
	server.RegisterClass(cls);
}

bool OBS::Main::Initialize(std::string workingDirectory, std::string appDataDirectory) {
	auto obs = OBS::Main::GetInstance();
	if (obs_initialized())
		return false;

	obs->m_workingDirectory = workingDirectory;
	if (appDataDirectory.length() == 0) {
		char* tmp;
		obs->m_appDataDirectory = tmp = os_get_config_path_ptr("slobs-client");
		bfree(tmp);
		obs->m_obsDataPath = tmp = os_get_config_path_ptr("slobs-client/node-obs");
		bfree(tmp);
		obs->m_pluginConfigPath = tmp = os_get_config_path_ptr("slobs-client/plugin_config");
		bfree(tmp);
	} else {
		obs->m_appDataDirectory = appDataDirectory;
		obs->m_obsDataPath = obs->m_appDataDirectory + "/node-obs";
		obs->m_pluginConfigPath = obs->m_appDataDirectory + "/plugin_config";
	}

	// Initialize OBS.
	if (!obs_startup("en-US", obs->m_pluginConfigPath.c_str(), NULL)) {
		return false;
	}

	// CPU Usage Info
	obs->m_cpuUsageInfo = os_cpu_usage_info_start();


	obs->m_isInitialized = obs_initialized();
	return obs->m_isInitialized;
}

bool OBS::Main::Finalize() {
	auto obs = OBS::Main::GetInstance();
	if (!obs->m_isInitialized)
		return false;

	// CPU Usage Info
	os_cpu_usage_info_destroy(obs->m_cpuUsageInfo);
	obs->m_cpuUsageInfo = nullptr;

	// OBS
	obs_shutdown();

	obs->m_isInitialized = obs_initialized();
	return !obs->m_isInitialized;
}
