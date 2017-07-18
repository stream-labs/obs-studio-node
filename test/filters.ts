import * as obs from '../node-obs/obs_node.js';
import { startup_shutdown } from './helpers/startup_shutdown'
import * as path from 'path';
import test from 'ava';

test('source creation and destruction', t => {
    startup_shutdown(t, (t) => {
        let test_source =
            obs.ObsInput.create('ffmpeg_source', 'test source');

        const iterations = 100;

        t.is(test_source.status, 0);
        t.is(test_source.name, 'test source');
        t.is(test_source.id, 'ffmpeg_source');
        t.is(test_source.configurable, true);

        let test_filters: obs.ObsFilter[] = [];
        let filter_types = obs.ObsFilter.types();

        for (var i = 0; i < iterations; i++) {
            filter_types.forEach((element) => {
                let filter =
                    obs.ObsFilter.create(element, `${element} ${i}`);

                t.is(filter.id, `${element}`);
                t.is(filter.name, `${element} ${i}`);

                t.is(filter.status, 0);
                test_source.addFilter(filter);
                test_filters.push(filter);
            });
        }

        t.is(iterations * filter_types.length, test_filters.length);

        let filter_list_fetch = test_source.filters;
        t.is(filter_list_fetch.length, test_filters.length);

        for (var i = 0; i < iterations; i++) {
            filter_types.forEach((element, idx) => {
                let index = (i * filter_types.length) + idx;
                t.is(test_filters[index].id, `${element}`);
                t.is(test_filters[index].name, `${element} ${i}`);
                t.is(test_filters[index].configurable, true);
                t.is(test_filters[index].type, obs.ESourceType.Filter);
                t.is(test_filters[index].status, 0);

                let found_filter = test_source.findFilter(test_filters[index].name);
                t.is(found_filter == null, false);

                test_source.removeFilter(test_filters[index]);
                let removed_filter = test_source.findFilter(test_filters[index].name);
                t.is(removed_filter, null);

                test_filters[index].release();
                
                if (test_filters[index].status == 0) {
                    console.log(`${test_filters[index].name} failed to destroy`);
                }

                t.is(test_filters[index].status, 1);
            });
        }

        test_source.release();
        t.is(test_source.status, 1, "Failed to destroy source");
    });
});