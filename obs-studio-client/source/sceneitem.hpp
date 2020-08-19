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
	class SceneItem : public Nan::ObjectWrap,
	                  public utilv8::InterfaceObject<osn::SceneItem>,
	                  public utilv8::ManagedObject<osn::SceneItem>
	{
		friend class utilv8::InterfaceObject<osn::SceneItem>;
		friend class utilv8::ManagedObject<osn::SceneItem>;

		public:
		uint64_t itemId;
		SceneItem(uint64_t id);

		// JavaScript
		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(v8::Local<v8::Object> exports);

		static void GetSource(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetScene(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Remove(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void IsVisible(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetVisible(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void IsSelected(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetSelected(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void IsStreamVisible(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetStreamVisible(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void IsRecordingVisible(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetRecordingVisible(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void GetPosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetPosition(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetRotation(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetRotation(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetScale(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetScale(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetScaleFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetScaleFilter(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetAlignment(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetAlignment(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetBounds(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetBounds(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetBoundsAlignment(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetBoundsAlignment(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetBoundsType(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetBoundsType(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetCrop(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetCrop(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetTransformInfo(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void SetTransformInfo(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void GetId(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void MoveUp(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void MoveDown(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void MoveTop(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void MoveBottom(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Move(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void DeferUpdateBegin(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void DeferUpdateEnd(const v8::FunctionCallbackInfo<v8::Value>& args);
	};
} // namespace osn
