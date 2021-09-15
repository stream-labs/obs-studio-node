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

#include "properties.hpp"
#include "isource.hpp"
#include "utility-v8.hpp"

std::shared_ptr<osn::property_map_t> osn::Properties::GetProperties()
{
	return properties;
}
Napi::FunctionReference osn::Properties::constructor;

Napi::Object osn::Properties::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Properties",
		{
			InstanceMethod("count", &osn::Properties::Count),
			InstanceMethod("first", &osn::Properties::First),
			InstanceMethod("last", &osn::Properties::Last),
			InstanceMethod("get", &osn::Properties::Get),
		});
	exports.Set("Properties", func);
	osn::Properties::constructor = Napi::Persistent(func);
	osn::Properties::constructor.SuppressDestruct();
	return exports;
}

osn::Properties::Properties(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<osn::Properties>(info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
	this->properties = std::make_shared<property_map_t>(*info[0].As<const Napi::External<property_map_t>>().Data());

	auto externalItem = info[1].As<Napi::External<obs_source_t*>>();
	this->m_source = *externalItem.Data();
}

Napi::Value osn::Properties::Count(const Napi::CallbackInfo& info)
{
	osn::Properties* obj =
		Napi::ObjectWrap<osn::Properties>::Unwrap(info.This().ToObject());
	if (!obj)
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), (uint32_t)obj->properties->size());
}

Napi::Value osn::Properties::First(const Napi::CallbackInfo& info)
{
	osn::Properties* obj =
		Napi::ObjectWrap<osn::Properties>::Unwrap(info.This().ToObject());
	if (!obj)
		return info.Env().Undefined();

	if (obj->properties->size() == 0)
		return info.Env().Undefined();

	auto iter = obj->properties->begin();
	auto instance =
		osn::PropertyObject::constructor.New({
			info.This(), Napi::Number::New(info.Env(), (uint32_t)iter->first)
			});
	return instance;
}

Napi::Value osn::Properties::Last(const Napi::CallbackInfo& info)
{
	osn::Properties* obj =
		Napi::ObjectWrap<osn::Properties>::Unwrap(info.This().ToObject());
	if (!obj)
		return info.Env().Undefined();

	if (obj->properties->size() == 0)
		return info.Env().Undefined();

	auto iter = --obj->properties->end();
	auto instance =
		osn::PropertyObject::constructor.New({
			info.This(), Napi::Number::New(info.Env(), (uint32_t)iter->first)
			});
	return instance;
}

Napi::Value osn::Properties::Get(const Napi::CallbackInfo& info)
{
	osn::Properties* obj =
		Napi::ObjectWrap<osn::Properties>::Unwrap(info.This().ToObject());
	if (!obj)
		return info.Env().Undefined();

	std::string name = info[0].ToString().Utf8Value();

	for (auto iter = obj->properties->begin(); iter != obj->properties->end(); iter++) {
		if (iter->second->name == name) {
			auto instance =
				osn::PropertyObject::constructor.New({
					info.This(), Napi::Number::New(info.Env(), (uint32_t)iter->first)
					});
			return instance;
		}
	}
	return info.Env().Undefined();
}

Napi::FunctionReference osn::PropertyObject::constructor;

Napi::Object osn::PropertyObject::Init(Napi::Env env, Napi::Object exports) {
	Napi::HandleScope scope(env);
	Napi::Function func =
		DefineClass(env,
		"Property",
		{
			InstanceMethod("previous", &osn::PropertyObject::Previous),
			InstanceMethod("next", &osn::PropertyObject::Next),
			InstanceMethod("is_first", &osn::PropertyObject::IsFirst),
			InstanceMethod("is_last", &osn::PropertyObject::IsLast),

			InstanceAccessor("value", &osn::PropertyObject::GetValue, nullptr),
			InstanceAccessor("name", &osn::PropertyObject::GetName, nullptr),
			InstanceAccessor("description", &osn::PropertyObject::GetDescription, nullptr),
			InstanceAccessor("longDescription", &osn::PropertyObject::GetLongDescription, nullptr),
			InstanceAccessor("enabled", &osn::PropertyObject::IsEnabled, nullptr),
			InstanceAccessor("visible", &osn::PropertyObject::IsVisible, nullptr),
			InstanceAccessor("details", &osn::PropertyObject::GetDetails, nullptr),
			InstanceAccessor("type", &osn::PropertyObject::GetType, nullptr),

			InstanceMethod("modified", &osn::PropertyObject::Modified),
			InstanceMethod("buttonClicked", &osn::PropertyObject::ButtonClicked),
		});
	exports.Set("Property", func);
	osn::PropertyObject::constructor = Napi::Persistent(func);
	osn::PropertyObject::constructor.SuppressDestruct();
	return exports;
}

