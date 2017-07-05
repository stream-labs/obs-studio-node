#include "obspp-input.hpp"

namespace obs {

input::input(std::string id, std::string name, obs_data_t *hotkey, obs_data_t *settings)
 : source(id, name, hotkey, settings)
{
    check_type(m_handle, OBS_SOURCE_TYPE_INPUT);
}

input::input(std::string id, std::string name, obs_data_t *settings, bool is_private)
 : source(id, name, settings, is_private)
{
    check_type(m_handle, OBS_SOURCE_TYPE_INPUT);
}

input::input(input &copy)
 : source(copy.m_handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_INPUT);
}

input::input(obs_source_t * handle)
 : source(handle)
{
    source::check_type(m_handle, OBS_SOURCE_TYPE_INPUT);
}

input input::duplicate(std::string name, bool is_private)
{
    return obs::input(obs_source_duplicate(m_handle, name.c_str(), is_private));
}

input input::from_name(std::string name)
{
    obs_source_t * source = obs_get_source_by_name(name.c_str());
    obs_source_release(source);
    return obs::input(source);
}

void input::volume(float volume)
{
    obs_source_set_volume(m_handle, volume);
}

float input::volume()
{
    return obs_source_get_volume(m_handle);
}

bool input::showing()
{
    return obs_source_showing(m_handle);
}

void input::audio_mixers(uint32_t flags)
{
    obs_source_set_audio_mixers(m_handle, flags);
}

uint32_t input::audio_mixers()
{
    return obs_source_get_audio_mixers(m_handle);
}

void input::add_filter(obs::filter filter)
{
    obs_source_filter_add(m_handle, filter.dangerous());
}

void input::remove_filter(obs::filter filter)
{
    obs_source_filter_remove(m_handle, filter.dangerous());
}

std::vector<filter> input::filters()
{
    return std::vector<filter>();
}

std::vector<std::string> input::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    for (int i = 0; obs_enum_input_types(i, &id); ++i) {
        type_list.push_back(id);
    }

    return type_list;
}

}