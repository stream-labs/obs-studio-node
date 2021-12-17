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

std::map<uint32_t, obs_source_t*> sourcesStore;
uint32_t idSourcesCount = 0;

void osn::ISource::Release(uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	blog(LOG_INFO, "Release source %s", obs_source_get_name(source));
	obs::Source::Release(source);
}

void osn::ISource::Remove(uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	blog(LOG_INFO, "Remove source %s", obs_source_get_name(source));
	obs::Source::Remove(source);
}

Napi::Value osn::ISource::IsConfigurable(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "IsConfigurable source %s", obs_source_get_name(source));
	return Napi::Boolean::New(info.Env(), obs::Source::IsConfigurable(source));
}

Napi::Value osn::ISource::GetProperties(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetProperties source %s", obs_source_get_name(source));
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
		case OBS_PROPERTY_CAPTURE: {
			propertyObject.Set("type", Napi::String::New(info.Env(), "OBS_PROPERTY_CAPTURE"));
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
		case OBS_PROPERTY_GROUP:
		case OBS_PROPERTY_FRAME_RATE:
			break;
		}
		// propertyObject.Set("modified", &osn::ISource::Modified);
		// propertyObject.Set("buttonClicked", &osn::PropertyObject::ButtonClicked);
		propertiesObject.Set(indexProperties++, propertyObject);
	}

	return propertiesObject;
}

Napi::Value osn::ISource::GetSettings(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetSettings source %s", obs_source_get_name(source));
	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function parse = json.Get("parse").As<Napi::Function>();

	auto res = obs::Source::GetSettings(source);
	Napi::String jsondata = Napi::String::New(info.Env(), res);
	Napi::Object jsonObj = parse.Call(json, {jsondata}).As<Napi::Object>();

	return jsonObj;
}

void osn::ISource::Update(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	blog(LOG_INFO, "Update source %s", obs_source_get_name(source));
	Napi::Object jsonObj = info[0].ToObject();

	Napi::Object json = info.Env().Global().Get("JSON").As<Napi::Object>();
	Napi::Function stringify = json.Get("stringify").As<Napi::Function>();

	std::string jsondata = stringify.Call(json, { jsonObj }).As<Napi::String>();

	obs::Source::Update(source, jsondata);
}

void osn::ISource::Load(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	blog(LOG_INFO, "Load source %s", obs_source_get_name(source));
	obs::Source::Load(source);
}

void osn::ISource::Save(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	blog(LOG_INFO, "Save source %s", obs_source_get_name(source));
	obs::Source::Save(source);
}

Napi::Value osn::ISource::GetType(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetType source %s", obs_source_get_name(source));
	return Napi::Number::New(info.Env(), obs::Source::GetType(source));
}

Napi::Value osn::ISource::GetName(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetName source %s", obs_source_get_name(source));
	return Napi::String::New(info.Env(), obs::Source::GetName(source));
}

void osn::ISource::SetName(const Napi::CallbackInfo& info, const Napi::Value &value, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	std::string name = value.ToString().Utf8Value();

	blog(LOG_INFO, "SetName source %s", obs_source_get_name(source));
	obs::Source::SetName(source, name);
}

Napi::Value osn::ISource::GetOutputFlags(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetOutputFlags source %s", obs_source_get_name(source));
	return Napi::Number::New(info.Env(), obs::Source::GetOutputFlags(source));
}

Napi::Value osn::ISource::GetFlags(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetFlags source %s", obs_source_get_name(source));
	return Napi::Number::New(info.Env(), obs::Source::GetFlags(source));
}

void osn::ISource::SetFlags(const Napi::CallbackInfo& info, const Napi::Value &value, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	uint32_t flags = value.ToNumber().Uint32Value();

	blog(LOG_INFO, "SetFlags source %s", obs_source_get_name(source));
	obs::Source::SetFlags(source, flags);
}

Napi::Value osn::ISource::GetStatus(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetStatus source %s", obs_source_get_name(source));
	return Napi::Number::New(info.Env(), obs::Source::GetStatus(source));
}

Napi::Value osn::ISource::GetId(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source) {
		blog(LOG_INFO, "Source not found");
		return info.Env().Undefined();
	}

	blog(LOG_INFO, "GetId source %s", obs_source_get_name(source));
	return Napi::String::New(info.Env(), obs::Source::GetId(source));
}

Napi::Value osn::ISource::GetMuted(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetMuted source %s", obs_source_get_name(source));
	return Napi::Boolean::New(info.Env(), obs::Source::GetMuted(source));
}

void osn::ISource::SetMuted(const Napi::CallbackInfo& info, const Napi::Value &value, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	bool muted = value.ToBoolean().Value();

	blog(LOG_INFO, "SetMuted source %s", obs_source_get_name(source));
	obs::Source::SetMuted(source, muted);
}

Napi::Value osn::ISource::GetEnabled(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return info.Env().Undefined();

	blog(LOG_INFO, "GetEnabled source %s", obs_source_get_name(source));
	return Napi::Boolean::New(info.Env(), obs::Source::GetEnabled(source));
}

void osn::ISource::SetEnabled(const Napi::CallbackInfo& info, const Napi::Value &value, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	bool enabled = value.ToBoolean().Value();

	blog(LOG_INFO, "SetEnabled source %s", obs_source_get_name(source));
	obs::Source::SetEnabled(source, enabled);
}

void osn::ISource::SendMouseClick(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

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

void osn::ISource::SendMouseMove(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

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

void osn::ISource::SendMouseWheel(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

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

void osn::ISource::SendFocus(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	bool focus = info[0].ToBoolean().Value();

	obs::Source::SendFocus(source, focus);
}

void osn::ISource::SendKeyClick(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

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

void osn::ISource::ButtonClicked(const Napi::CallbackInfo& info, uint32_t id)
{
	auto source = sourcesStore[id];
	if (!source)
		return;

	std::string propertyName = info[0].ToString().Utf8Value();
	obs_properties_t* props = obs_source_properties(source);
	obs_property_t* prop = obs_properties_get(props, propertyName.c_str());
	if (!prop) {
		obs_properties_destroy(props);
		blog(LOG_ERROR, "Failed to find property in source.");
		return;
	} else {
		obs_property_button_clicked(prop, source);
	}
	obs_properties_destroy(props);
}