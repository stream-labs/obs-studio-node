import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('source creation and destruction', t => {
    startup_shutdown(t, (t) => {
        let test_source = 
            obs.ObsInput.create('color_source', 'test source');

        t.is(test_source.status, 0);
        t.is(test_source.name, 'test source');
        t.is(test_source.id, 'color_source');
        t.is(test_source.configurable, true);
        t.is(test_source.type, obs.ESourceType.Input);

        let test_source_dup = test_source.duplicate('test source dup', true);
        t.is(test_source_dup.status, 0);
        t.is(test_source_dup.name, 'test source dup');
        t.is(test_source_dup.id, 'color_source');
        t.is(test_source_dup.configurable, true);
        t.is(test_source_dup.type, obs.ESourceType.Input);
        
        test_source.release();
        t.is(test_source.status, 1);

        test_source_dup.release();
        t.is(test_source_dup.status, 1);
    });
});