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
    });

    it('Simple mode - Start and stop streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        await sleep(10000);

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    });

    it('Simple mode - Start recording and stop', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        await sleep(10000);

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));
    });

    it('Simple mode - Start replay buffer, save replay and stop', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Wrote);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
    });

    it('Simple mode - Record while streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        await sleep(10000);

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    });

    it('Simple mode - Record replay while streaming and save', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Wrote);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    });

    it('Simple mode - Record and use replay buffer while streaming', async function() {
        logInfo(testName,'DEBUG - 0');
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', obs.os === 'win32' ? 'x264' : 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        logInfo(testName,'DEBUG - 1');
        osn.NodeObs.OBS_service_startStreaming();

        logInfo(testName,'DEBUG - 2');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        logInfo(testName,'DEBUG - 3');
        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startRecording();

        logInfo(testName,'DEBUG - 4');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        logInfo(testName,'DEBUG - 5');
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_startReplayBuffer();

        logInfo(testName,'DEBUG - 6');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopRecording();
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        logInfo(testName,'DEBUG - 7');
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Wrote);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        logInfo(testName,'DEBUG - 8');
        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopRecording();
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);

        logInfo(testName,'DEBUG - 9');
        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        logInfo(testName,'DEBUG - 10');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    });

    it('Advanced mode - Start and stop streaming', async function() {
        logInfo(testName,'DEBUG - 0');
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        logInfo(testName,'DEBUG - 1');
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        logInfo(testName,'DEBUG - 2');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        await sleep(10000);

        logInfo(testName,'DEBUG - 3');
        osn.NodeObs.OBS_service_stopStreaming(false);

        logInfo(testName,'DEBUG - 4');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        logInfo(testName,'DEBUG - 5');
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        logInfo(testName,'DEBUG - 6');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
        logInfo(testName,'DEBUG - 7');
    });

    it('Advanced mode - Start recording and stop', async function() {
        logInfo(testName,'DEBUG - 0');
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        logInfo(testName,'DEBUG - 0.1');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        logInfo(testName,'DEBUG - 0.2');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        logInfo(testName,'DEBUG - 0.3');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));
        logInfo(testName,'DEBUG - 0.4');

        let signalInfo: IOBSOutputSignalInfo;

        logInfo(testName,'DEBUG - 1');
        osn.NodeObs.OBS_service_startRecording();

        logInfo(testName,'DEBUG - 2');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        logInfo(testName,'DEBUG - 3');
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        await sleep(10000);

        logInfo(testName,'DEBUG - 4');
        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        logInfo(testName,'DEBUG - 5');
        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        logInfo(testName,'DEBUG - 6');
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        logInfo(testName,'DEBUG - 7');
    });

    it('Advanced mode - Start replay buffer, save replay and stop', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Wrote);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
    });

    it('Advanced mode - Record while streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        await sleep(10000);

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    });

    it('Advanced mode - Record replay while streaming and save', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Wrote);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    });

    it('Advanced mode - Record and use replay buffer while streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Activate);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Start);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Start);

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopRecording();
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Wrote);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopRecording();
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stopping);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Deactivate);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    });

    it('Fail test - Stream with invalid stream key', async function() {
        let signalInfo: IOBSOutputSignalInfo;

        obs.setSetting('Stream', 'key', 'invalid');

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Starting);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.Streaming, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.code).to.equal(-3, GetErrorMessage(ETestErrorMsg.StreamOutput));
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

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.code).to.equal(-4, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
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

        await sleep(10000);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Writing);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo(EOBSOutputType.ReplayBuffer, EOBSOutputSignal.Stop);
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.code).to.equal(-4, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
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
