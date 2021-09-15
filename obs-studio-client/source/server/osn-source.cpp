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

#include "osn-source.hpp"
#include <ipc-class.hpp>
#include <ipc-function.hpp>
#include <ipc-server.hpp>
#include <ipc-value.hpp>
#include <map>
#include <memory>
#include <obs-data.h>
#include <obs.h>
#include <obs.hpp>
#include "error.hpp"
#include "obs-property.hpp"
#include "osn-common.hpp"
#include "shared-server.hpp"
#include "callback-manager.h"
#include "memory-manager.h"

void obs::Source::initialize_global_signals()
{
	signal_handler_t* sh = obs_get_signal_handler();
	signal_handler_connect(sh, "source_create", obs::Source::global_source_create_cb, nullptr);
	signal_handler_connect(sh, "source_activate", obs::Source::global_source_activate_cb, nullptr);
	signal_handler_connect(sh, "source_deactivate", obs::Source::global_source_deactivate_cb, nullptr);
}

void obs::Source::finalize_global_signals()
{
	signal_handler_t* sh = obs_get_signal_handler();
	signal_handler_disconnect(sh, "source_create", obs::Source::global_source_create_cb, nullptr);
}

void obs::Source::attach_source_signals(obs_source_t* src)
{
	signal_handler_t* sh = obs_source_get_signal_handler(src);
	if (!sh)
		return;
	signal_handler_connect(sh, "destroy", obs::Source::global_source_destroy_cb, nullptr);
}

void obs::Source::detach_source_signals(obs_source_t* src)
{
	signal_handler_t* sh = obs_source_get_signal_handler(src);
	if (!sh)
		return;
	signal_handler_disconnect(sh, "destroy", obs::Source::global_source_destroy_cb, nullptr);
}

void obs::Source::global_source_create_cb(void* ptr, calldata_t* cd)
{
	obs_source_t* source = nullptr;
	if (!calldata_get_ptr(cd, "source", &source)) {
		throw std::runtime_error("calldata did not contain source pointer");
	}

	obs::Source::Manager::GetInstance().allocate(source);
	obs::Source::attach_source_signals(source);
	CallbackManager::addSource(source);
	MemoryManager::GetInstance().registerSource(source);
}

void obs::Source::global_source_activate_cb(void* ptr, calldata_t* cd)
{
	obs_source_t* source = nullptr;
	if (!calldata_get_ptr(cd, "source", &source)) {
		throw std::runtime_error("calldata did not contain source pointer");
	}
	MemoryManager::GetInstance().updateSourceCache(source);
}

void obs::Source::global_source_deactivate_cb(void* ptr, calldata_t* cd)
{
	obs_source_t* source = nullptr;
	if (!calldata_get_ptr(cd, "source", &source)) {
		throw std::runtime_error("calldata did not contain source pointer");
	}
	MemoryManager::GetInstance().updateSourceCache(source);
}

void obs::Source::global_source_destroy_cb(void* ptr, calldata_t* cd)
{
	obs_source_t* source = nullptr;
	if (!calldata_get_ptr(cd, "source", &source)) {
		throw std::runtime_error("calldata did not contain source pointer");
	}

	CallbackManager::removeSource(source);
	detach_source_signals(source);
	obs::Source::Manager::GetInstance().free(source);
	MemoryManager::GetInstance().unregisterSource(source);
}

void obs::Source::Remove(obs_source_t* source)
{
	obs_source_remove(source);
}

void obs::Source::Release(obs_source_t* source)
{
	obs_source_release(source);
}

bool obs::Source::IsConfigurable(obs_source_t* source)
{
	return obs_source_configurable(source);
}

std::vector<std::vector<char>> obs::Source::GetProperties(obs_source_t* source)
{
	std::vector<std::vector<char>> buff;
	bool updateSource = false;

	obs_properties_t* prp = obs_source_properties(source);
	obs_data* settings = obs_source_get_settings(source);

	buff = ProcessProperties(prp, settings, updateSource);

	obs_properties_destroy(prp);

	if (updateSource)
		obs_source_update(source, settings);

	obs_data_release(settings);
	
	return buff;
}

