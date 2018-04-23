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
#include <node.h>
#include <nan.h>

namespace osn {
	class Global {
		public:
		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static Nan::NAN_METHOD_RETURN_TYPE getOutputSource(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE setOutputSource(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE laggedFrames(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE totalFrames(Nan::NAN_METHOD_ARGS_TYPE info);

	};
}