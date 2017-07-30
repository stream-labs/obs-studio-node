import * as obs from 'obs-studio-node';
import * as path from 'path';
import test from 'ava';

test('startup and shutdown', async t => {
    t.plan(4);

    let locale = 'en-US';

    obs.ObsGlobal.startup(locale);
    t.is(obs.ObsGlobal.initialized, true);
    t.is(obs.ObsGlobal.locale, locale);

    obs.ObsGlobal.shutdown();
    t.is(obs.ObsGlobal.initialized, false);
    t.is(obs.ObsGlobal.locale, undefined);
});