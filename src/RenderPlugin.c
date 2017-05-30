#include <util/platform.h>

#include "RenderPlugin.h"

static DARRAY(struct osn_render_plugin) plugins = { 0 };

typedef void(*DisplayPluginLoadFunc)(obs_display_t*);
typedef void(*RenderPluginLoadFunc)(void);

void *osn_load_plugin(obs_display_t *display, const char * path)
{
    void *module = os_dlopen(path);

    if (!module) return NULL;

    DisplayPluginLoadFunc load_func
        = (DisplayPluginLoadFunc)(os_dlsym(module, "osn_plugin_load"));

    if (!load_func) goto sym_error;

    load_func(display);
        
    return module;
sym_error:
    os_dlclose(module);
    return NULL;
}

__declspec(dllexport)
void osn_register_render_plugin
        (obs_display_t *display, 
        struct osn_render_plugin *plugin)
{
    obs_display_add_draw_callback(display, plugin->draw, plugin);
}


