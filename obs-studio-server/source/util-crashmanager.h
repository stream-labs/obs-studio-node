/*
 * Modern effects for a modern Streamer
 * Copyright (C) 2017 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once
#include <cstddef>
#include <cstdlib>
#include <string>
#include <vector>
#include <ipc.hpp>
#include <mutex>
#include <queue>
#include <thread>

#undef strtoll
#include "nlohmann/json.hpp"

namespace util
{
	class CrashManager
	{
		public:
        enum OBSLogType
        {
            General,
            Errors,
            Warnings
        };

        class MetricsPipeClient
		{
			const static int StringSize = 64;

			enum class MessageType
			{
				Pid,
				Tag,
				Status,
				Shutdown
			};

			struct MetricsMessage
			{
				MessageType type;
				char        param1[StringSize];
				char        param2[StringSize];
			};

			public:
			~MetricsPipeClient();

			bool CreateClient(std::string name);
			void StartPolling();
			void SendStatus(std::string status);
			void SendTag(std::string tag, std::string value);

			private:
			bool SendPipeMessage(MetricsMessage& message);

			private:
			void*                      m_Pipe;
			bool                       m_PipeIsOpen  = false;
			bool                       m_StopPolling = false;
			std::thread                m_PollingThread;
			std::mutex                 m_PollingMutex;
			std::queue<MetricsMessage> m_AsyncData;
		};

		public:

		~CrashManager();

		bool Initialize();
		void Configure();
		void OpenConsole();

		static void MetricsFileOpen(std::string current_function_class_name, std::string current_version);
		static void IPCValuesToData(const std::vector<ipc::value>&, nlohmann::json&);
		static void AddWarning(const std::string& warning);
		static void AddBreadcrumb(const nlohmann::json& message);
		static void AddBreadcrumb(const std::string& message);
		static void ClearBreadcrumbs();
		static void DisableReports();

		static void ProcessPreServerCall(std::string cname, std::string fname, const std::vector<ipc::value>& args);
		static void ProcessPostServerCall(std::string cname, std::string fname, const std::vector<ipc::value>& args);

		private:
		static nlohmann::json RequestOBSLog(OBSLogType type);
		static nlohmann::json ComputeBreadcrumbs();
		static nlohmann::json ComputeWarnings();
		static bool SetupCrashpad();
		static bool TryHandleCrash(std::string format, std::string crashMessage);
		static void HandleExit() noexcept;
		static void HandleCrash(std::string crashInfo, bool callAbort = true) noexcept;
	};

}; // namespace util
