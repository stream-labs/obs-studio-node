import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('scene creation and destruction', t => {
    startup_shutdown(t, (t) => {
        {
            let test_scene = obs.ObsScene.create('test scene');
            let sources: obs.ObsSource[] = [];
            let items: obs.ObsSceneItem[] = [];
            const iterations = 100;

            for (let i = 0; i < iterations; i++) {
                let source = obs.ObsInput.create('color_source', `test source ${i}`);
                let item = test_scene.add(source);
                source.release();
                sources.push(item.source);
                t.is(sources[i].status, 0);
                items.push(item);
            };

            let test_scene_item_list = test_scene.getItems();
            t.is(test_scene_item_list.length, iterations);
            console.log(test_scene.settings);

            for (let i = 0; i < iterations; i++) {
                items[i].selected = true;
                t.is(items[i].selected, true);
                items[i].selected = false;
                t.is(items[i].selected, false);

                items[i].visible = true;
                t.is(items[i].visible, true);
                items[i].visible = false;
                t.is(items[i].visible, false);

                t.is(items[i].position.x, 0);
                t.is(items[i].position.y, 0);
                t.is(items[i].rotation, 0);
                t.is(items[i].scale.x, 1);
                t.is(items[i].scale.y, 1);
                t.is(items[i].alignment, obs.EAlignment.TopLeft);
                t.is(items[i].boundsAlignment, 0);
                t.is(items[i].boundsType, obs.EBoundsType.None);
                t.is(items[i].scaleFilter, obs.EScaleType.Default);
                t.is(items[i].id, i + 1);

                items[i].position = { x: 1, y: 1 }
                t.is(items[i].position.x, 1);
                t.is(items[i].position.y, 1);

                /* TODO: Use other move functions */
                items[i].move(items[i].id);
                t.is(items[i].id, i + 1);

                let transInfo = items[i].transformInfo;

                // Default values
                t.is(transInfo.pos.x, 1);
                t.is(transInfo.pos.y, 1);
                t.is(transInfo.rot, 0);
                t.is(transInfo.scale.x, 1);
                t.is(transInfo.scale.y, 1);
                t.is(transInfo.bounds.x, 0);
                t.is(transInfo.bounds.y, 0);
                t.is(transInfo.alignment, obs.EAlignment.TopLeft);
                t.is(transInfo.boundsAlignment, 0);
                t.is(transInfo.boundsType, obs.EBoundsType.None);

                let cropInfo = items[i].crop;

                t.is(cropInfo.left, 0);
                t.is(cropInfo.top, 0);
                t.is(cropInfo.right, 0);
                t.is(cropInfo.bottom, 0);

                items[i].crop = { left: 1, right: 1, top: 1, bottom: 1 }
                cropInfo = items[i].crop;

                t.is(cropInfo.left, 1);
                t.is(cropInfo.top, 1);
                t.is(cropInfo.right, 1);
                t.is(cropInfo.bottom, 1);

                items[i].alignment = obs.EAlignment.BottomRight;
                t.is(transInfo.alignment, obs.EAlignment.TopLeft);
                t.is(items[i].alignment, obs.EAlignment.BottomRight);
                t.is(items[i].transformInfo.alignment, obs.EAlignment.BottomRight);

            }

            for (let i = 0; i < iterations; i++) {
                t.is(items[i].id, i + 1);
                t.is(items[i].source.name, `test source ${i}`);
                t.is(items[i].scene.name, 'test scene');

                let found_scene_item = items[i].scene.findItem(items[i].source.name);
                t.is(found_scene_item.source.name, `test source ${i}`);
                t.is(found_scene_item.scene.name, 'test scene');

                let null_scene_item = items[i].scene.findItem('a bad name');
                t.is(null_scene_item, null);
            };

            test_scene.release();
            t.is(test_scene.status, 1);

            for (let i = 0; i < iterations; i++) {
                t.is(sources[i].status, 1);
            }
        }

        {
            let test_scene = obs.ObsScene.create('test scene');
            let sources: obs.ObsSource[] = [];
            let items: obs.ObsSceneItem[] = [];
            const iterations = 100;

            for (let i = 0; i < iterations; i++) {
                let source = obs.ObsInput.create('color_source', `test source ${i}`);
                let item = test_scene.add(source);
                sources.push(item.source);
                t.is(sources[i].status, 0);
                items.push(item);
            };

            let test_scene_item_list = test_scene.getItems();
            t.is(test_scene_item_list.length, iterations);

            for (let i = 0; i < iterations; i++) {
                t.is(items[i].id, i + 1);
                t.is(items[i].source.name, `test source ${i}`);
                t.is(items[i].scene.name, 'test scene');

                let found_scene_item = items[i].scene.findItem(items[i].source.name);
                t.is(found_scene_item.source.name, `test source ${i}`);
                t.is(found_scene_item.scene.name, 'test scene');

                let null_scene_item = items[i].scene.findItem('a bad name');
                t.is(null_scene_item, null);
            };

            test_scene_item_list.forEach((item) => {
                item.source.release();
            });

            test_scene.release();
            t.is(test_scene.status, 1);

            for (let i = 0; i < iterations; i++) {
                t.is(sources[i].status, 1);
            }
        }

        {
            let test_scene = obs.ObsScene.create('test scene');
            let sources: obs.ObsSource[] = [];
            let items: obs.ObsSceneItem[] = [];
            const iterations = 100;

            for (let i = 0; i < iterations; i++) {
                let source = obs.ObsInput.create('color_source', `test source ${i}`);
                let item = test_scene.add(source);
                sources.push(item.source);
                t.is(sources[i].status, 0);
                items.push(item);
                t.is(obs.ObsInput.fromName(source.name) != null, true);
                t.is(obs.ObsScene.fromName(test_scene.name) != null, true);
            };

            let test_scene_item_list = test_scene.getItems();
            t.is(test_scene_item_list.length, iterations);

            for (let i = 0; i < iterations; i++) {
                t.is(items[i].id, i + 1);
                t.is(items[i].source.name, `test source ${i}`);
                t.is(items[i].scene.name, 'test scene');

                let found_scene_item = items[i].scene.findItem(items[i].source.name);
                t.is(found_scene_item.source.name, `test source ${i}`);
                t.is(found_scene_item.scene.name, 'test scene');

                let found_id_scene_item = items[i].scene.findItem(items[i].id);
                t.is(found_id_scene_item.source.name, `test source ${i}`);
                t.is(found_id_scene_item.scene.name, 'test scene');

                let null_scene_item = items[i].scene.findItem('a bad name');
                t.is(null_scene_item, null);
                items[i].source.release();
            };

            test_scene.release();
            t.is(test_scene.status, 1);

            for (let i = 0; i < iterations; i++) {
                t.is(sources[i].status, 1);
            }
        }
    });
});