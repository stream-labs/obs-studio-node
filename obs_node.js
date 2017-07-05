const {remote} = require('electron');
const obs = remote.require('./obs_node.node');

export const ObsGlobal = obs.Global;
export const ObsInput = obs.Input;
export const ObsScene = obs.Scene;
export const ObsFilter = obs.Filter;
export const ObsTransition = obs.Transition;
export const ObsSceneItem = obs.SceneItem;
export const ObsProperties = obs.Properties;
export const ObsProperty = obs.Property;
export const ObsDisplay = obs.Display;
export const ObsSource = obs.ISource;