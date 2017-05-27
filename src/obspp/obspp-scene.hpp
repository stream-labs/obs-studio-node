#pragma once

#include "obspp-source.hpp"

namespace obs {

class scene : public source {

private:
    obs_scene_t *m_scene;

public:

    scene(std::string name);
    scene(scene &&copy);
    scene(obs_scene_t *source);

    bool operator!();
    void add(input &sceneitem);

    static std::list<std::string> types();
};

scene::scene(std::string name)
 : m_scene(obs_scene_create(name.c_str()))
{
    if (m_scene) {
		m_status = status_type::okay;
	}

    /* Will return NULL if m_scene is invalid */
    m_handle = obs_scene_get_source(m_scene);
}

scene::scene(scene &&move)
{
    m_scene = move.m_scene;
    m_handle = move.m_handle;
    
    if (!m_scene || !m_handle) {
        m_status = status_type::invalid;
    }
}

scene::scene(obs_scene_t *scene)
 : m_scene(scene)
{
    if (!scene)
        m_status = status_type::invalid;
        
    obs_scene_addref(scene);
    m_handle = obs_scene_get_source(scene);
}

bool scene::operator!()
{
    return m_status != status_type::okay;
}

void scene::add(input &sceneitem)
{
    obs_scene_add(m_scene, sceneitem.dangerous());
}

}