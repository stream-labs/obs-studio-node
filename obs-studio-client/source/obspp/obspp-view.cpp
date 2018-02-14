#include "obspp-view.hpp"

namespace obs {
    
view::view()
 : m_handle(obs_view_create()),
   m_status(view::status_type::okay)
{
    if (!m_handle) {
        m_status = view::status_type::invalid;
    }
}

view::~view()
{
    obs_view_destroy(m_handle);
}

void view::output(int channel, obs::source source)
{
    obs_view_set_source(m_handle, channel, source.dangerous());
}

obs::source view::output(int channel)
{
    return obs_view_get_source(m_handle, channel);
}

void view::render()
{
    obs_view_render(m_handle);
}

}