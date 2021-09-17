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

#include "osn-scene.hpp"
#include <list>
#include "error.hpp"
#include "osn-sceneitem.hpp"
#include "shared-server.hpp"

obs_source_t* obs::Scene::Create(std::string name)
{
	obs_scene_t* scene = obs_scene_create(name.c_str());
	if (!scene) {
		blog(LOG_ERROR, "Failed to create scene.");
		return nullptr;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		blog(LOG_ERROR, "Failed to get source from scene.");
		return nullptr;
	}

	return source;
}

obs_source_t* obs::Scene::CreatePrivate(std::string name)
{
	obs_scene_t* scene = obs_scene_create_private(name.c_str());
	if (!scene) {
		blog(LOG_ERROR, "Failed to create scene.");
		return nullptr;
	}

	obs_source_t* source = obs_scene_get_source(scene);
	if (!source) {
		blog(LOG_ERROR, "Failed to get source from scene.");
		return nullptr;
	}

	return source;
}

obs_source_t* obs::Scene::FromName(std::string name)
{
	obs_source_t* source = obs_get_source_by_name(name.c_str());
	if (!source) {
		blog(LOG_ERROR, "Failed to get source from scene.");
		return nullptr;
	}

	return source;
}

void obs::Scene::Release(obs_source_t* source)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return;
	}

	std::list<obs_sceneitem_t*> items;
	auto                        cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
        std::list<obs_sceneitem_t*>* items = reinterpret_cast<std::list<obs_sceneitem_t*>*>(data);
        obs_sceneitem_addref(item);
        items->push_back(item);
        return true;
	};
	obs_scene_enum_items(scene, cb, &items);

	obs_source_release(source);

	for (auto item : items) {
		obs_sceneitem_release(item);
	}
}

void obs::Scene::Remove(obs_source_t* source)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return;
	}

	std::list<obs_sceneitem_t*> items;
	auto                        cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
        std::list<obs_sceneitem_t*>* items = reinterpret_cast<std::list<obs_sceneitem_t*>*>(data);
        obs_sceneitem_addref(item);
        items->push_back(item);
        return true;
	};
	obs_scene_enum_items(scene, cb, &items);

	obs_source_remove(source);

	for (auto item : items) {
		obs_sceneitem_release(item);
		obs_sceneitem_release(item);
	}
}

obs_source_t* obs::Scene::Duplicate(obs_source_t* source, std::string name, int32_t duplicateType)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return nullptr;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return nullptr;
	}

	obs_scene_t* scene2 =
	    obs_scene_duplicate(scene, name.c_str(), (obs_scene_duplicate_type)duplicateType);
	if (!scene2) {
		blog(LOG_ERROR, "Failed to duplicate scene.");
		return nullptr;
	}

	obs_source_t* source2 = obs_scene_get_source(scene2);
	if (!source2) {
		blog(LOG_ERROR, "Failed to get source from duplicate scene.");
		return nullptr;
	}

	return source2;
}

obs_sceneitem_t* obs::Scene::AddSource(obs_source_t* source, obs_source_t* addedSource)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return nullptr;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return nullptr;
	}

	if (!addedSource) {
		blog(LOG_ERROR, "Source reference to add is not valid.");
		return nullptr;
	}

	obs_sceneitem_t* item = obs_scene_add(scene, addedSource);

	obs_sceneitem_addref(item);

	return item;
}

obs_sceneitem_t* obs::Scene::AddSource(obs_source_t* source, obs_source_t* addedSource, struct TransformInfo transform)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return nullptr;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return nullptr;
	}

	if (!addedSource) {
		blog(LOG_ERROR, "Source reference to add is not valid.");
		return nullptr;
	}

	obs_sceneitem_t* item = obs_scene_add(scene, addedSource);

	vec2 scale;
	scale.x = transform.scaleX;
	scale.y = transform.scaleY;
	obs_sceneitem_set_scale(item, &scale);

	obs_sceneitem_set_visible(item, transform.visible);

	vec2 pos;
	pos.x = transform.positionX;
	pos.y = transform.positionY;
	obs_sceneitem_set_pos(item, &pos);

	obs_sceneitem_set_rot(item, transform.rotation);

	obs_sceneitem_crop crop;
	crop.left   = transform.cropLeft;
	crop.top    = transform.cropTop;
	crop.right  = transform.cropRight;
	crop.bottom = transform.cropBottom;

	obs_sceneitem_set_crop(item, &crop);

	obs_sceneitem_set_stream_visible(item , transform.streamVisible);
	obs_sceneitem_set_recording_visible(item , transform.recordingVisible);

	obs_sceneitem_addref(item);

	return item;
}

