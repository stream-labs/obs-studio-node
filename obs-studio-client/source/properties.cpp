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

Nan::Persistent<v8::FunctionTemplate> osn::Properties::prototype;
Nan::Persistent<v8::FunctionTemplate> osn::PropertyObject::prototype;

osn::Properties::Properties()
{
	properties = std::make_shared<property_map_t>();
}

osn::Properties::Properties(property_map_t container)
{
	properties = std::make_shared<property_map_t>(std::move(container));
}

osn::Properties::Properties(property_map_t container, v8::Local<v8::Object> owner)
    : owner(v8::Isolate::GetCurrent(), owner)
{
	properties = std::make_shared<property_map_t>(std::move(container));
}

osn::Properties::~Properties()
{
	properties = nullptr; // Technically not needed, just here for testing.
	this->owner.Reset();
}

std::shared_ptr<osn::property_map_t> osn::Properties::GetProperties()
{
	return properties;
}

v8::Local<v8::Object> osn::Properties::GetOwner()
{
	return this->owner.Get(v8::Isolate::GetCurrent());
}

void osn::Properties::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Properties").ToLocalChecked());
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);

	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "count", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Count));
	utilv8::SetTemplateField(objtemplate, "first", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), First));
	utilv8::SetTemplateField(objtemplate, "last", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Last));
	utilv8::SetTemplateField(objtemplate, "get", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Get));

	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "Properties").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::Properties::Count(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	args.GetReturnValue().Set((uint32_t)obj->properties->size());
	return;
}

