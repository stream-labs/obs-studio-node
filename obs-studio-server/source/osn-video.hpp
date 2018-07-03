// Client module for the OBS Studio node module.
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
#include "utility.hpp"
#include <ipc-server.hpp>
#include <obs.h>

namespace osn {
	class Video {
	public:
		class Manager : public utility::unique_object_manager<video_t> {
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

	public:
		static void Register(ipc::server&);

		static void GetGlobal(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void GetSkippedFrames(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
		static void GetTotalFrames(void* data, const int64_t id, const std::vector<ipc::value>& args, std::vector<ipc::value>& rval);
	};
}
