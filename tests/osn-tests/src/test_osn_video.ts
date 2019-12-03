import 'mocha';
import * as osn from '../osn';
import * as logger from '../util/logger';
import { expect } from 'chai';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';

const testName = 'osn-video';

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

    it('Get skipped frames value', () => {
        // Getting skipped frames
        const skippedFrames = osn.Video.skippedFrames;

        // Checking if skipped frames was returned properly
        expect(skippedFrames).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.VideoSkippedFrames));
        expect(skippedFrames).to.equal(0, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
    });

    it('Get total frames value', () => {
        // Getting total frames value
        const totalFrames = osn.Video.encodedFrames;

        // Checking if total frames was returned properly
        expect(totalFrames).to.not.equal(undefined,  GetErrorMessage(ETestErrorMsg.VideoTotalFrames));
        expect(totalFrames).to.equal(0,  GetErrorMessage(ETestErrorMsg.VideoTotalFramesWrongValue));
    });
});
