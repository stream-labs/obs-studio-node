/******************************************************************************
    Copyright (C) 2016-2022 by Streamlabs (General Workings Inc)

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
#include <napi.h>
#include "worker.hpp"

namespace osn
{
	class SimpleStreaming :
		public Napi::ObjectWrap<osn::SimpleStreaming>,
		public Worker
	{
		public:
		uint64_t uid;		

		private:
		Napi::Function signalHandler;

		public:
		static Napi::FunctionReference constructor;
		static Napi::Object Init(Napi::Env env, Napi::Object exports);
		SimpleStreaming(const Napi::CallbackInfo& info);

		static Napi::Value Create(const Napi::CallbackInfo& info);

		Napi::Value GetService(const Napi::CallbackInfo& info);
		void SetService(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetVideoEncoder(const Napi::CallbackInfo& info);
		void SetVideoEncoder(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetEnforceServiceBirate(const Napi::CallbackInfo& info);
		void SetEnforceServiceBirate(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetEnableTwitchVOD(const Napi::CallbackInfo& info);
		void SetEnableTwitchVOD(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetAudioBitrate(const Napi::CallbackInfo& info);
		void SetAudioBitrate(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetSignalHandler(const Napi::CallbackInfo& info);
		void SetSignalHandler(const Napi::CallbackInfo& info, const Napi::Value& value);
		
		void Start(const Napi::CallbackInfo& info);
		void Stop(const Napi::CallbackInfo& info);

		void worker(void);
	};
}