#pragma once

#include <nan.h>

#include "obspp/obspp-audio.hpp"

#include "Common.h"
#include "IEncoder.h"

namespace osn {
	class Audio : public Nan::ObjectWrap {
		public:
		Audio(); // Global Object

		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;
		typedef common::Object<Audio, obs::audio> Object;
		friend Object;

		public:
		static void Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE);
		static Nan::NAN_METHOD_RETURN_TYPE New(Nan::NAN_METHOD_ARGS_TYPE);
		static Nan::NAN_METHOD_RETURN_TYPE reset(Nan::NAN_METHOD_ARGS_TYPE);
		static Nan::NAN_METHOD_RETURN_TYPE getGlobal(Nan::NAN_METHOD_ARGS_TYPE);

		private:
	};

	class AudioEncoder : public IEncoder {
		public:
		//AudioEncoder(std::string id, std::string name, obs_data_t *settings = NULL, size_t idx = 0, obs_data_t *hotkeys = NULL);
		//AudioEncoder(obs::audio_encoder encoder);

		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		typedef obs::weak<obs::audio_encoder> weak_handle_t;
		typedef common::Object<AudioEncoder, weak_handle_t> Object;
		friend Object;

		weak_handle_t handle;


		virtual obs::encoder GetHandle();
		static NAN_MODULE_INIT(Init);
		static NAN_METHOD(create);
		static NAN_METHOD(fromName);
		static NAN_METHOD(getAudio);
		static NAN_METHOD(setAudio);
		static NAN_METHOD(getSampleRate);
	};
}
