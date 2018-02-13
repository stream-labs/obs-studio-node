// Server program for the OBS Studio node module.
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

#pragma once
#include <inttypes.h>
#include <list>

namespace utility {
	class unique_id {
		typedef std::pair<uint64_t, uint64_t> range_t;

		public:
		unique_id();
		virtual ~unique_id();

		uint64_t allocate();
		void free(uint64_t);

		bool is_allocated(uint64_t);
		uint64_t count(bool count_free);

		protected:
		bool mark_used(uint64_t);
		void mark_used_range(uint64_t, uint64_t);
		bool mark_free(uint64_t);
		void mark_free_range(uint64_t, uint64_t);

		private:
		std::list<range_t> allocated;
	};
}
