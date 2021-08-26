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
#include <ipc-server.hpp>
#include <obs.h>
#include "utility-server.hpp"

namespace obs
{
	class Module
	{
		public:
		class Manager : public utility_server::unique_object_manager<obs_module_t>
		{
			friend class std::shared_ptr<Manager>;

			protected:
			Manager() {}
			~Manager() {}

			public:
			Manager(Manager const&) = delete;
			Manager operator=(Manager const&) = delete;

			public:
			static Manager& GetInstance();
		};


		// Functions
		static uint64_t Open(std::string bin_path, std::string data_path);
		static std::vector<std::string> Modules();
		static bool Initialize(uint64_t uid);

		// Methods
		static std::string GetName(uint64_t uid);
		static std::string GetFileName(uint64_t uid);
		static std::string GetAuthor(uint64_t uid);
		static std::string GetDescription(uint64_t uid);
		static std::string GetBinaryPath(uint64_t uid);
		static std::string GetDataPath(uint64_t uid);
	};
}