std::vector<std::vector<char>> obs::Source::ProcessProperties(
	obs_properties_t*           prp,
	obs_data*                   settings,
	bool&                       updateSource)
{
	std::vector<std::vector<char>> res;
	for (obs_property_t* p = obs_properties_first(prp); (p != nullptr); obs_property_next(&p)) {
		const char*                    buf  = nullptr;
		std::shared_ptr<obs::Property> prop;
		const char*                    name = obs_property_name(p);

		switch (obs_property_get_type(p)) {
		case OBS_PROPERTY_BOOL: {
			auto prop2   = std::make_shared<obs::BooleanProperty>();
			prop2->value = obs_data_get_bool(settings, name);
			prop         = prop2;
			break;
		}
		case OBS_PROPERTY_INT: {
			auto prop2        = std::make_shared<obs::IntegerProperty>();
			prop2->field_type = obs::NumberProperty::NumberType(obs_property_int_type(p));
			prop2->minimum    = obs_property_int_min(p);
			prop2->maximum    = obs_property_int_max(p);
			prop2->step       = obs_property_int_step(p);
			prop2->value      = (int)obs_data_get_int(settings, name);
			prop              = prop2;
			break;
		}
		case OBS_PROPERTY_FLOAT: {
			auto prop2        = std::make_shared<obs::FloatProperty>();
			prop2->field_type = obs::NumberProperty::NumberType(obs_property_float_type(p));
			prop2->minimum    = obs_property_float_min(p);
			prop2->maximum    = obs_property_float_max(p);
			prop2->step       = obs_property_float_step(p);
			prop2->value      = obs_data_get_double(settings, name);
			prop              = prop2;
			break;
		}
		case OBS_PROPERTY_TEXT: {
			auto prop2        = std::make_shared<obs::TextProperty>();
			prop2->field_type = obs::TextProperty::TextType(obs_proprety_text_type(p));
			prop2->value      = (buf = obs_data_get_string(settings, name)) != nullptr ? buf : "";
			prop              = prop2;
			break;
		}
		case OBS_PROPERTY_PATH: {
			auto prop2          = std::make_shared<obs::PathProperty>();
			prop2->field_type   = obs::PathProperty::PathType(obs_property_path_type(p));
			prop2->filter       = (buf = obs_property_path_filter(p)) != nullptr ? buf : "";
			prop2->default_path = (buf = obs_property_path_default_path(p)) != nullptr ? buf : "";
			prop2->value        = (buf = obs_data_get_string(settings, name)) != nullptr ? buf : "";
			prop                = prop2;
			break;
		}
		case OBS_PROPERTY_LIST: {
			auto prop2        = std::make_shared<obs::ListProperty>();
			prop2->field_type = obs::ListProperty::ListType(obs_property_list_type(p));
			prop2->format     = obs::ListProperty::Format(obs_property_list_format(p));
			size_t items      = obs_property_list_item_count(p);
			for (size_t idx = 0; idx < items; ++idx) {
				obs::ListProperty::Item entry;
				entry.name    = (buf = obs_property_list_item_name(p, idx)) != nullptr ? buf : "";
				entry.enabled = !obs_property_list_item_disabled(p, idx);
				switch (prop2->format) {
				case obs::ListProperty::Format::Integer:
					entry.value_int = obs_property_list_item_int(p, idx);
					break;
				case obs::ListProperty::Format::Float:
					entry.value_float = obs_property_list_item_float(p, idx);
					break;
				case obs::ListProperty::Format::String:
					entry.value_string = (buf = obs_property_list_item_string(p, idx)) != nullptr ? buf : "";
					break;
				}
				prop2->items.push_back(std::move(entry));
			}
			switch (prop2->format) {
			case obs::ListProperty::Format::Integer: {
				prop2->current_value_int = (int)obs_data_get_int(settings, name);
				break;
			}
			case obs::ListProperty::Format::Float: {
				prop2->current_value_float = obs_data_get_double(settings, name);
				break;
			}
			case obs::ListProperty::Format::String: {
				prop2->current_value_str = (buf = obs_data_get_string(settings, name)) != nullptr ? buf : "";
				if (prop2->current_value_str.compare("") == 0 && prop2->items.size() > 0 && obs_property_enabled(p)) {
					prop2->current_value_str = prop2->items.front().value_string;
					obs_data_set_string(settings, name, prop2->current_value_str.c_str());
					updateSource = true;
				}
				break;
			}
			}
			prop = prop2;
			break;
		}
		case OBS_PROPERTY_COLOR_ALPHA:
		case OBS_PROPERTY_COLOR: {
			auto prop2        = std::make_shared<obs::ColorProperty>();
			prop2->field_type = obs::NumberProperty::NumberType(obs_property_int_type(p));
			prop2->value      = (int)obs_data_get_int(settings, name);
			prop              = prop2;
			break;
		}
		case OBS_PROPERTY_BUTTON:
			prop = std::make_shared<obs::ButtonProperty>();
			break;
		case OBS_PROPERTY_FONT: {
			auto prop2           = std::make_shared<obs::FontProperty>();
			obs_data_t* font_obj = obs_data_get_obj(settings, name);
			prop2->face          = (buf = obs_data_get_string(font_obj, "face")) != nullptr ? buf : "";
			prop2->style         = (buf = obs_data_get_string(font_obj, "style")) != nullptr ? buf : "";
			prop2->path          = (buf = obs_data_get_string(font_obj, "path")) != nullptr ? buf : "";
			prop2->sizeF         = (int)obs_data_get_int(font_obj, "size");
			prop2->flags         = (uint32_t)obs_data_get_int(font_obj, "flags");
			prop                 = prop2;
			break;
		}
		case OBS_PROPERTY_EDITABLE_LIST: {
			auto prop2              = std::make_shared<obs::EditableListProperty>();
			prop2->field_type       = obs::EditableListProperty::ListType(obs_property_editable_list_type(p));
			prop2->filter           = (buf = obs_property_editable_list_filter(p)) != nullptr ? buf : "";
			prop2->default_path     = (buf = obs_property_editable_list_default_path(p)) != nullptr ? buf : "";

			obs_data_array_t* array = obs_data_get_array(settings, name);
			size_t            count = obs_data_array_count(array);

			for (size_t idx = 0; idx < count; ++idx) {
				obs_data_t* item = obs_data_array_item(array, idx);
				prop2->values.push_back((buf = obs_data_get_string(item, "value")) != nullptr ? buf : "");
			}

			prop                = prop2;
			break;
		}
		case OBS_PROPERTY_FRAME_RATE: {
			auto   prop2      = std::make_shared<obs::FrameRateProperty>();
			size_t num_ranges = obs_property_frame_rate_fps_ranges_count(p);
			for (size_t idx = 0; idx < num_ranges; idx++) {
				auto min = obs_property_frame_rate_fps_range_min(p, idx),
				     max = obs_property_frame_rate_fps_range_max(p, idx);

				obs::FrameRateProperty::Range range;
				range.minimum.first  = min.numerator;
				range.minimum.second = min.denominator;
				range.maximum.first  = max.numerator;
				range.maximum.second = max.denominator;

				prop2->ranges.push_back(std::move(range));
			}

			size_t num_options = obs_property_frame_rate_options_count(p);
			for (size_t idx = 0; idx < num_options; idx++) {
				auto min = obs_property_frame_rate_fps_range_min(p, idx),
				     max = obs_property_frame_rate_fps_range_max(p, idx);

				obs::FrameRateProperty::Option option;
				option.name        = (buf = obs_property_frame_rate_option_name(p, idx)) != nullptr ? buf : "";
				option.description = (buf = obs_property_frame_rate_option_description(p, idx)) != nullptr ? buf : "";

				prop2->options.push_back(std::move(option));
			}

			media_frames_per_second fps = {};
			if (obs_data_get_frames_per_second(settings, name, &fps, nullptr)) {
				prop2->current_numerator = fps.numerator;
				prop2->current_denominator = fps.denominator;
			}

			prop = prop2;
			break;
		}
		case OBS_PROPERTY_GROUP: {
			auto grp = obs_property_group_content(p);
			ProcessProperties(grp, settings, updateSource);
			prop = nullptr;
			break;
		}
		}

		if (!prop) {
			continue;
		}

		prop->name             = obs_property_name(p);
		prop->description      = obs_property_description(p) ? obs_property_description(p) : "";
		prop->long_description = obs_property_long_description(p) ? obs_property_long_description(p) : "";
		prop->enabled          = obs_property_enabled(p);
		prop->visible          = obs_property_visible(p);

		std::vector<char> bufffer(prop->size());
		if (prop->serialize(bufffer)) {
			res.push_back(bufffer);
		}
	}

	return res;
}

