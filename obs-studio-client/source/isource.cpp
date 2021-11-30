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
#include <functional>
#include <future>

#include "shared.hpp"
#include "utility-v8.hpp"
#include "utility.hpp"
#include "server/osn-source.hpp"

void osn::ISource::Release(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	obs::Source::Release(source);
}

void osn::ISource::Remove(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	obs::Source::Remove(source);
}

Napi::Value osn::ISource::IsConfigurable(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::Boolean::New(info.Env(), obs::Source::IsConfigurable(source));
}

Napi::Value osn::ISource::GetProperties(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	Napi::Array propertiesObject = Napi::Array::New(info.Env());

	uint32_t indexProperties = 0;
	bool updateSource = false;
	obs_properties_t* prp = obs_source_properties(source);
	obs_data* settings = obs_source_get_settings(source);

	for (obs_property_t* p = obs_properties_first(prp); (p != nullptr); obs_property_next(&p)) {
		Napi::Object propertyObject = Napi::Object::New(info.Env());
		std::string name = getSafeOBSstr(obs_property_name(p));

		propertyObject.Set("name", Napi::String::New(info.Env(), name));
		propertyObject.Set("description",
			Napi::String::New(info.Env(), 
			getSafeOBSstr(obs_property_description(p))));
		propertyObject.Set("longDescription",
			Napi::String::New(info.Env(), obs_property_long_description(p) ? obs_property_long_description(p) : ""));
		propertyObject.Set("enabled", Napi::Boolean::New(info.Env(), obs_property_enabled(p)));
		propertyObject.Set("visible", Napi::Boolean::New(info.Env(), obs_property_visible(p)));

		switch (obs_property_get_type(p)) {
		case OBS_PROPERTY_BOOL: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_BOOL"));
			propertyObject.Set("value", 
				Napi::Boolean::New(info.Env(), obs_data_get_bool(settings, name.c_str())));
			break;
		}
		case OBS_PROPERTY_INT: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_INT"));
			propertyObject.Set("value", 
				Napi::Number::New(info.Env(), (int)obs_data_get_int(settings, name.c_str())));
			propertyObject.Set("min", Napi::Number::New(info.Env(), obs_property_int_min(p)));
			propertyObject.Set("max", Napi::Number::New(info.Env(), obs_property_int_max(p)));
			propertyObject.Set("step", Napi::Number::New(info.Env(), obs_property_int_step(p)));
			switch (obs_property_int_type(p))
			{
			case OBS_NUMBER_SCROLLER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SCROLLER"));
				break;
			}
			case OBS_NUMBER_SLIDER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SLIDER"));
				break;
			}
			}
			break;
		}
		case OBS_PROPERTY_FLOAT: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_FLOAT"));
			propertyObject.Set("value",
				Napi::Number::New(info.Env(), obs_data_get_double(settings, name.c_str())));
			propertyObject.Set("min", Napi::Number::New(info.Env(), obs_property_float_min(p)));
			propertyObject.Set("max", Napi::Number::New(info.Env(), obs_property_float_max(p)));
			propertyObject.Set("step", Napi::Number::New(info.Env(), obs_property_float_step(p)));
			switch (obs_property_float_type(p))
			{
			case OBS_NUMBER_SCROLLER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SCROLLER"));
				break;
			}
			case OBS_NUMBER_SLIDER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SLIDER"));
				break;
			}
			}
			break;
		}
		case OBS_PROPERTY_TEXT: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_TEXT"));
			propertyObject.Set("value",
				Napi::String::New(info.Env(),
				getSafeOBSstr(obs_data_get_string(settings, name.c_str()))));
			switch (obs_proprety_text_type(p))
			{
			case OBS_TEXT_DEFAULT: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_TEXT_DEFAULT"));
				break;
			}
			case OBS_TEXT_PASSWORD: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_TEXT_PASSWORD"));
				break;
			}
			case OBS_TEXT_MULTILINE: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_TEXT_MULTILINE"));
				break;
			}
			}
			break;
		}
		case OBS_PROPERTY_PATH: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_PATH"));
			propertyObject.Set("defaultPath",
				Napi::String::New(info.Env(), getSafeOBSstr(obs_property_path_default_path(p))));
			propertyObject.Set("filter",
				Napi::String::New(info.Env(), getSafeOBSstr(obs_property_path_filter(p))));
			propertyObject.Set("value",
				Napi::String::New(info.Env(), getSafeOBSstr(obs_data_get_string(settings, name.c_str()))));
			switch (obs_property_path_type(p))
			{
			case OBS_PATH_FILE: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_PATH_FILE"));
				break;
			}
			case OBS_PATH_FILE_SAVE: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_PATH_FILE_SAVE"));
				break;
			}
			case OBS_PATH_DIRECTORY: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_PATH_DIRECTORY"));
				break;
			}
			}
			break;
		}
		case OBS_PROPERTY_LIST: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_LIST"));
			switch (obs_property_list_type(p))
			{
			case OBS_COMBO_TYPE_EDITABLE: {
				propertyObject.Set("fieldType",
					Napi::String::New(info.Env(), "OBS_COMBO_TYPE_EDITABLE"));
				break;
			}
			case OBS_COMBO_TYPE_LIST: {
				propertyObject.Set("fieldType",
					Napi::String::New(info.Env(), "OBS_COMBO_TYPE_LIST"));
				break;
			}
			}
			switch (obs_property_list_format(p))
			{
			case OBS_COMBO_FORMAT_INT: {
				propertyObject.Set("format",
					Napi::String::New(info.Env(), "OBS_COMBO_FORMAT_INT"));
				propertyObject.Set("value",
					Napi::Number::New(info.Env(), obs_data_get_int(settings, name.c_str())));
				break;
			}
			case OBS_COMBO_FORMAT_FLOAT: {
				propertyObject.Set("format",
					Napi::String::New(info.Env(), "OBS_COMBO_FORMAT_FLOAT"));
				propertyObject.Set("value",
					Napi::Number::New(info.Env(), obs_data_get_double(settings, name.c_str())));
				break;
			}
			case OBS_COMBO_FORMAT_STRING: {
				propertyObject.Set("format",
					Napi::String::New(info.Env(), "OBS_COMBO_FORMAT_STRING"));
				propertyObject.Set("value",
					Napi::String::New(info.Env(),
						getSafeOBSstr(obs_data_get_string(settings, name.c_str()))));
				break;
			}
			}
			Napi::Array itemsobj = Napi::Array::New(info.Env());
			for (size_t i = 0; i < obs_property_list_item_count(p); i++) {
				Napi::Object iobj = Napi::Object::New(info.Env());
				iobj.Set("name", 
					Napi::String::New(info.Env(), getSafeOBSstr(obs_property_list_item_name(p, i))));
				iobj.Set("enabled", Napi::Boolean::New(info.Env(), !obs_property_list_item_disabled(p, i)));
				switch (obs_property_list_format(p)) {
				case OBS_COMBO_FORMAT_INT: {
					iobj.Set("value",
						Napi::Number::New(info.Env(), obs_property_list_item_int(p, i)));
					break;
				}
				case OBS_COMBO_FORMAT_FLOAT: {
					iobj.Set("value",
						Napi::Number::New(info.Env(), obs_property_list_item_float(p, i)));
					break;
				}
				case OBS_COMBO_FORMAT_STRING: {
					iobj.Set("value",
						Napi::String::New(info.Env(),
						getSafeOBSstr(obs_property_list_item_string(p, i))));
					break;
				}
				}
				itemsobj.Set(i, iobj);
			}
			propertyObject.Set("items", itemsobj);
			break;
		}
		case OBS_PROPERTY_COLOR_ALPHA: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_COLOR_ALPHA"));
			propertyObject.Set("value",
				Napi::Number::New(info.Env(), obs_data_get_int(settings, name.c_str())));
			switch (obs_property_float_type(p))
			{
			case OBS_NUMBER_SCROLLER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SCROLLER"));
				break;
			}
			case OBS_NUMBER_SLIDER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SLIDER"));
				break;
			}
			}
			break;
		}
		case OBS_PROPERTY_COLOR: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_COLOR"));
			propertyObject.Set("value",
				Napi::Number::New(info.Env(), obs_data_get_int(settings, name.c_str())));
			switch (obs_property_float_type(p))
			{
			case OBS_NUMBER_SCROLLER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SCROLLER"));
				break;
			}
			case OBS_NUMBER_SLIDER: {
				propertyObject.Set("fieldType", Napi::String::New(info.Env(), "OBS_NUMBER_SLIDER"));
				break;
			}
			}
			break;
		}
		case OBS_PROPERTY_BUTTON: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_BUTTON"));
			break;
		}
		case OBS_PROPERTY_FONT: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_FONT"));
			obs_data_t* font_obj = obs_data_get_obj(settings, name.c_str());
			Napi::Object font = Napi::Object::New(info.Env());
			font.Set("face",
				Napi::String::New(info.Env(),
				getSafeOBSstr(obs_data_get_string(font_obj, "face"))));
			font.Set("style",
				Napi::String::New(info.Env(),
				getSafeOBSstr(obs_data_get_string(font_obj, "style"))));
			font.Set("path",
				Napi::String::New(info.Env(),
				getSafeOBSstr(obs_data_get_string(font_obj, "path"))));
			font.Set("size", Napi::Number::New(info.Env(), obs_data_get_int(font_obj, "size")));
			font.Set("flags", Napi::Number::New(info.Env(), obs_data_get_int(font_obj, "flags")));
			propertyObject.Set("value", font);
			break;
		}
		case OBS_PROPERTY_EDITABLE_LIST: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_EDITABLE_LIST"));
			propertyObject.Set("filter",
				Napi::String::New(info.Env(), getSafeOBSstr(obs_property_editable_list_filter(p))));
			propertyObject.Set("defaultPath",
				Napi::String::New(info.Env(), getSafeOBSstr(obs_property_editable_list_default_path(p))));
			switch (obs_property_editable_list_type(p))
			{
			case OBS_EDITABLE_LIST_TYPE_STRINGS: {
				propertyObject.Set("fieldType",
					Napi::String::New(info.Env(), "OBS_EDITABLE_LIST_TYPE_STRINGS"));
				break;
			}
			case OBS_EDITABLE_LIST_TYPE_FILES: {
				propertyObject.Set("fieldType",
					Napi::String::New(info.Env(), "OBS_EDITABLE_LIST_TYPE_FILES"));
				break;
			}
			case OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS: {
				propertyObject.Set("fieldType",
					Napi::String::New(info.Env(), "OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS"));
				break;
			}
			}
			Napi::Array values = Napi::Array::New(info.Env());
			obs_data_array_t* array = obs_data_get_array(settings, name.c_str());
			for (size_t i = 0; i < obs_data_array_count(array); i++) {
				obs_data_t* item = obs_data_array_item(array, i);
				Napi::Object iobj = Napi::Object::New(info.Env());
				iobj.Set("value",
					Napi::String::New(info.Env(),
					getSafeOBSstr(obs_data_get_string(item, "value"))));
				values.Set(i, iobj);
			}
			propertyObject.Set("value", values);
			break;
		}
		case OBS_PROPERTY_GROUP:
		case OBS_PROPERTY_FRAME_RATE:
			break;
		}
		propertiesObject.Set(indexProperties++, propertyObject);
	}

	return propertiesObject;
	// auto res = obs::Source::GetProperties(source);
	// osn::property_map_t pmap;
	// for (size_t idx = 0; idx < res.size(); ++idx) {
	// 	auto raw_property = obs::Property::deserialize(res[idx]);

	// 	std::shared_ptr<osn::Property> pr;

	// 	switch (raw_property->type()) {
	// 	case obs::Property::Type::Boolean: {
	// 		std::shared_ptr<obs::BooleanProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::BooleanProperty>(raw_property);
	// 		std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
	// 		pr2->bool_value.value                    = cast_property->value;
	// 		pr                                       = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::Integer: {
	// 		std::shared_ptr<obs::IntegerProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::IntegerProperty>(raw_property);
	// 		std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
	// 		pr2->field_type                          = osn::NumberProperty::Type(cast_property->field_type);
	// 		pr2->int_value.min                       = cast_property->minimum;
	// 		pr2->int_value.max                       = cast_property->maximum;
	// 		pr2->int_value.step                      = cast_property->step;
	// 		pr2->int_value.value                     = cast_property->value;
	// 		pr                                       = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::Color: {
	// 		std::shared_ptr<obs::ColorProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::ColorProperty>(raw_property);
	// 		std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
	// 		pr2->field_type                          = osn::NumberProperty::Type(cast_property->field_type);
	// 		pr2->int_value.value                     = cast_property->value;
	// 		pr                                       = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::Float: {
	// 		std::shared_ptr<obs::FloatProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::FloatProperty>(raw_property);
	// 		std::shared_ptr<osn::NumberProperty> pr2 = std::make_shared<osn::NumberProperty>();
	// 		pr2->field_type                          = osn::NumberProperty::Type(cast_property->field_type);
	// 		pr2->float_value.min                     = cast_property->minimum;
	// 		pr2->float_value.max                     = cast_property->maximum;
	// 		pr2->float_value.step                    = cast_property->step;
	// 		pr2->float_value.value                   = cast_property->value;
	// 		pr                                       = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::Text: {
	// 		std::shared_ptr<obs::TextProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::TextProperty>(raw_property);
	// 		std::shared_ptr<osn::TextProperty> pr2 = std::make_shared<osn::TextProperty>();
	// 		pr2->field_type                        = osn::TextProperty::Type(cast_property->field_type);
	// 		pr2->value                             = cast_property->value;
	// 		pr                                     = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::Path: {
	// 		std::shared_ptr<obs::PathProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::PathProperty>(raw_property);
	// 		std::shared_ptr<osn::PathProperty> pr2 = std::make_shared<osn::PathProperty>();
	// 		pr2->field_type                        = osn::PathProperty::Type(cast_property->field_type);
	// 		pr2->filter                            = cast_property->filter;
	// 		pr2->default_path                      = cast_property->default_path;
	// 		pr2->value                             = cast_property->value;
	// 		pr                                     = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::List: {
	// 		std::shared_ptr<obs::ListProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::ListProperty>(raw_property);
	// 		std::shared_ptr<osn::ListProperty> pr2 = std::make_shared<osn::ListProperty>();
	// 		pr2->field_type                        = osn::ListProperty::Type(cast_property->field_type);
	// 		pr2->item_format                       = osn::ListProperty::Format(cast_property->format);

	// 		switch (cast_property->format) {
	// 		case obs::ListProperty::Format::Integer:
	// 			pr2->current_value_int = cast_property->current_value_int;
	// 			break;
	// 		case obs::ListProperty::Format::Float:
	// 			pr2->current_value_float = cast_property->current_value_float;
	// 			break;
	// 		case obs::ListProperty::Format::String:
	// 			pr2->current_value_str = cast_property->current_value_str;
	// 			break;
	// 		}

	// 		for (auto& item : cast_property->items) {
	// 			osn::ListProperty::Item item2;
	// 			item2.name     = item.name;
	// 			item2.disabled = !item.enabled;
	// 			switch (cast_property->format) {
	// 			case obs::ListProperty::Format::Integer:
	// 				item2.value_int = item.value_int;
	// 				break;
	// 			case obs::ListProperty::Format::Float:
	// 				item2.value_float = item.value_float;
	// 				break;
	// 			case obs::ListProperty::Format::String:
	// 				item2.value_str = item.value_string;
	// 				break;
	// 			}
	// 			pr2->items.push_back(std::move(item2));
	// 		}
	// 		pr = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::Font: {
	// 		std::shared_ptr<obs::FontProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::FontProperty>(raw_property);
	// 		std::shared_ptr<osn::FontProperty> pr2 = std::make_shared<osn::FontProperty>();
	// 		pr2->face                              = cast_property->face;
	// 		pr2->style                             = cast_property->style;
	// 		pr2->path                              = cast_property->path;
	// 		pr2->sizeF                             = cast_property->sizeF;
	// 		pr2->flags                             = cast_property->flags;
	// 		pr                                     = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::EditableList: {
	// 		std::shared_ptr<obs::EditableListProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::EditableListProperty>(raw_property);
	// 		std::shared_ptr<osn::EditableListProperty> pr2 = std::make_shared<osn::EditableListProperty>();
	// 		pr2->field_type                                = osn::EditableListProperty::Type(cast_property->field_type);
	// 		pr2->filter                                    = cast_property->filter;
	// 		pr2->default_path                              = cast_property->default_path;

	// 		for (auto& item : cast_property->values) {
	// 			pr2->values.push_back(item);
	// 		}
	// 		pr = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	case obs::Property::Type::FrameRate: {
	// 		std::shared_ptr<obs::FrameRateProperty> cast_property =
	// 		    std::dynamic_pointer_cast<obs::FrameRateProperty>(raw_property);
	// 		std::shared_ptr<osn::ListProperty> pr2 = std::make_shared<osn::ListProperty>();
	// 		pr2->field_type                        = osn::ListProperty::Type::LIST;
	// 		pr2->item_format                       = osn::ListProperty::Format::STRING;

	// 		nlohmann::json fps;
	// 		fps["numerator"] = cast_property->current_numerator;
	// 		fps["denominator"] = cast_property->current_denominator;
	// 		pr2->current_value_str = fps.dump();

	// 		for (auto& option : cast_property->ranges) {
	// 			nlohmann::json fps;
	// 			fps["numerator"] = option.maximum.first;
	// 			fps["denominator"] = option.maximum.second;
	// 			osn::ListProperty::Item item2;
	// 			item2.name     = std::to_string(option.maximum.first / option.maximum.second);
	// 			item2.disabled = false;
	// 			item2.value_str = fps.dump();
	// 			pr2->items.push_back(std::move(item2));
	// 		}

	// 		pr = std::static_pointer_cast<osn::Property>(pr2);
	// 		break;
	// 	}
	// 	default: {
	// 		pr = std::make_shared<osn::Property>();
	// 		break;
	// 	}
	// 	}

	// 	if (pr) {
	// 		pr->name             = raw_property->name;
	// 		pr->description      = raw_property->description;
	// 		pr->long_description = raw_property->long_description;
	// 		pr->type             = osn::Property::Type(raw_property->type());
	// 		if (pr->type == osn::Property::Type::FRAMERATE)
	// 			pr->type = osn::Property::Type::LIST;
	// 		pr->enabled          = raw_property->enabled;
	// 		pr->visible          = raw_property->visible;

	// 		pmap.emplace(idx - 1, pr);
	// 	}
	// }

	// std::shared_ptr<property_map_t> pSomeObject = std::make_shared<property_map_t>(pmap);
	// auto prop_ptr = Napi::External<property_map_t>::New(info.Env(), pSomeObject.get());
	// auto instance =
	// 	osn::Properties::constructor.New({
	// 		prop_ptr,
	// 		Napi::External<obs_source_t*>::New(info.Env(), &source)
	// 	});
	// return instance;
}

