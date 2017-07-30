import * as obs from 'obs-studio-node';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('scene creation and destruction', async t => {
    await startup_shutdown(t, (t) => {
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

        let test_scene_from_name = obs.ObsScene.fromName('test scene');
        t.is(test_scene_from_name.status, 0);
        t.is(test_scene_from_name.id, 'scene');
        t.is(test_scene_from_name.name, 'test scene');
        t.is(test_scene_from_name.configurable, false);
        t.is(test_scene_from_name.properties, null);
        t.is(test_scene_from_name.type, obs.ESourceType.Scene);
        t.is(test_scene_from_name.source.id, 'scene');
        t.is(test_scene_from_name.source.name, 'test scene');
        t.is(test_scene_from_name.source.configurable, false);
        t.is(test_scene_from_name.source.properties, null);
        t.is(test_scene_from_name.source.type, obs.ESourceType.Scene);

        test_scene_from_name.release();
        t.is(test_scene.status, 1);
    });
});