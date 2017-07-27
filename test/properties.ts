import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('source properties', async t => {
    await startup_shutdown(t, (t) => {
        let test_source_1 = 
            obs.ObsInput.createPrivate('color_source', 'test source' /* color: 0xffffffff */);

        let test_source_2 = 
            obs.ObsInput.createPrivate('color_source', 'test source', { color: 0x00000000 });

        let test_source_3 = 
            obs.ObsInput.createPrivate('window_capture', 'test source');

        t.is(test_source_1.status, 0);
        t.is(test_source_1.name, 'test source');
        t.is(test_source_1.id, 'color_source');
        t.is(test_source_1.configurable, true);
        t.is(test_source_1.type, obs.ESourceType.Input);

        let test_source_2_settings = test_source_2.settings;

        t.is(test_source_1.settings["color"], 0xffffffff);

        t.is(test_source_2_settings["color"], 0x00000000);
        test_source_2_settings["color"] = 0xffffffff;
        test_source_2.update(test_source_2_settings);
        t.is(test_source_2.settings["color"], 0xffffffff);

        let test_source_1_props = test_source_1.properties;
        let prop = test_source_1_props.first();

        while (!prop.done) {
            console.log(prop);
            prop.next();
        }

        console.log(test_source_3.settings);
        console.log(test_source_3.properties);

        test_source_1.release();
        test_source_2.release();
        test_source_3.release();

        t.is(test_source_1.status, 1);
        t.is(test_source_2.status, 1);
        t.is(test_source_3.status, 1);
    });
});