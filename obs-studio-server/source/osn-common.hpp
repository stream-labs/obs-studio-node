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
#include <ipc-value.hpp>
#include <vector>
#include <obs.h>

namespace osn {
	namespace common {
		// obj -> IPC::Value
		static void OBSDataItemToIPCValue(obs_data_item_t* val, std::vector<IPC::Value>& arr);
		static void OBSDataArrayToIPCValue(obs_data_array_t* val, std::vector<IPC::Value>& arr);
		static void OBSDataToIPCValue(obs_data_t* val, std::vector<IPC::Value>& arr);

		// obj <- IPC::Value
		static void OBSDataItemFromIPCValue(obs_data_t* val, std::vector<IPC::Value>::const_iterator& begin, std::vector<IPC::Value>::const_iterator& end);
		static void OBSDataArrayFromIPCValue(obs_data_array_t* val, std::vector<IPC::Value>::const_iterator& begin, std::vector<IPC::Value>::const_iterator& end);
		static void OBSDataFromIPCValue(obs_data_t** val, std::vector<IPC::Value>::const_iterator& begin, std::vector<IPC::Value>::const_iterator& end);
		static void OBSDataFromIPCValue(obs_data_t** val, std::vector<IPC::Value> const& arr);

	}
}
