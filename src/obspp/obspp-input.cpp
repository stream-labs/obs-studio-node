#include "obspp-input.hpp"

namespace obs {

input::input(std::string id, std::string name, obs_data_t *settings, obs_data_t *hotkey)
 : source(id, name, settings, hotkey)
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

void input::deinterlace_mode(obs_deinterlace_mode mode)
{
    obs_source_set_deinterlace_mode(m_handle, mode);
}

obs_deinterlace_mode input::deinterlace_mode()
{
    return obs_source_get_deinterlace_mode(m_handle);
}

void input::deinterlace_field_order(obs_deinterlace_field_order order)
{
    obs_source_set_deinterlace_field_order(m_handle, order);
}

obs_deinterlace_field_order input::deinterlace_field_order()
{
    return obs_source_get_deinterlace_field_order(m_handle);
}

void input::monitoring_type(obs_monitoring_type type)
{
    obs_source_set_monitoring_type(m_handle, type);
}

obs_monitoring_type input::monitoring_type()
{
    return obs_source_get_monitoring_type(m_handle);
}

obs::filter input::find_filter(std::string name)
{
    return obs_source_get_filter_by_name(m_handle, name.c_str());
}

std::vector<obs::filter> input::filters()
{
    std::vector<obs::filter> filters;

    auto enum_cb = 
    [] (obs_source_t *parent, obs_source_t *filter, void *data) {
        std::vector<obs::filter> *filters = 
            reinterpret_cast<std::vector<obs::filter> *>(data);

        filters->push_back(obs_source_get_ref(filter));
    };

    obs_source_enum_filters(m_handle, enum_cb, &filters);

    return filters;
}

std::vector<std::string> input::types()
{
    const char *id = nullptr;
    std::vector<std::string> type_list;

    int i = 0;

    while (obs_enum_input_types(i++, &id)) {
        type_list.push_back(id);
    }

    return std::move(type_list);
}

std::vector<obs::input> input::public_sources()
{
    std::vector<obs::input> inputs;

    auto enum_cb = 
    [] (void *data, obs_source_t *source) {
        std::vector<obs::input> *inputs = 
            reinterpret_cast<std::vector<obs::input> *>(data);
        
        inputs->push_back(obs_source_get_ref(source));
        return true;
    };

    obs_enum_sources(enum_cb, &inputs);

    return std::move(inputs);
}
}