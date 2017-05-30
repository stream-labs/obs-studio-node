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
    typedef void(*draw_callback_t)(void *param, uint32_t cx, uint32_t cy);

    display(gs_init_data &data);
    ~display();

    obs_display_t *dangerous();
    status_type status();
    void destroy();
    void resize(uint32_t width, uint32_t height);
    void add_drawer(draw_callback_t drawer);
    void remove_drawer(draw_callback_t drawer);
    bool enabled();
    void enabled(bool is_enabled);
    void background_color(uint32_t color);
};

}