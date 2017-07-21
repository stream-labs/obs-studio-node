import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('source properties', t => {
    startup_shutdown(t, (t) => {
        let test_source_1 = 
            obs.ObsInput.create('color_source', 'test source');

        let test_source_2 = 
            obs.ObsInput.create('color_source', 'test source', { color: 0x00000000 });

        console.log(test_source_1);
        console.log(test_source_2);

        t.is(test_source_1.status, 0);
        t.is(test_source_1.name, 'test source');
        t.is(test_source_1.id, 'color_source');
        t.is(test_source_1.configurable, true);
        t.is(test_source_1.type, obs.ESourceType.Input);

        let test_source_2_settings = test_source_2.settings;
        console.log(test_source_2_settings);

        t.is(test_source_1.settings["color"], 0xffffffff);
        t.is(test_source_2_settings["color"], 0x00000000);
        test_source_2_settings["color"] = 0xffffffff;
        test_source_2.update(test_source_2_settings);
        t.is(test_source_2.settings["color"], 0xffffffff);

        test_source_1.release();
        test_source_2.release();

        t.is(test_source_1.status, 1);
        t.is(test_source_2.status, 1);
    });
});