void osn::Properties::First(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	if (obj->properties->size() == 0) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	auto                 iter    = obj->properties->begin();
	osn::PropertyObject* propobj = new osn::PropertyObject(args.This(), iter->first);
	args.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

void osn::Properties::Last(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	if (obj->properties->size() == 0) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	auto                 iter    = --obj->properties->end();
	osn::PropertyObject* propobj = new osn::PropertyObject(args.This(), iter->first);
	args.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

void osn::Properties::Get(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	std::string name;
	ASSERT_GET_VALUE(args[0], name);

	for (auto iter = obj->properties->begin(); iter != obj->properties->end(); iter++) {
		if (iter->second->name == name) {
			osn::PropertyObject* propobj = new osn::PropertyObject(args.This(), iter->first);
			args.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
			return;
		}
	}
	args.GetReturnValue().Set(Nan::Null());
	return;
}

osn::PropertyObject::PropertyObject(v8::Local<v8::Object> p_parent, size_t index)
    : parent(v8::Isolate::GetCurrent(), p_parent)
{
	this->index = index;
}

osn::PropertyObject::~PropertyObject() {}

void osn::PropertyObject::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Property").ToLocalChecked());
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);
	fnctemplate->PrototypeTemplate()->SetInternalFieldCount(1);

	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "previous", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Previous));
	utilv8::SetTemplateField(objtemplate, "next", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Next));
	utilv8::SetTemplateField(objtemplate, "is_first", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsFirst));
	utilv8::SetTemplateField(objtemplate, "is_last", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsLast));

	utilv8::SetTemplateAccessorProperty(objtemplate, "value", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetValue));

	utilv8::SetTemplateAccessorProperty(objtemplate, "name", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetName));
	utilv8::SetTemplateAccessorProperty(objtemplate, "description", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetDescription));
	utilv8::SetTemplateAccessorProperty(objtemplate, "longDescription", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetLongDescription));
	utilv8::SetTemplateAccessorProperty(objtemplate, "enabled", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsEnabled));
	utilv8::SetTemplateAccessorProperty(objtemplate, "visible", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsVisible));
	utilv8::SetTemplateAccessorProperty(objtemplate, "details", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetDetails));

	utilv8::SetTemplateAccessorProperty(objtemplate, "type", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetType));

	utilv8::SetTemplateField(objtemplate, "modified", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Modified));
	utilv8::SetTemplateField(objtemplate, "buttonClicked", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), ButtonClicked));

	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "Property").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::PropertyObject::Previous(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	if (iter == parent->GetProperties()->begin()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	/// Decrement iterator, which sounds really stupid but works fine. Until you invalidate it, that is.
	iter--;

	osn::PropertyObject* propobj = new osn::PropertyObject(self->parent.Get(args.GetIsolate()), iter->first);
	args.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

void osn::PropertyObject::Next(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	iter++;
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	osn::PropertyObject* propobj = new osn::PropertyObject(self->parent.Get(args.GetIsolate()), iter->first);
	args.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

void osn::PropertyObject::IsFirst(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	args.GetReturnValue().Set(iter == parent->GetProperties()->begin());
	return;
}

void osn::PropertyObject::IsLast(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = property_map_t::reverse_iterator(parent->GetProperties()->find(self->index));
	args.GetReturnValue().Set(iter == parent->GetProperties()->rbegin());
	return;
}

void osn::PropertyObject::GetValue(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	switch (iter->second->type) {
	case osn::Property::Type::INVALID:
		return;
	case osn::Property::Type::BOOL: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);
		args.GetReturnValue().Set(utilv8::ToValue(reinterpret_cast<bool*>(cast_property->bool_value.value)));
		break;
	}
	case osn::Property::Type::INT: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);

		args.GetReturnValue().Set(utilv8::ToValue(cast_property->int_value.value));
		break;
	}
	case osn::Property::Type::COLOR: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);

		args.GetReturnValue().Set(utilv8::ToValue(cast_property->int_value.value));
		break;
	}
	case osn::Property::Type::FLOAT: {
		std::shared_ptr<osn::NumberProperty> cast_property =
		    std::static_pointer_cast<osn::NumberProperty>(iter->second);

		args.GetReturnValue().Set(utilv8::ToValue(cast_property->float_value.value));
		break;
	}
	case osn::Property::Type::TEXT: {
		std::shared_ptr<osn::TextProperty> cast_property = std::static_pointer_cast<osn::TextProperty>(iter->second);
		args.GetReturnValue().Set(utilv8::ToValue(cast_property->value));
		break;
	}
	case osn::Property::Type::PATH: {
		std::shared_ptr<osn::PathProperty> cast_property = std::static_pointer_cast<osn::PathProperty>(iter->second);
		args.GetReturnValue().Set(utilv8::ToValue(cast_property->value));
		break;
	}

	case osn::Property::Type::LIST: {
		std::shared_ptr<osn::ListProperty> cast_property = std::static_pointer_cast<osn::ListProperty>(iter->second);
		switch (cast_property->item_format) {
		case ListProperty::Format::FLOAT:
			args.GetReturnValue().Set(utilv8::ToValue(cast_property->current_value_float));
			break;
		case ListProperty::Format::INT:
			args.GetReturnValue().Set(utilv8::ToValue(cast_property->current_value_int));
			break;
		case ListProperty::Format::STRING:
			args.GetReturnValue().Set(utilv8::ToValue(cast_property->current_value_str));
			break;
		}
		break;
	}
	case osn::Property::Type::EDITABLELIST: {
		std::shared_ptr<osn::EditableListProperty> cast_property =
		    std::static_pointer_cast<osn::EditableListProperty>(iter->second);

		v8::Local<v8::Array> values = Nan::New<v8::Array>();
		size_t               idx      = 0;
		for (auto value : cast_property->values) {
			v8::Local<v8::Object> iobj = Nan::New<v8::Object>();
			utilv8::SetObjectField(iobj, "value", value);

			utilv8::SetObjectField(values, (uint32_t)idx++, iobj);
		}

		args.GetReturnValue().Set(values);
		break;
	}
	case osn::Property::Type::BUTTON:
		break;
	case osn::Property::Type::FONT: {
		std::shared_ptr<osn::FontProperty> cast_property = std::static_pointer_cast<osn::FontProperty>(iter->second);
		v8::Local<v8::Object>              font          = v8::Object::New(args.GetIsolate());

		font->Set(Nan::GetCurrentContext(), utilv8::ToValue("face"), utilv8::ToValue(cast_property->face));
		font->Set(Nan::GetCurrentContext(), utilv8::ToValue("style"), utilv8::ToValue(cast_property->style));
		font->Set(Nan::GetCurrentContext(), utilv8::ToValue("path"), utilv8::ToValue(cast_property->path));
		font->Set(Nan::GetCurrentContext(), utilv8::ToValue("size"), utilv8::ToValue(cast_property->sizeF));
		font->Set(Nan::GetCurrentContext(), utilv8::ToValue("flags"), utilv8::ToValue(cast_property->flags));

		args.GetReturnValue().Set(font);
		break;
	}
	case osn::Property::Type::FRAMERATE:
		break;
	}
}

void osn::PropertyObject::GetName(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	args.GetReturnValue().Set(utilv8::ToValue(iter->second->name));
	return;
}

