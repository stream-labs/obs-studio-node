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

Napi::FunctionReference osn::SceneItem::constructor;

Napi::Object osn::SceneItem::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"SceneItem",
		{
			InstanceAccessor("source", &osn::SceneItem::GetSource, nullptr),
			InstanceAccessor("scene", &osn::SceneItem::GetScene, nullptr),
			InstanceAccessor("visible", &osn::SceneItem::IsVisible, &osn::SceneItem::SetVisible),
			InstanceAccessor("selected", &osn::SceneItem::IsSelected, &osn::SceneItem::SetSelected),
			InstanceAccessor("streamVisible", &osn::SceneItem::IsStreamVisible, &osn::SceneItem::SetStreamVisible),
			InstanceAccessor("recordingVisible", &osn::SceneItem::IsRecordingVisible, &osn::SceneItem::SetRecordingVisible),
			InstanceAccessor("position", &osn::SceneItem::GetPosition, &osn::SceneItem::SetPosition),
			InstanceAccessor("rotation", &osn::SceneItem::GetRotation, &osn::SceneItem::SetRotation),
			InstanceAccessor("scale", &osn::SceneItem::GetScale, &osn::SceneItem::SetScale),
			InstanceAccessor("alignment", &osn::SceneItem::GetAlignment, &osn::SceneItem::SetAlignment),
			InstanceAccessor("boundsAlignment", &osn::SceneItem::GetBoundsAlignment, &osn::SceneItem::SetBoundsAlignment),
			InstanceAccessor("bounds", &osn::SceneItem::GetBounds, &osn::SceneItem::SetBounds),
			InstanceAccessor("transformInfo", &osn::SceneItem::GetTransformInfo, &osn::SceneItem::SetTransformInfo),
			InstanceAccessor("boundsType", &osn::SceneItem::GetBoundsType, &osn::SceneItem::SetBoundsType),
			InstanceAccessor("crop", &osn::SceneItem::GetCrop, &osn::SceneItem::SetCrop),
			InstanceAccessor("scaleFilter", &osn::SceneItem::GetScaleFilter, &osn::SceneItem::SetScaleFilter),
			InstanceAccessor("id", &osn::SceneItem::GetId, nullptr),
			InstanceAccessor("blendingMethod", &osn::SceneItem::GetBlendingMethod, &osn::SceneItem::SetBlendingMethod),
			InstanceAccessor("blendingMode", &osn::SceneItem::GetBlendingMode, &osn::SceneItem::SetBlendingMode),

			InstanceMethod("moveUp", &osn::SceneItem::MoveUp),
			InstanceMethod("moveDown", &osn::SceneItem::MoveDown),
			InstanceMethod("moveTop", &osn::SceneItem::MoveTop),
			InstanceMethod("moveBottom", &osn::SceneItem::MoveBottom),
			InstanceMethod("move", &osn::SceneItem::Move),
			InstanceMethod("remove", &osn::SceneItem::Remove),
			InstanceMethod("deferUpdateBegin", &osn::SceneItem::DeferUpdateBegin),
			InstanceMethod("deferUpdateEnd", &osn::SceneItem::DeferUpdateEnd),
		});
	exports.Set("SceneItem", func);
	osn::SceneItem::constructor = Napi::Persistent(func);
	osn::SceneItem::constructor.SuppressDestruct();
	return exports;
}

osn::SceneItem::SceneItem(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::SceneItem>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();

    if (length <= 0 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
    }

	this->itemId = (uint64_t)info[0].ToNumber().Int64Value();
}

Napi::Value osn::SceneItem::GetSource(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetSource", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	uint64_t sourceId = response[1].value_union.ui64;

    auto instance =
        osn::Input::constructor.New({
            Napi::Number::New(info.Env(), sourceId)
            });

    return instance;
}

Napi::Value osn::SceneItem::GetScene(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetScene", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	uint64_t sourceId = response[1].value_union.ui64;

    auto instance =
        osn::Scene::constructor.New({
            Napi::Number::New(info.Env(), sourceId)
            });

    return instance;
}

Napi::Value osn::SceneItem::Remove(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("SceneItem", "Remove", std::vector<ipc::value>{ipc::value(this->itemId)});

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid) {
		SceneInfo* si = CacheManager<SceneInfo*>::getInstance().Retrieve(sid->scene_id);

		if (si) {
			si->itemsOrderCached = false;
		}
	}
	CacheManager<SceneItemData*>::getInstance().Remove(this->itemId);
	this->itemId = UINT64_MAX;
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::IsVisible(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->visibleChanged) {
		return Napi::Boolean::New(info.Env(), sid->isVisible);
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "IsVisible", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	bool flag = !!response[1].value_union.ui32;

	sid->isVisible      = flag;
	sid->visibleChanged = false;

	return Napi::Boolean::New(info.Env(), flag);
}