obs_sceneitem_t* obs::Scene::FindItem(obs_source_t* source, std::string name)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return nullptr;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return nullptr;
	}

	obs_sceneitem_t* item = obs_scene_find_source(scene, name.c_str());
	if (!item) {
		blog(LOG_ERROR, "Source not found.");
		return nullptr;
	}

	return item;
}

obs_sceneitem_t* obs::Scene::FindItem(obs_source_t* source, int64_t position)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return nullptr;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return nullptr;
	}

	obs_sceneitem_t* item = obs_scene_find_sceneitem_by_id(scene, position);
	if (!item) {
		blog(LOG_ERROR, "Source not found.");
		return nullptr;
	}

	return item;
}

void obs::Scene::OrderItems(obs_source_t* source, const std::vector<char> &new_items_order)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return;
	}

	size_t items_count = new_items_order.size()/sizeof(int64_t);

    obs_scene_set_items_order(scene, (int64_t*)new_items_order.data(), items_count);
}

bool obs::Scene::MoveItem(obs_source_t* source, int32_t from, int32_t to)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return false;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return false;
	}

	// Listen up! This function does not work as you might expect it to (lowest index is furthest
	//  back), instead it works on the inverted order, so the lowest index is the furthest in front.
	// While this may be weird at first, this does have some advantages as you do not have to guess
	//  on what the best index for moving something in front is. Downside is that it is kinda weird
	//  to deal with when you're used to things being the other way around.

	size_t num_items = 0;
	auto   cb_count  = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
        (*(reinterpret_cast<size_t*>(data)))++;
        return true;
	};
	obs_scene_enum_items(scene, cb_count, &num_items);

	struct EnumData
	{
		obs_sceneitem_t* item      = nullptr;
		int32_t          findindex = 0;
		int32_t          index     = 0;
	} ed;
	ed.findindex = (int32_t(num_items) - 1) - from;

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* items = reinterpret_cast<EnumData*>(data);
		if (items->index == items->findindex) {
			items->item = item;
			return false;
		}
		items->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	if (!ed.item) {
		blog(LOG_ERROR, "Index not found in Scene.");
		return false;
	}

	obs_sceneitem_set_order_position(ed.item, (int(num_items) - 1) - to);

	return true;
}

obs_sceneitem_t* obs::Scene::GetItem(obs_source_t* source, uint64_t index)
{
	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return nullptr;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return nullptr;
	}

	struct EnumData
	{
		obs_sceneitem_t* item      = nullptr;
		size_t           findindex = 0;
		size_t           index     = 0;
	} ed;
	ed.findindex = index;

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* items = reinterpret_cast<EnumData*>(data);
		if (items->index == items->findindex) {
			items->item = item;
			return false;
		}
		items->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	if (!ed.item) {
		blog(LOG_ERROR, "Index not found in Scene.");
		return nullptr;
	}

	return ed.item;
}

std::vector<obs_sceneitem_t*> obs::Scene::GetItems(obs_source_t* source)
{
	std::vector<obs_sceneitem_t*> items;

	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return items;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return items;
	}

	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		std::vector<obs_sceneitem_t*>* items = reinterpret_cast<std::vector<obs_sceneitem_t*>*>(data);
        items->push_back(item);
        return true;
	};
	obs_scene_enum_items(scene, cb, &items);

	return items;
}

std::vector<obs_sceneitem_t*> obs::Scene::GetItemsInRange(obs_source_t* source, uint64_t from, uint64_t to)
{
	struct EnumData
	{
		std::vector<obs_sceneitem_t*> items;
		size_t                      index_from = 0, index_to = 0;
		size_t                      index = 0;
	} ed;
	ed.index_from = from;
	ed.index_to   = to;

	if (!source) {
		blog(LOG_ERROR, "Source reference is not valid.");
		return ed.items;
	}

	obs_scene_t* scene = obs_scene_from_source(source);
	if (!scene) {
		blog(LOG_ERROR, "Source reference is not a scene.");
		return ed.items;
	}


	auto cb = [](obs_scene_t* scene, obs_sceneitem_t* item, void* data) {
		EnumData* ed = reinterpret_cast<EnumData*>(data);
		if ((ed->index >= ed->index_from) && (ed->index <= ed->index_to)) {
			ed->items.push_back(item);
		} else if (ed->index > ed->index_to) {
			return false;
		}
		ed->index++;
		return true;
	};
	obs_scene_enum_items(scene, cb, &ed);

	return ed.items;
}
