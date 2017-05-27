#pragma once

#include <obs.h>
#include <functional>

namespace obs {

class display {
public:
    enum status_type {
        okay,
        invalid
    };

private:
    obs_display_t *m_handle;
    status_type m_status;

public:
    typedef std::functional<void(uint32_t cx, uint32_t cy)> draw_callback_t;

    display(gs_init_data data);
    ~display();

    void resize(uint32_t width, uint32_t height);
    void add_drawer(draw_callback_t drawer);
    void remove_drawer(draw_callback_t drawer);
    void enable();
    void disable();
    bool enabled();
    void background_color(uint32_t color);
};

display::display(gs_init_data &data) 
 : m_handle(obs_display_create(data))
{
    if (!m_handle) {
        m_status = status_type::invalid;
        return;
    }
}

display::~display()
{
    obs_display_destroy(m_handle);
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

void display::enable()
{
    obs_display_set_enabled(m_handle, true);
}

void display::disable()
{
    obs_display_set_enabled(m_handle, false);
}

bool display::enabled()
{
    obs_display_enabled(m_handle);
}

void display::background_color(uint32_t color)
{
    obs_display_set_background_color(display, color);
}

}