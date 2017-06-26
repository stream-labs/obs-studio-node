const { remote } = require('electron');
import path = require('path');

import '../node-obs';

obs.startup('en-US');

console.log("Status: " + obs.status);
console.log("Locale: " + obs.locale);

let version = obs.version
let major = (version & 0xFF000000) >> 24;
let minor = (version & 0x00FF0000) >> 16;
let patch = (version & 0x0000FFFF);

console.log("Version: " + major + "." + minor + "." + patch);

obs.module.add_path(
    path.resolve('node-obs'),
    path.resolve('node-obs/data')
);

obs.module.add_path(
    path.resolve('node-obs/obs-plugins'),
    path.resolve('node-obs/data/obs-plugins/%module%/data')
);

obs.module.load_all();
obs.module.log_loaded();

let error = obs.video.reset({
    'graphics_module' : 'libobs-d3d11',
    'fps_num' : 30,
    'fps_den' : 1,
    'output_width' : 800,
    'output_height' : 600,
    'output_format' : 6, /* RGBA */
    'base_width' : 800,
    'base_height' : 600,
    'gpu_conversion' : true,
    'adapter' : 0, 
    'colorspace' : 0,
    'range' : 0,
    'scale_type' : 0
});

if (error) {
    console.log("Failed to reset video");
}

let display_init = {
    'hwnd': remote.getCurrentWindow().getNativeWindowHandle(),
    'width': 800,
    'height': 600,
    'format': 3,
    'zsformat': 0
};

let test_input = new obs.Input('monitor_capture', 'test input');
let test_scene = new obs.Scene('test scene');
let test_item  = test_scene.add(test_input);

obs.video.output(0, test_scene);

let test_1 = test_input;
let test_2 = test_input;
let test_3 = test_1;

let item_1 = test_item;
let item_2 = test_item;
let item_3 = item_1;

let scene_1 = test_scene;
let scene_2 = test_scene;
let scene_3 = scene_1;

setTimeout(() => {
    obs.video.output(0, null);
    item_3.remove();
    test_3.release();
    scene_3.release();

}, 10000)

let display = new obs.Display();
display.add_drawer(path.resolve('node-obs/simple_draw'));