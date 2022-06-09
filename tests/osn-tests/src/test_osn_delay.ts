import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { OBSHandler } from '../util/obs_handler'
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums';

const testName = 'osn-delay';

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

    it('Create delay', () => {
        const delay = osn.DelayFactory.create();
        expect(delay).to.not.equal(undefined, 'Invalid delay creation');

        expect(delay.enabled).to.equal(false, 'Invalid delay enabled default value');
        expect(delay.delaySec).to.equal(20, 'Invalid delay sec default value');
        expect(delay.preserveDelay).to.equal(true, 'Invalid preserve delay default value');
    });

    it('Update delay', () => {
        const delay = osn.DelayFactory.create();
        expect(delay).to.not.equal(undefined, 'Invalid delay creation');

        delay.enabled = true;
        delay.delaySec = 50;
        delay.preserveDelay = false;

        expect(delay.enabled).to.equal(true, 'Invalid delay enabled default value');
        expect(delay.delaySec).to.equal(50, 'Invalid delay sec default value');
        expect(delay.preserveDelay).to.equal(false, 'Invalid preserve delay default value');
    });
});
