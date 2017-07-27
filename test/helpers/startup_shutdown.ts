import * as obs from '../../node-obs/obs_node.js';
import * as path from 'path';
import test from 'ava';

let node_dist_dir = 'node-obs'

export function startup_shutdown(t: any, cb: (t: any) => void, locale?: string, start_path?: string) {
    if (arguments.length == 2) {
        var locale = 'en-US';
    }

    obs.ObsGlobal.startup(locale);

    /* Video Context Setup */
    let obs_d3d11_path = path.resolve(`${node_dist_dir}/libobs-d3d11`);

    console.log(`Searching for libobs-d3d11 at ${obs_d3d11_path}`);

    let error = obs.ObsVideo.reset({
        'graphics_module': obs_d3d11_path,
        'fps_num': 30,
        'fps_den': 1,
        'output_width': 800,
        'output_height': 600,
        'output_format': obs.EOutputFormat.NV12,
        'base_width': 800,
        'base_height': 600,
        'gpu_conversion': true,
        'adapter': 0,
        'colorspace': 0,
        'range': 0,
        'scale_type': 0
    });

    if (error) t.fail();

    /* Module Loading */
    let bin_path = path.resolve(`${node_dist_dir}`);
    let data_path = path.resolve(`${node_dist_dir}/data`);
    let plugin_bin_path = path.resolve(`${node_dist_dir}/obs-plugins`);
    let plugin_data_path = path.resolve(`${node_dist_dir}/data/obs-plugins/%module%/data`);
    console.log(`Bin Path: ${bin_path}`);
    console.log(`Data Path: ${data_path}`);
    console.log(`Plugin Bin Path: ${plugin_bin_path}`);
    console.log(`Plugin Data Path: ${plugin_data_path}`);
    obs.ObsModule.addPath(bin_path, data_path);
    obs.ObsModule.addPath(plugin_bin_path, plugin_data_path);
    obs.ObsModule.loadAll();
    obs.ObsModule.logLoaded();

    /* Dummy Display */
    var display_init = {
        'width': 800,
        'height': 600,
        'format': obs.EColorFormat.RGBA,
        'zsformat': obs.EZStencilFormat.None
    };

    let display = obs.ObsDisplay.create(display_init);
    let simple_draw_path = path.resolve(`${node_dist_dir}/simple_draw`);
    display.addDrawer(simple_draw_path);

    cb(t);

    display.removeDrawer(simple_draw_path);
    display.destroy();
    obs.ObsGlobal.shutdown();
}