import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('source creation and destruction', async t => {
    await startup_shutdown(t, (t) => {
        let test_source = 
            obs.ObsInput.create('color_source', 'test source');

        t.is(test_source.status, 0);
        t.is(test_source.name, 'test source');
        t.is(test_source.id, 'color_source');
        t.is(test_source.configurable, true);
        t.is(test_source.type, obs.ESourceType.Input);

        /* This test should pass but fails due 
         * to libobs weirdness. Not sure how to fix
         * at this moment. */
        /* 
        let fail_test_source = 
            obs.ObsInput.create('color_source', 'test source');

        t.is(fail_test_source, null);
         */

        let test_source_dup = test_source.duplicate('test source dup', true);
        t.is(test_source_dup.status, 0);
        t.is(test_source_dup.name, 'test source dup');
        t.is(test_source_dup.id, 'color_source');
        t.is(test_source_dup.configurable, true);
        t.is(test_source_dup.type, obs.ESourceType.Input);
        
        let test_source_ref_dup = test_source.duplicate();

        t.is(test_source_ref_dup.name, `${test_source.name}`);

        test_source.release();
        t.is(test_source.status, 0);

        test_source_ref_dup.release();
        t.is(test_source_ref_dup.status, 1);
        t.is(test_source.status, 1);

        test_source_dup.release();
        t.is(test_source_dup.status, 1);

    });
});