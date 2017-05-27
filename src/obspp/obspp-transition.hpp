#pragma once

#include "obspp-source.hpp"

namespace obs {

class transition : public source {
public:
    transition(std::string &id, std::string &name, obs_data_t *settings = nullptr);
    transition(transition &&copy);
    transition(obs_source_t *source);

    static std::list<std::string> types();
};

transition::transition(std::string &id, std::string &name, obs_data_t *settings)
 : source(id, name, settings)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_TRANSITION);
}

transition::transition(transition &&copy)
 : source(copy.m_handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_TRANSITION);
}

transition::transition(obs_source_t * handle)
 : source(handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_TRANSITION);
}


std::list<std::string> transition::types()
{
    const char *id = nullptr;
    std::list<std::string> type_list;

    for (int i = 0; obs_enum_transition_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return std::move(type_list);
}

}