var obs_renderer = require('../dist/obs-node');
var {remote} = require('electron');
var obs_main = remote.require('../dist/obs-node');

console.log("Renderer OBS Status: " + obs_renderer.obs.status);
console.log("Main OBS Status: " + obs_main.obs.status);

var some_source = new obs_main.obs.Input('monitor_capture', 'renderer_test');
console.log(some_source);
console.log("Renderer Test Source Name: " + some_source.name);
some_source.release();