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

Nan::Persistent<v8::FunctionTemplate> osn::SceneItem::prototype;

osn::SceneItem::SceneItem(uint64_t id)
{
	this->itemId = id;
}

void osn::SceneItem::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->SetClassName(Nan::New<v8::String>("SceneItem").ToLocalChecked());

	// Prototype/Class Template
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateAccessorProperty(objtemplate, "source", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetSource));
	utilv8::SetTemplateAccessorProperty(objtemplate, "scene", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetScene));
	utilv8::SetTemplateAccessorProperty(objtemplate, "visible", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsVisible), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetVisible));
	utilv8::SetTemplateAccessorProperty(objtemplate, "selected", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsSelected), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetSelected));
	utilv8::SetTemplateAccessorProperty(objtemplate, "streamVisible", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsStreamVisible), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetStreamVisible));
	utilv8::SetTemplateAccessorProperty(objtemplate, "recordingVisible", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsRecordingVisible), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetRecordingVisible));
	utilv8::SetTemplateAccessorProperty(objtemplate, "position", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetPosition), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetPosition));
	utilv8::SetTemplateAccessorProperty(objtemplate, "rotation", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetRotation), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetRotation));
	utilv8::SetTemplateAccessorProperty(objtemplate, "scale", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetScale), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetScale));
	utilv8::SetTemplateAccessorProperty(objtemplate, "alignment", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetAlignment), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetAlignment));
	utilv8::SetTemplateAccessorProperty(objtemplate, "boundsAlignment", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetBoundsAlignment), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetBoundsAlignment));
	utilv8::SetTemplateAccessorProperty(objtemplate, "bounds", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetBounds), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetBounds));
	utilv8::SetTemplateAccessorProperty(objtemplate, "transformInfo", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetTransformInfo), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetTransformInfo));
	utilv8::SetTemplateAccessorProperty(objtemplate, "boundsType", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetBoundsType), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetBoundsType));
	utilv8::SetTemplateAccessorProperty(objtemplate, "crop", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetCrop), v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetCrop));
	utilv8::SetTemplateAccessorProperty(objtemplate, "id", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetId));
	utilv8::SetTemplateField(objtemplate, "moveUp", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), MoveUp));
	utilv8::SetTemplateField(objtemplate, "moveDown", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), MoveDown));
	utilv8::SetTemplateField(objtemplate, "moveTop", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), MoveTop));
	utilv8::SetTemplateField(objtemplate, "moveBottom", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), MoveBottom));
	utilv8::SetTemplateField(objtemplate, "move",v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Move) );
	utilv8::SetTemplateField(objtemplate, "remove", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Remove));
	utilv8::SetTemplateField(objtemplate, "deferUpdateBegin", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), DeferUpdateBegin));
	utilv8::SetTemplateField(objtemplate, "deferUpdateEnd", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), DeferUpdateEnd));

	// Stuff
	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "SceneItem").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::SceneItem::GetSource(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;

	if (!Retrieve(args.This(), item)) {
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
	args.GetReturnValue().Set(osn::Input::Store(obj));
}

void osn::SceneItem::GetScene(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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
	args.GetReturnValue().Set(osn::Scene::Store(obj));
}

void osn::SceneItem::Remove(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

void osn::SceneItem::IsVisible(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->visibleChanged) {
		args.GetReturnValue().Set(sid->isVisible);
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

	args.GetReturnValue().Set(flag);
}

void osn::SceneItem::SetVisible(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool visible;

	ASSERT_GET_VALUE(args[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

void osn::SceneItem::IsSelected(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && sid->cached && !sid->selectedChanged) {
		args.GetReturnValue().Set(utilv8::ToValue(item->IsSelected));
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
	args.GetReturnValue().Set(flag);
}

void osn::SceneItem::SetSelected(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool selected;

	ASSERT_GET_VALUE(args[0], selected);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

void osn::SceneItem::IsStreamVisible(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->streamVisibleChanged) {
		args.GetReturnValue().Set(utilv8::ToValue(item->IsStreamVisible));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "IsStreamVisible", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	bool streamVisible = !!response[1].value_union.ui32;


	if (sid) {
		sid->streamVisibleChanged = false;
		sid->isStreamVisible      = streamVisible;
	}
	args.GetReturnValue().Set(streamVisible);
}

void osn::SceneItem::SetStreamVisible(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool streamVisible;

	ASSERT_GET_VALUE(args[0], streamVisible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid == nullptr) {
		return;
	}

	if (streamVisible == sid->isStreamVisible) {
		sid->streamVisibleChanged = false;
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call(
	    "SceneItem",
	    "SetStreamVisible",
	    std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(streamVisible)});

	sid->streamVisibleChanged = true;
	sid->isStreamVisible      = streamVisible;
}

void osn::SceneItem::IsRecordingVisible(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->recordingVisibleChanged) {
		args.GetReturnValue().Set(utilv8::ToValue(item->IsRecordingVisible));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "SceneItem", "IsRecordingVisible", std::vector<ipc::value>{ipc::value(item->itemId)});

	if (!ValidateResponse(response))
		return;
	bool recordingVisible = !!response[1].value_union.ui32;

	sid->recordingVisibleChanged = false;
	sid->isRecordingVisible      = recordingVisible;
	args.GetReturnValue().Set(recordingVisible);
}

void osn::SceneItem::SetRecordingVisible(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool recordingVisible;

	ASSERT_GET_VALUE(args[0], recordingVisible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid == nullptr) {
		return;
	}

	if (recordingVisible == sid->isRecordingVisible) {
		sid->recordingVisibleChanged = false;
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call(
	    "SceneItem",
	    "SetRecordingVisible",
	    std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(recordingVisible)});

	if (sid) {
		sid->recordingVisibleChanged = true;
		sid->isRecordingVisible      = recordingVisible;
	}
}

void osn::SceneItem::GetPosition(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->posChanged) {
		auto obj = Nan::New<v8::Object>();
		utilv8::SetObjectField(obj, "x", sid->posX);
		utilv8::SetObjectField(obj, "y", sid->posY);
		args.GetReturnValue().Set(obj);
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

	args.GetReturnValue().Set(obj);
}

void osn::SceneItem::SetPosition(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> vector;
	float_t               x;
	float_t               y;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "x", x);
	ASSERT_GET_OBJECT_FIELD(vector, "y", y);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

void osn::SceneItem::GetRotation(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->rotationChanged) {
		args.GetReturnValue().Set(sid->rotation);
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

	args.GetReturnValue().Set(utilv8::ToValue(rotation));
}

void osn::SceneItem::SetRotation(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	float_t vector;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], vector);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

void osn::SceneItem::GetScale(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->scaleChanged) {
		auto obj = Nan::New<v8::Object>();
		utilv8::SetObjectField(obj, "x", sid->scaleX);
		utilv8::SetObjectField(obj, "y", sid->scaleY);
		args.GetReturnValue().Set(obj);
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

	args.GetReturnValue().Set(obj);
}

void osn::SceneItem::SetScale(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> vector;
	float_t               x;
	float_t               y;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "x", x);
	ASSERT_GET_OBJECT_FIELD(vector, "y", y);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

void osn::SceneItem::GetScaleFilter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

	args.GetReturnValue().Set(flag);
}

void osn::SceneItem::SetScaleFilter(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	int32_t visible;

	ASSERT_GET_VALUE(args[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetScaleFilter", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

void osn::SceneItem::GetAlignment(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

	args.GetReturnValue().Set(flag);
}

void osn::SceneItem::SetAlignment(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	uint32_t visible;

	ASSERT_GET_VALUE(args[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetAlignment", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

void osn::SceneItem::GetBounds(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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
	args.GetReturnValue().Set(obj);
}

void osn::SceneItem::SetBounds(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> vector;
	float_t               x;
	float_t               y;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "x", x);
	ASSERT_GET_OBJECT_FIELD(vector, "y", y);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetBounds", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(x), ipc::value(y)});
}

void osn::SceneItem::GetBoundsAlignment(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

	args.GetReturnValue().Set(bounds_alignment);
}

void osn::SceneItem::SetBoundsAlignment(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	uint32_t visible;

	ASSERT_GET_VALUE(args[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetBoundsAlignment", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

void osn::SceneItem::GetBoundsType(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

	args.GetReturnValue().Set(bounds_type);
}

void osn::SceneItem::SetBoundsType(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	int32_t visible;

	ASSERT_GET_VALUE(args[0], visible);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "SetBoundsType", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(visible)});
}

void osn::SceneItem::GetCrop(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(item->itemId);

	if (sid && !sid->cropChanged) {
		auto obj = Nan::New<v8::Object>();
		utilv8::SetObjectField(obj, "left", sid->cropLeft);
		utilv8::SetObjectField(obj, "top", sid->cropTop);
		utilv8::SetObjectField(obj, "right", sid->cropRight);
		utilv8::SetObjectField(obj, "bottom", sid->cropBottom);
		args.GetReturnValue().Set(obj);
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

	args.GetReturnValue().Set(obj);
}

void osn::SceneItem::SetCrop(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> vector;
	int32_t               left;
	int32_t               top;
	int32_t               right;
	int32_t               bottom;

	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], vector);
	ASSERT_GET_OBJECT_FIELD(vector, "left", left);
	ASSERT_GET_OBJECT_FIELD(vector, "top", top);
	ASSERT_GET_OBJECT_FIELD(vector, "right", right);
	ASSERT_GET_OBJECT_FIELD(vector, "bottom", bottom);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

void osn::SceneItem::GetTransformInfo(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

	args.GetReturnValue().Set(obj);
}

void osn::SceneItem::SetTransformInfo(const v8::FunctionCallbackInfo<v8::Value>& args)
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

void osn::SceneItem::GetId(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
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

	args.GetReturnValue().Set(utilv8::ToValue(sid->obs_itemId));
}

void osn::SceneItem::MoveUp(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

    conn->call("SceneItem", "MoveUp", std::vector<ipc::value>{ipc::value(item->itemId)});
}

void osn::SceneItem::MoveDown(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

    conn->call("SceneItem", "MoveDown", std::vector<ipc::value>{ipc::value(item->itemId)});
}

void osn::SceneItem::MoveTop(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "MoveTop", std::vector<ipc::value>{ipc::value(item->itemId)});
}

void osn::SceneItem::MoveBottom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

    conn->call("SceneItem", "MoveBottom", std::vector<ipc::value>{ipc::value(item->itemId)});
}

void osn::SceneItem::Move(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	ASSERT_INFO_LENGTH(args, 1);
	int32_t position;
	ASSERT_GET_VALUE(args[0], position);

	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "Move", std::vector<ipc::value>{ipc::value(item->itemId), ipc::value(position)});
}

void osn::SceneItem::DeferUpdateBegin(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "DeferUpdateBegin", std::vector<ipc::value>{ipc::value(item->itemId)});
}

void osn::SceneItem::DeferUpdateEnd(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::SceneItem* item = nullptr;
	if (!Retrieve(args.This(), item)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("SceneItem", "DeferUpdateEnd", std::vector<ipc::value>{ipc::value(item->itemId)});
}
