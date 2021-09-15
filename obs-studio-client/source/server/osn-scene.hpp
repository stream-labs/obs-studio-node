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

	class Scene : public obs::Source
	{
		public:
		static obs_source_t* Create(std::string name);
		static obs_source_t* CreatePrivate(std::string name);
		static obs_source_t* FromName(std::string name);

		static void Release(obs_source_t* source);
		static void Remove(obs_source_t* source);

		static obs_source_t* Duplicate(obs_source_t*, std::string name, int32_t duplicateType);

		static obs_sceneitem_t* AddSource(obs_source_t* source, obs_source_t* addedSource);
		static obs_sceneitem_t* AddSource(obs_source_t* source, obs_source_t* addedSource, struct TransformInfo transform);
		static obs_sceneitem_t* FindItem(obs_source_t* source, std::string name);
		static obs_sceneitem_t* FindItem(obs_source_t* source, int64_t position);
		static void OrderItems(obs_source_t* source, const std::vector<char> &new_items_order);
		static bool MoveItem(obs_source_t* source, int32_t from, int32_t to);
		static obs_sceneitem_t* GetItem(obs_source_t* source, uint64_t index);
		static std::vector<obs_sceneitem_t*> GetItems(obs_source_t* source);
		static std::vector<obs_sceneitem_t*> GetItemsInRange(obs_source_t* source, uint64_t from, uint64_t to);
	};
}
