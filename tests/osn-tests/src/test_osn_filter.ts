import 'mocha'
import { expect } from 'chai'
import { OBSProcessHandler } from '../util/obs_process_handler'
import * as osn from 'obs-studio-node';
import { IFilter } from 'obs-studio-node';

describe('osn-filter', () => {
    let obs: OBSProcessHandler;

    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() != true)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    after(function() {
        obs.shutdown();
        obs = null;
    });

    context('# Create', () => {
        let filter: IFilter

        it('should create filter', () => {
            try {
                filter = osn.FilterFactory.create('test_filter', 'filter1');
            } catch(e) {
                throw new Error("failed to create filter");
            }
            
            expect(filter.id).to.equal('test_filter');
            expect(filter.name).to.equal('filter1');
        });
    });
});