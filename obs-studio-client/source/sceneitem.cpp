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
#include "server/osn-sceneitem.hpp"

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
			InstanceAccessor("boundsType", &osn::SceneItem::GetBoundsType, &osn::SceneItem::SetBoundsType),
			InstanceAccessor("crop", &osn::SceneItem::GetCrop, &osn::SceneItem::SetCrop),
			InstanceAccessor("scaleFilter", &osn::SceneItem::GetScaleFilter, &osn::SceneItem::SetScaleFilter),
			InstanceAccessor("id", &osn::SceneItem::GetId, nullptr),

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
    auto instance =
        osn::Input::constructor.New({
            Napi::Number::New(info.Env(), obs::SceneItem::GetSource(this->itemId))
            });

    return instance;
}

Napi::Value osn::SceneItem::GetScene(const Napi::CallbackInfo& info)
{
    auto instance =
        osn::Scene::constructor.New({
            Napi::Number::New(info.Env(), obs::SceneItem::GetScene(this->itemId))
            });

    return instance;
}

Napi::Value osn::SceneItem::Remove(const Napi::CallbackInfo& info)
{
	obs::SceneItem::Remove(this->itemId);

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

	bool flag = obs::SceneItem::IsVisible(this->itemId);
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

	obs::SceneItem::SetVisible(this->itemId, visible);
	sid->isVisible = visible;
}

Napi::Value osn::SceneItem::IsSelected(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && sid->cached && !sid->selectedChanged) {
		return Napi::Boolean::New(info.Env(), sid->isSelected);
	}

	bool flag = obs::SceneItem::IsSelected(this->itemId);
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

	obs::SceneItem::SetSelected(this->itemId, selected);
	sid->selectedChanged = true;
	sid->cached          = true;
	sid->isSelected      = selected;
}

Napi::Value osn::SceneItem::IsStreamVisible(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->streamVisibleChanged)
		return Napi::Boolean::New(info.Env(), sid->isStreamVisible);

	bool streamVisible = obs::SceneItem::IsStreamVisible(this->itemId);
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

	obs::SceneItem::SetStreamVisible(this->itemId, streamVisible);
	sid->streamVisibleChanged = true;
	sid->isStreamVisible      = streamVisible;
}

Napi::Value osn::SceneItem::IsRecordingVisible(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->recordingVisibleChanged) {
		return Napi::Boolean::New(info.Env(), sid->isRecordingVisible);
	}

	bool recordingVisible = obs::SceneItem::IsRecordingVisible(this->itemId);
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

	obs::SceneItem::SetRecordingVisible(this->itemId, recordingVisible);
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

	auto res = obs::SceneItem::GetPosition(this->itemId);
	float x = res.first;
	float y = res.second;

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

	obs::SceneItem::SetPosition(this->itemId, x, y);
	sid->posX = x;
	sid->posY = y;
}

Napi::Value osn::SceneItem::GetRotation(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && !sid->rotationChanged)
		return Napi::Number::New(info.Env(), sid->rotation);

	float rotation = obs::SceneItem::GetRotation(this->itemId);
	sid->rotation        = rotation;
	sid->rotationChanged = false;

	return Napi::Number::New(info.Env(), rotation);
}

void osn::SceneItem::SetRotation(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	float_t rotation = info[0].ToNumber().FloatValue();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && rotation == sid->rotation)
		return;

	obs::SceneItem::SetRotation(this->itemId, rotation);
	sid->rotation = rotation;
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

	auto res = obs::SceneItem::GetScale(this->itemId);
	float x = res.first;
	float y = res.second;

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

	obs::SceneItem::SetScale(this->itemId, x, y);
	sid->scaleX = x;
	sid->scaleY = y;
}

Napi::Value osn::SceneItem::GetScaleFilter(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::SceneItem::GetScaleFilter(this->itemId));
}

