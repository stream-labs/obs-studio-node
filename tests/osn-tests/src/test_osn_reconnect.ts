import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { OBSHandler } from '../util/obs_handler'
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums';

const testName = 'osn-reconnect';

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

    it('Create reconnect', () => {
        const reconnect = osn.ReconnectFactory.create();
        expect(reconnect).to.not.equal(undefined, 'Invalid reconnect creation');

        expect(reconnect.enabled).to.equal(true, 'Invalid reconnect enabled default value');
        expect(reconnect.retryDelay).to.equal(10, 'Invalid reconnect retry delay default value');
        expect(reconnect.maxRetries).to.equal(20, 'Invalid reconnect max retries default value');
    });

    it('Update reconnect', () => {
        const reconnect = osn.ReconnectFactory.create();
        expect(reconnect).to.not.equal(undefined, 'Invalid reconnect creation');

        reconnect.enabled = false;
        reconnect.retryDelay = 50;
        reconnect.maxRetries = 100;

        expect(reconnect.enabled).to.equal(false, 'Invalid reconnect enabled value');
        expect(reconnect.retryDelay).to.equal(50, 'Invalid reconnect retry delay value');
        expect(reconnect.maxRetries).to.equal(100, 'reconnect max retries value');
    });
});
