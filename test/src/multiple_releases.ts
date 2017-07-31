import * as obs from 'obs-studio-node';
import { startup_shutdown } from '../helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test.failing('multiple releases', async t => {
    await startup_shutdown(t, async (t) => {
        let test_source = 
            obs.ObsInputFactory.create('color_source', 'test source');

        let test_scene = 
            obs.ObsSceneFactory.create('test scene');

        test_scene.add(test_source);

        let test_ref_1 = obs.ObsInputFactory.fromName('test source');
        let test_ref_2 = obs.ObsInputFactory.getPublicSources()[0];

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

        test_scene.release();

        test_ref_2.release();
        
        t.is(test_source.status, 1);

        /* Since we added the source to a scene, there's now two references. 
           This means, the internal weak pointer will still resolve to a 
           strong reference even though we shouldn't have anymore handles 
           from javascript since we already called release. Thus, the 
           following will crash. If we destroyed the scene first, the 
           source would be destroyed and these would turn into no-ops. 
           However, even with a fix in place, this can always be abused. 
           For instance, even if I nullify all handles currently held in 
           javascript, you can refetch those handles by requesting scene items 
           from javascript and fetching their corresponding input sources. 
           I'm going to let this fail on purpose to act as a reminder. FIXME */
        await t.notThrows(test_ref_1.release());
        await t.notThrows(test_source.release());
    });
});