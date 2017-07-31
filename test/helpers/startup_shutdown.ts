import * as obs from 'obs-studio-node';
import * as path from 'path';
import test from 'ava';

let node_dist_dir = 'node-obs'

export async function startup_shutdown(t: any, cb: (t: any) => void, locale?: string, start_path?: string) {
    if (arguments.length == 2) {
        var locale = 'en-US';
    }

    obs.ObsGlobal.startup(locale);

    /* Video Context Setup */
    let obs_d3d11_path = obs.DefaultD3D11Path;

    console.log(`Searching for libobs-d3d11 at ${obs_d3d11_path}`);

    let error = obs.ObsVideo.reset({
        'graphicsModule': obs_d3d11_path,
        'fpsNum': 30,
        'fpsDen': 1,
        'outputWidth': 800,
        'outputHeight': 600,
        'outputFormat': obs.EOutputFormat.NV12,
        'baseWidth': 800,
        'baseHeight': 600,
        'gpuConversion': true,
        'adapter': 0,
        'colorspace': 0,
        'range': 0,
        'scaleType': 0
    });

    if (error) t.fail();

    /* Module Loading */
    let bin_path = obs.DefaultBinPath;
    let data_path = obs.DefaultDataPath;
    let plugin_bin_path = obs.DefaultPluginPath;
    let plugin_data_path = `D:/Projects/obs-studio-nan/test/node_modules/obs-studio-node/distribute/data/obs-plugins/%module%/data`;
    console.log(`Bin Path: ${bin_path}`);
    console.log(`Data Path: ${data_path}`);
    console.log(`Plugin Bin Path: ${plugin_bin_path}`);
    console.log(`Plugin Data Path: ${plugin_data_path}`);
    obs.ObsModuleFactory.addPath(bin_path, data_path);
    obs.ObsModuleFactory.addPath(plugin_bin_path, plugin_data_path);
    obs.ObsModuleFactory.loadAll();
    obs.ObsModuleFactory.logLoaded();

    /* Dummy Display */
    var display_init = {
        'width': 800,
        'height': 600,
        'format': obs.EColorFormat.RGBA,
        'zsformat': obs.EZStencilFormat.None
    };

    let display = obs.ObsDisplayFactory.create(display_init);
    let simple_draw_path = obs.DefaultDrawPluginPath;
    display.addDrawer(simple_draw_path);

    await cb(t);

    display.removeDrawer(simple_draw_path);
    display.destroy();
    obs.ObsGlobal.shutdown();
}