osn::PropertyObject::PropertyObject(const Napi::CallbackInfo& info)
	: Napi::ObjectWrap<osn::PropertyObject>(info) {
	Napi::Env env = info.Env();
	Napi::HandleScope scope(env);
	this->parent = Napi::ObjectWrap<osn::Properties>::Unwrap(info[0].ToObject());
	this->index = (uint64_t)info[1].ToNumber().Uint32Value();
}

Napi::Value osn::PropertyObject::Previous(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	if (iter == parent->GetProperties()->begin())
		return info.Env().Undefined();

	iter--;

	auto instance =
		osn::PropertyObject::constructor.New({
			parent->Get(info), Napi::Number::New(info.Env(), (uint32_t)iter->first)
			});
	return instance;
}

Napi::Value osn::PropertyObject::Next(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	iter++;
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	auto prop_ptr = Napi::External<property_map_t>::New(info.Env(), parent->properties.get());
	auto obj = osn::Properties::constructor.New(
		{
			prop_ptr,
			Napi::External<obs_source_t*>::New(info.Env(), &parent->m_source),
		});

	auto instance =
		osn::PropertyObject::constructor.New({
			obj, Napi::Number::New(info.Env(), (uint32_t)iter->first)
			});
	return instance;
}

Napi::Value osn::PropertyObject::IsFirst(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	return Napi::Boolean::New(info.Env(), iter == parent->GetProperties()->begin());

}

Napi::Value osn::PropertyObject::IsLast(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = property_map_t::reverse_iterator(parent->GetProperties()->find(self->index));
	return Napi::Boolean::New(info.Env(), iter == parent->GetProperties()->rbegin());
}

Napi::Value osn::PropertyObject::GetValue(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	switch (iter->second->type) {
	case osn::Property::Type::INVALID:
		return info.Env().Undefined();
	case osn::Property::Type::BOOL: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);
		return Napi::Boolean::New(info.Env(), cast_property->bool_value.value);
	}
	case osn::Property::Type::INT: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);
		return Napi::Number::New(info.Env(), cast_property->int_value.value);
	}
	case osn::Property::Type::COLOR: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);
		return Napi::Number::New(info.Env(), cast_property->int_value.value);
	}
	case osn::Property::Type::FLOAT: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);
		return Napi::Number::New(info.Env(), cast_property->float_value.value);
	}
	case osn::Property::Type::TEXT: {
		std::shared_ptr<osn::TextProperty> cast_property = std::static_pointer_cast<osn::TextProperty>(iter->second);
		return Napi::String::New(info.Env(), cast_property->value);
	}
	case osn::Property::Type::PATH: {
		std::shared_ptr<osn::PathProperty> cast_property = std::static_pointer_cast<osn::PathProperty>(iter->second);
		return Napi::String::New(info.Env(), cast_property->value);
	}

	case osn::Property::Type::LIST: {
		std::shared_ptr<osn::ListProperty> cast_property = std::static_pointer_cast<osn::ListProperty>(iter->second);
		switch (cast_property->item_format) {
		case ListProperty::Format::FLOAT:
			return Napi::Number::New(info.Env(), cast_property->current_value_float);
		case ListProperty::Format::INT:
			return Napi::Number::New(info.Env(), cast_property->current_value_int);
		case ListProperty::Format::STRING:
			return Napi::String::New(info.Env(), cast_property->current_value_str);
		}
		break;
	}
	case osn::Property::Type::EDITABLELIST: {
		std::shared_ptr<osn::EditableListProperty> cast_property =
		    std::static_pointer_cast<osn::EditableListProperty>(iter->second);

		Napi::Array values = Napi::Array::New(info.Env());
		size_t idx = 0;
		for (auto value : cast_property->values) {
			Napi::Object iobj = Napi::Object::New(info.Env());
			iobj.Set("value", value);
			values.Set((uint32_t)idx++, iobj);
		}

		return values;
	}
	case osn::Property::Type::BUTTON:
		break;
	case osn::Property::Type::FONT: {
		std::shared_ptr<osn::FontProperty> cast_property = std::static_pointer_cast<osn::FontProperty>(iter->second);
		Napi::Object font = Napi::Object::New(info.Env());
		font.Set("face", cast_property->face);
		font.Set("style", cast_property->style);
		font.Set("path", cast_property->path);
		font.Set("size", Napi::Number::New(info.Env(), cast_property->sizeF));
		font.Set("flags", Napi::Number::New(info.Env(), cast_property->flags));
		return font;
	}
	case osn::Property::Type::FRAMERATE:
		break;
	}
	return info.Env().Undefined();
}

