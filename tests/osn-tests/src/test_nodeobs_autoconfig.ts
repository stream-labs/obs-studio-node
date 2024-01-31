import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine, logWarning } from '../util/logger';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { OBSHandler, IConfigProgress } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';

const testName = 'nodeobs_autoconfig';

describe(testName, function() {
    let obs: OBSHandler;
    let hasTestFailed: boolean = false;

    // Initialize OBS process
    before(async function() {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName, false);

        obs.instantiateUserPool(testName);

        // Reserving user from pool
        await obs.reserveUser();
    });

    // Shutdown OBS process
    after(async function() {

        // Releasing user got from pool
        await obs.releaseUser();
        
        // Closing OBS process
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

    it('Run autoconfig', async function() {
        let progressInfo: IConfigProgress;
	    let settingValue: any;

        obs.startAutoconfig();

        osn.NodeObs.StartBandwidthTest();

        progressInfo = await obs.getNextProgressInfo('Bandwidth test');

	    if (progressInfo.event != 'error') {
            expect(progressInfo.event).to.equal('stopping_step', GetErrorMessage(ETestErrorMsg.BandwidthTest));
            expect(progressInfo.description).to.equal('bandwidth_test',  GetErrorMessage(ETestErrorMsg.BandwidthTest));
            expect(progressInfo.percentage).to.equal(100,  GetErrorMessage(ETestErrorMsg.BandwidthTest));

            osn.NodeObs.StartStreamEncoderTest();

            progressInfo = await obs.getNextProgressInfo('Stream Encoder test');
            expect(progressInfo.event).to.equal('stopping_step',  GetErrorMessage(ETestErrorMsg.StreamEncoderTest));
            expect(progressInfo.description).to.equal('streamingEncoder_test',  GetErrorMessage(ETestErrorMsg.StreamEncoderTest));
            expect(progressInfo.percentage).to.equal(100,  GetErrorMessage(ETestErrorMsg.StreamEncoderTest));

            osn.NodeObs.StartRecordingEncoderTest();

            progressInfo = await obs.getNextProgressInfo('Recording Encoder test');
            expect(progressInfo.event).to.equal('stopping_step',  GetErrorMessage(ETestErrorMsg.RecordingEncoderTest));
            expect(progressInfo.description).to.equal('recordingEncoder_test',  GetErrorMessage(ETestErrorMsg.RecordingEncoderTest));
            expect(progressInfo.percentage).to.equal(100,  GetErrorMessage(ETestErrorMsg.RecordingEncoderTest));

            osn.NodeObs.StartCheckSettings();

            progressInfo = await obs.getNextProgressInfo('Check Settings');
            expect(progressInfo.event).to.equal('stopping_step',  GetErrorMessage(ETestErrorMsg.CheckSettings));
            expect(progressInfo.description).to.equal('checking_settings',  GetErrorMessage(ETestErrorMsg.CheckSettings));
            expect(progressInfo.percentage).to.equal(100,  GetErrorMessage(ETestErrorMsg.CheckSettings));
        } else {
            logWarning(testName, 'Bandwidth test failed with ' + progressInfo.description + '. Setting default settings');

            osn.NodeObs.UseAutoConfigDefaultSettings();

            const settings = osn.NodeObs.GetNewSettings() as Array<[string, string, any]>;
            console.log("settings", JSON.stringify(settings));

            // Checking default settings
            settingValue = settings[0];
            expect(settingValue[0]).to.equal('Output',  GetErrorMessage(ETestErrorMsg.DefaultOutputMode));
            expect(settingValue[1]).to.equal('Mode',  GetErrorMessage(ETestErrorMsg.DefaultOutputMode));
            expect(settingValue[2]).to.equal('Simple',  GetErrorMessage(ETestErrorMsg.DefaultOutputMode));

            settingValue = settings[1];
            expect(settingValue[0]).to.equal('Output', GetErrorMessage(ETestErrorMsg.DefaultVBitrate));
            expect(settingValue[1]).to.equal('VBitrate', GetErrorMessage(ETestErrorMsg.DefaultVBitrate));
            expect(settingValue[2]).to.equal(2500, GetErrorMessage(ETestErrorMsg.DefaultVBitrate));

            settingValue = settings[2];
            expect(settingValue[0]).to.equal('Output', GetErrorMessage(ETestErrorMsg.DefaultStreamEncoder));
            expect(settingValue[1]).to.equal('StreamEncoder', GetErrorMessage(ETestErrorMsg.DefaultStreamEncoder));
            expect(settingValue[2]).to.equal('x264', GetErrorMessage(ETestErrorMsg.DefaultStreamEncoder));

            settingValue = settings[3];
            expect(settingValue[0]).to.equal('Output', GetErrorMessage(ETestErrorMsg.DefaultRecQuality));
            expect(settingValue[1]).to.equal('RecQuality', GetErrorMessage(ETestErrorMsg.DefaultRecQuality));
            expect(settingValue[2]).to.equal('Small', GetErrorMessage(ETestErrorMsg.DefaultRecQuality));

            settingValue = settings[4];
            expect(settingValue[0]).to.equal('Video', GetErrorMessage(ETestErrorMsg.DefaultVideoOutput));
            expect(settingValue[1]).to.equal('outputWidth', GetErrorMessage(ETestErrorMsg.DefaultVideoOutput));
            expect(settingValue[2]).to.equal(1280, GetErrorMessage(ETestErrorMsg.DefaultVideoOutput));

            settingValue = settings[5];;
            expect(settingValue[0]).to.equal('Video', GetErrorMessage(ETestErrorMsg.DefaultVideoOutput));
            expect(settingValue[1]).to.equal('outputHeight', GetErrorMessage(ETestErrorMsg.DefaultVideoOutput));
            expect(settingValue[2]).to.equal(720, GetErrorMessage(ETestErrorMsg.DefaultVideoOutput));

            settingValue = settings[6];
            expect(settingValue[0]).to.equal('Advanced', GetErrorMessage(ETestErrorMsg.DefaultDinamicBitrate));
            expect(settingValue[1]).to.equal('DynamicBitrate', GetErrorMessage(ETestErrorMsg.DefaultDinamicBitrate));
            expect(settingValue[2]).to.equal(0, GetErrorMessage(ETestErrorMsg.DefaultDinamicBitrate));

            settingValue = settings[7];;
            expect(settingValue[0]).to.equal('Video', GetErrorMessage(ETestErrorMsg.DefaultFPSType));
            expect(settingValue[1]).to.equal('fpsType', GetErrorMessage(ETestErrorMsg.DefaultFPSType));
            expect(settingValue[2]).to.equal(0, GetErrorMessage(ETestErrorMsg.DefaultFPSType));

            settingValue = settings[8];;
            expect(settingValue[0]).to.equal('Video', GetErrorMessage(ETestErrorMsg.DefaultFPSCommon));
            expect(settingValue[1]).to.equal('fpsNum', GetErrorMessage(ETestErrorMsg.DefaultFPSCommon));
            expect(settingValue[2]).to.equal(30, GetErrorMessage(ETestErrorMsg.DefaultFPSCommon));

            settingValue = settings[9];
            expect(settingValue[0]).to.equal('Video', GetErrorMessage(ETestErrorMsg.DefaultFPSCommon));
            expect(settingValue[1]).to.equal('fpsDen', GetErrorMessage(ETestErrorMsg.DefaultFPSCommon));
            expect(settingValue[2]).to.equal(1, GetErrorMessage(ETestErrorMsg.DefaultFPSCommon));
        }

	osn.NodeObs.TerminateAutoConfig();
    });
});
