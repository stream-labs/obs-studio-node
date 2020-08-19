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

#include "isource.hpp"
#include <error.hpp>
#include <functional>
#include "controller.hpp"
#include "shared.hpp"
#include "utility-v8.hpp"
#include "utility.hpp"

osn::ISource*                         sourceObject;
Nan::Persistent<v8::FunctionTemplate> osn::ISource::prototype;

osn::ISource::~ISource() {}

void osn::ISource::Register(v8::Local<v8::Object> exports)
{
	auto fnctemplate = Nan::New<v8::FunctionTemplate>();
	fnctemplate->SetClassName(Nan::New<v8::String>("Source").ToLocalChecked());
	fnctemplate->InstanceTemplate()->SetInternalFieldCount(1);

	v8::Local<v8::ObjectTemplate> objtemplate = fnctemplate->PrototypeTemplate();
	utilv8::SetTemplateField(objtemplate, "release", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Release));
	utilv8::SetTemplateField(objtemplate, "remove", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Remove));

	utilv8::SetTemplateAccessorProperty(objtemplate, "configurable", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), IsConfigurable));
	utilv8::SetTemplateAccessorProperty(objtemplate, "properties", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetProperties));
	utilv8::SetTemplateAccessorProperty(objtemplate, "settings", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetSettings));
	utilv8::SetTemplateField(objtemplate, "update", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Update));
	utilv8::SetTemplateField(objtemplate, "load", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Load));
	utilv8::SetTemplateField(objtemplate, "save", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), Save));

	utilv8::SetTemplateAccessorProperty(objtemplate, "type", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetType));
	utilv8::SetTemplateAccessorProperty(objtemplate, "name",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetName),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetName));
	utilv8::SetTemplateAccessorProperty(objtemplate, "outputFlags",v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetOutputFlags));
	utilv8::SetTemplateAccessorProperty(objtemplate, "flags",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetFlags),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetFlags));
	utilv8::SetTemplateAccessorProperty(objtemplate, "status", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetStatus));
	utilv8::SetTemplateAccessorProperty(objtemplate, "id", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetId));
	utilv8::SetTemplateAccessorProperty(objtemplate, "muted",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetMuted),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetMuted));
	utilv8::SetTemplateAccessorProperty(objtemplate, "enabled",
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), GetEnabled),
		v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SetEnabled));

	utilv8::SetTemplateField(objtemplate, "sendMouseClick", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SendMouseClick));
	utilv8::SetTemplateField(objtemplate, "sendMouseMove", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SendMouseMove));
	utilv8::SetTemplateField(objtemplate, "sendMouseWheel", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SendMouseWheel));
	utilv8::SetTemplateField(objtemplate, "sendFocus", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SendFocus));
	utilv8::SetTemplateField(objtemplate, "sendKeyClick", v8::FunctionTemplate::New(v8::Isolate::GetCurrent(), SendKeyClick));

	exports->Set(
		Nan::GetCurrentContext(),
		v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), "Source").ToLocalChecked(),
		fnctemplate->GetFunction(Nan::GetCurrentContext()).ToLocalChecked()).FromJust();
	prototype.Reset(fnctemplate);
}

void osn::ISource::Release(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "Release", {ipc::value(obj->sourceId)});
}

void osn::ISource::Remove(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	CacheManager<SourceDataInfo*>::getInstance().Remove(is->sourceId);

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "Remove", {ipc::value(is->sourceId)});

	is->sourceId = UINT64_MAX;
	return;
}

void osn::ISource::IsConfigurable(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "IsConfigurable", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set((bool)response[1].value_union.i32);
	return;
}