void osn::SceneItem::SetVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool visible = value.ToBoolean().Value();
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && visible == sid->isVisible)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetVisible", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(visible)});

	sid->isVisible = visible;
}

Napi::Value osn::SceneItem::IsSelected(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && sid->cached && !sid->selectedChanged) {
		return Napi::Boolean::New(info.Env(), sid->isSelected);
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "IsSelected", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	bool flag = !!response[1].value_union.ui32;

	sid->selectedChanged = false;
	sid->cached          = true;
	sid->isSelected      = flag;
	return Napi::Boolean::New(info.Env(), flag);
}

void osn::SceneItem::SetSelected(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool selected = value.ToBoolean().Value();
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid == nullptr) {
		return;
	}

	if (selected == sid->isSelected) {
		sid->selectedChanged = false;
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetSelected", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(selected)});

	sid->selectedChanged = true;
	sid->cached          = true;
	sid->isSelected      = selected;
}

Napi::Value osn::SceneItem::IsStreamVisible(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->streamVisibleChanged)
		return Napi::Boolean::New(info.Env(), sid->isStreamVisible);

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "IsStreamVisible", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	bool streamVisible = !!response[1].value_union.ui32;


	if (sid) {
		sid->streamVisibleChanged = false;
		sid->isStreamVisible      = streamVisible;
	}
	return Napi::Boolean::New(info.Env(), streamVisible);
}

void osn::SceneItem::SetStreamVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool streamVisible = value.ToBoolean().Value();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid == nullptr) {
		return;
	}

	if (streamVisible == sid->isStreamVisible) {
		sid->streamVisibleChanged = false;
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(
	    "SceneItem",
	    "SetStreamVisible",
	    std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(streamVisible)});

	sid->streamVisibleChanged = true;
	sid->isStreamVisible      = streamVisible;
}

Napi::Value osn::SceneItem::IsRecordingVisible(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->recordingVisibleChanged) {
		return Napi::Boolean::New(info.Env(), sid->isRecordingVisible);
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "SceneItem", "IsRecordingVisible", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	bool recordingVisible = !!response[1].value_union.ui32;

	sid->recordingVisibleChanged = false;
	sid->isRecordingVisible      = recordingVisible;
	return Napi::Boolean::New(info.Env(), recordingVisible);
}

void osn::SceneItem::SetRecordingVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool recordingVisible = value.ToBoolean().Value();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid == nullptr) {
		return;
	}

	if (recordingVisible == sid->isRecordingVisible) {
		sid->recordingVisibleChanged = false;
		return;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call(
	    "SceneItem",
	    "SetRecordingVisible",
	    std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(recordingVisible)});

	if (sid) {
		sid->recordingVisibleChanged = true;
		sid->isRecordingVisible      = recordingVisible;
	}
}

Napi::Value osn::SceneItem::GetPosition(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->posChanged) {
		Napi::Object obj = Napi::Object::New(info.Env());
		obj.Set("x", Napi::Number::New(info.Env(), sid->posX));
		obj.Set("y", Napi::Number::New(info.Env(), sid->posY));
		return obj;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetPosition", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	float x = response[1].value_union.fp32;
	float y = response[2].value_union.fp32;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));

	sid->posX       = x;
	sid->posY       = y;
	sid->posChanged = false;

	return obj;
}

void osn::SceneItem::SetPosition(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && x == sid->posX && y == sid->posY)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetPosition", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(x), ipc::value(y)});

	sid->posX = x;
	sid->posY = y;
}

Napi::Value osn::SceneItem::GetRotation(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->rotationChanged)
		return Napi::Number::New(info.Env(), sid->rotation);

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetRotation", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	float rotation = response[1].value_union.fp32;

	sid->rotation        = rotation;
	sid->rotationChanged = false;

	return Napi::Number::New(info.Env(), rotation);
}

void osn::SceneItem::SetRotation(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	float_t vector = info[0].ToNumber().FloatValue();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && vector == sid->rotation)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetRotation", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(vector)});

	sid->rotation = vector;
}

Napi::Value osn::SceneItem::GetScale(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->scaleChanged) {
		Napi::Object obj = Napi::Object::New(info.Env());
		obj.Set("x", Napi::Number::New(info.Env(), sid->scaleX));
		obj.Set("y", Napi::Number::New(info.Env(), sid->scaleY));
		return obj;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetScale", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	float x = response[1].value_union.fp32;
	float y = response[2].value_union.fp32;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));

	sid->scaleX       = x;
	sid->scaleY       = y;
	sid->scaleChanged = false;

	return obj;
}

void osn::SceneItem::SetScale(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && x == sid->scaleX && y == sid->scaleY)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetScale", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(x), ipc::value(y)});

	sid->scaleX = x;
	sid->scaleY = y;
}

