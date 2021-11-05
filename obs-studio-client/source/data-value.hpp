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
#include <inttypes.h>
#include <string>
#include <vector>

typedef float float_t;
typedef double double_t; 

namespace data {
	enum class type : uint32_t{
		Null,
		Float,
		Double,
		Int32,
		Int64,
		UInt32,
		UInt64,
		String,
		Binary,
	};

	struct value {
		type type;
		union {
			float fp32;
			double fp64;
			int32_t i32;
			int64_t i64;
			uint32_t ui32;
			uint64_t ui64;
		} value_union;
		std::string value_str;
		std::vector<char> value_bin;

		value();
		value(float);
		value(double);
		value(int32_t);
		value(int64_t);
		value(uint32_t);
		value(uint64_t);
		value(const std::string & p_value);
		value(const std::vector<char>& p_value);
	};
}