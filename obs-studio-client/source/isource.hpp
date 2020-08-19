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
#include <nan.h>
#include <node.h>
#include "utility-v8.hpp"
#include "properties.hpp"
#include "obs-property.hpp"
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

	typedef utilv8::managed_callback<std::shared_ptr<std::vector<SourceHotkeyInfo>>>	SourceCallback;

	class ISource : public Nan::ObjectWrap, public utilv8::InterfaceObject<osn::ISource>
	{
		friend class utilv8::InterfaceObject<osn::ISource>;

		public:
		uint64_t sourceId;

		private:
		uint64_t m_uid;
	
		public:
		~ISource();
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(v8::Local<v8::Object> exports);

		static void Release(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Remove(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void IsConfigurable(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetProperties(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSettings(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Update(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Save(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void GetType(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetName(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetName(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetOutputFlags(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetFlags(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetFlags(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetStatus(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetId(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetMuted(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetMuted(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetEnabled(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetEnabled(const v8::FunctionCallbackInfo<v8::Value>& args);

		// Browser source interaction
		static void SendMouseClick(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SendMouseMove(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SendMouseWheel(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SendFocus(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SendKeyClick(const v8::FunctionCallbackInfo<v8::Value>& args);
	};
} // namespace osn