void osn::ISource::GetProperties(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* hndl = nullptr;
	if (!utilv8::SafeUnwrap<osn::ISource>(args, hndl)) {
		return;
	}

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(hndl->sourceId);

	if (sdi && !sdi->propertiesChanged && sdi->properties.size() > 0) {
		osn::Properties* props = new osn::Properties(sdi->properties, args.This());
		args.GetReturnValue().Set(osn::Properties::Store(props));
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetProperties", {ipc::value(hndl->sourceId)});

	if (!ValidateResponse(response))
		return;

	if (response.size() == 1) {
		args.GetReturnValue().Set(Nan::Null());
		return;
	}

	// Parse the massive structure of properties we were just sent.
	osn::property_map_t pmap;
	for (size_t idx = 1; idx < response.size(); ++idx) {
		// !FIXME! Use the already existing obs::Property object instead of copying data.
		auto raw_property = obs::Property::deserialize(response[idx].value_bin);

		std::shared_ptr<osn::Property> pr;

		switch (raw_property->type()) {
		case obs::Property::Type::Boolean: {
			std::shared_ptr<obs::BooleanProperty> cast_property =
			    std::dynamic_pointer_cast<obs::BooleanProperty>(raw_property);
			std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
			pr2->bool_value.value                    = cast_property->value;
			pr                                       = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::Integer: {
			std::shared_ptr<obs::IntegerProperty> cast_property =
			    std::dynamic_pointer_cast<obs::IntegerProperty>(raw_property);
			std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
			pr2->field_type                          = osn::NumberProperty::Type(cast_property->field_type);
			pr2->int_value.min                       = cast_property->minimum;
			pr2->int_value.max                       = cast_property->maximum;
			pr2->int_value.step                      = cast_property->step;
			pr2->int_value.value                     = cast_property->value;
			pr                                       = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::Color: {
			std::shared_ptr<obs::ColorProperty> cast_property =
			    std::dynamic_pointer_cast<obs::ColorProperty>(raw_property);
			std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
			pr2->field_type                          = osn::NumberProperty::Type(cast_property->field_type);
			pr2->int_value.value                     = cast_property->value;
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
			pr2->float_value.value                   = cast_property->value;
			pr                                       = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::Text: {
			std::shared_ptr<obs::TextProperty> cast_property =
			    std::dynamic_pointer_cast<obs::TextProperty>(raw_property);
			std::shared_ptr<osn::TextProperty> pr2 = std::make_shared<osn::TextProperty>();
			pr2->field_type                        = osn::TextProperty::Type(cast_property->field_type);
			pr2->value                             = cast_property->value;
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
			pr2->value                             = cast_property->value;
			pr                                     = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::List: {
			std::shared_ptr<obs::ListProperty> cast_property =
			    std::dynamic_pointer_cast<obs::ListProperty>(raw_property);
			std::shared_ptr<osn::ListProperty> pr2 = std::make_shared<osn::ListProperty>();
			pr2->field_type                        = osn::ListProperty::Type(cast_property->field_type);
			pr2->item_format                       = osn::ListProperty::Format(cast_property->format);

			switch (cast_property->format) {
			case obs::ListProperty::Format::Integer:
				pr2->current_value_int = cast_property->current_value_int;
				break;
			case obs::ListProperty::Format::Float:
				pr2->current_value_float = cast_property->current_value_float;
				break;
			case obs::ListProperty::Format::String:
				pr2->current_value_str = cast_property->current_value_str;
				break;
			}

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
		case obs::Property::Type::Font: {
			std::shared_ptr<obs::FontProperty> cast_property =
			    std::dynamic_pointer_cast<obs::FontProperty>(raw_property);
			std::shared_ptr<osn::FontProperty> pr2 = std::make_shared<osn::FontProperty>();
			pr2->face                              = cast_property->face;
			pr2->style                             = cast_property->style;
			pr2->path                              = cast_property->path;
			pr2->sizeF                             = cast_property->sizeF;
			pr2->flags                             = cast_property->flags;
			pr                                     = std::static_pointer_cast<osn::Property>(pr2);
			break;
		}
		case obs::Property::Type::EditableList: {
			std::shared_ptr<obs::EditableListProperty> cast_property =
			    std::dynamic_pointer_cast<obs::EditableListProperty>(raw_property);
			std::shared_ptr<osn::EditableListProperty> pr2 = std::make_shared<osn::EditableListProperty>();
			pr2->field_type                                = osn::EditableListProperty::Type(cast_property->field_type);
			pr2->filter                                    = cast_property->filter;
			pr2->default_path                              = cast_property->default_path;

			for (auto& item : cast_property->values) {
				pr2->values.push_back(item);
			}
			pr = std::static_pointer_cast<osn::Property>(pr2);
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

	if (sdi) {
		sdi->properties        = pmap;
		sdi->propertiesChanged = false;
	}

	// obj = std::move(pmap);
	osn::Properties* props = new osn::Properties(std::move(pmap), args.This());
	args.GetReturnValue().Set(osn::Properties::Store(props));
	return;
}

void osn::ISource::GetSettings(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::cout << "GetSettings 0" << std::endl;
	osn::ISource* hndl = nullptr;
	if (!utilv8::SafeUnwrap<osn::ISource>(args, hndl)) {
		return;
	}

	std::cout << "GetSettings 1" << std::endl;
	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(hndl->sourceId);

	if (sdi && !sdi->settingsChanged && sdi->setting.size() > 0) {
		v8::Local<v8::String> jsondata = Nan::New<v8::String>(sdi->setting).ToLocalChecked();
		v8::Local<v8::Value>  json =
	    v8::JSON::Parse(args.GetIsolate()->GetCurrentContext(), jsondata).ToLocalChecked();
		args.GetReturnValue().Set(json);
		return;
	}

	std::cout << "GetSettings 2" << std::endl;
	auto conn = GetConnection();
	if (!conn)
		return;

	std::cout << "GetSettings 3" << std::endl;
	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetSettings", {ipc::value(hndl->sourceId)});

	if (!ValidateResponse(response))
		return;

	std::cout << "GetSettings 4" << std::endl;
	v8::Local<v8::String> jsondata = Nan::New<v8::String>(response[1].value_str).ToLocalChecked();
	v8::Local<v8::Value>  json     = v8::JSON::Parse(args.GetIsolate()->GetCurrentContext(), jsondata).ToLocalChecked();

	if (sdi) {
		sdi->setting         = response[1].value_str;
		sdi->settingsChanged = false;
	}

	std::cout << "GetSettings 5" << std::endl;
	args.GetReturnValue().Set(json);
	std::cout << "GetSettings 6" << std::endl;
	return;
}

void osn::ISource::Update(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::Local<v8::Object> json;
	ASSERT_GET_VALUE(args[0], json);
	bool shouldUpdate = true;

	// Retrieve Object
	osn::ISource* hndl = nullptr;
	if (!Retrieve(args.This(), hndl)) {
		return;
	}

	// Turn json into string
	v8::Local<v8::String> jsondata = v8::JSON::Stringify(args.GetIsolate()->GetCurrentContext(), json).ToLocalChecked();
	v8::String::Utf8Value jsondatautf8(v8::Isolate::GetCurrent(), jsondata);

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(hndl->sourceId);

	if (sdi && sdi->setting.size() > 0) {
		auto newSettings = nlohmann::json::parse(std::string(*jsondatautf8, (size_t)jsondatautf8.length()));
		auto settings    = nlohmann::json::parse(sdi->setting);

		nlohmann::json::iterator it = newSettings.begin();
		while (!shouldUpdate && it != newSettings.end()) {
			nlohmann::json::iterator item = settings.find(it.key());
			if (item != settings.end()) {
				if (it.value() != item.value()) {
					shouldUpdate = false;
				}
			}
			it++;
		}
	}

	if (shouldUpdate) {
		auto conn = GetConnection();
		if (!conn)
			return;

		std::vector<ipc::value> response = conn->call_synchronous_helper(
		    "Source",
		    "Update",
		    {ipc::value(hndl->sourceId), ipc::value(std::string(*jsondatautf8, (size_t)jsondatautf8.length()))});

		if (!ValidateResponse(response))
			return;

		if (sdi) {
			sdi->setting           = response[1].value_str;
			sdi->settingsChanged   = false;
			sdi->propertiesChanged = true;
		}
	}
	args.GetReturnValue().Set(true);
	return;
}

void osn::ISource::Load(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "Load", {ipc::value(is->sourceId)});
}

void osn::ISource::Save(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	return;
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "Save", {ipc::value(is->sourceId)});
}

void osn::ISource::GetType(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetType", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.i32));
}

void osn::ISource::GetName(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(is->sourceId);

	if (sdi) {
		if (sdi->name.size() > 0) {
			args.GetReturnValue().Set(utilv8::ToValue(sdi->name));
			return;
		}
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetName", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	if (sdi)
		sdi->name = response[1].value_str.c_str();

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

void osn::ISource::SetName(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::string name;
	ASSERT_GET_VALUE(args[0], name);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "SetName", {ipc::value(is->sourceId), ipc::value(name)});
}

void osn::ISource::GetOutputFlags(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetOutputFlags", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

void osn::ISource::GetFlags(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetFlags", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_union.ui32));
}

void osn::ISource::SetFlags(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	uint32_t flags;
	ASSERT_GET_VALUE(args[0], flags);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "SetFlags", {ipc::value(is->sourceId), ipc::value(flags)});
}

void osn::ISource::GetStatus(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetStatus", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set(response[1].value_union.ui32);
}

void osn::ISource::GetId(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(is->sourceId);

	if (sdi) {
		if (sdi->obs_sourceId.size() > 0) {
			args.GetReturnValue().Set(utilv8::ToValue(sdi->obs_sourceId));
			return;
		}
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetId", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	if (sdi) {
		sdi->obs_sourceId = response[1].value_str;
	}

	args.GetReturnValue().Set(utilv8::ToValue(response[1].value_str));
}

void osn::ISource::GetMuted(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}
	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(is->sourceId);

	if (sdi) {
		if (sdi && !sdi->mutedChanged) {
			args.GetReturnValue().Set(sdi->isMuted);
			return;
		}
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response = conn->call_synchronous_helper("Source", "GetMuted", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	if (sdi) {
		sdi->isMuted      = (bool)response[1].value_union.i32;
		sdi->mutedChanged = false;
	}

	args.GetReturnValue().Set((bool)response[1].value_union.i32);
	return;
}

void osn::ISource::SetMuted(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool muted;

	ASSERT_GET_VALUE(args[0], muted);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "SetMuted", {ipc::value(is->sourceId), ipc::value(muted)});

	SourceDataInfo* sdi = CacheManager<SourceDataInfo*>::getInstance().Retrieve(is->sourceId);
	if (sdi) {
		sdi->mutedChanged = true;
	}
}

void osn::ISource::GetEnabled(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	std::vector<ipc::value> response =
	    conn->call_synchronous_helper("Source", "GetEnabled", {ipc::value(is->sourceId)});

	if (!ValidateResponse(response))
		return;

	args.GetReturnValue().Set((bool)response[1].value_union.i32);
}

void osn::ISource::SetEnabled(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool enabled;

	ASSERT_GET_VALUE(args[0], enabled);

	osn::ISource* is;
	if (!utilv8::SafeUnwrap(args, is)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	conn->call("Source", "SetEnabled", {ipc::value(is->sourceId), ipc::value(enabled)});
}

void osn::ISource::SendMouseClick(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	v8::Local<v8::Object> mouse_event_obj;
	uint32_t              type;
	bool                  mouse_up;
	uint32_t              click_count;

	ASSERT_GET_VALUE(args[0], mouse_event_obj);
	ASSERT_GET_VALUE(args[1], type);
	ASSERT_GET_VALUE(args[2], mouse_up);
	ASSERT_GET_VALUE(args[3], click_count);

	uint32_t modifiers, x, y;

	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "modifiers", modifiers);
	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "x", x);
	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "y", y);

	conn->call(
	    "Source",
	    "SendMouseClick",
	    {
			ipc::value(obj->sourceId),
			ipc::value(modifiers),
			ipc::value(x),
			ipc::value(y),
			ipc::value(type),
			ipc::value(mouse_up),
			ipc::value(click_count)
		}
	);
}

void osn::ISource::SendMouseMove(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	v8::Local<v8::Object> mouse_event_obj;
	bool                  mouse_leave;

	ASSERT_GET_VALUE(args[0], mouse_event_obj);
	ASSERT_GET_VALUE(args[1], mouse_leave);

	uint32_t modifiers, x, y;

	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "modifiers", modifiers);
	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "x", x);
	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "y", y);

	conn->call("Source", "SendMouseMove", {ipc::value(obj->sourceId), ipc::value(modifiers), ipc::value(x), ipc::value(y), ipc::value(mouse_leave)});
}

void osn::ISource::SendMouseWheel(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	v8::Local<v8::Object> mouse_event_obj;
	int                   x_delta, y_delta;

	ASSERT_GET_VALUE(args[0], mouse_event_obj);
	ASSERT_GET_VALUE(args[1], x_delta);
	ASSERT_GET_VALUE(args[2], y_delta);

	uint32_t modifiers, x, y;

	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "modifiers", modifiers);
	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "x", x);
	ASSERT_GET_OBJECT_FIELD(mouse_event_obj, "y", y);

	conn->call(
	    "Source",
	    "SendMouseWheel",
	    {
			ipc::value(obj->sourceId),
			ipc::value(modifiers),
			ipc::value(x),
			ipc::value(y),
			ipc::value(x_delta),
			ipc::value(y_delta)
		}
	);
}

void osn::ISource::SendFocus(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	bool focus;

	ASSERT_GET_VALUE(args[0], focus);

	auto conn = GetConnection();
	if (!conn)
		return;

    conn->call("Source", "SendFocus", {ipc::value(obj->sourceId), ipc::value(focus)});
}

void osn::ISource::SendKeyClick(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	osn::ISource* obj;
	if (!utilv8::SafeUnwrap(args, obj)) {
		return;
	}

	auto conn = GetConnection();
	if (!conn)
		return;

	v8::Local<v8::Object> key_event_obj;
	bool                  key_up;

	ASSERT_GET_VALUE(args[0], key_event_obj);
	ASSERT_GET_VALUE(args[1], key_up);

	uint32_t    modifiers, native_modifiers, native_scancode, native_vkey;
	std::string text;

	ASSERT_GET_OBJECT_FIELD(key_event_obj, "modifiers", modifiers);
	ASSERT_GET_OBJECT_FIELD(key_event_obj, "text", text);
	ASSERT_GET_OBJECT_FIELD(key_event_obj, "nativeModifiers", native_modifiers);
	ASSERT_GET_OBJECT_FIELD(key_event_obj, "nativeScancode", native_scancode);
	ASSERT_GET_OBJECT_FIELD(key_event_obj, "nativeVkey", native_vkey);

	conn->call(
	    "Source",
	    "SendKeyClick",
	    {
			ipc::value(obj->sourceId),
			ipc::value(modifiers),
			ipc::value(text),
			ipc::value(native_modifiers),
			ipc::value(native_scancode),
			ipc::value(native_vkey),
			ipc::value((int32_t)key_up)
		}
	);
}