void osn::PropertyObject::GetDescription(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	args.GetReturnValue().Set(utilv8::ToValue(iter->second->description));
	return;
}

void osn::PropertyObject::GetLongDescription(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	args.GetReturnValue().Set(utilv8::ToValue(iter->second->long_description));
	return;
}

void osn::PropertyObject::IsEnabled(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	args.GetReturnValue().Set(iter->second->enabled);
	return;
}

void osn::PropertyObject::IsVisible(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	args.GetReturnValue().Set(iter->second->visible);
	return;
}

void osn::PropertyObject::GetType(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	args.GetReturnValue().Set(utilv8::ToValue((uint32_t)iter->second->type));
	return;
}

void osn::PropertyObject::GetDetails(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::PropertyObject* self;
	osn::Properties*     parent;

	if (!Retrieve(args.This(), self)) {
		return;
	}
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	v8::Local<v8::Object> object = Nan::New<v8::Object>();

	switch (iter->second->type) {
	case osn::Property::Type::INT: {
		std::shared_ptr<osn::NumberProperty> prop = std::static_pointer_cast<osn::NumberProperty>(iter->second);

		utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
		utilv8::SetObjectField(object, "min", prop->int_value.min);
		utilv8::SetObjectField(object, "max", prop->int_value.max);
		utilv8::SetObjectField(object, "step", prop->int_value.step);
		break;
	}
	case osn::Property::Type::FLOAT: {
		std::shared_ptr<osn::NumberProperty> prop = std::static_pointer_cast<osn::NumberProperty>(iter->second);

		utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
		utilv8::SetObjectField(object, "min", prop->float_value.min);
		utilv8::SetObjectField(object, "max", prop->float_value.max);
		utilv8::SetObjectField(object, "step", prop->float_value.step);
		break;
	}
	case osn::Property::Type::TEXT: {
		std::shared_ptr<osn::TextProperty> prop = std::static_pointer_cast<osn::TextProperty>(iter->second);

		utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
		break;
	}
	case osn::Property::Type::PATH: {
		std::shared_ptr<osn::PathProperty> prop = std::static_pointer_cast<osn::PathProperty>(iter->second);

		utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
		utilv8::SetObjectField(object, "filter", prop->filter);
		utilv8::SetObjectField(object, "defaultPath", prop->default_path);
		break;
	}
	case osn::Property::Type::LIST: {
		std::shared_ptr<osn::ListProperty> prop = std::static_pointer_cast<osn::ListProperty>(iter->second);

		utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
		utilv8::SetObjectField(object, "format", (uint32_t)prop->item_format);

		v8::Local<v8::Array> itemsobj = Nan::New<v8::Array>();
		size_t               idx      = 0;
		for (auto itm : prop->items) {
			v8::Local<v8::Object> iobj = Nan::New<v8::Object>();
			utilv8::SetObjectField(iobj, "name", itm.name);
			utilv8::SetObjectField(iobj, "enabled", !itm.disabled);
			switch (prop->item_format) {
			case ListProperty::Format::INT:
				utilv8::SetObjectField(iobj, "value", itm.value_int);
				break;
			case ListProperty::Format::FLOAT:
				utilv8::SetObjectField(iobj, "value", itm.value_float);
				break;
			case ListProperty::Format::STRING:
				utilv8::SetObjectField(iobj, "value", itm.value_str);
				break;
			}

			utilv8::SetObjectField(itemsobj, (uint32_t)idx++, iobj);
		}
		utilv8::SetObjectField(object, "items", itemsobj);

		break;
	}
	case osn::Property::Type::EDITABLELIST: {
		std::shared_ptr<osn::EditableListProperty> prop =
		    std::static_pointer_cast<osn::EditableListProperty>(iter->second);

		utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
		utilv8::SetObjectField(object, "filter", prop->filter);
		utilv8::SetObjectField(object, "defaultPath", prop->default_path);
		break;
	}
	case osn::Property::Type::FRAMERATE: {
		std::shared_ptr<osn::FrameRateProperty> prop = std::static_pointer_cast<osn::FrameRateProperty>(iter->second);

		v8::Local<v8::Array> rangesobj = Nan::New<v8::Array>();
		size_t               idx       = 0;
		for (auto itm : prop->ranges) {
			v8::Local<v8::Object> minobj = Nan::New<v8::Object>();
			utilv8::SetObjectField(minobj, "numerator", itm.first.numerator);
			utilv8::SetObjectField(minobj, "denominator", itm.first.denominator);

			v8::Local<v8::Object> maxobj = Nan::New<v8::Object>();
			utilv8::SetObjectField(maxobj, "numerator", itm.second.numerator);
			utilv8::SetObjectField(maxobj, "denominator", itm.second.denominator);

			v8::Local<v8::Object> iobj = Nan::New<v8::Object>();
			utilv8::SetObjectField(iobj, "min", minobj);
			utilv8::SetObjectField(iobj, "max", maxobj);

			utilv8::SetObjectField(rangesobj, (uint32_t)idx++, iobj);
		}
		utilv8::SetObjectField(object, "ranges", rangesobj);

		v8::Local<v8::Array> itemsobj = Nan::New<v8::Array>();
		idx                           = 0;
		for (auto itm : prop->options) {
			v8::Local<v8::Object> iobj = Nan::New<v8::Object>();
			utilv8::SetObjectField(iobj, "name", itm.name);
			utilv8::SetObjectField(iobj, "description", itm.description);
			utilv8::SetObjectField(rangesobj, (uint32_t)idx++, iobj);
		}
		utilv8::SetObjectField(object, "items", itemsobj);

		break;
	}
	}

	args.GetReturnValue().Set(object);
	return;
}

