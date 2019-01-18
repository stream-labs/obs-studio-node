import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IFilter } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { getCppErrorMsg } from '../util/general';

describe('osn-filter', () => {
    let obs: OBSProcessHandler;

    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() != true)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    after(function(done) {
        this.timeout(5000);
        obs.shutdown();
        obs = null;
        setTimeout(done, 3000);
    });

    context('# Create', () => {
        it('should create filter', () => {
            let filter: IFilter

            try {
                filter = osn.FilterFactory.create('test_filter', 'filter1');
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }
            
            expect(filter.id).to.equal('test_filter');
            expect(filter.name).to.equal('filter1');
        });
    });

    context('# Types', () => {
        it('should get all filter types', () => {
            let filterTypes: string[];

            try {
                filterTypes = osn.FilterFactory.types();
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }

            expect(filterTypes.length).to.not.equal(0);
        });
    });
});