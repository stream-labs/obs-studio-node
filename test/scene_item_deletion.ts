import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('scene item deletion', async t => {
    await startup_shutdown(t, (t) => {
        let test_source = 
            obs.ObsInput.create('color_source', 'test source');

        let test_scene = 
            obs.ObsScene.create('test scene');

        let item1 = test_scene.add(test_source);
        let item2 = test_scene.add(test_source);

        t.is(item1.id, 1);
        t.is(item2.id, 2);

        item1.remove();
        item1 = test_scene.add(test_source);

        t.is(item2.id, 2);
        t.is(item1.id, 3);

        /* Test movement */
        t.is(test_scene.getItems()[0].id, 2);
        test_scene.moveItem(1, 0);
        t.is(test_scene.getItems()[0].id, 3);
        test_scene.moveItem(0, 1);
        t.is(test_scene.getItems()[0].id, 2);

        test_scene.moveItem(4, 0);
        t.is(test_scene.getItems()[0].id, 2);

        let test_item1 = test_scene.getItemAtIdx(0);
        t.is(test_item1.id, 3);

        test_scene.release();
        test_source.release();
    })
});