std::string obs::Source::GetSettings(obs_source_t* source)
{
	obs_data_t* sets = obs_source_get_settings(source);
	std::string res(obs_data_get_full_json(sets));
	obs_data_release(sets);
	return res;
}

std::string obs::Source::Update(obs_source_t* source, std::string jsonData)
{
	obs_data_t* sets = obs_data_create_from_json(jsonData.c_str());

	if (strcmp(obs_source_get_id(source), "av_capture_input") == 0) {
		const char* frame_rate_string = obs_data_get_string(sets, "frame_rate");
		if (frame_rate_string && strcmp(frame_rate_string, "") != 0) {
			nlohmann::json fps = nlohmann::json::parse(frame_rate_string);
			media_frames_per_second obs_fps = {};
			obs_fps.numerator = fps["numerator"];
			obs_fps.denominator = fps["denominator"];
			obs_data_set_frames_per_second(sets, "frame_rate", obs_fps, nullptr);
		}
	}

	obs_source_update(source, sets);
	obs_data_release(sets);

	obs_data_t* updatedSettings = obs_source_get_settings(source);

	std::string res(obs_data_get_full_json(updatedSettings));
	obs_data_release(updatedSettings);
	return res;
}

void obs::Source::Load(obs_source_t* source)
{
	obs_source_load(source);
}

