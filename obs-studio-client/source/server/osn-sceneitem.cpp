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

uint64_t obs::SceneItem::GetSource(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT64_MAX;
	}

	obs_source_t* source = obs_sceneitem_get_source(item);
	if (!source) {
		blog(LOG_ERROR, "Item does not contain a source.");
		return UINT64_MAX;
	}

	return osn::Source::Manager::GetInstance().find(source);
}

uint64_t obs::SceneItem::GetScene(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT64_MAX;
	}

	obs_scene_t* scene = obs_sceneitem_get_scene(item);
	if (!scene) {
		blog(LOG_ERROR, "Item does not contain a source.");
		return UINT64_MAX;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		blog(LOG_ERROR, "Scene is invalid.");
		return UINT64_MAX;
	}

	return  osn::Source::Manager::GetInstance().find(source);
}

void obs::SceneItem::Remove(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs::SceneItem::Manager::GetInstance().free(itemId);
	obs_sceneitem_release(item);
	obs_sceneitem_remove(item);
}

bool obs::SceneItem::IsVisible(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	return obs_sceneitem_visible(item);
}

bool obs::SceneItem::SetVisible(uint64_t itemId, bool visible)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	obs_sceneitem_set_visible(item, visible);

	return obs_sceneitem_visible(item);
}

bool obs::SceneItem::IsSelected(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	return obs_sceneitem_selected(item);
}

bool obs::SceneItem::SetSelected(uint64_t itemId, bool selected)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	obs_sceneitem_select(item, selected);

	return obs_sceneitem_selected(item);
}

bool obs::SceneItem::IsStreamVisible(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	return obs_sceneitem_stream_visible(item);
}

bool obs::SceneItem::SetStreamVisible(uint64_t itemId, bool streamVisible)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	obs_sceneitem_set_stream_visible(item, streamVisible);

	return obs_sceneitem_stream_visible(item);
}

bool obs::SceneItem::IsRecordingVisible(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	return obs_sceneitem_recording_visible(item);
}

bool obs::SceneItem::SetRecordingVisible(uint64_t itemId, bool recordingVisible)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return false;
	}

	obs_sceneitem_set_recording_visible(item, recordingVisible);

	return obs_sceneitem_recording_visible(item);
}

std::pair<float_t, float_t> obs::SceneItem::GetPosition(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return std::make_pair(0.0, 0.0);
	}

	vec2 pos;
	obs_sceneitem_get_pos(item, &pos);
	return std::make_pair(pos.x, pos.y);
}

std::pair<float_t, float_t> obs::SceneItem::SetPosition(uint64_t itemId, float_t x, float_t y)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return std::make_pair(0.0, 0.0);
	}

	vec2 pos;
	pos.x = x;
	pos.y = y;

	obs_sceneitem_set_pos(item, &pos);
	obs_sceneitem_get_pos(item, &pos);

	return std::make_pair(pos.x, pos.y);
}

float_t obs::SceneItem::GetRotation(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return 0.0;
	}

	return obs_sceneitem_get_rot(item);
}

float_t obs::SceneItem::SetRotation(uint64_t itemId, float_t rotation)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return 0.0;
	}

	obs_sceneitem_set_rot(item, rotation);

	return obs_sceneitem_get_rot(item);;
}

std::pair<float_t, float_t> obs::SceneItem::GetScale(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return std::make_pair(0.0, 0.0);
	}

	vec2 scale;
	obs_sceneitem_get_scale(item, &scale);

	return std::make_pair(scale.x, scale.y);
}

std::pair<float_t, float_t> obs::SceneItem::SetScale(uint64_t itemId, float_t scaleX, float_t scaleY)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return std::make_pair(0.0, 0.0);
	}

	vec2 scale;
	scale.x = scaleX;
	scale.y = scaleY;

	obs_sceneitem_set_scale(item, &scale);
	obs_sceneitem_get_scale(item, &scale);

	return std::make_pair(scale.x, scale.y);
}

