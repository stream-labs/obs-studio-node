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

#include "osn-sceneitem.hpp"
#include <error.hpp>
#include "osn-source.hpp"
#include "shared-server.hpp"

obs_source_t* obs::SceneItem::GetSource(obs_sceneitem_t* item)
{
	obs_source_t* source = obs_sceneitem_get_source(item);
	if (!source) {
		blog(LOG_ERROR, "Item does not contain a source.");
		return nullptr;
	}

	return source;
}

obs_source_t* obs::SceneItem::GetScene(obs_sceneitem_t* item)
{
	obs_scene_t* scene = obs_sceneitem_get_scene(item);
	if (!scene) {
		blog(LOG_ERROR, "Item does not contain a source.");
		return nullptr;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		blog(LOG_ERROR, "Scene is invalid.");
		return nullptr;
	}

	return  source;
}

void obs::SceneItem::Remove(obs_sceneitem_t* item)
{
	obs_sceneitem_release(item);
	obs_sceneitem_remove(item);
}

bool obs::SceneItem::IsVisible(obs_sceneitem_t* item)
{
	return obs_sceneitem_visible(item);
}

bool obs::SceneItem::SetVisible(obs_sceneitem_t* item, bool visible)
{
	obs_sceneitem_set_visible(item, visible);

	return obs_sceneitem_visible(item);
}

bool obs::SceneItem::IsSelected(obs_sceneitem_t* item)
{
	return obs_sceneitem_selected(item);
}

bool obs::SceneItem::SetSelected(obs_sceneitem_t* item, bool selected)
{
	obs_sceneitem_select(item, selected);

	return obs_sceneitem_selected(item);
}

bool obs::SceneItem::IsStreamVisible(obs_sceneitem_t* item)
{
	return obs_sceneitem_stream_visible(item);
}

bool obs::SceneItem::SetStreamVisible(obs_sceneitem_t* item, bool streamVisible)
{
	obs_sceneitem_set_stream_visible(item, streamVisible);

	return obs_sceneitem_stream_visible(item);
}

bool obs::SceneItem::IsRecordingVisible(obs_sceneitem_t* item)
{
	return obs_sceneitem_recording_visible(item);
}

bool obs::SceneItem::SetRecordingVisible(obs_sceneitem_t* item, bool recordingVisible)
{
	obs_sceneitem_set_recording_visible(item, recordingVisible);

	return obs_sceneitem_recording_visible(item);
}

std::pair<float_t, float_t> obs::SceneItem::GetPosition(obs_sceneitem_t* item)
{
	vec2 pos;
	obs_sceneitem_get_pos(item, &pos);
	return std::make_pair(pos.x, pos.y);
}

std::pair<float_t, float_t> obs::SceneItem::SetPosition(obs_sceneitem_t* item, float_t x, float_t y)
{
	vec2 pos;
	pos.x = x;
	pos.y = y;

	obs_sceneitem_set_pos(item, &pos);
	obs_sceneitem_get_pos(item, &pos);

	return std::make_pair(pos.x, pos.y);
}

float_t obs::SceneItem::GetRotation(obs_sceneitem_t* item)
{
	return obs_sceneitem_get_rot(item);
}

float_t obs::SceneItem::SetRotation(obs_sceneitem_t* item, float_t rotation)
{
	obs_sceneitem_set_rot(item, rotation);

	return obs_sceneitem_get_rot(item);;
}

std::pair<float_t, float_t> obs::SceneItem::GetScale(obs_sceneitem_t* item)
{
	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);

	return std::make_pair(scale.x, scale.y);
}

std::pair<float_t, float_t> obs::SceneItem::SetScale(obs_sceneitem_t* item, float_t scaleX, float_t scaleY)
{
	vec2 scale;
	scale.x = scaleX;
	scale.y = scaleY;

	obs_sceneitem_set_scale(item, &scale);
	obs_sceneitem_get_scale(item, &scale);

	return std::make_pair(scale.x, scale.y);
}

