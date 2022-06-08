import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { EFPSType } from '../osn';

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
        const skippedFrames = osn.VideoFactory.skippedFrames;

        // Checking if skipped frames was returned properly
        expect(skippedFrames).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.VideoSkippedFrames));
        expect(skippedFrames).to.equal(0, GetErrorMessage(ETestErrorMsg.VideoSkippedFramesWrongValue));
    });

    it('Get total frames value', () => {
        // Getting total frames value
        const totalFrames = osn.VideoFactory.encodedFrames;

        // Checking if total frames was returned properly
        expect(totalFrames).to.not.equal(undefined,  GetErrorMessage(ETestErrorMsg.VideoTotalFrames));
        expect(totalFrames).to.equal(0,  GetErrorMessage(ETestErrorMsg.VideoTotalFramesWrongValue));
    });

    it('Get and set video context', () => {
        let currentVideo = osn.VideoFactory.videoContext;

        // Check if the current video context correctly returns
        // the default values
        expect(currentVideo.fpsNum).to.equal(30, GetErrorMessage(ETestErrorMsg.VideoDefaultFPSNum));
        expect(currentVideo.fpsDen).to.equal(1, GetErrorMessage(ETestErrorMsg.VideoDefaultFPSDen));
        // The default base resolution is set depending on the screen that is connected.
        // I'm commenting checking for these values since it will vary from a machine to another
        // expect(currentVideo.baseWidth).to.equal(1920, GetErrorMessage(ETestErrorMsg.VideoDefaultBaseHeight));
        // expect(currentVideo.baseHeight).to.equal(1080, GetErrorMessage(ETestErrorMsg.VideoDefaultBaseWidth));
        expect(currentVideo.outputWidth).to.equal(1280, GetErrorMessage(ETestErrorMsg.VideoDefaultOutputWidth));
        expect(currentVideo.outputHeight).to.equal(720, GetErrorMessage(ETestErrorMsg.VideoDefaultOutputHeight));
        expect(currentVideo.outputFormat).to.equal(osn.EVideoFormat.NV12, GetErrorMessage(ETestErrorMsg.VideoDefaultOutputFormat));
        expect(currentVideo.colorspace).to.equal(osn.EColorSpace.CS601, GetErrorMessage(ETestErrorMsg.VideoDefaultColorSpace));
        expect(currentVideo.range).to.equal(osn.ERangeType.Partial, GetErrorMessage(ETestErrorMsg.VideoDefaultRange));
        expect(currentVideo.scaleType).to.equal(osn.EScaleType.Bicubic, GetErrorMessage(ETestErrorMsg.VideoDefaultScaleType));


        const newVideoContext: osn.IVideo = {
            fpsNum: 120,
            fpsDen: 2,
            baseWidth: 3840,
            baseHeight: 2160,
            outputWidth: 1920,
            outputHeight: 1080,
            outputFormat: osn.EVideoFormat.I420,
            colorspace: osn.EColorSpace.CS709,
            range: osn.ERangeType.Full,
            scaleType: osn.EScaleType.Lanczos,
            fpsType: EFPSType.Fractional
        };
        osn.VideoFactory.videoContext = newVideoContext;

        currentVideo = osn.VideoFactory.videoContext;
        expect(currentVideo.fpsNum).to.equal(120, GetErrorMessage(ETestErrorMsg.VideoSetFPSNum));
        expect(currentVideo.fpsDen).to.equal(2, GetErrorMessage(ETestErrorMsg.VideoSetFPSDen));
        expect(currentVideo.baseWidth).to.equal(3840, GetErrorMessage(ETestErrorMsg.VideoSetBaseWidth));
        expect(currentVideo.baseHeight).to.equal(2160, GetErrorMessage(ETestErrorMsg.VideoSetBaseHeight));
        expect(currentVideo.outputWidth).to.equal(1920, GetErrorMessage(ETestErrorMsg.VideoSetOutputWidth));
        expect(currentVideo.outputHeight).to.equal(1080, GetErrorMessage(ETestErrorMsg.VideoSetOutputHeight));
        expect(currentVideo.outputFormat).to.equal(osn.EVideoFormat.I420, GetErrorMessage(ETestErrorMsg.VideoSetOutputFormat));
        expect(currentVideo.colorspace).to.equal(osn.EColorSpace.CS709, GetErrorMessage(ETestErrorMsg.VideoSetColorFormat));
        expect(currentVideo.range).to.equal(osn.ERangeType.Full, GetErrorMessage(ETestErrorMsg.VideoSetRange));
        expect(currentVideo.scaleType).to.equal(osn.EScaleType.Lanczos, GetErrorMessage(ETestErrorMsg.VideoSetScaleType));
    });
});
