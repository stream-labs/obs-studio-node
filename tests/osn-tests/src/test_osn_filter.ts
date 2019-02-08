import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IFilter, ISettings } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSFilterTypes } from '../util/general';

describe('osn-filter', () => {
    let obs: OBSProcessHandler;
    let filterTypes: string[];

    // Initialize OBS process
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() != true)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    // Shutdown OBS process
    after(function(done) {
        this.timeout(3000);
        obs.shutdown();
        obs = null;
        setTimeout(done, 3000);
    });

    context('# Types', () => {
        it('Get all filter types', () => {
            // Gettin all filter types
            filterTypes = osn.FilterFactory.types();

            // Checking if filterTypes array contains the basic obs filter types
            expect(filterTypes.length).to.not.equal(0);
            expect(filterTypes).to.include.members(basicOBSFilterTypes);
        });
    });

    context('# Create', () => {
        it('Create all filter types', () => {
            let filterType: string;
            let filter: IFilter;

            // Create each filter type available
            for (filterType of filterTypes)
            {
                filter = osn.FilterFactory.create(filterType, 'filter');

                // Checking if filter was created correctly
                expect(filter).to.not.equal(undefined);
                expect(filter.id).to.equal(filterType);
                expect(filter.name).to.equal('filter');
                filter.release();
            }
        });

        it('Create all filter types with settings', () => {
            let filterType: string;
            let filter: IFilter;
            let settings: ISettings = {};
            settings['test'] = 1;
            
            // Create each filter type availabe passing settings parameter
            for (filterType of filterTypes)
            {
                filter = osn.FilterFactory.create(filterType, 'filter', settings);

                // Checking if filter was created correctly
                expect(filter).to.not.equal(undefined);
                expect(filter.id).to.equal(filterType);
                expect(filter.name).to.equal('filter');
                filter.release();
            }
        });
    });
});