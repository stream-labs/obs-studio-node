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

#pragma once
#include <ipc-server.hpp>
#include <obs.h>
#include "utility-server.hpp"
#undef strtoll
#include "nlohmann/json.hpp"

namespace obs
{
	class Source
	{
		public:
		class Manager : public utility_server::unique_object_manager<obs_source_t>
		{
			friend class std::shared_ptr<Manager>;

			protected:
			Manager() {}
			~Manager() {}

			public:
			Manager(Manager const&) = delete;
			Manager operator=(Manager const&) = delete;

			public:
			static Manager& GetInstance();
		};

		static void initialize_global_signals();
		static void finalize_global_signals();
		static void global_source_create_cb(void* ptr, calldata_t* cd);
		static void global_source_activate_cb(void* ptr, calldata_t* cd);
		static void global_source_deactivate_cb(void* ptr, calldata_t* cd);
		static void global_source_destroy_cb(void* ptr, calldata_t* cd);

		static void attach_source_signals(obs_source_t* src);
		static void detach_source_signals(obs_source_t* src);

		public:
		// References
		static void Remove(obs_source_t* source);
		static void Release(obs_source_t* source);

		// Settings & Properties
		static bool IsConfigurable(obs_source_t* source);
		static std::vector<std::vector<char>> GetProperties(obs_source_t* source);
		static std::vector<std::vector<char>> ProcessProperties(
		    obs_properties_t*              prp,
		    obs_data*                      settings,
		    bool&                          updateSource);
		static std::string GetSettings(obs_source_t* source);
		static std::string Update(obs_source_t* source, std::string jsonData);
		static void Load(obs_source_t* source);
		static void Save(obs_source_t* source);

		static uint32_t GetType(obs_source_t* source);
		static std::string GetName(obs_source_t* source);
		static std::string SetName(obs_source_t* source, std::string name);
		static uint32_t GetOutputFlags(obs_source_t* source);
		static uint32_t GetFlags(obs_source_t* source);
		static uint32_t SetFlags(obs_source_t* source, uint32_t flags);
		static bool GetStatus(obs_source_t* source);
		static std::string GetId(obs_source_t* source);

		// Flags
		static bool GetMuted(obs_source_t* source);
		static bool SetMuted(obs_source_t* source, bool muted);
		static bool GetEnabled(obs_source_t* source);
		static bool SetEnabled(obs_source_t* source, bool enabled);

		// Browser source interaction
		static void SendMouseClick(
				obs_source_t* source, uint32_t modifiers,
				int32_t x, int32_t y, int32_t type,
				bool mouseUp, uint32_t clickCount);
		static void SendMouseMove(
		    obs_source_t* source, uint32_t modifiers,
			int32_t x, int32_t y, bool mouseLeave);
		static void SendMouseWheel(
		    obs_source_t* source, uint32_t modifiers,
			int32_t x, int32_t y, int32_t x_delta,
			int32_t y_delta);
		static void SendFocus(obs_source_t* source, bool focus);
		static void SendKeyClick(
		    obs_source_t* source, std::string a_text, uint32_t modifiers,
			uint32_t nativeModifiers, uint32_t nativeScancode,
			uint32_t nativeVkey, int32_t keyUp);
	};
}
