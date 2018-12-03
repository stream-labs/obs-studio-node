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

#ifndef _DEBUG
#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/settings.h"
#endif

namespace util
{
	class CrashManager
	{
		public:

#ifndef _DEBUG
		struct CrashpadInfo
		{
			base::FilePath            handler;
			base::FilePath            db;
			std::string               url;
			std::vector<std::string>  arguments;
			crashpad::CrashpadClient  client;
		};
#else	
		struct CrashpadInfo {};
#endif

		~CrashManager();
		bool Initialize();
		void Configure();
		void OpenConsole();
	};

}; // namespace util