Napi::Value osn::ISource::GetSettings(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function parse = json.Get("parse").As<Napi::Function>();

	auto res = obs::Source::GetSettings(source);
	Napi::String jsondata = Napi::String::New(info.Env(), res);
	Napi::Object jsonObj = parse.Call(json, {jsondata}).As<Napi::Object>();

	return jsonObj;
}

void osn::ISource::Update(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	Napi::Object jsonObj = info[0].ToObject();

	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

	std::string jsondata = stringify.Call(json, { jsonObj }).As<Napi::String>();

	obs::Source::Update(source, jsondata);
}

void osn::ISource::Load(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	obs::Source::Load(source);
}

void osn::ISource::Save(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	obs::Source::Save(source);
}

Napi::Value osn::ISource::GetType(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Source::GetType(source));
}

Napi::Value osn::ISource::GetName(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::String::New(info.Env(), obs::Source::GetName(source));
}

void osn::ISource::SetName(const Napi::CallbackInfo& info, const Napi::Value &value, obs_source_t* source)
{
	PROFINY_SCOPE
	std::string name = value.ToString().Utf8Value();

	obs::Source::SetName(source, name);
}

Napi::Value osn::ISource::GetOutputFlags(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Source::GetOutputFlags(source));
}

Napi::Value osn::ISource::GetFlags(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Source::GetFlags(source));
}

