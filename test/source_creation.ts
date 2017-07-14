import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('source creation and destruction', t => {
    startup_shutdown(t, (t) => {
        let test_source = 
            obs.ObsInput.create('monitor_capture', 'test source');

        t.is(test_source.status, 0);
        t.is(test_source.name, 'test source');
        t.is(test_source.id, 'monitor_capture');
        t.is(test_source.configurable, true);
        
        test_source.release();
        
        t.is(test_source.status, 1);
    });
});