void osn::SceneItem::SetScaleFilter(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t scaleFilter = value.ToNumber().Uint32Value();

	obs::SceneItem::SetScaleFilter(this->itemId, scaleFilter);
}

Napi::Value osn::SceneItem::GetAlignment(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::SceneItem::GetAlignment(this->itemId));
}

void osn::SceneItem::SetAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t alignment = value.ToNumber().Uint32Value();

	obs::SceneItem::SetAlignment(this->itemId, alignment);
}

Napi::Value osn::SceneItem::GetBounds(const Napi::CallbackInfo& info)
{
	auto res = obs::SceneItem::GetBounds(this->itemId);
	float x = res.first;
	float y = res.second;

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

	obs::SceneItem::SetBounds(this->itemId, x, y);
}

Napi::Value osn::SceneItem::GetBoundsAlignment(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::SceneItem::GetBoundsAlignment(this->itemId));
}

void osn::SceneItem::SetBoundsAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t aligment = value.ToNumber().Uint32Value();

	obs::SceneItem::SetBoundsAlignment(this->itemId, aligment);
}

Napi::Value osn::SceneItem::GetBoundsType(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::SceneItem::GetBoundsType(this->itemId));
}

void osn::SceneItem::SetBoundsType(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int32_t boundsType = value.ToNumber().Int32Value();

	obs::SceneItem::SetBoundsType(this->itemId, boundsType);
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

	auto res = obs::SceneItem::GetCrop(this->itemId);
	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("left", Napi::Number::New(info.Env(), res.left));
	obj.Set("top", Napi::Number::New(info.Env(), res.top));
	obj.Set("right", Napi::Number::New(info.Env(), res.right));
	obj.Set("bottom", Napi::Number::New(info.Env(), res.bottom));

	sid->cropLeft    = res.left;
	sid->cropTop     = res.top;
	sid->cropRight   = res.right;
	sid->cropBottom  = res.bottom;
	sid->cropChanged = false;

	return obj;
}

void osn::SceneItem::SetCrop(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object vector = info[0].ToObject();
	obs_sceneitem_crop crop;
	crop.left = vector.Get("left").ToNumber().Int32Value();
	crop.top = vector.Get("top").ToNumber().Int32Value();
	crop.right = vector.Get("right").ToNumber().Int32Value();
	crop.bottom = vector.Get("bottom").ToNumber().Int32Value();

	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid && crop.left == sid->cropLeft && crop.top == sid->cropTop &&
		crop.right == sid->cropRight && crop.bottom == sid->cropBottom)
		return;

	obs::SceneItem::SetCrop(this->itemId, crop);
	sid->cropLeft   = crop.left;
	sid->cropTop    = crop.top;
	sid->cropRight  = crop.right;
	sid->cropBottom = crop.bottom;
}

Napi::Value osn::SceneItem::GetId(const Napi::CallbackInfo& info)
{
	SceneItemData* sid = CacheManager<SceneItemData*>::getInstance().Retrieve(this->itemId);

	if (sid == nullptr)
		return info.Env().Undefined();

	if (sid->obs_itemId < 0)
		sid->obs_itemId = obs::SceneItem::GetId(this->itemId);

	return Napi::Number::New(info.Env(), sid->obs_itemId);
}

Napi::Value osn::SceneItem::MoveUp(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveUp(this->itemId);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveDown(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveDown(this->itemId);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveTop(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveTop(this->itemId);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveBottom(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveBottom(this->itemId);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::Move(const Napi::CallbackInfo& info)
{
	int32_t position = info[0].ToNumber().Int32Value();

	obs::SceneItem::Move(this->itemId, position);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateBegin(const Napi::CallbackInfo& info)
{
	obs::SceneItem::DeferUpdateBegin(this->itemId);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateEnd(const Napi::CallbackInfo& info)
{
	obs::SceneItem::DeferUpdateEnd(this->itemId);

	return info.Env().Undefined();
}
