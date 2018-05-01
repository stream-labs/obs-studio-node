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
#include <queue>
#include <functional>
#include <inttypes.h>
#include "ipc-value.hpp"
#include <vector>
#include <sstream>

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64)   //WINDOWS
#define __FUNCTION_NAME__   __FUNCTION__
#else          //*NIX
#define __FUNCTION_NAME__   __func__
#endif
#endif

static inline std::string StringFromIPCValueVector(std::vector<ipc::value> const& val) {
	std::stringstream mystream;
	bool prevParam = false;
	for (auto& v : val) {
		if (prevParam) {
			mystream << ", ";
		}
		switch (v.type) {
			case ipc::type::Null:
				mystream << "void";
				break;
			case ipc::type::Float:
				mystream << v.value_union.fp32;
				break;
			case ipc::type::Double:
				mystream << v.value_union.fp64;
				break;
			case ipc::type::Int32:
				mystream << v.value_union.i32;
				break;
			case ipc::type::Int64:
				mystream << v.value_union.i64;
				break;
			case ipc::type::UInt32:
				mystream << v.value_union.ui32;
				break;
			case ipc::type::UInt64:
				mystream << v.value_union.ui64;
				break;
			case ipc::type::String:
				mystream << '"' << v.value_str << '"';
				break;
			case ipc::type::Binary:
				mystream << "binary";
				//mystream << '"' << v.value_str << '"';
				break;
		}
		prevParam = true;
	}

	return mystream.str();
}

#ifndef _DEBUG
#define AUTO_DEBUG
#else
#define AUTO_DEBUG blog(LOG_DEBUG, __FUNCTION_NAME__ "(%s) = %s", StringFromIPCValueVector(args).c_str(), StringFromIPCValueVector(rval).c_str());
#endif
