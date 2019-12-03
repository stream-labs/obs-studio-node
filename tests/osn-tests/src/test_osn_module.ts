import 'mocha';
import * as osn from '../osn';
import * as path from 'path';
import * as fs from 'fs';
import * as logger from '../util/logger';
import { expect } from 'chai';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';

const testName = 'osn-module';

describe(testName, () => {
    let obs: OBSHandler;

    // Initialize OBS process
    before(function() {
        logger.logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
        logger.logInfo(testName, 'Finished ' + testName + ' tests');
        logger.logEmptyLine();
    });

    it('Open all module types and initialize them', () => {
        let moduleTypes: string[] = [];

        fs.readdirSync(path.join(path.normalize(osn.DefaultPluginPath), '64bit')).forEach(file => {
            if (file.endsWith('.dll')) {
                if (file != 'chrome_elf.dll' && 
                    file != 'libcef.dll' &&
                    file != 'libEGL.dll' &&
                    file != 'libGLESv2.dll') {
                    // Opening module
                    const moduleType = osn.ModuleFactory.open(path.join(path.normalize(osn.DefaultPluginPath), '64bit/' + file), path.normalize(osn.DefaultDataPath));

                    // Checking if module was opened properly
                    expect(moduleType).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.OpenModule, file));

                    // Initializing module
                    expect(function () {
                        moduleType.initialize();
                    }).to.not.throw();

                    // Adding to moduleArrays to use in check later
                    moduleTypes.push(file);
                }   
            }
        });

        // Getting all modules
        const modules = osn.ModuleFactory.modules();

        // Checking if returned modules are the ones opened
        expect(modules).to.include.members(moduleTypes, GetErrorMessage(ETestErrorMsg.Modules));
    });
});
