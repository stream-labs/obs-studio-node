"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const obs = require('./obs-studio-client.node');
const path = require("path");
const fs = require("fs");
exports.DefaultD3D11Path = path.resolve(__dirname, `libobs-d3d11.dll`);
exports.DefaultOpenGLPath = path.resolve(__dirname, `libobs-opengl.dll`);
exports.DefaultDrawPluginPath = path.resolve(__dirname, `simple_draw.dll`);
exports.DefaultBinPath = path.resolve(__dirname);
exports.DefaultDataPath = path.resolve(__dirname, `data`);
exports.DefaultPluginPath = path.resolve(__dirname, `obs-plugins`);
exports.DefaultPluginDataPath = path.resolve(__dirname, `data/obs-plugins/%module%`);
;
exports.Global = obs.Global;
exports.OutputFactory = obs.Output;
exports.AudioEncoderFactory = obs.AudioEncoder;
exports.VideoEncoderFactory = obs.VideoEncoder;
exports.ServiceFactory = obs.Service;
exports.InputFactory = obs.Input;
exports.SceneFactory = obs.Scene;
exports.FilterFactory = obs.Filter;
exports.TransitionFactory = obs.Transition;
exports.DisplayFactory = obs.Display;
exports.VolmeterFactory = obs.Volmeter;
exports.FaderFactory = obs.Fader;
exports.AudioFactory = obs.Audio;
exports.VideoFactory = obs.Video;
exports.ModuleFactory = obs.Module;
exports.IPC = obs.IPC;
var EDelayFlags;
(function (EDelayFlags) {
    EDelayFlags[EDelayFlags["PreserveDelay"] = 1] = "PreserveDelay";
})(EDelayFlags = exports.EDelayFlags || (exports.EDelayFlags = {}));
;
;
;
;
function addItems(scene, sceneItems) {
    const items = [];
    if (Array.isArray(sceneItems)) {
        sceneItems.forEach(function (sceneItem) {
            const source = obs.Input.fromName(sceneItem.name);
            const item = scene.add(source);
            item.position = { x: sceneItem.x, y: sceneItem.y };
            item.scale = { x: sceneItem.scaleX, y: sceneItem.scaleY };
            item.visible = sceneItem.visible;
            item.rotation = sceneItem.rotation;
            const cropModel = {
                top: Math.round(sceneItem.crop.top),
                right: Math.round(sceneItem.crop.right),
                bottom: Math.round(sceneItem.crop.bottom),
                left: Math.round(sceneItem.crop.left)
            };
            item.crop = cropModel;
            items.push(item);
        });
    }
    return items;
}
exports.addItems = addItems;
function createSources(sources) {
    const items = [];
    if (!Array.isArray(sources))
        return items;
    sources.forEach(function (source) {
        const obsSource = obs.Input.create(source.type, source.name, source.settings);
        if (!obsSource) {
            throw Error(`Failed to create input source ${obsSource.name} with type ${obsSource.type}\n` +
                `Settings: ${JSON.stringify(obsSource.settings)}`);
        }
        if (obsSource.audioMixers) {
            obsSource.muted = (source.muted != null) ? source.muted : false;
            obsSource.volume = (source.volume != null) ? source.volume : 1;
        }
        items.push(obsSource);
        const filters = source.filters;
        if (!Array.isArray(filters))
            return;
        filters.forEach(function (filter) {
            const obsFilter = obs.Filter.create(filter.type, filter.name, filter.settings);
            if (!obsFilter) {
                throw Error(`Failed to create input source ${obsSource.name} with type ${obsSource.type}\n` +
                    `Settings: ${JSON.stringify(obsSource.settings)}`);
            }
            obsFilter.enabled = (filter.enabled != null) ? filter.enabled : true;
            obsSource.addFilter(obsFilter);
            obsFilter.release();
        });
    });
    return items;
}
exports.createSources = createSources;
function getSourcesSize(sourcesNames) {
    const sourcesSize = [];
    if (!Array.isArray(sourcesNames))
        return sourcesSize;
    sourcesNames.forEach(function (sourceName) {
        const obsInput = obs.Input.fromName(sourceName);
        if (!obsInput) {
            throw Error(`Failed to fetch input source ${sourceName} by name`);
            return;
        }
        sourcesSize.push({
            name: sourceName,
            height: obsInput.height,
            width: obsInput.width,
            outputFlags: obsInput.outputFlags
        });
    });
    return sourcesSize;
}
exports.getSourcesSize = getSourcesSize;
if (fs.existsSync(path.resolve(__dirname, `obs64.exe`).replace('app.asar', 'app.asar.unpacked'))) {
    obs.IPC.setServerPath(path.resolve(__dirname, `obs64.exe`).replace('app.asar', 'app.asar.unpacked'), path.resolve(__dirname).replace('app.asar', 'app.asar.unpacked'));
}
else {
    obs.IPC.setServerPath(path.resolve(__dirname, `obs32.exe`).replace('app.asar', 'app.asar.unpacked'), path.resolve(__dirname).replace('app.asar', 'app.asar.unpacked'));
}
exports.NodeObs = obs;
