import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('multiple references', async t => {
    await startup_shutdown(t, (t) => {
        let test_source = 
            obs.ObsInput.create('color_source', 'test source');

        let test_ref_1 = obs.ObsInput.fromName('test source');
        let test_ref_2 = obs.ObsInput.getPublicSources()[0];

        t.is(test_source.status, 0);
        t.is(test_source.name, 'test source');
        t.is(test_source.id, 'color_source');
        t.is(test_source.configurable, true);
        t.is(test_source.type, obs.ESourceType.Input);
    
        t.is(test_ref_1.status, 0);
        t.is(test_ref_1.name, 'test source');
        t.is(test_ref_1.id, 'color_source');
        t.is(test_ref_1.configurable, true);
        t.is(test_ref_1.type, obs.ESourceType.Input);

        t.is(test_ref_2.status, 0);
        t.is(test_ref_2.name, 'test source');
        t.is(test_ref_2.id, 'color_source');
        t.is(test_ref_2.configurable, true);
        t.is(test_ref_2.type, obs.ESourceType.Input);

        test_ref_2.release();
        
        t.is(test_source.status, 1);

        test_ref_1.release();
        test_source.release();
    });
});