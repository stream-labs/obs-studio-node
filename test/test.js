const { app } = require('electron');
var path = require('path');
const obs = require('../node-obs/obs_node.node');

let test_transition;
let test_input;
let test_scene;

function ready() {
    obs.Global.startup('en-US');
    console.log("Status: " + obs.Global.status);
    console.log("Locale: " + obs.Global.locale);

    /* Version */
    var version = obs.Global.version;
    var major = (version & 0xFF000000) >> 24;
    var minor = (version & 0x00FF0000) >> 16;
    var patch = (version & 0x0000FFFF);
    console.log("Version: " + major + "." + minor + "." + patch);

    /* Module Loading */
    obs.module.add_path(path.resolve('node-obs'), path.resolve('node-obs/data'));
    obs.module.add_path(path.resolve('node-obs/obs-plugins'), path.resolve('node-obs/data/obs-plugins/%module%/data'));
    obs.module.load_all();
    obs.module.log_loaded();

    /* We should load this stuff before module loading... but
       our DLLs aren't placed correctly so it won't find libobs-d3d11. */
    var error = obs.Video.reset({
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
        'width': 800,
        'height': 600,
        'format': 3,
        'zsformat': 0
    };

    display = new obs.Display(display_init);
    display.addDrawer(path.resolve('node-obs/simple_draw'));

    /* Create test scenes and inputs */
    test_input = obs.Input.create('monitor_capture', 'test input');
    test_scene = obs.Scene.create('test scene');
    var test_item = test_scene.add(test_input);
    var some_other_input = obs.Input.fromName('test input');

    console.log(obs.Input.types());
    console.log(obs.Transition.types());
    console.log(obs.Filter.types());

    console.log('Status of some_other_input' + some_other_input.status);

    console.log(test_input.id);
    console.log(test_input.name);
    console.log(test_scene.name);

    /* Apply a source to the main view */
    test_transition = obs.Transition.create('cut_transition', 'CUT MY LIFE INTO PIECES');
    test_transition.set(test_scene);
    obs.Video.setOutputSource(0, test_transition);
    var some_output_input = obs.Video.getOutputSource(0);
    obs.Video.setOutputSource(0, some_output_input);

    /* Object aliases don't create new references. */
    var test_1 = test_input;
    var test_2 = test_input;
    var test_3 = test_1;
    var item_1 = test_item;
    var item_2 = test_item;
    var item_3 = item_1;
    var scene_1 = test_scene;
    var scene_2 = test_scene;
    var scene_3 = scene_1;

    /* Property Testing */
    var myProperties = test_input.properties;

    var myProperty = myProperties.first();

    console.log('Properties of test_input: ');

    do{
        console.log(myProperty);
        console.log(myProperty.details);
        myProperty.next();
    } while (myProperty.done != true);

    var mySettings = test_input.settings;

    console.log('Settings of test_input: ');
    console.log(mySettings);

    console.log('New settings for test_input: ');
    mySettings.monitor = 1;

    console.log(mySettings);
    test_input.update(mySettings);

    var mySceneItems = test_scene.getItems();

    console.log(mySceneItems);

    var yetAnotherInputSource = mySceneItems[0].getSource();

    console.log(yetAnotherInputSource.name);

    var omfgMoreSources = obs.Input.getPublicSources();
    console.log(omfgMoreSources);

    setTimeout(function () {
        obs.Video.setOutputSource(0, null);
        item_3.remove();
        test_3.release();
        scene_3.release();
        test_transition.release();
        display.destroy();

        console.log("Moment of truth... (should get messages about null source)");
        console.log(some_other_input.name);
    }, 10000);
}; 

app.on('ready', () => {
    ready();
})

app.on('quit', () => {
    obs.Global.shutdown();
});