#pragma once

#include "obspp-scene.hpp"

namespace obs {

scene::scene(std::string name, bool is_private)
{
    if (is_private) 
        m_scene = obs_scene_create_private(name.c_str());
    else 
        m_scene = obs_scene_create(name.c_str());
        
    m_status = status_type::okay;

    if (!m_scene) {
		m_status = status_type::invalid;
	}

    /* Will return NULL if m_scene is invalid */
    m_handle = obs_scene_get_source(m_scene);
}

scene::scene(scene &copy)
    : m_scene(copy.m_scene),
      obs::source(copy.m_handle)
{
}

scene::scene(obs_scene_t *scene)
 : m_scene(scene)
{
    m_status = status_type::okay;

    if (!scene)
        m_status = status_type::invalid;
        
    m_handle = obs_scene_get_source(scene);
}

scene::scene(obs_source_t *source)
 : m_scene(obs_scene_from_source(source))
{
    m_status = status_type::okay;

    if (!m_scene) {
        m_status = status_type::invalid;
        m_handle = nullptr;
        return;
    }

    m_handle = source;
}

obs_scene_t *scene::dangerous_scene() 
{
    return m_scene;
}

obs::scene scene::duplicate(std::string name, enum obs_scene_duplicate_type type)
{
    return obs_scene_duplicate(m_scene, name.c_str(), type);
}

scene scene::from_name(std::string name)
{
    obs_source_t *source = obs_get_source_by_name(name.c_str());
    obs_source_release(source);
    return obs::scene(source);
}

bool scene::operator!()
{
    return m_status != status_type::okay;
}

obs::input scene::source()
{
    return m_handle;
}

scene::item scene::add(input source)
{
    return obs_scene_add(m_scene, source.dangerous());
}

scene::item scene::find_item(std::string name)
{
    return obs_scene_find_source(m_scene, name.c_str());
}

scene::item scene::find_item(int64_t position)
{
    return obs_scene_find_sceneitem_by_id(m_scene, position);
}

std::vector<scene::item> scene::items()
{
    std::vector<scene::item> items;

    auto enum_cb =
    [] (obs_scene_t*, obs_sceneitem_t *item, void *data) {
        std::vector<scene::item> *items = 
            reinterpret_cast<std::vector<scene::item> *>(data);

        items->push_back(item);
        return true;
    };

    obs_scene_enum_items(m_scene, enum_cb, &items);

    return std::move(items);
}


/* 
    Scene Item implementation
 */

scene::item::item()
    : m_handle(nullptr),
      m_status(source::status_type::invalid)
{

}

scene::item::item(obs_sceneitem_t *item)
    : m_handle(item), 
      m_status(source::status_type::okay)
{
    if (!m_handle)
        m_status = source::status_type::invalid;
}

scene::item::item(item &copy)
 : m_handle(copy.m_handle),
   m_status(source::status_type::okay)
{
    if (!m_handle)
        m_status = source::status_type::invalid;
}

obs::source::status_type scene::item::status()
{
    return m_status;
}

void scene::item::remove()
{
    obs_sceneitem_remove(m_handle);
}

bool scene::item::visible()
{
    return obs_sceneitem_visible(m_handle);
}

void scene::item::visible(bool is_visible)
{
    obs_sceneitem_set_visible(m_handle, is_visible);
}

obs::scene scene::item::scene()
{
    return obs_sceneitem_get_scene(m_handle);
}

obs::input scene::item::source()
{
    return obs_sceneitem_get_source(m_handle);
}

uint64_t scene::item::id()
{
    return obs_sceneitem_get_id(m_handle);
}

bool scene::item::selected()
{
    return obs_sceneitem_selected(m_handle);
}

void scene::item::selected(bool select)
{
    obs_sceneitem_select(m_handle, select);
}

void scene::item::position(const vec2 &pos)
{
    obs_sceneitem_set_pos(m_handle, &pos);
}

vec2 scene::item::position()
{
    vec2 pos;
    obs_sceneitem_get_pos(m_handle, &pos);
    return pos;
}

void scene::item::rotation(float rot)
{
    obs_sceneitem_set_rot(m_handle, rot);
}

float scene::item::rotation()
{
    return obs_sceneitem_get_rot(m_handle);
}

void scene::item::scale(const vec2 &scale)
{
    obs_sceneitem_set_scale(m_handle, &scale);
}

vec2 scene::item::scale()
{
    vec2 scale;
    obs_sceneitem_get_scale(m_handle, &scale);
    return scale;
}

void scene::item::alignment(uint32_t alignment)
{
    obs_sceneitem_set_alignment(m_handle, alignment);
}

uint32_t scene::item::alignment()
{
    return obs_sceneitem_get_alignment(m_handle);
}

void scene::item::bounds_alignment(uint32_t alignment)
{
    obs_sceneitem_set_bounds_alignment(m_handle, alignment);
}

uint32_t scene::item::bounds_alignment()
{
    return obs_sceneitem_get_bounds_alignment(m_handle);
}

void scene::item::bounds(const vec2 &bounds)
{
    obs_sceneitem_set_bounds(m_handle, &bounds);
}

vec2 scene::item::bounds()
{
    vec2 bounds;
    obs_sceneitem_get_bounds(m_handle, &bounds);
    return bounds;
}

void scene::item::transform_info(const obs_transform_info &info)
{
    obs_sceneitem_set_info(m_handle, &info);
}

obs_transform_info scene::item::transform_info()
{
    obs_transform_info info;
    obs_sceneitem_get_info(m_handle, &info);
    return info;
}

void scene::item::order(obs_order_movement movement)
{
    obs_sceneitem_set_order(m_handle, movement);
}

void scene::item::order_position(int position)
{
    obs_sceneitem_set_order_position(m_handle, position);
}

void scene::item::bounds_type(obs_bounds_type type)
{
    obs_sceneitem_set_bounds_type(m_handle, type);
}

obs_bounds_type scene::item::bounds_type()
{
    return obs_sceneitem_get_bounds_type(m_handle);
}

void scene::item::crop(const obs_sceneitem_crop &crop)
{
    obs_sceneitem_set_crop(m_handle, &crop);
}

obs_sceneitem_crop scene::item::crop()
{
    obs_sceneitem_crop crop;
    obs_sceneitem_get_crop(m_handle, &crop);
    return crop;
}

void scene::item::scale_filter(obs_scale_type filter)
{
    obs_sceneitem_set_scale_filter(m_handle, filter);
}

obs_scale_type scene::item::scale_filter()
{
    return obs_sceneitem_get_scale_filter(m_handle);
}

/* When accessing tranform data (crop, tranform, etc.)
 * you need to do it between a begin and end call. Else
 * It's a data race. This acts as a basic barrier to 
 * the data from updating while you're accessing it. */
void scene::item::defer_update_begin()
{
    obs_sceneitem_defer_update_begin(m_handle);
}

void scene::item::defer_update_end()
{
    obs_sceneitem_defer_update_end(m_handle);
}


}