Napi::Value osn::SceneItem::GetScaleFilter(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->scaleFilter)
		return Napi::Number::New(info.Env(), sid->scaleFilter);

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetScaleFilter", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	bool filter = !!response[1].value_union.ui32;

	sid->scaleFilter = filter;
	sid->scaleFilterChanged = false;

	return Napi::Boolean::New(info.Env(), filter);
}

void osn::SceneItem::SetScaleFilter(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int32_t filter = value.ToNumber().Int32Value();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);
	if (sid && sid->scaleFilter == filter)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetScaleFilter", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(filter)});

	sid->scaleFilter = filter;
	sid->scaleFilterChanged = false;
}

Napi::Value osn::SceneItem::GetAlignment(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetAlignment", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	bool flag = !!response[1].value_union.ui32;

	return Napi::Boolean::New(info.Env(), flag);
}

void osn::SceneItem::SetAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t visible = value.ToNumber().Uint32Value();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetAlignment", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(visible)});
}

Napi::Value osn::SceneItem::GetBounds(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetBounds", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	float x = response[1].value_union.fp32;
	float y = response[2].value_union.fp32;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));
	return obj;
}

void osn::SceneItem::SetBounds(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetBounds", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(x), ipc::value(y)});
}

Napi::Value osn::SceneItem::GetBoundsAlignment(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "SceneItem", "GetBoundsAlignment", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	uint32_t bounds_alignment = response[1].value_union.ui32;

	return Napi::Number::New(info.Env(), bounds_alignment);
}

void osn::SceneItem::SetBoundsAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t visible = value.ToNumber().Uint32Value();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetBoundsAlignment", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(visible)});
}

Napi::Value osn::SceneItem::GetBoundsType(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetBoundsType", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	uint32_t bounds_type = response[1].value_union.ui32;

	return Napi::Number::New(info.Env(), bounds_type);
}

void osn::SceneItem::SetBoundsType(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int32_t boundsType = value.ToNumber().Int32Value();

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetBoundsType", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(boundsType)});
}

Napi::Value osn::SceneItem::GetCrop(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->cropChanged) {
		Napi::Object obj = Napi::Object::New(info.Env());
		obj.Set("left", Napi::Number::New(info.Env(), sid->cropLeft));
		obj.Set("top", Napi::Number::New(info.Env(), sid->cropTop));
		obj.Set("right", Napi::Number::New(info.Env(), sid->cropRight));
		obj.Set("bottom", Napi::Number::New(info.Env(), sid->cropBottom));
		return obj;
	}

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
		conn->call_synchronous_helper("SceneItem", "GetCrop", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	uint32_t left   = response[1].value_union.i32;
	uint32_t top    = response[2].value_union.i32;
	uint32_t right  = response[3].value_union.i32;
	uint32_t bottom = response[4].value_union.i32;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("left", Napi::Number::New(info.Env(), left));
	obj.Set("top", Napi::Number::New(info.Env(), top));
	obj.Set("right", Napi::Number::New(info.Env(), right));
	obj.Set("bottom", Napi::Number::New(info.Env(), bottom));

	sid->cropLeft    = left;
	sid->cropTop     = top;
	sid->cropRight   = right;
	sid->cropBottom  = bottom;
	sid->cropChanged = false;

	return obj;
}

void osn::SceneItem::SetCrop(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object vector = info[0].ToObject();
	int32_t left = vector.Get("left").ToNumber().Int32Value();
	int32_t top = vector.Get("top").ToNumber().Int32Value();
	int32_t right = vector.Get("right").ToNumber().Int32Value();
	int32_t bottom = vector.Get("bottom").ToNumber().Int32Value();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && left == sid->cropLeft && top == sid->cropTop &&
		right == sid->cropRight && bottom == sid->cropBottom)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetCrop",
		std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(left),
		ipc::value(top), ipc::value(right), ipc::value(bottom)});

	sid->cropLeft   = left;
	sid->cropTop    = top;
	sid->cropRight  = right;
	sid->cropBottom = bottom;
}