uint32_t obs::SceneItem::GetScaleFilter(obs_sceneitem_t* item)
{
	return obs_sceneitem_get_scale_filter(item);
}

uint32_t obs::SceneItem::SetScaleFilter(obs_sceneitem_t* item, uint32_t scaleFilter)
{
	obs_sceneitem_set_scale_filter(item, (obs_scale_type)scaleFilter);

	return obs_sceneitem_get_scale_filter(item);
}

uint32_t obs::SceneItem::GetAlignment(obs_sceneitem_t* item)
{
	return obs_sceneitem_get_alignment(item);
}

uint32_t obs::SceneItem::SetAlignment(obs_sceneitem_t* item, uint32_t align)
{
	obs_sceneitem_set_alignment(item, align);

	return obs_sceneitem_get_alignment(item);
}

std::pair<float_t, float_t> obs::SceneItem::GetBounds(obs_sceneitem_t* item)
{
	vec2 bounds;
	obs_sceneitem_get_bounds(item, &bounds);

	return std::make_pair(bounds.x, bounds.y);
}

std::pair<float_t, float_t>
	obs::SceneItem::SetBounds(obs_sceneitem_t* item, float_t boundsX, float_t boundsY)
{
	vec2 bounds;
	bounds.x = boundsX;
	bounds.y = boundsY;

	obs_sceneitem_get_bounds(item, &bounds);

	return std::make_pair(bounds.x, bounds.y);
}

uint32_t obs::SceneItem::GetBoundsAlignment(obs_sceneitem_t* item)
{
	return obs_sceneitem_get_bounds_alignment(item);
}

uint32_t obs::SceneItem::SetBoundsAlignment(obs_sceneitem_t* item, uint32_t aligment)
{
	obs_sceneitem_set_bounds_alignment(item, aligment);

	return obs_sceneitem_get_bounds_alignment(item);
}

uint32_t obs::SceneItem::GetBoundsType(obs_sceneitem_t* item)
{
	return obs_sceneitem_get_bounds_type(item);
}

uint32_t obs::SceneItem::SetBoundsType(obs_sceneitem_t* item, uint32_t boundsType)
{
	obs_sceneitem_set_bounds_type(item, (obs_bounds_type)boundsType);
	return obs_sceneitem_get_bounds_type(item);
}

obs_sceneitem_crop obs::SceneItem::GetCrop(obs_sceneitem_t* item)
{
	obs_sceneitem_crop crop;
	obs_sceneitem_get_crop(item, &crop);

	return crop;
}

obs_sceneitem_crop obs::SceneItem::SetCrop(obs_sceneitem_t* item, obs_sceneitem_crop crop)
{
	obs_sceneitem_set_crop(item, &crop);
	obs_sceneitem_get_crop(item, &crop);

	return crop;
}

int64_t obs::SceneItem::GetId(obs_sceneitem_t* item)
{
	return obs_sceneitem_get_id(item);
}

void obs::SceneItem::MoveUp(obs_sceneitem_t* item)
{
	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_UP);
}

void obs::SceneItem::MoveDown(obs_sceneitem_t* item)
{
	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_DOWN);
}

void obs::SceneItem::MoveTop(obs_sceneitem_t* item)
{
	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_TOP);
}

void obs::SceneItem::MoveBottom(obs_sceneitem_t* item)
{
	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_BOTTOM);
}

void obs::SceneItem::Move(obs_sceneitem_t* item, int32_t position)
{
	obs_sceneitem_set_order_position(item, position);
}

void obs::SceneItem::DeferUpdateBegin(obs_sceneitem_t* item)
{
	obs_sceneitem_defer_update_begin(item);
}

void obs::SceneItem::DeferUpdateEnd(obs_sceneitem_t* item)
{
	obs_sceneitem_defer_update_end(item);
}

obs::SceneItem::Manager& obs::SceneItem::Manager::GetInstance()
{
	// Thread Safe since C++13 (Visual Studio 2015, GCC 4.3).
	static Manager instance;
	return instance;
}
