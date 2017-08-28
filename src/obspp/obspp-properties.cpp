#include "obspp-properties.hpp"
#include "obspp.hpp"

namespace obs {

properties::properties(obs::properties &&properties)
 : m_handle(properties.m_handle),
   m_status(properties.m_status)
{
    properties.m_handle = nullptr;
    properties.m_status = status_type::invalid;
}

properties::properties(obs_properties_t *properties)
 : m_handle(properties),
   m_status(status_type::okay)
{
    if (!m_handle) {
        m_status = status_type::invalid;
    }
}

properties::properties(std::string id, object_type type)
{
    switch (type) {
    case object_type::source:
        m_handle = obs_get_source_properties(id.c_str());
        break;
    case object_type::encoder:
        obs_get_encoder_properties(id.c_str());
        break;
    case object_type::service:
        obs_get_service_properties(id.c_str());
        break;
    case object_type::output:
        obs_get_output_properties(id.c_str());
        break;
    default:
        m_handle = nullptr;
    }

    if (!m_handle) m_status = status_type::invalid;
}

properties::~properties()
{
    obs_properties_destroy(m_handle);
}

properties::status_type properties::status()
{
    return m_status;
}

property properties::first()
{
    return obs::property(dangerous(), obs_properties_first(m_handle));
}

property properties::get(std::string property_name)
{
    return obs::property(dangerous(), obs_properties_get(m_handle, property_name.c_str()));
}

obs_properties_t *properties::dangerous()
{
    return m_handle;
}

property::property(obs_properties_t *parent, obs_property_t *handle)
 : m_handle(handle),
   m_parent(parent),
   m_status(status_type::okay)
{
    if (!m_handle || !m_parent) m_status = status_type::invalid;
}

property::status_type property::status()
{
    return m_status;
}

obs_property_t *property::dangerous()
{
    return m_handle;
}

std::string property::name()
{
    const char * value = obs_property_name(m_handle);

    return value ? value : "";
}

std::string property::description()
{
    const char * value = obs_property_description(m_handle);

    return value ? value : "";
}

std::string property::long_description()
{
    const char * value = obs_property_long_description(m_handle);

    return value ? value : "";
}

obs_property_type property::type()
{
    return obs_property_get_type(m_handle);
}

bool property::enabled()
{
    return obs_property_enabled(m_handle);
}

bool property::visible()
{
    return obs_property_visible(m_handle);
}

obs::property property::next()
{
    obs_property_t *next = m_handle;
    obs_property_next(&next);

    return obs::property(m_parent, next);
}

bool property::modified(obs_data_t *settings)
{
    return obs_property_modified(m_handle, settings);
}

button_property property::button_property()
{
    return obs::button_property(m_parent, m_handle);
}

list_property property::list_property()
{
    return obs::list_property(m_parent, m_handle);
}

editable_list_property property::editable_list_property()
{
    return obs::editable_list_property(m_parent, m_handle);
}

integer_property property::integer_property()
{
    return obs::integer_property(m_parent, m_handle);
}

float_property property::float_property()
{
    return obs::float_property(m_parent, m_handle);
}

text_property property::text_property()
{
    return obs::text_property(m_parent, m_handle);
}

path_property property::path_property()
{
    return obs::path_property(m_parent, m_handle);
}

button_property::button_property(obs_properties_t *parent, obs_property_t *handle)
    : property(parent, handle)
{
    
}

void button_property::clicked()
{
    obs_property_button_clicked(m_handle, obs_properties_get_param(m_parent));
}

list_property::list_property(obs_properties_t *parent, obs_property_t *handle) 
    : property(parent, handle)
{
    
}

obs_combo_format list_property::format()
{
    return obs_property_list_format(m_handle);
}

obs_combo_type list_property::type()
{
    return obs_property_list_type(m_handle);
}

const char *list_property::get_name(size_t idx)
{
    return obs_property_list_item_name(m_handle, idx);
}

size_t list_property::count()
{
    return obs_property_list_item_count(m_handle);
}

const char* list_property::get_string(size_t idx)
{
    return obs_property_list_item_string(m_handle, idx);
}

long long list_property::get_integer(size_t idx)
{
    return obs_property_list_item_int(m_handle, idx);
}

double list_property::get_float(size_t idx)
{
    return obs_property_list_item_float(m_handle, idx);
}

editable_list_property::editable_list_property(obs_properties_t *parent, obs_property_t *handle)
    : list_property(parent, handle)
{
}

obs_editable_list_type editable_list_property::type()
{
    return obs_property_editable_list_type(m_handle);
}

const char *editable_list_property::filter()
{
    return obs_property_editable_list_filter(m_handle);
}

const char *editable_list_property::default_path()
{
    return obs_property_editable_list_default_path(m_handle);
}

float_property::float_property(obs_properties_t *parent, obs_property_t *handle)
    : property(parent, handle)
{
}

obs_number_type float_property::type()
{
    return obs_property_float_type(m_handle);
}

double float_property::min()
{
    return obs_property_float_min(m_handle);
}

double float_property::max()
{
    return obs_property_float_max(m_handle);
}

double float_property::step()
{
    return obs_property_float_step(m_handle);
}

integer_property::integer_property(obs_properties_t *parent, obs_property_t *handle)
    : property(parent, handle)
{
}

obs_number_type integer_property::type()
{
    return obs_property_int_type(m_handle);
}

int integer_property::min()
{
    return obs_property_int_min(m_handle);
}

int integer_property::max()
{
    return obs_property_int_max(m_handle);
}

int integer_property::step()
{
    return obs_property_int_step(m_handle);
}

text_property::text_property(obs_properties_t *parent, obs_property_t *handle)
    : property(parent, handle)
{
}

obs_text_type text_property::type()
{
    return obs_proprety_text_type(m_handle);
}

path_property::path_property(obs_properties_t *parent, obs_property_t *handle)
    : property(parent, handle)
{
}

obs_path_type path_property::type()
{
    return obs_property_path_type(m_handle);
}

const char *path_property::filter()
{
    return obs_property_path_filter(m_handle);
}

const char *path_property::default_path()
{
    return obs_property_path_filter(m_handle);
}


}