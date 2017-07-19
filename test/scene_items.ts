import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('scene creation and destruction', t => {
    startup_shutdown(t, (t) => {
        let test_scene = obs.ObsScene.create('test scene');
        let sources: obs.ObsSource[] = [];
        let items: obs.ObsSceneItem[] = [];
        const iterations = 100;

        for (let i = 0; i < iterations; i++) {
            let source = obs.ObsInput.create('color_source', `test source ${i}`);
            let item = test_scene.add(source);
            sources.push(source);
            items.push(item);
        };

        for (let i = 0; i < iterations; i++) {
            t.is(items[i].id, i + 1);
            t.is(items[i].source.name, `test source ${i}`);
            t.is(items[i].scene.name, 'test scene');

            let found_scene_item = items[i].scene.findItem(items[i].source.name);
            t.is(found_scene_item.source.name, `test source ${i}`);
            t.is(found_scene_item.scene.name, 'test scene');

            let null_scene_item = items[i].scene.findItem('a bad name');
            t.is(null_scene_item, null);

            items[i].remove();
            sources[i].release();
            t.is(sources[i].status, 1);
        };

        test_scene.release();
        t.is(test_scene.status, 1);
    });
});