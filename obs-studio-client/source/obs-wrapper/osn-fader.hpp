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
#include <memory>
#include "obs.h"
#include "obs-utility.hpp"

namespace obs
{
	class Fader
	{
		public:
		class Manager : public utility_server::unique_object_manager<obs_fader_t>
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
		static void ClearFaders();

		static uint64_t Create(int32_t fader_type);
		static void Destroy(uint64_t uid);

		static float_t GetDeziBel(uint64_t uid);
		static float_t SetDeziBel(uint64_t uid, float_t db);
		static float_t GetDeflection(uint64_t uid);
		static float_t SetDeflection(uint64_t uid, float_t deflection);
		static float_t GetMultiplier(uint64_t uid);
		static float_t SetMultiplier(uint64_t uid, float_t mul);
		static void Attach(uint64_t uid_fader, obs_source_t* source);
		static void Detach(uint64_t uid);
	};
} // namespace osn