Napi::Value osn::SceneItem::GetTransformInfo(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "SceneItem", "GetTransformInfo", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();

	/* Guess we forgot about alignment, not sure where this goes */
	uint32_t alignment = response[7].value_union.ui32;

	Napi::Object positionObj = Napi::Object::New(info.Env());
	positionObj.Set("x", Napi::Number::New(info.Env(), response[1].value_union.fp32));
	positionObj.Set("y", Napi::Number::New(info.Env(), response[2].value_union.fp32));

	Napi::Object scaleObj = Napi::Object::New(info.Env());
	scaleObj.Set("x", Napi::Number::New(info.Env(), response[3].value_union.fp32));
	scaleObj.Set("y", Napi::Number::New(info.Env(), response[4].value_union.fp32));

	Napi::Object boundsObj = Napi::Object::New(info.Env());
	boundsObj.Set("x", Napi::Number::New(info.Env(), response[8].value_union.fp32));
	boundsObj.Set("y", Napi::Number::New(info.Env(), response[9].value_union.fp32));

	Napi::Object cropObj = Napi::Object::New(info.Env());
	boundsObj.Set("left", Napi::Number::New(info.Env(), response[12].value_union.ui32));
	boundsObj.Set("top", Napi::Number::New(info.Env(), response[13].value_union.ui32));
	boundsObj.Set("right", Napi::Number::New(info.Env(), response[14].value_union.ui32));
	boundsObj.Set("bottom", Napi::Number::New(info.Env(), response[15].value_union.ui32));

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("pos", positionObj);
	obj.Set("scale", scaleObj);
	obj.Set("bounds", boundsObj);
	obj.Set("crop", cropObj);
	obj.Set("scaleFilter", Napi::Number::New(info.Env(), response[5].value_union.ui32));
	obj.Set("rotation", Napi::Number::New(info.Env(), response[6].value_union.fp32));
	obj.Set("boundsType", Napi::Number::New(info.Env(), response[10].value_union.ui32));
	obj.Set("boundsAlignment", Napi::Number::New(info.Env(), response[11].value_union.ui32));

	return obj;
}

void osn::SceneItem::SetTransformInfo(const Napi::CallbackInfo& info, const Napi::Value &value)
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

Napi::Value osn::SceneItem::GetId(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid == nullptr) {
		return info.Env().Undefined();
    }

	if (sid->obs_itemId < 0) {
		auto conn = GetConnection(info);
		if (!conn)
		return info.Env().Undefined();

		std::vector<ipc::value> response =
			conn->call_synchronous_helper("SceneItem", "GetId", std::vector<ipc::value>{ipc::value(this->itemId)});

		if (!ValidateResponse(info, response))
		return info.Env().Undefined();

		sid->obs_itemId = response[1].value_union.ui64;
	}

	return Napi::Number::New(info.Env(), sid->obs_itemId);
}

Napi::Value osn::SceneItem::MoveUp(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

    conn->call("SceneItem", "MoveUp", std::vector<ipc::value>{ipc::value(this->itemId)});
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveDown(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

    conn->call("SceneItem", "MoveDown", std::vector<ipc::value>{ipc::value(this->itemId)});
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveTop(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("SceneItem", "MoveTop", std::vector<ipc::value>{ipc::value(this->itemId)});
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveBottom(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

    conn->call("SceneItem", "MoveBottom", std::vector<ipc::value>{ipc::value(this->itemId)});
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::Move(const Napi::CallbackInfo& info)
{
	int32_t position = info[0].ToNumber().Int32Value();

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("SceneItem", "Move", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(position)});
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateBegin(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("SceneItem", "DeferUpdateBegin", std::vector<ipc::value>{ipc::value(this->itemId)});
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateEnd(const Napi::CallbackInfo& info)
{
	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	conn->call("SceneItem", "DeferUpdateEnd", std::vector<ipc::value>{ipc::value(this->itemId)});
	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::GetBlendingMethod(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->blendingMethodChanged)
		return Napi::Number::New(info.Env(), sid->blendingMethod);

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetBlendingMethod", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	uint32_t mode = !!response[1].value_union.ui32;

	sid->blendingMethod = mode;
	sid->blendingMethodChanged = false;

	return Napi::Number::New(info.Env(), mode);
}

void osn::SceneItem::SetBlendingMethod(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t mode = value.ToNumber().Uint32Value();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);
	if (sid && sid->blendingMethod == mode)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetBlendingMethod", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(mode)});

	sid->blendingMethod = mode;
	sid->blendingMethodChanged = false;
}

Napi::Value osn::SceneItem::GetBlendingMode(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->blendingModeChanged)
		return Napi::Number::New(info.Env(), sid->blendingMode);

	auto conn = GetConnection(info);
	if (!conn)
		return info.Env().Undefined();

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("SceneItem", "GetBlendingMode", std::vector<ipc::value>{ipc::value(this->itemId)});

	if (!ValidateResponse(info, response))
		return info.Env().Undefined();
	uint32_t mode = !!response[1].value_union.ui32;

	sid->blendingMode = mode;
	sid->blendingModeChanged = false;

	return Napi::Number::New(info.Env(), mode);
}

void osn::SceneItem::SetBlendingMode(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t mode = value.ToNumber().Uint32Value();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);
	if (sid && sid->blendingMode == mode)
		return;

	auto conn = GetConnection(info);
	if (!conn)
		return;

	conn->call("SceneItem", "SetBlendingMode", std::vector<ipc::value>{ipc::value(this->itemId), ipc::value(mode)});

	sid->blendingMode = mode;
	sid->blendingModeChanged = false;
}