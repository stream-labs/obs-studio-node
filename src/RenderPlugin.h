#pragma once

#include <obs.h>

#ifdef __cplusplus
extern "C" {
#endif

struct osn_render_plugin
{
    void (*draw)(void *, uint32_t cx, uint32_t cy);
    void (*close)(void *);
    const char * name;
};

void *osn_load_plugin(obs_display_t *display, const char * path);

/* Generally called from the plugin. */
__declspec(dllexport)
void osn_register_render_plugin
        (obs_display_t *display, 
        struct osn_render_plugin *plugin);

#ifdef __cplusplus
}
#endif
