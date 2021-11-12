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
	this->m_item = *externalItem.Data();

}

void osn::SceneItem::SetOBSSceneItem(obs_sceneitem_t* item)
{
	this->m_item = item;
}

Napi::Value osn::SceneItem::GetSource(const Napi::CallbackInfo& info)
{
	auto source = obs::SceneItem::GetSource(this->m_item);

    auto instance =
        osn::Input::constructor.New({
			Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::SceneItem::GetScene(const Napi::CallbackInfo& info)
{
	auto source = obs::SceneItem::GetScene(this->m_item);
    auto instance =
        osn::Scene::constructor.New({
            Napi::External<obs_source_t*>::New(info.Env(), &source)
		});

    return instance;
}

Napi::Value osn::SceneItem::Remove(const Napi::CallbackInfo& info)
{
	obs::SceneItem::Remove(this->m_item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::IsVisible(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::SceneItem::IsVisible(this->m_item));
}

void osn::SceneItem::SetVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool visible = value.ToBoolean().Value();
	obs::SceneItem::SetVisible(this->m_item, visible);
}

Napi::Value osn::SceneItem::IsSelected(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::SceneItem::IsSelected(this->m_item));
}

void osn::SceneItem::SetSelected(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool selected = value.ToBoolean().Value();
	obs::SceneItem::SetSelected(this->m_item, selected);
}

Napi::Value osn::SceneItem::IsStreamVisible(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::SceneItem::IsStreamVisible(this->m_item));
}

void osn::SceneItem::SetStreamVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool streamVisible = value.ToBoolean().Value();

	obs::SceneItem::SetStreamVisible(this->m_item, streamVisible);
}

Napi::Value osn::SceneItem::IsRecordingVisible(const Napi::CallbackInfo& info)
{
	bool recordingVisible = obs::SceneItem::IsRecordingVisible(this->m_item);

	return Napi::Boolean::New(info.Env(), recordingVisible);
}

void osn::SceneItem::SetRecordingVisible(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	bool recordingVisible = value.ToBoolean().Value();

	obs::SceneItem::SetRecordingVisible(this->m_item, recordingVisible);
}

Napi::Value osn::SceneItem::GetPosition(const Napi::CallbackInfo& info)
{
	auto res = obs::SceneItem::GetPosition(this->m_item);
	float x = res.first;
	float y = res.second;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));

	return obj;
}

void osn::SceneItem::SetPosition(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	obs::SceneItem::SetPosition(this->m_item, x, y);
}

Napi::Value osn::SceneItem::GetRotation(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::SceneItem::GetRotation(this->m_item));
}

void osn::SceneItem::SetRotation(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	float_t rotation = info[0].ToNumber().FloatValue();

	obs::SceneItem::SetRotation(this->m_item, rotation);
}

Napi::Value osn::SceneItem::GetScale(const Napi::CallbackInfo& info)
{
	auto res = obs::SceneItem::GetScale(this->m_item);
	float x = res.first;
	float y = res.second;

	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("x", Napi::Number::New(info.Env(), x));
	obj.Set("y", Napi::Number::New(info.Env(), y));

	return obj;
}

void osn::SceneItem::SetScale(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	Napi::Object vector = info[0].ToObject();
	float_t x = vector.Get("x").ToNumber().FloatValue();
	float_t y = vector.Get("y").ToNumber().FloatValue();

	obs::SceneItem::SetScale(this->m_item, x, y);
}

Napi::Value osn::SceneItem::GetScaleFilter(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::SceneItem::GetScaleFilter(this->m_item));
}

void osn::SceneItem::SetScaleFilter(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t scaleFilter = value.ToNumber().Uint32Value();

	obs::SceneItem::SetScaleFilter(this->m_item, scaleFilter);
}

Napi::Value osn::SceneItem::GetAlignment(const Napi::CallbackInfo& info)
{
	return Napi::Boolean::New(info.Env(), obs::SceneItem::GetAlignment(this->m_item));
}

void osn::SceneItem::SetAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t alignment = value.ToNumber().Uint32Value();

	obs::SceneItem::SetAlignment(this->m_item, alignment);
}

Napi::Value osn::SceneItem::GetBounds(const Napi::CallbackInfo& info)
{
	auto res = obs::SceneItem::GetBounds(this->m_item);
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

	obs::SceneItem::SetBounds(this->m_item, x, y);
}

Napi::Value osn::SceneItem::GetBoundsAlignment(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::SceneItem::GetBoundsAlignment(this->m_item));
}

void osn::SceneItem::SetBoundsAlignment(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	uint32_t aligment = value.ToNumber().Uint32Value();

	obs::SceneItem::SetBoundsAlignment(this->m_item, aligment);
}

Napi::Value osn::SceneItem::GetBoundsType(const Napi::CallbackInfo& info)
{
	return Napi::Number::New(info.Env(), obs::SceneItem::GetBoundsType(this->m_item));
}

void osn::SceneItem::SetBoundsType(const Napi::CallbackInfo& info, const Napi::Value &value)
{
	int32_t boundsType = value.ToNumber().Int32Value();

	obs::SceneItem::SetBoundsType(this->m_item, boundsType);
}

Napi::Value osn::SceneItem::GetCrop(const Napi::CallbackInfo& info)
{
	auto res = obs::SceneItem::GetCrop(this->m_item);
	Napi::Object obj = Napi::Object::New(info.Env());
	obj.Set("left", Napi::Number::New(info.Env(), res.left));
	obj.Set("top", Napi::Number::New(info.Env(), res.top));
	obj.Set("right", Napi::Number::New(info.Env(), res.right));
	obj.Set("bottom", Napi::Number::New(info.Env(), res.bottom));

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

	obs::SceneItem::SetCrop(this->m_item, crop);
}

Napi::Value osn::SceneItem::GetId(const Napi::CallbackInfo& info)
{
	auto id = obs::SceneItem::GetId(this->m_item);

	return Napi::Number::New(info.Env(), id);
}

Napi::Value osn::SceneItem::MoveUp(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveUp(this->m_item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveDown(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveDown(this->m_item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveTop(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveTop(this->m_item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::MoveBottom(const Napi::CallbackInfo& info)
{
	obs::SceneItem::MoveBottom(this->m_item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::Move(const Napi::CallbackInfo& info)
{
	int32_t position = info[0].ToNumber().Int32Value();

	obs::SceneItem::Move(this->m_item, position);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateBegin(const Napi::CallbackInfo& info)
{
	obs::SceneItem::DeferUpdateBegin(this->m_item);

	return info.Env().Undefined();
}

Napi::Value osn::SceneItem::DeferUpdateEnd(const Napi::CallbackInfo& info)
{
	obs::SceneItem::DeferUpdateEnd(this->m_item);

	return info.Env().Undefined();
}
