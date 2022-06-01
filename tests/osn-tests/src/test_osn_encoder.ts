import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { OBSHandler } from '../util/obs_handler'
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums';

const testName = 'osn-encoder';

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

    it('Create a video encoder', async () => {
        await new Promise(r => setTimeout(r, 5000));

        const types = osn.EncoderFactory.types();
        console.log(types);

        const encoder = osn.EncoderFactory.create('obs_x264', "My x264", {});
        console.log(encoder);

        const props = encoder.properties;
        let prop: any = props.first();
        while (prop) {
            console.log(prop.name);
            prop = prop.next();
        }

        console.log(encoder.settings);
        encoder.update({ bitrate: 6000 });
        console.log(encoder.settings);
        console.log(encoder.name);
        encoder.name = 'New name';
        console.log(encoder.name);
        console.log(encoder.type);
        console.log(encoder.active);
        console.log(encoder.id);
        console.log(encoder.lastError);
    });
});
