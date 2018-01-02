#pragma once

#include "obspp-transition.hpp"

namespace obs {

transition::transition(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey)
 : source(id, name, settings, hotkey)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_TRANSITION);
}

transition::transition(std::string id, std::string name, obs_data_t *settings, bool is_private)
 : source(id, name, settings, is_private)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_TRANSITION);
}

transition::transition(transition &copy)
 : source(copy.m_handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_TRANSITION);
}

transition::transition(obs_source_t * handle)
 : source(handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_TRANSITION);
}

obs::source transition::get_active_source()
{
    return obs_transition_get_active_source(m_handle);
}

void transition::clear()
{
    obs_transition_clear(m_handle);
}

void transition::set(obs::source &source) 
{
    obs_transition_set(m_handle, source.dangerous());
}

void transition::start(int ms, obs::source &source)
{
    obs_transition_start(m_handle, 
        OBS_TRANSITION_MODE_AUTO, ms, source.dangerous());
}

std::vector<std::string> transition::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    for (int i = 0; obs_enum_transition_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return std::move(type_list);
}

}