void osn::ISource::SetFlags(const Napi::CallbackInfo& info, const Napi::Value &value, obs_source_t* source)
{
	PROFINY_SCOPE
	uint32_t flags = value.ToNumber().Uint32Value();

	obs::Source::SetFlags(source, flags);
}

Napi::Value osn::ISource::GetStatus(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::Number::New(info.Env(), obs::Source::GetStatus(source));
}

Napi::Value osn::ISource::GetId(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::String::New(info.Env(), obs::Source::GetId(source));
}

Napi::Value osn::ISource::GetMuted(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::Boolean::New(info.Env(), obs::Source::GetMuted(source));
}

void osn::ISource::SetMuted(const Napi::CallbackInfo& info, const Napi::Value &value, obs_source_t* source)
{
	PROFINY_SCOPE
	bool muted = value.ToBoolean().Value();

	obs::Source::SetMuted(source, muted);
}

Napi::Value osn::ISource::GetEnabled(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	return Napi::Boolean::New(info.Env(), obs::Source::GetEnabled(source));
}

void osn::ISource::SetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value, obs_source_t* source)
{
	PROFINY_SCOPE
	bool enabled = value.ToBoolean().Value();

	obs::Source::SetEnabled(source, enabled);
}

void osn::ISource::SendMouseClick(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	Napi::Object mouse_event_obj = info[0].ToObject();
	uint32_t type = info[1].ToNumber().Uint32Value();
	bool mouse_up = info[2].ToBoolean().Value();
	uint32_t click_count = info[3].ToNumber().Uint32Value();

	uint32_t modifiers = mouse_event_obj.Get("modifiers").ToNumber().Uint32Value();
	uint32_t x = mouse_event_obj.Get("x").ToNumber().Uint32Value();
	uint32_t y = mouse_event_obj.Get("y").ToNumber().Uint32Value();

	obs::Source::SendMouseClick(
		source,
		modifiers,
		x,
		y,
		type,
		mouse_up,
		click_count
	);
}

