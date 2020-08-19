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
#include "isource.hpp"
#include "utility-v8.hpp"

namespace osn
{
	class Input : public osn::ISource, public utilv8::ManagedObject<osn::Input>
	{
		friend class utilv8::ManagedObject<osn::Input>;

		public:
		Input(uint64_t id);

		// JavaScript
		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(v8::Local<v8::Object> exports);

		// Functions
		static void Types(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Create(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CreatePrivate(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FromName(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetPublicSources(const v8::FunctionCallbackInfo<v8::Value>& args);

		// Methods
		static void Duplicate(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void Active(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Showing(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Width(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Height(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetVolume(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetVolume(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetSyncOffset(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSyncOffset(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetAudioMixers(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetAudioMixers(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetMonitoringType(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetMonitoringType(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetDeinterlaceFieldOrder(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetDeinterlaceFieldOrder(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetDeinterlaceMode(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetDeinterlaceMode(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void Filters(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void AddFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void RemoveFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetFilterOrder(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void FindFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void CopyFilters(const v8::FunctionCallbackInfo<v8::Value>& args);
	};
} // namespace osn
