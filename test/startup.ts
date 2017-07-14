import * as obs from '../node-obs/obs_node.js';
import * as path from 'path';
import test from 'ava';

test('startup and shutdown', t => {
    t.plan(4);

    let locale = 'en-US';

    obs.ObsGlobal.startup(locale);
    t.is(obs.ObsGlobal.initialized, true);
    t.is(obs.ObsGlobal.locale, locale);

    obs.ObsGlobal.shutdown();
    t.is(obs.ObsGlobal.initialized, false);
    t.is(obs.ObsGlobal.locale, undefined);
});