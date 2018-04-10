// Client module for the OBS Studio node module.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#include "properties.hpp"
#include "utility-v8.hpp"

Nan::Persistent<v8::FunctionTemplate> osn::Properties::prototype = Nan::Persistent<v8::FunctionTemplate>();
Nan::Persistent<v8::FunctionTemplate> osn::PropertyObject::prototype = Nan::Persistent<v8::FunctionTemplate>();

osn::Properties::Properties() {
	properties = std::make_shared<property_map_t>();
}

osn::Properties::Properties(property_map_t container) {
	properties = std::make_shared<property_map_t>(std::move(container));
}

osn::Properties::~Properties() {
	properties = nullptr; // Technically not needed, just here for testing.
}

void osn::Properties::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Properties").ToLocalChecked());
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();

	utilv8::SetTemplateAccessorProperty(objtemplate, "count", Count);
	utilv8::SetTemplateAccessorProperty(objtemplate, "first", First);
	utilv8::SetTemplateAccessorProperty(objtemplate, "last", Last);
	utilv8::SetTemplateField(objtemplate, "get", Get);

	utilv8::SetObjectField(target, "Properties", fnctemplate->GetFunction());
	osn::Properties::prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::Properties::Count(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	info.GetReturnValue().Set((uint32_t)obj->properties->size());
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Properties::First(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	if (obj->properties->size() == 0) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	auto iter = obj->properties->begin();
	osn::PropertyObject* propobj = new osn::PropertyObject(info.This(), iter);
	info.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Properties::Last(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	if (obj->properties->size() == 0) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	auto iter = --obj->properties->end();
	osn::PropertyObject* propobj = new osn::PropertyObject(info.This(), iter);
	info.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::Properties::Get(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::Properties* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	std::string name;
	ASSERT_GET_VALUE(info[0], name);

	auto iter = obj->properties->find(name);
	if (iter == obj->properties->end()) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	osn::PropertyObject* propobj = new osn::PropertyObject(info.This(), iter);
	info.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

osn::PropertyObject::PropertyObject(v8::Local<v8::Object> parent, property_map_t::iterator iter) {
	this->parent = parent;
	this->iter = iter;
}

osn::PropertyObject::~PropertyObject() {

}

void osn::PropertyObject::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target) {
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Property").ToLocalChecked());
	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();

	utilv8::SetTemplateField(objtemplate, "previous", Previous);
	utilv8::SetTemplateField(objtemplate, "next", Next);
	utilv8::SetTemplateField(objtemplate, "is_first", IsFirst);
	utilv8::SetTemplateField(objtemplate, "is_last", IsLast);

	utilv8::SetTemplateAccessorProperty(objtemplate, "name", GetName);
	utilv8::SetTemplateAccessorProperty(objtemplate, "description", GetDescription);
	utilv8::SetTemplateAccessorProperty(objtemplate, "longDescription", GetLongDescription);
	utilv8::SetTemplateAccessorProperty(objtemplate, "enabled", IsEnabled);
	utilv8::SetTemplateAccessorProperty(objtemplate, "visible", IsVisible);
	utilv8::SetTemplateAccessorProperty(objtemplate, "details", IsVisible);

	utilv8::SetTemplateField(objtemplate, "modified", Modified);
	utilv8::SetTemplateField(objtemplate, "buttonClicked", ButtonClicked);

	utilv8::SetObjectField(target, "Property", fnctemplate->GetFunction());
	osn::PropertyObject::prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::Previous(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	osn::Properties* pobj;
	if (!utilv8::SafeUnwrap(info, pobj)) {
		return;
	}

	// This is required because there is no way to check if we are still "in" the map otherwise.
	property_map_t::reverse_iterator iter2(obj->iter);
	iter2++;

	if (iter2 == pobj->properties->rend()) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	osn::PropertyObject* propobj = new osn::PropertyObject(info.This(), iter2.base());
	info.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::Next(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	osn::Properties* pobj;
	if (!utilv8::SafeUnwrap(info, pobj)) {
		return;
	}

	auto iter2 = obj->iter++;
	if (iter2 == pobj->properties->end()) {
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	osn::PropertyObject* propobj = new osn::PropertyObject(info.This(), iter2);
	info.GetReturnValue().Set(osn::PropertyObject::Store(propobj));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::IsFirst(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	osn::Properties* pobj;
	if (!utilv8::SafeUnwrap(info, pobj)) {
		return;
	}

	// This is required because there is no way to check if we are still "in" the map otherwise.
	property_map_t::reverse_iterator iter2(obj->iter);
	iter2++;

	info.GetReturnValue().Set(iter2 == pobj->properties->rend());
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::IsLast(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	osn::Properties* pobj;
	if (!utilv8::SafeUnwrap(info, pobj)) {
		return;
	}

	auto iter2 = obj->iter++;
	info.GetReturnValue().Set(iter2 == pobj->properties->end());
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::GetName(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	info.GetReturnValue().Set(utilv8::ToValue(obj->iter->second->name));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::GetDescription(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	info.GetReturnValue().Set(utilv8::ToValue(obj->iter->second->description));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::GetLongDescription(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	info.GetReturnValue().Set(utilv8::ToValue(obj->iter->second->long_description));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::IsEnabled(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	info.GetReturnValue().Set(obj->iter->second->enabled);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::IsVisible(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	info.GetReturnValue().Set(obj->iter->second->visible);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::GetType(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	info.GetReturnValue().Set((uint32_t)obj->iter->second->type);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::GetDetails(Nan::NAN_METHOD_ARGS_TYPE info) {
	osn::PropertyObject* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	v8::Local<v8::Object> object = Nan::New<v8::Object>();

	switch (obj->iter->second->type) {
		case osn::Property::Type::INT:
		{
			std::shared_ptr<osn::NumberProperty> prop = std::static_pointer_cast<osn::NumberProperty>(obj->iter->second);

			utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
			utilv8::SetObjectField(object, "min", prop->Int.min);
			utilv8::SetObjectField(object, "max", prop->Int.max);
			utilv8::SetObjectField(object, "step", prop->Int.step);
			break;
		}
		case osn::Property::Type::FLOAT:
		{
			std::shared_ptr<osn::NumberProperty> prop = std::static_pointer_cast<osn::NumberProperty>(obj->iter->second);

			utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
			utilv8::SetObjectField(object, "min", prop->Float.min);
			utilv8::SetObjectField(object, "max", prop->Float.max);
			utilv8::SetObjectField(object, "step", prop->Float.step);
			break;
		}
		case osn::Property::Type::TEXT:
		{
			std::shared_ptr<osn::TextProperty> prop = std::static_pointer_cast<osn::TextProperty>(obj->iter->second);

			utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
			break;
		}
		case osn::Property::Type::PATH:
		{
			std::shared_ptr<osn::PathProperty> prop = std::static_pointer_cast<osn::PathProperty>(obj->iter->second);

			utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
			utilv8::SetObjectField(object, "filter", prop->filter);
			utilv8::SetObjectField(object, "defaultPath", prop->default_path);
			break;
		}
		case osn::Property::Type::LIST:
		{
			std::shared_ptr<osn::ListProperty> prop = std::static_pointer_cast<osn::ListProperty>(obj->iter->second);

			utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
			utilv8::SetObjectField(object, "format", (uint32_t)prop->item_format);

			v8::Local<v8::Array> itemsobj = Nan::New<v8::Array>();
			size_t idx = 0;
			for (auto itm : prop->items) {
				v8::Local<v8::Object> iobj = Nan::New<v8::Object>();
				utilv8::SetObjectField(iobj, "name", itm.second.name);
				utilv8::SetObjectField(iobj, "enabled", !itm.second.disabled);
				switch (prop->item_format) {
					case ListProperty::Format::INT:
						utilv8::SetObjectField(iobj, "value", itm.second.value_int);
						break;
					case ListProperty::Format::FLOAT:
						utilv8::SetObjectField(iobj, "value", itm.second.value_float);
						break;
					case ListProperty::Format::STRING:
						utilv8::SetObjectField(iobj, "value", itm.second.value_str);
						break;
				}

				utilv8::SetObjectField(itemsobj, (uint32_t)idx++, iobj);
			}
			utilv8::SetObjectField(object, "items", itemsobj);

			break;
		}
		case osn::Property::Type::EDITABLELIST:
		{
			std::shared_ptr<osn::EditableListProperty> prop = std::static_pointer_cast<osn::EditableListProperty>(obj->iter->second);

			utilv8::SetObjectField(object, "type", (uint32_t)prop->field_type);
			utilv8::SetObjectField(object, "filter", prop->filter);
			utilv8::SetObjectField(object, "defaultPath", prop->default_path);
			break;
		}
		case osn::Property::Type::FRAMERATE:
		{
			std::shared_ptr<osn::FrameRateProperty> prop = std::static_pointer_cast<osn::FrameRateProperty>(obj->iter->second);

			v8::Local<v8::Array> rangesobj = Nan::New<v8::Array>();
			size_t idx = 0;
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
			idx = 0;
			for (auto itm : prop->items) {
				v8::Local<v8::Object> iobj = Nan::New<v8::Object>();
				utilv8::SetObjectField(iobj, "name", itm.second.name);
				utilv8::SetObjectField(iobj, "description", itm.second.description);
				utilv8::SetObjectField(rangesobj, (uint32_t)idx++, iobj);
			}
			utilv8::SetObjectField(object, "items", itemsobj);
			
			break;
		}
	}
	
	info.GetReturnValue().Set(object);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::Modified(Nan::NAN_METHOD_ARGS_TYPE info) {
	// Call on Server
	// Requires current settings as json.
}

Nan::NAN_METHOD_RETURN_TYPE osn::PropertyObject::ButtonClicked(Nan::NAN_METHOD_ARGS_TYPE info) {
	// Call on Server
}
