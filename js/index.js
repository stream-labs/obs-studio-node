"use strict";
function __export(m) {
    for (var p in m) if (!exports.hasOwnProperty(p)) exports[p] = m[p];
}
Object.defineProperty(exports, "__esModule", { value: true });
__export(require("./module"));
__export(require("./type_check"));


const obs = require('./obs_studio_client.node');
console.log('Starting electron');
obs.IPC.host('slobs');

obs.SetWorkingDirectory('/Users/eddygharbi/streamlabs/obs-studio-node/dist');

// Initialize OBS API
const ret = obs.OBS_API_initAPI(
            'en-US',
            '/Users/eddygharbi/streamlabs/obs-studio-node/cache/',
            "0.0.0",
            );

console.log('Result OBS initiliazation: ' + ret);

// obs.IPC.disconnect();
// console.log(apiResult);