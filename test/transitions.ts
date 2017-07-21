import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('transition creation and destruction', t => {
    startup_shutdown(t, (t) => {
        let test_source_a = 
            obs.ObsInput.createPrivate('monitor_capture', 'test source a');

        let test_source_b = 
            obs.ObsInput.createPrivate('color_source', 'test source b');

        let test_scene =
            obs.ObsScene.create('test scene');

        let test_transition = 
            obs.ObsTransition.create('fade_transition', 'test transition');

        t.is(test_source_a.status, 0);
        t.is(test_source_a.name, 'test source a');
        t.is(test_source_a.id, 'monitor_capture');
        t.is(test_source_a.configurable, true);
        t.is(test_source_a.type, obs.ESourceType.Input);

        t.is(test_source_b.status, 0);
        t.is(test_source_b.name, 'test source b');
        t.is(test_source_b.id, 'color_source');
        t.is(test_source_b.configurable, true);
        t.is(test_source_b.type, obs.ESourceType.Input);

        test_scene.add(test_source_b);

        t.is(test_transition.status, 0);
        t.is(test_transition.name, 'test transition');
        t.is(test_transition.id, 'fade_transition');
        t.is(test_transition.type, obs.ESourceType.Transition);
        t.is(test_transition.configurable, false);
        
        test_transition.set(test_source_a);
        test_transition.start(0, test_source_b);
        test_transition.start(0, test_scene);

        let active_source = test_transition.getActiveSource();

        t.is(active_source.name, 'test scene');
        t.is(active_source.type, obs.ESourceType.Scene);

        test_transition.start(0, test_source_b);
        active_source = test_transition.getActiveSource();
        
        t.is(active_source.name, 'test source b');
        t.is(active_source.type, obs.ESourceType.Input);

        test_transition.release();
        test_source_a.release();
        test_source_b.release();
        test_scene.release();
        
        t.is(test_source_a.status, 1, "Failed to destroy source");
        t.is(test_source_b.status, 1, "Failed to destroy source");
        t.is(test_scene.status, 1, "Failed to destroy scene");
        t.is(test_transition.status, 1, "Failed to destroy transitions");
    });
});