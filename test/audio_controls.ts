import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('audio controls', t => {
    startup_shutdown(t, (t) => {
        let test_source = 
            obs.ObsInput.create('ffmpeg_source', 'test source');

        t.is(test_source.status, 0);
        t.is(test_source.name, 'test source');
        t.is(test_source.id, 'ffmpeg_source');
        t.is(test_source.configurable, true);
        t.is(test_source.type, obs.ESourceType.Input);
        
        let audio_volmeter = obs.ObsVolmeter.create(obs.EFaderType.Log);

        audio_volmeter.attach(test_source);
        audio_volmeter.detach();

        audio_volmeter.attach(test_source);

        let cbPromise = new Promise((resolve, reject) => {
            audio_volmeter.addCallback(({level, magnitude, peak, muted}) => {
                console.log("WHYYYY");
            });
        });

        return cbPromise;
    });
});