uint32_t obs::SceneItem::GetScaleFilter(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	return obs_sceneitem_get_scale_filter(item);
}

uint32_t obs::SceneItem::SetScaleFilter(uint64_t itemId, uint32_t scaleFilter)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	obs_sceneitem_set_scale_filter(item, (obs_scale_type)scaleFilter);

	return obs_sceneitem_get_scale_filter(item);
}

uint32_t obs::SceneItem::GetAlignment(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	return obs_sceneitem_get_alignment(item);
}

uint32_t obs::SceneItem::SetAlignment(uint64_t itemId, uint32_t align)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	obs_sceneitem_set_alignment(item, align);

	return obs_sceneitem_get_alignment(item);
}

std::pair<float_t, float_t> obs::SceneItem::GetBounds(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return std::make_pair(0.0, 0.0);
	}

	vec2 bounds;
	obs_sceneitem_get_bounds(item, &bounds);

	return std::make_pair(bounds.x, bounds.y);
}

std::pair<float_t, float_t>
	obs::SceneItem::SetBounds(uint64_t itemId, float_t boundsX, float_t boundsY)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return std::make_pair(0.0, 0.0);
	}

	vec2 bounds;
	bounds.x = boundsX;
	bounds.y = boundsY;

	obs_sceneitem_get_bounds(item, &bounds);

	return std::make_pair(bounds.x, bounds.y);
}

uint32_t obs::SceneItem::GetBoundsAlignment(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	return obs_sceneitem_get_bounds_alignment(item);
}

uint32_t obs::SceneItem::SetBoundsAlignment(uint64_t itemId, uint32_t aligment)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	obs_sceneitem_set_bounds_alignment(item, aligment);

	return obs_sceneitem_get_bounds_alignment(item);
}

uint32_t obs::SceneItem::GetBoundsType(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	return obs_sceneitem_get_bounds_type(item);
}

uint32_t obs::SceneItem::SetBoundsType(uint64_t itemId, uint32_t boundsType)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return UINT32_MAX;
	}

	obs_sceneitem_set_bounds_type(item, (obs_bounds_type)boundsType);
	return obs_sceneitem_get_bounds_type(item);
}

obs_sceneitem_crop obs::SceneItem::GetCrop(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return { 0 };
	}

	obs_sceneitem_crop crop;
	obs_sceneitem_get_crop(item, &crop);

	return crop;
}

obs_sceneitem_crop obs::SceneItem::SetCrop(uint64_t itemId, obs_sceneitem_crop crop)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return { 0 };
	}

	obs_sceneitem_set_crop(item, &crop);
	obs_sceneitem_get_crop(item, &crop);

	return crop;
}

int64_t obs::SceneItem::GetId(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return INT64_MAX;
	}

	return obs_sceneitem_get_id(item);
}

void obs::SceneItem::MoveUp(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_UP);
}

void obs::SceneItem::MoveDown(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_DOWN);
}

void obs::SceneItem::MoveTop(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_TOP);
}

void obs::SceneItem::MoveBottom(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs_sceneitem_set_order(item, OBS_ORDER_MOVE_BOTTOM);
}

void obs::SceneItem::Move(uint64_t itemId, int32_t position)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs_sceneitem_set_order_position(item, position);
}

void obs::SceneItem::DeferUpdateBegin(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs_sceneitem_defer_update_begin(item);
}

void obs::SceneItem::DeferUpdateEnd(uint64_t itemId)
{
	obs_sceneitem_t* item = obs::SceneItem::Manager::GetInstance().find(itemId);
	if (!item) {
		blog(LOG_ERROR, "Item reference is not valid.");
		return;
	}

	obs_sceneitem_defer_update_end(item);
}

obs::SceneItem::Manager& obs::SceneItem::Manager::GetInstance()
{
	// Thread Safe since C++13 (Visual Studio 2015, GCC 4.3).
	static Manager instance;
	return instance;
}
