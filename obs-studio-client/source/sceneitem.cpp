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

#include "input.hpp"
#include "scene.hpp"
#include "sceneitem.hpp"
#include "shared.hpp"
#include "utility.hpp"

Napi::FunctionReference osn::SceneItem::constructor;
std::map<uint32_t, obs_sceneitem_t*> sceneItems;
uint32_t idSceneItemsCount = 0;

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

    if (length <= 0) {
        Napi::TypeError::New(env, "Too few arguments.").ThrowAsJavaScriptException();
        return;
    }

	auto externalItem = info[0].As<Napi::External<obs_sceneitem_t*>>();
	auto item = *externalItem.Data();
	this->id = idSceneItemsCount++;
	sceneItems.insert_or_assign(this->id, item);
}

Napi::Value osn::SceneItem::GetSource(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	auto source = obs::SceneItem::GetSource(item);

    auto instance =
        osn::Input::constructor.New({
			Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::SceneItem::GetScene(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	auto source = obs::SceneItem::GetScene(item);
    auto instance =
        osn::Scene::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::SceneItem::Remove(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	obs::SceneItem::Remove(item);

	sceneItems.erase(this->id);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::IsVisible(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), obs::SceneItem::IsVisible(item));
}

void osn::SceneItem::SetVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	bool visible = value.ToBoolean().Value();
	obs::SceneItem::SetVisible(item, visible);
}

Napi::Value osn::SceneItem::IsSelected(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), obs::SceneItem::IsSelected(item));
}

void osn::SceneItem::SetSelected(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	bool selected = value.ToBoolean().Value();
	obs::SceneItem::SetSelected(item, selected);
}

Napi::Value osn::SceneItem::IsStreamVisible(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), obs::SceneItem::IsStreamVisible(item));
}

void osn::SceneItem::SetStreamVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	bool streamVisible = value.ToBoolean().Value();

	obs::SceneItem::SetStreamVisible(item, streamVisible);
}

Napi::Value osn::SceneItem::IsRecordingVisible(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	bool recordingVisible = obs::SceneItem::IsRecordingVisible(item);

	return Napi::Boolean::New(info.Env(), recordingVisible);
}

void osn::SceneItem::SetRecordingVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	bool recordingVisible = value.ToBoolean().Value();

	obs::SceneItem::SetRecordingVisible(item, recordingVisible);
}

Napi::Value osn::SceneItem::GetPosition(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	auto res = obs::SceneItem::GetPosition(item);
	float x = res.first;
	float y = res.second;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));

	return obj;
}

void osn::SceneItem::SetPosition(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	obs::SceneItem::SetPosition(item, x, y);
}

Napi::Value osn::SceneItem::GetRotation(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::SceneItem::GetRotation(item));
}

void osn::SceneItem::SetRotation(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	float_t rotation = info[0].ToNumber().FloatValue();

	obs::SceneItem::SetRotation(item, rotation);
}

Napi::Value osn::SceneItem::GetScale(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	auto res = obs::SceneItem::GetScale(item);
	float x = res.first;
	float y = res.second;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));

	return obj;
}

void osn::SceneItem::SetScale(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	obs::SceneItem::SetScale(item, x, y);
}

Napi::Value osn::SceneItem::GetScaleFilter(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::SceneItem::GetScaleFilter(item));
}

void osn::SceneItem::SetScaleFilter(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	uint32_t scaleFilter = value.ToNumber().Uint32Value();

	obs::SceneItem::SetScaleFilter(item, scaleFilter);
}

Napi::Value osn::SceneItem::GetAlignment(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), obs::SceneItem::GetAlignment(item));
}

void osn::SceneItem::SetAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	uint32_t alignment = value.ToNumber().Uint32Value();

	obs::SceneItem::SetAlignment(item, alignment);
}

Napi::Value osn::SceneItem::GetBounds(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	auto res = obs::SceneItem::GetBounds(item);
	float x = res.first;
	float y = res.second;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));
	return obj;
}

void osn::SceneItem::SetBounds(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	obs::SceneItem::SetBounds(item, x, y);
}

Napi::Value osn::SceneItem::GetBoundsAlignment(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::SceneItem::GetBoundsAlignment(item));
}

void osn::SceneItem::SetBoundsAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	uint32_t aligment = value.ToNumber().Uint32Value();

	obs::SceneItem::SetBoundsAlignment(item, aligment);
}

Napi::Value osn::SceneItem::GetBoundsType(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), obs::SceneItem::GetBoundsType(item));
}

void osn::SceneItem::SetBoundsType(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	int32_t boundsType = value.ToNumber().Int32Value();

	obs::SceneItem::SetBoundsType(item, boundsType);
}

Napi::Value osn::SceneItem::GetCrop(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	auto res = obs::SceneItem::GetCrop(item);
	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("left", Napi::Number::New(info.Env(), res.left));
	obj.Set("top", Napi::Number::New(info.Env(), res.top));
	obj.Set("right", Napi::Number::New(info.Env(), res.right));
	obj.Set("bottom", Napi::Number::New(info.Env(), res.bottom));

	return obj;
}

void osn::SceneItem::SetCrop(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	auto item = sceneItems[this->id];
	if (!item)
		return;

	Napi::Object vector = info[0].ToObject();
	obs_sceneitem_crop crop;
	crop.left = vector.Get("left").ToNumber().Int32Value();
	crop.top = vector.Get("top").ToNumber().Int32Value();
	crop.right = vector.Get("right").ToNumber().Int32Value();
	crop.bottom = vector.Get("bottom").ToNumber().Int32Value();

	obs::SceneItem::SetCrop(item, crop);
}

Napi::Value osn::SceneItem::GetId(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	auto id = obs::SceneItem::GetId(item);

	return Napi::Number::New(info.Env(), id);
}

Napi::Value osn::SceneItem::MoveUp(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	obs::SceneItem::MoveUp(item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveDown(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	obs::SceneItem::MoveDown(item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveTop(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	obs::SceneItem::MoveTop(item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveBottom(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	obs::SceneItem::MoveBottom(item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::Move(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	int32_t position = info[0].ToNumber().Int32Value();

	obs::SceneItem::Move(item, position);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateBegin(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	obs::SceneItem::DeferUpdateBegin(item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateEnd(const Napi::CallbackInfo& info)
{
	auto item = sceneItems[this->id];
	if (!item)
		return info.Env().Undefined();

	obs::SceneItem::DeferUpdateEnd(item);

	return info.Env().Undefined();
}
