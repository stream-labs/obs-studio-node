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
#include <memory>
#include <queue>
#include <array>
#include "obs.h"
#include "utility-server.hpp"

#define MAKE_FLOAT_SANE(db) (std::isfinite(db) ? db : (db > 0 ? 0.0f : -65535.0f))

extern std::mutex mtx;

namespace obs
{
	class Volmeter
	{
		public:
		class Manager : public utility_server::generic_object_manager<std::shared_ptr<Volmeter>>
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

		public:
		obs_volmeter_t* self;
		uint64_t        id;
		size_t          callback_count = 0;
		uint64_t*       id2            = nullptr;
		uint64_t        uid_source     = 0;
		void*           m_jsThread     = nullptr;
		bool            cbReady;

		std::chrono::steady_clock::time_point lastProcessed;

		public:
		Volmeter(obs_fader_type type);
		~Volmeter();

		public:
		static std::pair<uint64_t, uint32_t> Create(int32_t a_type);
		static void Destroy(uint64_t uid);

		static void Attach(uint64_t uid_fader, uint64_t uid_source);
		static void Detach(uint64_t uid);
		static void AddCallback(
			uint64_t uid,
			obs_volmeter_updated_t callback,
			void* jsThread);
		static void RemoveCallback(
			uint64_t uid,
			obs_volmeter_updated_t callback);
	};
}