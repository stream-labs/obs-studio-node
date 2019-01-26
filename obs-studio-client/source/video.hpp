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
#include <nan.h>
#include <node.h>
#include "utility-v8.hpp"

namespace osn
{
	class Video : public Nan::ObjectWrap, public utilv8::ManagedObject<osn::Video>
	{
		friend class utilv8::ManagedObject<osn::Video>;

		public:
		Video(uint64_t id)
		{
			this->handler = id;
		};

		public:
		uint64_t                                     handler;

		public:
		Video();

		public:
		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static Nan::NAN_METHOD_RETURN_TYPE GetGlobal(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetSkippedFrames(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetTotalFrames(Nan::NAN_METHOD_ARGS_TYPE info);
	};
} // namespace osn
