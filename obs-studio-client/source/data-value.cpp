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

#include <iostream>
#include "data-value.hpp"

data::value::value() {
	this->type = type::Null;
}

data::value::value(const std::vector<char>& p_value):type(type::Binary), value_bin(p_value) {
}

data::value::value(const std::string & p_value):type(type::String), value_str(p_value) {
}

data::value::value(uint64_t p_value) {
	this->type = type::UInt64;
	this->value_union.ui64 = p_value;
}

data::value::value(uint32_t p_value) {
	this->type = type::UInt32;
	this->value_union.ui32 = p_value;
}

data::value::value(int64_t p_value) {
	this->type = type::Int64;
	this->value_union.i64 = p_value;
}

data::value::value(int32_t p_value) {
	this->type = type::Int32;
	this->value_union.i32 = p_value;
}

data::value::value(double p_value) {
	this->type = type::Double;
	this->value_union.fp64 = p_value;
}

data::value::value(float p_value) {
	this->type = type::Float;
	this->value_union.fp32 = p_value;
}