Napi::Value osn::PropertyObject::GetName(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), iter->second->name);
}

Napi::Value osn::PropertyObject::GetDescription(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), iter->second->description);
}

Napi::Value osn::PropertyObject::GetLongDescription(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	return Napi::String::New(info.Env(), iter->second->long_description);
}

Napi::Value osn::PropertyObject::IsEnabled(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), iter->second->enabled);
}

Napi::Value osn::PropertyObject::IsVisible(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	return Napi::Boolean::New(info.Env(), iter->second->visible);
}

Napi::Value osn::PropertyObject::GetType(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	return Napi::Number::New(info.Env(), (uint32_t)iter->second->type);
}

Napi::Value osn::PropertyObject::GetDetails(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Undefined();

	Napi::Object object = Napi::Object::New(info.Env());

	switch (iter->second->type) {
	case osn::Property::Type::INT: {
		std::shared_ptr<osn::NumberProperty> prop = std::static_pointer_cast<osn::NumberProperty>(iter->second);

		object.Set("type", Napi::Number::New(info.Env(), (uint32_t)prop->field_type));
		object.Set("min", Napi::Number::New(info.Env(), prop->int_value.min));
		object.Set("max", Napi::Number::New(info.Env(), prop->int_value.max));
		object.Set("step", Napi::Number::New(info.Env(), prop->int_value.step));
		break;
	}
	case osn::Property::Type::FLOAT: {
		std::shared_ptr<osn::NumberProperty> prop = std::static_pointer_cast<osn::NumberProperty>(iter->second);

		object.Set("type", Napi::Number::New(info.Env(), (uint32_t)prop->field_type));
		object.Set("min", Napi::Number::New(info.Env(), prop->float_value.min));
		object.Set("max", Napi::Number::New(info.Env(), prop->float_value.max));
		object.Set("step", Napi::Number::New(info.Env(), prop->float_value.step));
		break;
	}
	case osn::Property::Type::TEXT: {
		std::shared_ptr<osn::TextProperty> prop = std::static_pointer_cast<osn::TextProperty>(iter->second);

		object.Set("type", Napi::Number::New(info.Env(), (uint32_t)prop->field_type));
		break;
	}
	case osn::Property::Type::PATH: {
		std::shared_ptr<osn::PathProperty> prop = std::static_pointer_cast<osn::PathProperty>(iter->second);

		object.Set("type", Napi::Number::New(info.Env(), (uint32_t)prop->field_type));
		object.Set("filter", Napi::String::New(info.Env(), prop->filter));
		object.Set("defaultPath", Napi::String::New(info.Env(), prop->default_path));
		break;
	}
	case osn::Property::Type::LIST: {
		std::shared_ptr<osn::ListProperty> prop = std::static_pointer_cast<osn::ListProperty>(iter->second);

		object.Set("type", Napi::Number::New(info.Env(), (uint32_t)prop->field_type));
		object.Set("format", Napi::Number::New(info.Env(), (uint32_t)prop->item_format));

		Napi::Array itemsobj = Napi::Array::New(info.Env());
		size_t idx = 0;
		for (auto itm : prop->items) {
			Napi::Object iobj = Napi::Object::New(info.Env());
			iobj.Set("name", Napi::String::New(info.Env(), itm.name));
			iobj.Set("enabled", Napi::Boolean::New(info.Env(), !itm.disabled));

			switch (prop->item_format) {
			case ListProperty::Format::INT:
				iobj.Set("value", Napi::Number::New(info.Env(), itm.value_int));
				break;
			case ListProperty::Format::FLOAT:
				iobj.Set("value", Napi::Number::New(info.Env(), itm.value_float));
				break;
			case ListProperty::Format::STRING:
				iobj.Set("value", Napi::String::New(info.Env(), itm.value_str));
				break;
			}
			itemsobj.Set((uint32_t)idx++, iobj);
		}
		object.Set("items", itemsobj);
		break;
	}
	case osn::Property::Type::EDITABLELIST: {
		std::shared_ptr<osn::EditableListProperty> prop =
		    std::static_pointer_cast<osn::EditableListProperty>(iter->second);

		object.Set("type", Napi::Number::New(info.Env(), (uint32_t)prop->field_type));
		object.Set("filter", Napi::String::New(info.Env(), prop->filter));
		object.Set("defaultPath", Napi::String::New(info.Env(), prop->default_path));
		break;
	}
	case osn::Property::Type::FRAMERATE: {
		std::shared_ptr<osn::FrameRateProperty> prop = std::static_pointer_cast<osn::FrameRateProperty>(iter->second);

		Napi::Array rangesobj = Napi::Array::New(info.Env());
		size_t idx = 0;
		for (auto itm : prop->ranges) {
			Napi::Object minobj = Napi::Object::New(info.Env());
			minobj.Set("numerator", Napi::Number::New(info.Env(), itm.first.numerator));
			minobj.Set("denominator", Napi::Number::New(info.Env(), itm.first.denominator));

			Napi::Object maxobj = Napi::Object::New(info.Env());
			maxobj.Set("numerator", Napi::Number::New(info.Env(), itm.second.numerator));
			maxobj.Set("denominator", Napi::Number::New(info.Env(), itm.second.denominator));

			Napi::Object iobj = Napi::Object::New(info.Env());
			iobj.Set("min", minobj);
			iobj.Set("max", maxobj);

			rangesobj.Set((uint32_t)idx++, iobj);
		}
		object.Set("ranges", rangesobj);

		Napi::Array itemsobj = Napi::Array::New(info.Env());
		idx = 0;
		for (auto itm : prop->options) {
			Napi::Object iobj = Napi::Object::New(info.Env());
			iobj.Set("name", Napi::String::New(info.Env(), itm.name));
			iobj.Set("description", Napi::String::New(info.Env(), itm.description));
			rangesobj.Set((uint32_t)idx++, iobj);
		}
		object.Set("items", itemsobj);
		break;
	}
	}

	return object;
}

Napi::Value osn::PropertyObject::Modified(const Napi::CallbackInfo& info)
{
	Napi::Object settings = info[0].ToObject();

	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Null();

	Napi::String settings_str = stringify.Call(json, { settings }).As<Napi::String>();
	std::string value = settings_str.Utf8Value();

	return Napi::Boolean::New(info.Env(), obs::Properties::Modified(parent->m_source, iter->second->name, value));
}

Napi::Value osn::PropertyObject::ButtonClicked(const Napi::CallbackInfo& info)
{
	osn::PropertyObject* self =
		Napi::ObjectWrap<osn::PropertyObject>::Unwrap(info.This().ToObject());
	if (!self)
		return info.Env().Undefined();
	osn::Properties* parent = self->parent;
	if (!parent)
		return info.Env().Undefined();

	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end())
		return info.Env().Null();

	bool res = obs::Properties::Clicked(parent->m_source, iter->second->name);

	return Napi::Boolean::New(info.Env(), res);
}
