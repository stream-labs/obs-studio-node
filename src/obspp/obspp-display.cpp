#include "obspp-display.hpp"

namespace obs {



display::display(gs_init_data &data) 
 : m_handle(obs_display_create(&data))
{
    if (!m_handle) {
        m_status = status_type::invalid;
    } else {
        m_status = status_type::okay;
    }
}

display::~display()
{
    obs_display_destroy(m_handle);
}

display::status_type display::status()
{
    return m_status;
}

obs_display_t *display::dangerous()
{
    return m_handle;
}

void display::destroy()
{
    obs_display_destroy(m_handle);
    m_handle = nullptr;
    m_status = status_type::invalid;
}

void display::resize(uint32_t width, uint32_t height)
{
    obs_display_resize(m_handle, width, height);
}

void display::add_drawer(draw_callback_t drawer)
{

}

void display::remove_drawer(draw_callback_t drawer)
{

}

void display::enabled(bool is_enabled)
{
    obs_display_set_enabled(m_handle, is_enabled);
}

bool display::enabled()
{
    return obs_display_enabled(m_handle);
}

void display::background_color(uint32_t color)
{
    obs_display_set_background_color(m_handle, color);
}

}