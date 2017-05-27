#pragma once

#include "obspp-source.hpp"
#include "obspp-filter.hpp"
#include <list>
#include <string>

namespace obs {

class input : public source {
public:
    input(std::string id, std::string name, obs_data_t *settings = nullptr);
    input(input &&copy);
    input(obs_source_t *source);

    void volume(float volume);
    float volume();

    int64_t sync_offset();
    void sync_offset(int64_t offset);

    bool showing();

    uint32_t flags();
    void flags(uint32_t flag);

    void audio_mixer(uint32_t mixer);
    uint32_t audio_mixer();

    std::list<filter> filters();

    static std::list<std::string> types();
};

input::input(std::string id, std::string name, obs_data_t *settings)
 : source(id, name, settings)
{
    check_type(m_handle, OBS_SOURCE_TYPE_INPUT);
}

input::input(input &&copy)
 : source(std::move(copy.m_handle))
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_INPUT);
}

input::input(obs_source_t * handle)
 : source(handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_INPUT);
}

std::list<filter> input::filters()
{
    return std::list<filter>();
}

std::list<std::string> input::types()
{
    const char *id = nullptr;
    std::list<std::string> type_list;

    for (int i = 0; obs_enum_input_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}
}