void obs::Source::Save(obs_source_t* source)
{
	obs_source_save(source);
}

uint32_t obs::Source::GetType(obs_source_t* source)
{
	return obs_source_get_type(source);
}

std::string obs::Source::GetName(obs_source_t* source)
{
	return std::string(obs_source_get_name(source));
}

std::string obs::Source::SetName(obs_source_t* source, std::string name)
{
	obs_source_set_name(source, name.c_str());

	return obs_source_get_name(source);
}

uint32_t obs::Source::GetOutputFlags(obs_source_t* source)
{
	return obs_source_get_output_flags(source);
}

uint32_t obs::Source::GetFlags(obs_source_t* source)
{
	return obs_source_get_flags(source);
}

uint32_t obs::Source::SetFlags(obs_source_t* source, uint32_t flags)
{
	obs_source_set_flags(source, flags);

	return obs_source_get_flags(source);
}

bool obs::Source::GetStatus(obs_source_t* source)
{
	return true;
}

std::string obs::Source::GetId(obs_source_t* source)
{
	const char* sid = obs_source_get_id(source);
	if (sid)
		return std::string(sid);
	else
		return std::string("");
}

bool obs::Source::GetMuted(obs_source_t* source)
{
	return obs_source_muted(source);
}

bool obs::Source::SetMuted(obs_source_t* source, bool muted)
{
	obs_source_set_muted(source, muted);

	return obs_source_muted(source);
}

bool obs::Source::GetEnabled(obs_source_t* source)
{
	return obs_source_enabled(source);
}

bool obs::Source::SetEnabled(obs_source_t* source, bool enabled)
{
	obs_source_set_enabled(source, enabled);

	return obs_source_enabled(source);
}

void obs::Source::SendMouseClick(
	obs_source_t* source, uint32_t modifiers,
	int32_t x, int32_t y, int32_t type,
	bool mouseUp, uint32_t clickCount)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_mouse_event event = {
	    modifiers,
	    x,
	    y,
	};

	obs_source_send_mouse_click(source, &event, type, mouseUp, clickCount);
}

void obs::Source::SendMouseMove(
	obs_source_t* source, uint32_t modifiers,
	int32_t x, int32_t y, bool mouseLeave)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_mouse_event event = {
		modifiers,
		x,
		y
	};

	obs_source_send_mouse_move(source, &event, mouseLeave);}

void obs::Source::SendMouseWheel(
		obs_source_t* source, uint32_t modifiers,
		int32_t x, int32_t y, int32_t x_delta,
		int32_t y_delta)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_mouse_event event = {
		modifiers,
		x,
		y
	};

	obs_source_send_mouse_wheel(source, &event, x_delta, y_delta);
}

void obs::Source::SendFocus(obs_source_t* source, bool focus)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_source_send_focus(source, focus);
}

void obs::Source::SendKeyClick(
    obs_source_t* source, std::string a_text, uint32_t modifiers,
	uint32_t nativeModifiers, uint32_t nativeScancode,
	uint32_t nativeVkey, int32_t keyUp)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	char* text = new char[a_text.size() + 1];
	strcpy(text, a_text.c_str());


	obs_key_event event = {
	    modifiers,
		text,
		nativeModifiers,
		nativeScancode,
		nativeVkey
	};

	obs_source_send_key_click(source, &event, keyUp);

	delete[] text;
}


obs::Source::Manager& obs::Source::Manager::GetInstance()
{
	static obs::Source::Manager _inst;
	return _inst;
}
