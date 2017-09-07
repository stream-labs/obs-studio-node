"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const obs = require('./obs_node.node');
const path = require("path");
exports.DefaultD3D11Path = path.resolve(__dirname, `libobs-d3d11.dll`);
exports.DefaultDrawPluginPath = path.resolve(__dirname, `simple_draw.dll`);
exports.DefaultBinPath = path.resolve(__dirname);
exports.DefaultDataPath = path.resolve(__dirname, `data`);
exports.DefaultPluginPath = path.resolve(__dirname, `obs-plugins`);
exports.DefaultPluginDataPath = path.resolve(__dirname, `data/obs-plugins/%module%/data`);
;
exports.Global = obs.Global;
exports.InputFactory = obs.Input;
exports.SceneFactory = obs.Scene;
exports.FilterFactory = obs.Filter;
exports.TransitionFactory = obs.Transition;
exports.DisplayFactory = obs.Display;
exports.VolmeterFactory = obs.Volmeter;
exports.FaderFactory = obs.Fader;
exports.Audio = obs.Audio;
exports.Video = obs.Video;
exports.ModuleFactory = obs.Module;
function addItems(scene, sceneItems) {
    const items = [];
    if (Array.isArray(sceneItems)) {
        sceneItems.forEach(function (sceneItem) {
            const source = obs.Input.fromName(sceneItem.name);
            const item = scene.add(source);
            item.position = { x: sceneItem.x, y: sceneItem.y };
            item.scale = { x: sceneItem.scaleX, y: sceneItem.scaleY };
            item.visible = sceneItem.visible;
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
            items.push(newSource);
            const filters = source.filters.data.items;
            if (Array.isArray(filters)) {
                filters.forEach(function (filter) {
                    const ObsFilter = obs.Filter.create(filter.type, filter.name, filter.settings);
                    newSource.addFilter(ObsFilter);
                });
            }
        });
    }
    return items;
}
exports.createSources = createSources;
function getSourcesSize(sources) {
    const sourcesSize = [];
    if (Array.isArray(sources)) {
        sources.forEach(function (source) {
            const ObsInput = obs.Input.fromName(source.source.displayName);
            sourcesSize.push({ id: source.source.sourceState.id, height: ObsInput.height, width: ObsInput.width, outputFlags: ObsInput.outputFlags });
        });
    }
    return sourcesSize;
}
exports.getSourcesSize = getSourcesSize;
