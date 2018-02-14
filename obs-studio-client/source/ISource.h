// Client program for the OBS Studio node module.
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
#include "obspp/obspp-source.hpp"
#include "obspp/obspp-weak.hpp"
#include "Common.h"

namespace osn {

	class ISourceHandle {
		public:
		virtual obs::source GetHandle() = 0;
	};

	class ISource : public ISourceHandle, public Nan::ObjectWrap {
		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static obs::source GetHandle(v8::Local<v8::Object> object);

		static NAN_MODULE_INIT(Init);
		static NAN_METHOD(get_type);
		static NAN_METHOD(get_name);
		static NAN_METHOD(set_name);
		static NAN_METHOD(get_outputFlags);
		static NAN_METHOD(get_flags);
		static NAN_METHOD(set_flags);
		static NAN_METHOD(remove);
		static NAN_METHOD(release);
		static NAN_METHOD(save);
		static NAN_METHOD(get_status);
		static NAN_METHOD(get_id);
		static NAN_METHOD(get_configurable);
		static NAN_METHOD(get_properties);
		static NAN_METHOD(get_settings);
		static NAN_METHOD(update);
		static NAN_METHOD(get_muted);
		static NAN_METHOD(set_muted);
		static NAN_METHOD(get_enabled);
		static NAN_METHOD(set_enabled);
	};

}