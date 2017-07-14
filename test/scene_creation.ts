import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('scene creation and destruction', t => {
    startup_shutdown(t, (t) => {
        let test_scene = obs.ObsScene.create('test scene');

        t.is(test_scene.status, 0);
        t.is(test_scene.id, 'scene');
        t.is(test_scene.name, 'test scene');
        t.is(test_scene.configurable, false);
        t.is(test_scene.properties, null);
        t.is(test_scene.type, obs.ESourceType.Scene);
        t.is(test_scene.source.id, 'scene');
        t.is(test_scene.source.name, 'test scene');
        t.is(test_scene.source.configurable, false);
        t.is(test_scene.source.properties, null);
        t.is(test_scene.source.type, obs.ESourceType.Scene);
    });
});