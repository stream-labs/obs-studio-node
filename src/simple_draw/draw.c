#include "../RenderPlugin.h"

struct simple_context {
    int pad;
};

void simple_draw(void *data, uint32_t cx, uint32_t cy)
{
    obs_render_main_view();
}

void simple_close(void *data)
{

}

struct osn_render_plugin simple_plugin = {
    .draw = simple_draw,
    .name = "simple_draw",
    .close = simple_close
};


/* Super compiler/platform specific. */
__declspec(dllexport) 
void osn_plugin_load(obs_display_t *display)
{
    osn_register_render_plugin(display, &simple_plugin);
}
