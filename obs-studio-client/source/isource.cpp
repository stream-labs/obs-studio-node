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

#include "isource.hpp"
#include <error.hpp>
#include "controller.hpp"
#include "obs-property.hpp"
#include "properties.hpp"
#include "shared.hpp"
#include "utility-v8.hpp"
#include "utility.hpp"

Nan::Persistent<v8::FunctionTemplate> osn::ISource::prototype = Nan::Persistent<v8::FunctionTemplate>();

void osn::ISource::Register(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Source").ToLocalChecked());
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);

	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "release", Release);
	utilv8::SetTemplateField(objtemplate, "remove", Remove);

	utilv8::SetTemplateAccessorProperty(objtemplate, "configurable", IsConfigurable);
	utilv8::SetTemplateAccessorProperty(objtemplate, "properties", GetProperties);
	utilv8::SetTemplateAccessorProperty(objtemplate, "settings", GetSettings);
	utilv8::SetTemplateField(objtemplate, "update", Update);
	utilv8::SetTemplateField(objtemplate, "load", Load);
	utilv8::SetTemplateField(objtemplate, "save", Save);

	utilv8::SetTemplateAccessorProperty(objtemplate, "type", GetType);
	utilv8::SetTemplateAccessorProperty(objtemplate, "name", GetName, SetName);
	utilv8::SetTemplateAccessorProperty(objtemplate, "outputFlags", GetOutputFlags);
	utilv8::SetTemplateAccessorProperty(objtemplate, "flags", GetFlags, SetFlags);
	utilv8::SetTemplateAccessorProperty(objtemplate, "status", GetStatus);
	utilv8::SetTemplateAccessorProperty(objtemplate, "id", GetId);
	utilv8::SetTemplateAccessorProperty(objtemplate, "muted", GetMuted, SetMuted);
	utilv8::SetTemplateAccessorProperty(objtemplate, "enabled", GetEnabled, SetEnabled);

	utilv8::SetObjectField(target, "Source", fnctemplate->GetFunction());
	prototype.Reset(fnctemplate);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Release(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(info, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "Release", {ipc::value(obj->sourceId)});

	if (!ValidateResponse(response))
		return;

	obj->sourceId = UINT64_MAX;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Remove(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "Remove", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	is->sourceId = UINT64_MAX;
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::IsConfigurable(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "IsConfigurable", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set((bool)response[1].value_union.i32);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetProperties(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* hndl = nullptr;
	if (!utilv8::SafeUnwrap<osn::ISource>(info, hndl)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetProperties", {ipc::value(hndl->sourceId)});

	if (!ValidateResponse(response))
		return;

	// Parse the massive structure of properties we were just sent.
	osn::property_map_t pmap;
	for (size_t idx = 1; idx < response.size(); ++idx) {
		// !FIXME! Use the already existing obs::Property object instead of copying data.
		auto raw_property = obs::Property::deserialize(response[idx].value_bin);

		std::shared_ptr<osn::Property> pr;

		switch (raw_property->type()) {
		case obs::Property::Type::Integer: {
			std::shared_ptr<obs::IntegerProperty> cast_property =
			    std::dynamic_pointer_cast<obs::IntegerProperty>(raw_property);
			std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
			pr2->field_type                          = osn::NumberProperty::Type(cast_property->field_type);
			pr2->int_value.min                       = cast_property->minimum;
			pr2->int_value.max                       = cast_property->maximum;
			pr2->int_value.step                      = cast_property->step;
			pr                                       = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::Float: {
			std::shared_ptr<obs::FloatProperty> cast_property =
			    std::dynamic_pointer_cast<obs::FloatProperty>(raw_property);
			std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
			pr2->field_type                          = osn::NumberProperty::Type(cast_property->field_type);
			pr2->float_value.min                     = cast_property->minimum;
			pr2->float_value.max                     = cast_property->maximum;
			pr2->float_value.step                    = cast_property->step;
			pr                                       = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::Text: {
			std::shared_ptr<obs::TextProperty> cast_property =
			    std::dynamic_pointer_cast<obs::TextProperty>(raw_property);
			std::shared_ptr<osn::TextProperty> pr2 = std::make_shared<osn::TextProperty>();
			pr2->field_type                        = osn::TextProperty::Type(cast_property->field_type);
			pr                                     = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::Path: {
			std::shared_ptr<obs::PathProperty> cast_property =
			    std::dynamic_pointer_cast<obs::PathProperty>(raw_property);
			std::shared_ptr<osn::PathProperty> pr2 = std::make_shared<osn::PathProperty>();
			pr2->field_type                        = osn::PathProperty::Type(cast_property->field_type);
			pr2->filter                            = cast_property->filter;
			pr2->default_path                      = cast_property->default_path;
			pr                                     = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::List: {
			std::shared_ptr<obs::ListProperty> cast_property =
			    std::dynamic_pointer_cast<obs::ListProperty>(raw_property);
			std::shared_ptr<osn::ListProperty> pr2 = std::make_shared<osn::ListProperty>();
			pr2->field_type                        = osn::ListProperty::Type(cast_property->field_type);
			pr2->item_format                       = osn::ListProperty::Format(cast_property->format);
			for (auto& item : cast_property->items) {
				osn::ListProperty::Item item2;
				item2.name     = item.name;
				item2.disabled = !item.enabled;
				switch (cast_property->format) {
				case obs::ListProperty::Format::Integer:
					item2.value_int = item.value_int;
					break;
				case obs::ListProperty::Format::Float:
					item2.value_float = item.value_float;
					break;
				case obs::ListProperty::Format::String:
					item2.value_str = item.value_string;
					break;
				}
				pr2->items.push_back(std::move(item2));
			}
			pr = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::EditableList: {
			std::shared_ptr<obs::EditableListProperty> cast_property =
			    std::dynamic_pointer_cast<obs::EditableListProperty>(raw_property);
			std::shared_ptr<osn::EditableListProperty> pr2 = std::make_shared<osn::EditableListProperty>();
			pr2->field_type                                = osn::EditableListProperty::Type(cast_property->field_type);
			pr2->filter                                    = cast_property->filter;
			pr2->default_path                              = cast_property->default_path;
			pr                                             = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::FrameRate: {
			std::shared_ptr<obs::FrameRateProperty> cast_property =
			    std::dynamic_pointer_cast<obs::FrameRateProperty>(raw_property);
			std::shared_ptr<osn::FrameRateProperty> pr2 = std::make_shared<osn::FrameRateProperty>();
			for (auto& option : cast_property->ranges) {
				std::pair<osn::FrameRateProperty::FrameRate, osn::FrameRateProperty::FrameRate> range2;
				range2.first.numerator    = option.minimum.first;
				range2.first.denominator  = option.minimum.second;
				range2.second.numerator   = option.maximum.first;
				range2.second.denominator = option.maximum.second;
				pr2->ranges.push_back(std::move(range2));
			}
			for (auto& option : cast_property->options) {
				osn::FrameRateProperty::Option option2;
				option2.name        = option.name;
				option2.description = option.description;
				pr2->options.push_back(std::move(option2));
			}

			break;
		}
		default: {
			pr = std::make_shared<osn::Property>();
			break;
		}
		}

		if (pr) {
			pr->name             = raw_property->name;
			pr->description      = raw_property->description;
			pr->long_description = raw_property->long_description;
			pr->type             = osn::Property::Type(raw_property->type());
			pr->enabled          = raw_property->enabled;
			pr->visible          = raw_property->visible;

			pmap.emplace(idx - 1, pr);
		}
	}

	// obj = std::move(pmap);
	osn::Properties* props = new osn::Properties(std::move(pmap), info.This());
	info.GetReturnValue().Set(osn::Properties::Store(props));
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetSettings(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* hndl = nullptr;
	if (!utilv8::SafeUnwrap<osn::ISource>(info, hndl)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetSettings", {ipc::value(hndl->sourceId)});

	if (!ValidateResponse(response))
		return;

	v8::Local<v8::String> jsondata = Nan::New<v8::String>(response[1].value_str).ToLocalChecked();
	v8::Local<v8::Value>  json     = v8::JSON::Parse(info.GetIsolate()->GetCurrentContext(), jsondata).ToLocalChecked();
	info.GetReturnValue().Set(json);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Update(Nan::NAN_METHOD_ARGS_TYPE info)
{
	v8::Local<v8::Object> json;
	ASSERT_GET_VALUE(info[0], json);

	// Retrieve Object
	osn::ISource* hndl = nullptr;
	if (!Retrieve(info.This(), hndl)) {
		return;
	}

	// Turn json into string
	v8::Local<v8::String> jsondata = v8::JSON::Stringify(info.GetIsolate()->GetCurrentContext(), json).ToLocalChecked();
	v8::String::Utf8Value jsondatautf8(jsondata);

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper(
	    "Source",
	    "Update",
	    {ipc::value(hndl->sourceId), ipc::value(std::string(*jsondatautf8, (size_t)jsondatautf8.length()))});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(true);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Load(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "Load", {ipc::value(is->sourceId)});

	ValidateResponse(response);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::Save(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "Save", {ipc::value(is->sourceId)});

	ValidateResponse(response);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetType(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetType", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetName(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetName", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetName(Nan::NAN_METHOD_ARGS_TYPE info)
{
	std::string name;
	ASSERT_GET_VALUE(info[0], name);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "SetName", {ipc::value(is->sourceId), ipc::value(name)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str == name));
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetOutputFlags(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetOutputFlags", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetFlags(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetFlags", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetFlags(Nan::NAN_METHOD_ARGS_TYPE info)
{
	uint32_t flags;
	ASSERT_GET_VALUE(info[0], flags);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "SetFlags", {ipc::value(is->sourceId), ipc::value(flags)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32 != flags));
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetStatus(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetStatus", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(response[1].value_union.ui32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetId(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetId", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetMuted(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetMuted", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set((bool)response[1].value_union.i32);
	return;
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetMuted(Nan::NAN_METHOD_ARGS_TYPE info)
{
	bool muted;

	ASSERT_GET_VALUE(info[0], muted);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "SetMuted", {ipc::value(is->sourceId), ipc::value(muted)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set((bool)response[1].value_union.i32 == muted);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::GetEnabled(Nan::NAN_METHOD_ARGS_TYPE info)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetEnabled", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set((bool)response[1].value_union.i32);
}

Nan::NAN_METHOD_RETURN_TYPE osn::ISource::SetEnabled(Nan::NAN_METHOD_ARGS_TYPE info)
{
	bool enabled;

	ASSERT_GET_VALUE(info[0], enabled);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(info, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "SetEnabled", {ipc::value(is->sourceId), ipc::value(enabled)});

	if (!ValidateResponse(response))
		return;

	info.GetReturnValue().Set((bool)response[1].value_union.i32 == enabled);
}