void osn::ISource::SendMouseMove(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	Napi::Object mouse_event_obj = info[0].ToObject();
	bool mouse_leave = info[1].ToBoolean().Value();

	uint32_t modifiers = mouse_event_obj.Get("modifiers").ToNumber().Uint32Value();
	uint32_t x = mouse_event_obj.Get("x").ToNumber().Uint32Value();
	uint32_t y = mouse_event_obj.Get("y").ToNumber().Uint32Value();

	obs::Source::SendMouseMove(
		source,
		modifiers,
		x,
		y,
		mouse_leave
	);
}

void osn::ISource::SendMouseWheel(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	Napi::Object mouse_event_obj = info[0].ToObject();
	int32_t x_delta = info[1].ToNumber().Int32Value();
	int32_t y_delta = info[2].ToNumber().Int32Value();

	uint32_t modifiers = mouse_event_obj.Get("modifiers").ToNumber().Uint32Value();
	uint32_t x = mouse_event_obj.Get("x").ToNumber().Uint32Value();
	uint32_t y = mouse_event_obj.Get("y").ToNumber().Uint32Value();

	obs::Source::SendMouseWheel(
		source,
		modifiers,
		x,
		y,
		x_delta,
		y_delta
	);
}

void osn::ISource::SendFocus(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	bool focus = info[0].ToBoolean().Value();

	obs::Source::SendFocus(source, focus);
}

void osn::ISource::SendKeyClick(const Napi::CallbackInfo& info, obs_source_t* source)
{
	PROFINY_SCOPE
	Napi::Object key_event_obj = info[0].ToObject();
	bool key_up = info[1].ToBoolean().Value();

	uint32_t modifiers = key_event_obj.Get("modifiers").ToNumber().Uint32Value();
	uint32_t native_modifiers = key_event_obj.Get("nativeModifiers").ToNumber().Uint32Value();
	uint32_t native_scancode = key_event_obj.Get("nativeScancode").ToNumber().Uint32Value();
	uint32_t native_vkey = key_event_obj.Get("nativeVkey").ToNumber().Uint32Value();
	std::string text = key_event_obj.Get("text").ToString().Utf8Value();

	obs::Source::SendKeyClick(
		source,
		text,
		modifiers,
		native_modifiers,
		native_scancode,
		native_vkey,
		key_up
	);
}