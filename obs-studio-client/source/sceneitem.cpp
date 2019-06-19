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

#include <condition_variable>
#include <mutex>
#include <string>

#include "controller.hpp"
#include "error.hpp"
#include "input.hpp"
#include "ipc-value.hpp"
#include "scene.hpp"
#include "sceneitem.hpp"
#include "shared.hpp"
#include "utility.hpp"

osn::SceneItem::SceneItem(uint64_t id)
{
	this->itemId = id;
}

Nan::Persistent<v8::FunctionTemplate> osn::SceneItem::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::SceneItem::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("SceneItem").ToLocalChecked());

	// Prototype/Class Template
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateAccessorProperty(objtemplate, "source", GetSource);
	utilv8::SetTemplateAccessorProperty(objtemplate, "scene", GetScene);
	utilv8::SetTemplateAccessorProperty(objtemplate, "visible", IsVisible, SetVisible);
	utilv8::SetTemplateAccessorProperty(objtemplate, "selected", IsSelected, SetSelected);
	utilv8::SetTemplateAccessorProperty(objtemplate, "position", GetPosition, SetPosition);
	utilv8::SetTemplateAccessorProperty(objtemplate, "rotation", GetRotation, SetRotation);
	utilv8::SetTemplateAccessorProperty(objtemplate, "scale", GetScale, SetScale);
	utilv8::SetTemplateAccessorProperty(objtemplate, "alignment", GetAlignment, SetAlignment);
	utilv8::SetTemplateAccessorProperty(objtemplate, "boundsAlignment", GetBoundsAlignment, SetBoundsAlignment);
	utilv8::SetTemplateAccessorProperty(objtemplate, "bounds", GetBounds, SetBounds);
	utilv8::SetTemplateAccessorProperty(objtemplate, "transformInfo", GetTransformInfo, SetTransformInfo);
	utilv8::SetTemplateAccessorProperty(objtemplate, "boundsType", GetBoundsType, SetBoundsType);
	utilv8::SetTemplateAccessorProperty(objtemplate, "crop", GetCrop, SetCrop);
	utilv8::SetTemplateAccessorProperty(objtemplate, "scaleFilter", GetScaleFilter, SetScaleFilter);
	utilv8::SetTemplateAccessorProperty(objtemplate, "id", GetId);
	utilv8::SetTemplateField(objtemplate, "moveUp", MoveUp);
	utilv8::SetTemplateField(objtemplate, "moveDown", MoveDown);
	utilv8::SetTemplateField(objtemplate, "moveTop", MoveTop);
	utilv8::SetTemplateField(objtemplate, "moveBottom", MoveBottom);
	utilv8::SetTemplateField(objtemplate, "move", Move);
	utilv8::SetTemplateField(objtemplate, "remove", Remove);
	utilv8::SetTemplateField(objtemplate, "deferUpdateBegin", DeferUpdateBegin);
	utilv8::SetTemplateField(objtemplate, "deferUpdateEnd", DeferUpdateEnd);

	// Stuff
	utilv8::SetObjectField(target, "SceneItem", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetSource(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;

	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetSource", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	uint64_t sourceId = response[1].value_union.ui64;

	osn::Input* obj = new osn::Input(sourceId);
	info.GetReturnValue().Set(osn::Input::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetScene(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetScene", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	uint64_t sourceId = response[1].value_union.ui64;

	osn::Scene* obj = new osn::Scene(sourceId);
	info.GetReturnValue().Set(osn::Scene::Store(obj));
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::Remove(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "Remove", std::vector<ipc::value>{ipc::value(item->itemId)});

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid) {
		SceneInfo* si = CacheManager<SceneInfo*>::getInstance().Retrieve(sid->scene_id);

		if (si) {
			si->itemsOrderCached = false;
		}
	}
	CacheManager<SceneItemData*>::getInstance().Remove(item->itemId);
	item->itemId = UINT64_MAX;
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::IsVisible(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->visibleChanged) {
		info.GetReturnValue().Set(sid->isVisible);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "IsVisible", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	bool flag = !!response[1].value_union.ui32;

	sid->isVisible      = flag;
	sid->visibleChanged = false;

	info.GetReturnValue().Set(flag);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetVisible(Nan::NAN_METHOD_ARGS_TYPE info)
{
	bool visible;

	ASSERT_GET_VALUE(info[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && visible == sid->isVisible)
		return;

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetVisible", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});

	sid->isVisible = visible;
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::IsSelected(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && sid->cached && !sid->selectedChanged) {
		info.GetReturnValue().Set(utilv8::ToValue(item->IsSelected));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "IsSelected", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	bool flag = !!response[1].value_union.ui32;

	sid->selectedChanged = false;
	sid->cached          = true;
	sid->isSelected      = flag;
	info.GetReturnValue().Set(flag);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetSelected(Nan::NAN_METHOD_ARGS_TYPE info)
{
	bool selected;

	ASSERT_GET_VALUE(info[0], selected);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid == nullptr) {
		return;
    }

	if (selected == sid->isSelected) {
		sid->selectedChanged = false;
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetSelected", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(selected)});

	sid->selectedChanged = true;
	sid->cached          = true;
	sid->isSelected      = selected;
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetPosition(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->posChanged) {
		auto obj = Nan::New<v8::Object>();
		utilv8::SetObjectField(obj, "x", sid->posX);
		utilv8::SetObjectField(obj, "y", sid->posY);
		info.GetReturnValue().Set(obj);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetPosition", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	float x = response[1].value_union.fp32;
	float y = response[2].value_union.fp32;

	auto obj = Nan::New<v8::Object>();
	utilv8::SetObjectField(obj, "x", x);
	utilv8::SetObjectField(obj, "y", y);

	sid->posX       = x;
	sid->posY       = y;
	sid->posChanged = false;

	info.GetReturnValue().Set(obj);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetPosition(Nan::NAN_METHOD_ARGS_TYPE info)
{
	v8::Local<v8::Object> vector;
	float_t               x;
	float_t               y;

	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "x", x);
	ASSERT_GET_OBJECT_FIELD(vector, "y", y);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && x == sid->posX && y == sid->posY)
		return;

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetPosition", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(x), ipc::value(y)});

	sid->posX = x;
	sid->posY = y;
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetRotation(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->rotationChanged) {
		info.GetReturnValue().Set(sid->rotation);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetRotation", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	float rotation = response[1].value_union.fp32;

	sid->rotation        = rotation;
	sid->rotationChanged = false;

	info.GetReturnValue().Set(utilv8::ToValue(rotation));
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetRotation(Nan::NAN_METHOD_ARGS_TYPE info)
{
	float_t vector;

	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], vector);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && vector == sid->rotation)
		return;

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetRotation", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(vector)});

	sid->rotation = vector;
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetScale(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->scaleChanged) {
		auto obj = Nan::New<v8::Object>();
		utilv8::SetObjectField(obj, "x", sid->scaleX);
		utilv8::SetObjectField(obj, "y", sid->scaleY);
		info.GetReturnValue().Set(obj);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetScale", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	float x = response[1].value_union.fp32;
	float y = response[2].value_union.fp32;

	auto obj = Nan::New<v8::Object>();
	utilv8::SetObjectField(obj, "x", x);
	utilv8::SetObjectField(obj, "y", y);

	sid->scaleX       = x;
	sid->scaleY       = y;
	sid->scaleChanged = false;

	info.GetReturnValue().Set(obj);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetScale(Nan::NAN_METHOD_ARGS_TYPE info)
{
	v8::Local<v8::Object> vector;
	float_t               x;
	float_t               y;

	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "x", x);
	ASSERT_GET_OBJECT_FIELD(vector, "y", y);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && x == sid->scaleX && y == sid->scaleY)
		return;

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetScale", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(x), ipc::value(y)});

	sid->scaleX = x;
	sid->scaleY = y;
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetScaleFilter(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetScaleFilter", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	bool flag = !!response[1].value_union.ui32;

	info.GetReturnValue().Set(flag);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetScaleFilter(Nan::NAN_METHOD_ARGS_TYPE info)
{
	int32_t visible;

	ASSERT_GET_VALUE(info[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetScaleFilter", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetAlignment(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetAlignment", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;

	bool flag = !!response[1].value_union.ui32;

	info.GetReturnValue().Set(flag);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetAlignment(Nan::NAN_METHOD_ARGS_TYPE info)
{
	uint32_t visible;

	ASSERT_GET_VALUE(info[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetAlignment", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetBounds(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetBounds", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	float x = response[1].value_union.fp32;
	float y = response[2].value_union.fp32;

	auto obj = Nan::New<v8::Object>();
	utilv8::SetObjectField(obj, "x", x);
	utilv8::SetObjectField(obj, "y", y);
	info.GetReturnValue().Set(obj);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetBounds(Nan::NAN_METHOD_ARGS_TYPE info)
{
	v8::Local<v8::Object> vector;
	float_t               x;
	float_t               y;

	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "x", x);
	ASSERT_GET_OBJECT_FIELD(vector, "y", y);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetBounds", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(x), ipc::value(y)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetBoundsAlignment(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "SceneItem", "GetBoundsAlignment", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	uint32_t bounds_alignment = response[1].value_union.ui32;

	info.GetReturnValue().Set(bounds_alignment);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetBoundsAlignment(Nan::NAN_METHOD_ARGS_TYPE info)
{
	uint32_t visible;

	ASSERT_GET_VALUE(info[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetBoundsAlignment", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetBoundsType(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetBoundsType", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	uint32_t bounds_type = response[1].value_union.ui32;

	info.GetReturnValue().Set(bounds_type);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetBoundsType(Nan::NAN_METHOD_ARGS_TYPE info)
{
	int32_t visible;

	ASSERT_GET_VALUE(info[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetBoundsType", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetCrop(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->cropChanged) {
		auto obj = Nan::New<v8::Object>();
		utilv8::SetObjectField(obj, "left", sid->cropLeft);
		utilv8::SetObjectField(obj, "top", sid->cropTop);
		utilv8::SetObjectField(obj, "right", sid->cropRight);
		utilv8::SetObjectField(obj, "bottom", sid->cropBottom);
		info.GetReturnValue().Set(obj);
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetCrop", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;

	uint32_t left   = response[1].value_union.i32;
	uint32_t top    = response[2].value_union.i32;
	uint32_t right  = response[3].value_union.i32;
	uint32_t bottom = response[4].value_union.i32;

	auto obj = Nan::New<v8::Object>();
	utilv8::SetObjectField(obj, "left", left);
	utilv8::SetObjectField(obj, "top", top);
	utilv8::SetObjectField(obj, "right", right);
	utilv8::SetObjectField(obj, "bottom", bottom);

	sid->cropLeft    = left;
	sid->cropTop     = top;
	sid->cropRight   = right;
	sid->cropBottom  = bottom;
	sid->cropChanged = false;

	info.GetReturnValue().Set(obj);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetCrop(Nan::NAN_METHOD_ARGS_TYPE info)
{
	v8::Local<v8::Object> vector;
	int32_t               left;
	int32_t               top;
	int32_t               right;
	int32_t               bottom;

	ASSERT_INFO_LENGTH(info, 1);
	ASSERT_GET_VALUE(info[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "left", left);
	ASSERT_GET_OBJECT_FIELD(vector, "top", top);
	ASSERT_GET_OBJECT_FIELD(vector, "right", right);
	ASSERT_GET_OBJECT_FIELD(vector, "bottom", bottom);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && left == sid->cropLeft && top == sid->cropTop &&
		right == sid->cropRight && bottom == sid->cropBottom)
		return;

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetCrop",
		std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(left),
		ipc::value(top), ipc::value(right), ipc::value(bottom)});

	sid->cropLeft   = left;
	sid->cropTop    = top;
	sid->cropRight  = right;
	sid->cropBottom = bottom;
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetTransformInfo(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "SceneItem", "GetTransformInfo", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;

	/* Guess we forgot about alignment, not sure where this goes */
	uint32_t alignment = response[7].value_union.ui32;

	auto positionObj = Nan::New<v8::Object>();
	utilv8::SetObjectField(positionObj, "x", response[1].value_union.fp32);
	utilv8::SetObjectField(positionObj, "y", response[2].value_union.fp32);

	auto scaleObj = Nan::New<v8::Object>();
	utilv8::SetObjectField(scaleObj, "x", response[3].value_union.fp32);
	utilv8::SetObjectField(scaleObj, "y", response[4].value_union.fp32);

	auto boundsObj = Nan::New<v8::Object>();
	utilv8::SetObjectField(boundsObj, "x", response[8].value_union.fp32);
	utilv8::SetObjectField(boundsObj, "y", response[9].value_union.fp32);

	auto cropObj = Nan::New<v8::Object>();
	utilv8::SetObjectField(cropObj, "left", response[12].value_union.ui32);
	utilv8::SetObjectField(cropObj, "top", response[13].value_union.ui32);
	utilv8::SetObjectField(cropObj, "right", response[14].value_union.ui32);
	utilv8::SetObjectField(cropObj, "bottom", response[15].value_union.ui32);

	auto obj = Nan::New<v8::Object>();
	utilv8::SetObjectField(obj, "pos", positionObj);
	utilv8::SetObjectField(obj, "scale", scaleObj);
	utilv8::SetObjectField(obj, "bounds", boundsObj);
	utilv8::SetObjectField(obj, "crop", cropObj);
	utilv8::SetObjectField(obj, "scaleFilter", response[5].value_union.ui32);
	utilv8::SetObjectField(obj, "rotation", response[6].value_union.fp32);
	utilv8::SetObjectField(obj, "boundsType", response[10].value_union.ui32);
	utilv8::SetObjectField(obj, "boundsAlignment", response[11].value_union.ui32);

	info.GetReturnValue().Set(obj);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::SetTransformInfo(Nan::NAN_METHOD_ARGS_TYPE info)
{
	//obs::scene::item &handle = SceneItem::Object::GetHandle(info.Holder());

	//v8::Local<v8::Object> tf_info_object;

	//ASSERT_GET_VALUE(info[0], tf_info_object);

	//obs_transform_info tf_info;

	//ASSERT_GET_OBJECT_FIELD(tf_info_object, "pos", tf_info.pos);
	//ASSERT_GET_OBJECT_FIELD(tf_info_object, "rot", tf_info.rot);
	//ASSERT_GET_OBJECT_FIELD(tf_info_object, "scale", tf_info.scale);
	//ASSERT_GET_OBJECT_FIELD(tf_info_object, "alignment", tf_info.alignment);
	//ASSERT_GET_OBJECT_FIELD(tf_info_object, "boundsType", tf_info.bounds_type);
	//ASSERT_GET_OBJECT_FIELD(tf_info_object, "boundsAlignment", tf_info.bounds_alignment);
	//ASSERT_GET_OBJECT_FIELD(tf_info_object, "bounds", tf_info.bounds);

	//handle.transform_info(tf_info);
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::GetId(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid == nullptr) {
		return;
    }

	if (sid->obs_itemId < 0) {
		auto conn = GetConnection();
		if (!conn)
			return;

		std::vector<ipc::value> response =
			conn->call_synchronous_helper("SceneItem", "GetId", std::vector<ipc::value>{ipc::value(item->itemId)});

		if (!ValidateResponse(response))
			return;

		sid->obs_itemId = response[1].value_union.ui64;
	}

	info.GetReturnValue().Set(utilv8::ToValue(sid->obs_itemId));
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::MoveUp(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

    conn->call("SceneItem", "MoveUp", std::vector<ipc::value>{ipc::value(item->itemId)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::MoveDown(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

    conn->call("SceneItem", "MoveDown", std::vector<ipc::value>{ipc::value(item->itemId)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::MoveTop(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "MoveTop", std::vector<ipc::value>{ipc::value(item->itemId)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::MoveBottom(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

    conn->call("SceneItem", "MoveBottom", std::vector<ipc::value>{ipc::value(item->itemId)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::Move(Nan::NAN_METHOD_ARGS_TYPE info)
{
	ASSERT_INFO_LENGTH(info, 1);
	int32_t position;
	ASSERT_GET_VALUE(info[0], position);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "Move", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(position)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::DeferUpdateBegin(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "DeferUpdateBegin", std::vector<ipc::value>{ipc::value(item->itemId)});
}

Nan::NAN_METHOD_RETURN_TYPE osn::SceneItem::DeferUpdateEnd(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(info.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "DeferUpdateEnd", std::vector<ipc::value>{ipc::value(item->itemId)});
}
