import 'mocha';
import { expect } from 'chai';
import * as osn from 'obs-studio-node';
import * as path from 'path';
import * as fs from 'fs';
import { OBSProcessHandler } from '../util/obs_process_handler';

describe('osn-module', () => {
    let obs: OBSProcessHandler;

    // Initialize OBS process
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
    });

    context('# Open, Initialize and Modules', () => {
        let moduleTypes: string[] = [];

        it('Open all module types and initialize them', () => {
            fs.readdirSync(path.join(path.normalize(osn.DefaultPluginPath), '64bit')).forEach(function(file) {
                if (file.endsWith('.dll')) {
                    // Opening module
                    const module = osn.ModuleFactory.open(path.join(path.normalize(osn.DefaultPluginPath), '64bit/' + file), path.normalize(osn.DefaultDataPath));

                    // Checking if module was opened properly
                    expect(module).to.not.equal(undefined);

                    // Initializing module
                    expect(function () {
                        module.initialize();
                    }).to.not.throw;

                    // Adding to moduleArrays to use in check later
                    moduleTypes.push(file);
                }
            });
        });

        it('Get all opened modules', () => {
            // Getting all modules
            const modules = osn.ModuleFactory.modules();

            // Checking if returned modules are the ones opened
            expect(modules).to.include.members(moduleTypes);
        });
    });
});
