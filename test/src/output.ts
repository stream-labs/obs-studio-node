import * as obs from 'obs-studio-node';
import { startup_shutdown } from '../helpers/startup_shutdown';
import * as path from 'path';
import test from 'ava';

test('output channel setting', async t => {
    await startup_shutdown(t, (t) => {
        let test_source = 
            obs.ObsInputFactory.createPrivate(
                'color_source',
                'test source',
            );

        let test_scene =
            obs.ObsSceneFactory.create('test scene');

        let test_transition = 
            obs.ObsTransitionFactory.create('fade_transition', 'test transition');

        test_transition.set(test_scene);
        test_scene.add(test_source);
        
        let source_check;

        obs.ObsGlobal.setOutputSource(0, test_source);
        source_check = obs.ObsGlobal.getOutputSource(0);
        source_check = obs.ObsGlobal.getOutputSource(0);
        source_check = obs.ObsGlobal.getOutputSource(0);
        t.is(source_check.name, 'test source');

        obs.ObsGlobal.setOutputSource(0, test_scene);
        source_check = obs.ObsGlobal.getOutputSource(0);
        source_check = obs.ObsGlobal.getOutputSource(0);
        source_check = obs.ObsGlobal.getOutputSource(0);
        t.is(source_check.name, 'test scene');

        obs.ObsGlobal.setOutputSource(0, test_transition);
        source_check = obs.ObsGlobal.getOutputSource(0);
        source_check = obs.ObsGlobal.getOutputSource(0);
        source_check = obs.ObsGlobal.getOutputSource(0);
        t.is(source_check.name, 'test transition');

        test_source.release();
        test_scene.release();
        test_transition.release();

        t.is(test_source.status, 0);
        t.is(test_scene.status, 0);
        t.is(test_transition.status, 0);

        obs.ObsGlobal.setOutputSource(0, null);

        t.is(test_source.status, 1);
        t.is(test_scene.status, 1);
        t.is(test_transition.status, 1);
    });
});