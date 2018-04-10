"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const obs = require('./obs-studio-client.node');
const path = require("path");
//obs.SetWorkingDirectory(__dirname);
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
var EDelayFlags;
(function (EDelayFlags) {
    EDelayFlags[EDelayFlags["PreserveDelay"] = 1] = "PreserveDelay";
})(EDelayFlags = exports.EDelayFlags || (exports.EDelayFlags = {}));
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
    if (Array.isArray(sources)) {
        sources.forEach(function (source) {
            const newSource = obs.Input.create(source.type, source.name, source.settings);
            if (newSource.audioMixers) {
                newSource.muted = (source.muted != null) ? source.muted : false;
                newSource.volume = (source.volume != null) ? source.volume : 1;
            }
            items.push(newSource);
            const filters = source.filters;
            if (Array.isArray(filters)) {
                filters.forEach(function (filter) {
                    const ObsFilter = obs.Filter.create(filter.type, filter.name, filter.settings);
                    ObsFilter.enabled = (filter.enabled != null) ? filter.enabled : true;
                    newSource.addFilter(ObsFilter);
                });
            }
        });
    }
    return items;
}
exports.createSources = createSources;
function getSourcesSize(sourcesNames) {
    const sourcesSize = [];
    if (Array.isArray(sourcesNames)) {
        sourcesNames.forEach(function (sourceName) {
            const ObsInput = obs.Input.fromName(sourceName);
            sourcesSize.push({ name: sourceName, height: ObsInput.height, width: ObsInput.width, outputFlags: ObsInput.outputFlags });
        });
    }
    return sourcesSize;
}
exports.getSourcesSize = getSourcesSize;
exports.NodeObs = obs;
