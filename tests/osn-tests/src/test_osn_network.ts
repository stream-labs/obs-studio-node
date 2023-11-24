import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { OBSHandler } from '../util/obs_handler'
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums';

const testName = 'osn-network';

describe(testName, () => {
    let obs: OBSHandler;
    let hasTestFailed: boolean = false;

    // Initialize OBS process
    before(function() {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
    });

    // Shutdown OBS process
    after(async function() {
        obs.shutdown();

        if (hasTestFailed === true) {
            logInfo(testName, 'One or more test cases failed. Uploading cache');
            await obs.uploadTestCache();
        }

        obs = null;
        deleteConfigFiles();
        logInfo(testName, 'Finished ' + testName + ' tests');
        logEmptyLine();
    });

    afterEach(function() {
        if (this.currentTest.state == 'failed') {
            hasTestFailed = true;
        }
    });

    it('Create network', () => {
        const network = osn.NetworkFactory.create();
        expect(network).to.not.equal(undefined, 'Invalid network creation');

        expect(network.bindIP).to.equal('default', 'Invalid network bindIP default value');
        expect(network.enableDynamicBitrate).to.equal(false, 'Invalid network enableDynamicBitrate default value');
        expect(network.enableOptimizations).to.equal(false, 'Invalid network enableOptimizations default value');
        expect(network.enableLowLatency).to.equal(false, 'Invalid network enableLowLatency default value');
    });

    it('Update reconnect', () => {
        const network = osn.NetworkFactory.create();
        expect(network).to.not.equal(undefined, 'Invalid network creation');

        network.enableDynamicBitrate = true;
        network.enableOptimizations = true;
        network.enableLowLatency = true;

        expect(network.enableDynamicBitrate).to.equal(true, 'Invalid network enableDynamicBitrate value');
        expect(network.enableOptimizations).to.equal(true, 'Invalid network enableOptimizations value');
        expect(network.enableLowLatency).to.equal(true, 'Invalid network enableLowLatency value');
    });
});
