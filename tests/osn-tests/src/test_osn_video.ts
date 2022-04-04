import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';

const testName = 'osn-video';

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

    it('Get and set video context', () => {
        let currentVideo = osn.Video.get();

        // Check if the current video context correctly returns
        // the default values
        expect(currentVideo.fpsNum).to.equal(30, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.fpsDen).to.equal(1, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        // The default base resolution is set depending the screen that is connected.
        // I'm commenting checking for these values since it will vary from a machine to another
        // expect(currentVideo.baseWidth).to.equal(1920, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        // expect(currentVideo.baseHeight).to.equal(1080, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.outputWidth).to.equal(1280, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.outputHeight).to.equal(720, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.outputFormat).to.equal(osn.EVideoFormat.NV12, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.colorspace).to.equal(osn.EColorSpace.CS601, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.range).to.equal(osn.ERangeType.Partial, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.scaleType).to.equal(osn.EScaleType.Bicubic, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));


        const newVideoContext: osn.VideoContext = {
            fpsNum: 120,
            fpsDen: 2,
            baseWidth: 3840,
            baseHeight: 2160,
            outputWidth: 1920,
            outputHeight: 1080,
            outputFormat: osn.EVideoFormat.I420,
            colorspace: osn.EColorSpace.CS709,
            range: osn.ERangeType.Full,
            scaleType: osn.EScaleType.Lanczos
        };
        osn.Video.set(newVideoContext);

        currentVideo = osn.Video.get();
        expect(currentVideo.fpsNum).to.equal(120, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.fpsDen).to.equal(2, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.baseWidth).to.equal(3840, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.baseHeight).to.equal(2160, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.outputWidth).to.equal(1920, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.outputHeight).to.equal(1080, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.outputFormat).to.equal(osn.EVideoFormat.I420, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.colorspace).to.equal(osn.EColorSpace.CS709, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.range).to.equal(osn.ERangeType.Full, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
        expect(currentVideo.scaleType).to.equal(osn.EScaleType.Lanczos, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
    });
});
