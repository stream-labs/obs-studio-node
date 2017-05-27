#pragma once

#include "obspp-source.hpp"

#include <list>

namespace obs {

class filter : public source {
public:
    filter(std::string &id, std::string &name, obs_data_t *settings = nullptr);
    filter(filter &&copy);
    filter(obs_source_t *source);

    static std::list<std::string> types();
};

filter::filter(std::string &id, std::string &name, obs_data_t *settings)
 : source(id, name, settings)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_FILTER);
}

filter::filter(filter &&copy)
 : source(copy.m_handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_FILTER);
}

filter::filter(obs_source_t * source_)
 : source(source_)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_FILTER);
}

std::list<std::string> filter::types()
{
    const char *id = nullptr;
    std::list<std::string> type_list;

    for (int i = 0; obs_enum_filter_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return std::move(type_list);
}

}