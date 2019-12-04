import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';

const testName = 'osn-video';

describe(testName, () => {
    let obs: OBSHandler;

    // Initialize OBS process
    before(function() {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
        logInfo(testName, 'Finished ' + testName + ' tests');
        logEmptyLine();
    });

    context('# GetSkippedFrames', () => {
        it('Get skipped frames value', () => {
            // Getting skipped frames
            const skippedFrames = osn.Video.skippedFrames;

            // Checking if skipped frames was returned properly
            expect(skippedFrames).to.not.equal(undefined);
            expect(skippedFrames).to.equal(0);
        });
    });

    context('# GetTotalFrames', () => {
        it('Get total frames value', () => {
            // Getting total frames value
            const totalFrames = osn.Video.encodedFrames;

            // Checking if total frames was returned properly
            expect(totalFrames).to.not.equal(undefined);
            expect(totalFrames).to.equal(0);
        });
    });
});
