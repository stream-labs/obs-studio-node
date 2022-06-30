import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { OBSHandler, IOBSOutputSignalInfo } from '../util/obs_handler';
import { deleteConfigFiles, sleep } from '../util/general';
import { EOBSOutputType, EOBSOutputSignal, EOBSSettingsCategories } from '../util/obs_enums';

const testName = 'nodeobs_service';

describe(testName, function() {
    let obs: OBSHandler;
    let hasTestFailed: boolean = false;
    const path = require('path');

    before(async function() {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);

        obs.instantiateUserPool(testName);

        // Reserving user from pool
        await obs.reserveUser();

        // Connecting output signals
        obs.connectOutputSignals();
    });

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
        sleep(500);
    });

    beforeEach(async function() {
        await sleep(500);
    });
 
    it('Fail test - Stream with invalid stream key', async function() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.ServiceFactory.serviceContext.update({ key: 'invalid' });

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.code).to.equal(-3, GetErrorMessage(ETestErrorMsg.StreamOutput));

        obs.setStreamKey(obs.userStreamKey);
    });

    it('Fail test - Simple mode - Record with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'invalidPath'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.code).to.equal(-4, GetErrorMessage(ETestErrorMsg.RecordingOutput));
    });

    it('Fail test - Simple mode - Start replay buffer with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'invalidPath'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(500);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.code).to.equal(-8, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
    });

    it('Fail test - Advanced mode - Record with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'invalidPath'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.code).to.equal(-4, GetErrorMessage(ETestErrorMsg.RecordingOutput));
    });

    it('Fail test - Advanced mode - Start replay buffer with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'invalidPath'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(500);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.code).to.equal(-8, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
    });

    it('Reset video context', function() {
        expect(function() {
            osn.NodeObs.OBS_service_resetVideoContext();
        }).to.not.throw();
    });

    it('Reset audio context', function() {
        expect(function() {
            osn.NodeObs.OBS_service_resetAudioContext();
        }).to.not.throw();
    });
});
