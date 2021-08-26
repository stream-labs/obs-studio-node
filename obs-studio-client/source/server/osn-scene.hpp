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
#include "osn-source.hpp"

namespace obs
{
	struct TransformInfo {
		float_t scaleX = 0.0;
		float_t scaleY = 0.0;
		bool visible = false;
		float_t positionX = 0.0;
		float_t positionY = 0.0;
		float_t rotation = 0.0;
		int64_t cropLeft = 0;
		int64_t cropTop = 0;
		int64_t cropRight = 0;
		int64_t cropBottom = 0;
		bool streamVisible = false;
		bool recordingVisible = false;
	};

	class Scene : public osn::Source
	{
		public:
		static uint64_t Create(std::string name);
		static uint64_t CreatePrivate(std::string name);
		static uint64_t FromName(std::string name);

		static void Release(uint64_t uid);
		static void Remove(uint64_t uid);

		static uint64_t Duplicate(uint64_t sourceId, std::string name, int32_t duplicateType);

		static std::pair<uint64_t, int64_t> AddSource(uint64_t uid, uint64_t sourceId);
		static std::pair<uint64_t, int64_t> AddSource(uint64_t uid, uint64_t sourceId, struct TransformInfo transform);
		static uint64_t FindItem(uint64_t sourceid, std::string name);
		static uint64_t FindItem(uint64_t sourceid, int64_t position);
		static std::vector<std::pair<uint64_t, int64_t>>
		    OrderItems(uint64_t uid, const std::vector<char> &new_items_order);
		static std::vector<std::pair<uint64_t, int64_t>>
		    MoveItem(uint64_t uid, int32_t from, int32_t to);
		static uint64_t GetItem(uint64_t sourceId, uint64_t index);
		static std::vector<std::pair<uint64_t, int64_t>> GetItems(uint64_t sourceId);
		static std::vector<uint64_t> GetItemsInRange(uint64_t sourceId, uint64_t from, uint64_t to);
	};
}
