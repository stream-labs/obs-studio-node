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

namespace obs
{
	class SceneItem
	{
		public:
		class Manager : public utility_server::unique_object_manager<obs_sceneitem_t>
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
		static uint64_t GetSource(uint64_t itemId);
		static uint64_t GetScene(uint64_t itemId);

		static void Remove(uint64_t itemId);

		static bool IsVisible(uint64_t itemId);
		static bool SetVisible(uint64_t itemId, bool visible);
		static bool IsSelected(uint64_t itemId);
		static bool SetSelected(uint64_t itemId, bool selected);
		static bool IsStreamVisible(uint64_t itemId);
		static bool SetStreamVisible(uint64_t itemId, bool streamVisible);
		static bool IsRecordingVisible(uint64_t itemId);
		static bool SetRecordingVisible(uint64_t itemId, bool recordingVisible);
		static std::pair<float_t, float_t> GetPosition(uint64_t itemId);
		static std::pair<float_t, float_t> SetPosition(uint64_t itemId, float_t x, float_t y);
		static float_t GetRotation(uint64_t itemId);
		static float_t SetRotation(uint64_t itemId, float_t rotation);
		static std::pair<float_t, float_t> GetScale(uint64_t itemId);
		static std::pair<float_t, float_t> SetScale(uint64_t itemId, float_t scaleX, float_t scaleY);
		static uint32_t GetAlignment(uint64_t itemId);
		static uint32_t SetAlignment(uint64_t itemId, uint32_t align);
		static std::pair<float_t, float_t> GetBounds(uint64_t itemId);
		static std::pair<float_t, float_t> SetBounds(uint64_t itemId, float_t boundsX, float_t boundsY);
		static uint32_t GetBoundsAlignment(uint64_t itemId);
		static uint32_t SetBoundsAlignment(uint64_t itemId, uint32_t aligment);
		static uint32_t GetBoundsType(uint64_t itemId);
		static uint32_t SetBoundsType(uint64_t itemId, uint32_t boundsType);
		static obs_sceneitem_crop GetCrop(uint64_t itemId);
		static obs_sceneitem_crop SetCrop(uint64_t itemId, obs_sceneitem_crop crop);
		static uint32_t GetScaleFilter(uint64_t itemId);
		static uint32_t SetScaleFilter(uint64_t itemId, uint32_t scaleFilter);
		static int64_t GetId(uint64_t itemId);

		static void MoveUp(uint64_t itemId);
		static void MoveDown(uint64_t itemId);
		static void MoveTop(uint64_t itemId);
		static void MoveBottom(uint64_t itemId);
		static void Move(uint64_t itemId, int32_t position);

		static void DeferUpdateBegin(uint64_t itemId);
		static void DeferUpdateEnd(uint64_t itemId);
	};
}
