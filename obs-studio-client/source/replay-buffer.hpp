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
#include "worker-signals.hpp"

namespace osn
{
	class ReplayBuffer :
		public WorkerSignals
	{
		public:
		uint64_t uid;
		ReplayBuffer(): WorkerSignals() {};

		protected:
		Napi::Function signalHandler;
		std::string className;

		Napi::Value GetPath(const Napi::CallbackInfo& info);
		void SetPath(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetFormat(const Napi::CallbackInfo& info);
		void SetFormat(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetMuxerSettings(const Napi::CallbackInfo& info);
		void SetMuxerSettings(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetFileFormat(const Napi::CallbackInfo& info);
		void SetFileFormat(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetOverwrite(const Napi::CallbackInfo& info);
		void SetOverwrite(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetNoSpace(const Napi::CallbackInfo& info);
		void SetNoSpace(const Napi::CallbackInfo& info, const Napi::Value& value);

		Napi::Value GetDuration(const Napi::CallbackInfo& info);
		void SetDuration(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetPrefix(const Napi::CallbackInfo& info);
		void SetPrefix(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetSuffix(const Napi::CallbackInfo& info);
		void SetSuffix(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetUsesStream(const Napi::CallbackInfo& info);
		void SetUsesStream(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetVideoEncoder(const Napi::CallbackInfo& info);
		void SetVideoEncoder(const Napi::CallbackInfo& info, const Napi::Value& value);
		Napi::Value GetSignalHandler(const Napi::CallbackInfo& info);
		void SetSignalHandler(const Napi::CallbackInfo& info, const Napi::Value& value);
		
		void Start(const Napi::CallbackInfo& info);
		void Stop(const Napi::CallbackInfo& info);

		void Save(const Napi::CallbackInfo& info);
		Napi::Value GetLastReplay(const Napi::CallbackInfo& info);
	};
}