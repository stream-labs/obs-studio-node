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

#include "obspp/obspp-audio.hpp"

#include "Common.h"
#include "IEncoder.h"

namespace osn {
	class Audio : public Nan::ObjectWrap {
		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		typedef common::Object<Audio, obs::audio> Object;
		friend Object;

		obs::audio handle;

		Audio(audio_t *audio);
		Audio(obs::audio audio);

		static NAN_MODULE_INIT(Init);
		static NAN_METHOD(New);
		static NAN_METHOD(reset);
		static NAN_METHOD(getGlobal);
	};

	class AudioEncoder : public IEncoder {
		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		typedef obs::weak<obs::audio_encoder> weak_handle_t;
		typedef common::Object<AudioEncoder, weak_handle_t> Object;
		friend Object;

		weak_handle_t handle;

		AudioEncoder(std::string id, std::string name, obs_data_t *settings = NULL, size_t idx = 0, obs_data_t *hotkeys = NULL);
		AudioEncoder(obs::audio_encoder encoder);

		virtual obs::encoder GetHandle();
		static NAN_MODULE_INIT(Init);
		static NAN_METHOD(create);
		static NAN_METHOD(getAudio);
		static NAN_METHOD(setAudio);
		static NAN_METHOD(getSampleRate);
	};
}