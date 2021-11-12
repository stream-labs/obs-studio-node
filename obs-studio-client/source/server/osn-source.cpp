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
#include <map>
#include <memory>
#include <obs-data.h>
#include <obs.h>
#include <obs.hpp>
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
