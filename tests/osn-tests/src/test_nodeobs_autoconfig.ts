import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { OBSHandler, IConfigProgress } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';

describe('nodeobs_autoconfig', function() {
    let obs: OBSHandler;

    // Initialize OBS process
    before(async function() {
        deleteConfigFiles();
        obs = new OBSHandler();

        // Reserving user from pool
        await obs.reserveUser();
    });

    // Shutdown OBS process
    after(async function() {
        // Releasing user got from pool
        await obs.releaseUser();
        
        // Closing OBS process
        obs.shutdown();
        obs = null;

        deleteConfigFiles();
    });

    it('# Run autoconfig successfully', async function() {
        let progressInfo: IConfigProgress;

        obs.startAutoconfig();

        osn.NodeObs.StartBandwidthTest();

        progressInfo = await obs.getNextProgressInfo();

        expect(progressInfo.event).to.equal('stopping_step', 'BandwidthTest error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('bandwidth_test');
        expect(progressInfo.percentage).to.equal(100);

        osn.NodeObs.StartStreamEncoderTest();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'StreamEncoderTest error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('streamingEncoder_test');
        expect(progressInfo.percentage).to.equal(100);

        osn.NodeObs.StartRecordingEncoderTest();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'RecordingEncoderTest error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('recordingEncoder_test');
        expect(progressInfo.percentage).to.equal(100);

        osn.NodeObs.StartCheckSettings();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'CheckSettings error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('checking_settings');
        expect(progressInfo.percentage).to.equal(100);

        osn.NodeObs.StartSaveStreamSettings();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'SaveStreamSettings error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('saving_service');
        expect(progressInfo.percentage).to.equal(100);

        osn.NodeObs.StartSaveSettings();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'SaveSettings error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('saving_settings');
        expect(progressInfo.percentage).to.equal(100);

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('done');

        osn.NodeObs.TerminateAutoConfig();
    });

    it('# Run set default settings step and check settings', async function() {
        let progressInfo: IConfigProgress;
        let settingValue: any;

        obs.startAutoconfig();

        osn.NodeObs.StartSetDefaultSettings();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'SetDefaultSettings error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('setting_default_settings');
        expect(progressInfo.percentage).to.equal(100);

        osn.NodeObs.StartSaveStreamSettings();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'SaveStreamSettings error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('saving_service');
        expect(progressInfo.percentage).to.equal(100);

        osn.NodeObs.StartSaveSettings();

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('stopping_step', 'SaveSettings error - ' + progressInfo.description);
        expect(progressInfo.description).to.equal('saving_settings');
        expect(progressInfo.percentage).to.equal(100);

        progressInfo = await obs.getNextProgressInfo();
        expect(progressInfo.event).to.equal('done');

        osn.NodeObs.TerminateAutoConfig();

        // Checking default settings
        settingValue = obs.getSetting('Output', 'Mode');
        expect(settingValue).to.equal('Simple');

        settingValue = obs.getSetting('Output', 'VBitrate');
        expect(settingValue).to.equal(2500);

        settingValue = obs.getSetting('Output', 'StreamEncoder');
        expect(settingValue).to.equal('x264');

        settingValue = obs.getSetting('Output', 'RecQuality');
        expect(settingValue).to.equal('Small');

        settingValue = obs.getSetting('Advanced', 'DynamicBitrate');
        expect(settingValue).to.equal(false);

        settingValue = obs.getSetting('Video', 'Output');
        expect(settingValue).to.equal('1280x720');

        settingValue = obs.getSetting('Video', 'FPSType');
        expect(settingValue).to.equal('Common FPS Values');

        settingValue = obs.getSetting('Video', 'FPSCommon');
        expect(settingValue).to.equal('30');
    });
});
