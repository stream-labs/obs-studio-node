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

#undef strtoll
#include "nlohmann/json.hpp"

namespace util
{
	class CrashManager
	{
        enum OBSLogType
        {
            General,
            Errors,
            Warnings
        };

		public:

		bool Initialize();
		void Configure();
		void OpenConsole();

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
