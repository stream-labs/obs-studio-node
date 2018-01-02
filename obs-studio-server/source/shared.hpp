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

#pragma once
#include <inttypes.h>
#include <vector>
#include <string>
#include <memory>
#include <ipc-value.hpp>
#include <ipc-server.hpp>

namespace OBS {
	class Main {
	#pragma region Singleton
		friend class std::shared_ptr<OBS::Main>;

		public:
		static std::shared_ptr<OBS::Main> GetInstance();

		public: // No Copy
		Main(Main&) = delete;
		Main& operator=(Main&) = delete;

		protected:
		Main();
		~Main();
	#pragma endregion Singleton

		public: // IPC
		static void Register(IPC::Server& server);

		public:
		static bool Initialize(std::string workingDirectory, std::string appDataDirectory);
		static bool Finalize();

		os_cpu_usage_info_t* GetCPUUsageInfo();
		
		private:
		bool m_isInitialized;
		std::string m_workingDirectory;
		std::string m_appDataDirectory;
		std::string m_obsDataPath;
		std::string m_pluginConfigPath;

		// OBS
		os_cpu_usage_info_t* m_cpuUsageInfo;
	};
}
