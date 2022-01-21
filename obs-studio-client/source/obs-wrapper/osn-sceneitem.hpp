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
#include <obs.h>
#include "obs-utility.hpp"

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
		static obs_source_t* GetSource(obs_sceneitem_t* item);
		static obs_source_t* GetScene(obs_sceneitem_t* item);

		static void Remove(obs_sceneitem_t* item);

		static bool IsVisible(obs_sceneitem_t* item);
		static bool SetVisible(obs_sceneitem_t* item, bool visible);
		static bool IsSelected(obs_sceneitem_t* item);
		static bool SetSelected(obs_sceneitem_t* item, bool selected);
		static bool IsStreamVisible(obs_sceneitem_t* item);
		static bool SetStreamVisible(obs_sceneitem_t* item, bool streamVisible);
		static bool IsRecordingVisible(obs_sceneitem_t* item);
		static bool SetRecordingVisible(obs_sceneitem_t* item, bool recordingVisible);
		static std::pair<float_t, float_t> GetPosition(obs_sceneitem_t* item);
		static std::pair<float_t, float_t> SetPosition(obs_sceneitem_t* item, float_t x, float_t y);
		static float_t GetRotation(obs_sceneitem_t* item);
		static float_t SetRotation(obs_sceneitem_t* item, float_t rotation);
		static std::pair<float_t, float_t> GetScale(obs_sceneitem_t* item);
		static std::pair<float_t, float_t> SetScale(obs_sceneitem_t* item, float_t scaleX, float_t scaleY);
		static uint32_t GetAlignment(obs_sceneitem_t* item);
		static uint32_t SetAlignment(obs_sceneitem_t* item, uint32_t align);
		static std::pair<float_t, float_t> GetBounds(obs_sceneitem_t* item);
		static std::pair<float_t, float_t> SetBounds(obs_sceneitem_t* item, float_t boundsX, float_t boundsY);
		static uint32_t GetBoundsAlignment(obs_sceneitem_t* item);
		static uint32_t SetBoundsAlignment(obs_sceneitem_t* item, uint32_t aligment);
		static uint32_t GetBoundsType(obs_sceneitem_t* item);
		static uint32_t SetBoundsType(obs_sceneitem_t* item, uint32_t boundsType);
		static obs_sceneitem_crop GetCrop(obs_sceneitem_t* item);
		static obs_sceneitem_crop SetCrop(obs_sceneitem_t* item, obs_sceneitem_crop crop);
		static uint32_t GetScaleFilter(obs_sceneitem_t* item);
		static uint32_t SetScaleFilter(obs_sceneitem_t* item, uint32_t scaleFilter);
		static int64_t GetId(obs_sceneitem_t* item);

		static void MoveUp(obs_sceneitem_t* item);
		static void MoveDown(obs_sceneitem_t* item);
		static void MoveTop(obs_sceneitem_t* item);
		static void MoveBottom(obs_sceneitem_t* item);
		static void Move(obs_sceneitem_t* item, int32_t position);

		static void DeferUpdateBegin(obs_sceneitem_t* item);
		static void DeferUpdateEnd(obs_sceneitem_t* item);
	};
}
