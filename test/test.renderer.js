"use strict";
var remote = require('electron').remote;
var path = require("path");
var obs = remote.require('../node-obs').obs;
obs.startup('en-US');
console.log("Status: " + obs.status);
console.log("Locale: " + obs.locale);
var version = obs.version;
var major = (version & 0xFF000000) >> 24;
var minor = (version & 0x00FF0000) >> 16;
var patch = (version & 0x0000FFFF);
console.log("Version: " + major + "." + minor + "." + patch);
obs.module.add_path(path.resolve('node-obs'), path.resolve('node-obs/data'));
obs.module.add_path(path.resolve('node-obs/obs-plugins'), path.resolve('node-obs/data/obs-plugins/%module%/data'));
obs.module.load_all();
obs.module.log_loaded();
var error = obs.video.reset({
    'graphics_module': 'libobs-d3d11',
    'fps_num': 30,
    'fps_den': 1,
    'output_width': 800,
    'output_height': 600,
    'output_format': 6,
    'base_width': 800,
    'base_height': 600,
    'gpu_conversion': true,
    'adapter': 0,
    'colorspace': 0,
    'range': 0,
    'scale_type': 0
});
if (error) {
    console.log("Failed to reset video");
}
var display_init = {
    'hwnd': remote.getCurrentWindow().getNativeWindowHandle(),
    'width': 800,
    'height': 600,
    'format': 3,
    'zsformat': 0
};
var test_input = new obs.Input('monitor_capture', 'test input');
var test_scene = new obs.Scene('test scene');
var test_item = test_scene.add(test_input);
console.log(test_input.id);
console.log(test_input.name);
console.log(test_scene.name);
obs.video.output(0, test_scene);
var test_1 = test_input;
var test_2 = test_input;
var test_3 = test_1;
var item_1 = test_item;
var item_2 = test_item;
var item_3 = item_1;
var scene_1 = test_scene;
var scene_2 = test_scene;
var scene_3 = scene_1;
setTimeout(function () {
    obs.video.output(0, null);
    item_3.remove();
    test_3.release();
    scene_3.release();
}, 10000);
var display = new obs.Display();
display.add_drawer(path.resolve('node-obs/simple_draw'));
