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
#include <napi.h>
#include "utility-v8.hpp"
// #include "properties.hpp"
// #include "obs-property.hpp"
#include "cache-manager.hpp"

#undef strtoll
#include "nlohmann/json.hpp"

namespace osn
{
	struct SourceHotkeyInfo
	{
		std::string sourceName;
		std::string hotkeyName;
		std::string hotkeyDesc;
		size_t		hotkeyId;
	};

	class ISource: public Napi::ObjectWrap<osn::ISource>
	{
		public:
		uint64_t sourceId;

		public:
		uint64_t m_uid;
	
		public:
		static Napi::FunctionReference constructor;
		static Napi::Object Init(Napi::Env env, Napi::Object exports);
		ISource(const Napi::CallbackInfo& info);

		static void napi_inherits(napi_env env, napi_value ctor, napi_value super_ctor);

		static void Release(const Napi::CallbackInfo& info, uint64_t id);
		static void Remove(const Napi::CallbackInfo& info, uint64_t id);
		static void Update(const Napi::CallbackInfo& info, uint64_t id);
		static void Load(const Napi::CallbackInfo& info, uint64_t id);
		static void Save(const Napi::CallbackInfo& info, uint64_t id);

		Napi::Value IsConfigurable(const Napi::CallbackInfo& info);
		Napi::Value GetProperties(const Napi::CallbackInfo& info);
		Napi::Value GetSettings(const Napi::CallbackInfo& info);

		Napi::Value GetType(const Napi::CallbackInfo& info);
		Napi::Value GetName(const Napi::CallbackInfo& info);
		void SetName(const Napi::CallbackInfo& info, const Napi::Value &value);
		Napi::Value GetOutputFlags(const Napi::CallbackInfo& info);
		Napi::Value GetFlags(const Napi::CallbackInfo& info);
		void SetFlags(const Napi::CallbackInfo& info, const Napi::Value &value);
		Napi::Value GetStatus(const Napi::CallbackInfo& info);
		Napi::Value GetId(const Napi::CallbackInfo& info);
		Napi::Value GetMuted(const Napi::CallbackInfo& info);
		void SetMuted(const Napi::CallbackInfo& info, const Napi::Value &value);
		Napi::Value GetEnabled(const Napi::CallbackInfo& info);
		void SetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value);

		static void SendMouseClick(const Napi::CallbackInfo& info, uint64_t id);
		static void SendMouseMove(const Napi::CallbackInfo& info, uint64_t id);
		static void SendMouseWheel(const Napi::CallbackInfo& info, uint64_t id);
		static void SendFocus(const Napi::CallbackInfo& info, uint64_t id);
		static void SendKeyClick(const Napi::CallbackInfo& info, uint64_t id);

		Napi::Value GetSourceId(const Napi::CallbackInfo& info);
		void SetSourceId(const Napi::CallbackInfo& info, const Napi::Value &value);
	};
}