void osn::PropertyObject::Modified(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> settings;

	// Value Stuff
	/// Arguments
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], settings);
	/// Self
	osn::PropertyObject* self;
	if (!Retrieve(args.This(), self)) {
		return;
	}
	/// Parent
	osn::Properties* parent;
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}
	/// Parent Source (if one exists).
	osn::ISource* parent_source;
	if (!osn::ISource::Retrieve(parent->GetOwner(), parent_source)) {
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	// Stringify settings
	v8::MaybeLocal<v8::String> settings_str = v8::JSON::Stringify(args.GetIsolate()->GetCurrentContext(), settings);
	std::string                value;
	if (!utilv8::FromValue(settings_str.ToLocalChecked(), value)) {
		Nan::Error("Unable to convert Settings Object to String");
	}

	// Call
	auto conn = GetConnection();
	if (!conn) {
		return;
	}
	auto rval = conn->call_synchronous_helper(
	    "Properties",
	    "Modified",
	    {ipc::value(parent_source->sourceId), ipc::value(iter->second->name), ipc::value(value)});

	if (!ValidateResponse(rval)) {
		args.GetReturnValue().Set(false);
		return;
	}

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(parent_source->sourceId);
	if (sdi) {
		sdi->propertiesChanged = true;
		sdi->settingsChanged   = true;
	}

	args.GetReturnValue().Set(!!rval[1].value_union.i32);
}

void osn::PropertyObject::ButtonClicked(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> source_obj;

	// Value Stuff
	/// Arguments
	ASSERT_INFO_LENGTH(args, 1);
	ASSERT_GET_VALUE(args[0], source_obj); // This object is not necessary, we keep track of the parent source here.
	/// Self
	osn::PropertyObject* self;
	if (!Retrieve(args.This(), self)) {
		return;
	}
	/// Parent
	osn::Properties* parent;
	if (!osn::Properties::Retrieve(self->parent.Get(args.GetIsolate()), parent)) {
		Nan::ThrowReferenceError("Parent invalidated while child is still alive.");
		return;
	}
	/// Parent Source (if one exists).
	osn::ISource* parent_source;
	if (!osn::ISource::Retrieve(parent->GetOwner(), parent_source)) {
		return;
	}
	/// Target Source
	osn::ISource* source;
	if (!osn::ISource::Retrieve(source_obj, source)) {
		return;
	}

	// !FIXME! Optimize so we can directly access the map whenever possible, if at all possible.
	auto iter = parent->GetProperties()->find(self->index);
	if (iter == parent->GetProperties()->end()) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	// Call
	auto conn = GetConnection();
	if (!conn) {
		return;
	}
	auto rval = conn->call_synchronous_helper(
	    "Properties", "Clicked", {ipc::value(parent_source->sourceId), ipc::value(iter->second->name)});

	if (!ValidateResponse(rval)) {
		args.GetReturnValue().Set(false);
		return;
	}

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(source->sourceId);
	if (sdi) {
		sdi->propertiesChanged = true;
	}

	args.GetReturnValue().Set(true);
}
