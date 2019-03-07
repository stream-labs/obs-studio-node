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
		int64_t  obs_itemId = -1;
		bool     isSelected = false;
		bool     selectedChanged = true;
		SceneItem(uint64_t id);

		// JavaScript
		public:
		static Nan::Persistent<v8::FunctionTemplate> prototype;

		static void Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target);

		static Nan::NAN_METHOD_RETURN_TYPE GetSource(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetScene(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Remove(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE IsVisible(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetVisible(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE IsSelected(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetSelected(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetPosition(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetPosition(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetRotation(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetRotation(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetScale(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetScale(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetScaleFilter(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetScaleFilter(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetAlignment(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetAlignment(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetBounds(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetBounds(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetBoundsAlignment(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetBoundsAlignment(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetBoundsType(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetBoundsType(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetCrop(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetCrop(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetTransformInfo(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE SetTransformInfo(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE GetId(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE MoveUp(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE MoveDown(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE MoveTop(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE MoveBottom(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE Move(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE DeferUpdateBegin(Nan::NAN_METHOD_ARGS_TYPE info);
		static Nan::NAN_METHOD_RETURN_TYPE DeferUpdateEnd(Nan::NAN_METHOD_ARGS_TYPE info);
	};
} // namespace osn
