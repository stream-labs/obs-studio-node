import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import * as logger from '../util/logger';
import { OBSHandler, IOBSOutputSignalInfo } from '../util/obs_handler';
import { deleteConfigFiles, sleep } from '../util/general';
import { EOBSOutputType, EOBSOutputSignal, EOBSSettingsCategories } from '../util/obs_enums';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';

const testName = 'nodeobs_service';

describe(testName, function() {
    let obs: OBSHandler;
    const path = require('path');

    before(async function() {
        logger.logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
		
		obs.instantiateUserPool();

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
        obs = null;

        deleteConfigFiles();
        logger.logInfo(testName, 'Finished ' + testName + ' tests');
        logger.logEmptyLine();
    });

    it('Simple mode - Start and stop streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runStreamOutputTest();
    });

    it('Simple mode - Start recording and stop', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runRecordOutputTest();
    });

    it('Simple mode - Start replay buffer, save replay and stop', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runReplayBufferTest();
    });

    it('Simple mode - Record while streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runRecordAndStreamTest();
    });

    it('Simple mode - Record replay while streaming and save', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runReplayAndStreamTest();
    });

    it('Simple mode - Record and use replay buffer while streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runAllOutputsTest();
    });

    it('Advanced mode - Start and stop streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runStreamOutputTest();
    });

    it('Advanced mode - Start recording and stop', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runRecordOutputTest();
    });

    it('Advanced mode - Start replay buffer, save replay and stop', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runReplayBufferTest();
    });

    it('Advanced mode - Record while streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runRecordAndStreamTest();
    });

    it('Advanced mode - Record replay while streaming and save', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runReplayAndStreamTest();
    });

    it('Advanced mode - Record and use replay buffer while streaming', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));

        await runAllOutputsTest();
    });

    it('Fail test - Stream with invalid stream key', async function() {
        let signalInfo: IOBSOutputSignalInfo;

        obs.setSetting('Stream', 'key', 'invalid');

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
        expect(signalInfo.code).to.equal(-3);
    });

    it('Fail test - Simple mode - Record with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', 'C:\\Test');

        await runRecordWithInvalidPath();
    });

    it('Fail test - Simple mode - Start replay buffer with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Simple');
        obs.setSetting(EOBSSettingsCategories.Output, 'StreamEncoder', 'x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'FilePath', 'C:\\Test');

        await runReplayWithInvalidPath();
    });

    it('Fail test - Advanced mode - Record with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', 'C:\\Test');

        await runRecordWithInvalidPath();
    });

    it('Fail test - Advanced mode - Start replay buffer with invalid path', async function() {
        // Preparing environment
        obs.setSetting(EOBSSettingsCategories.Output, 'Mode', 'Advanced');
        obs.setSetting(EOBSSettingsCategories.Output, 'Encoder', 'obs_x264');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecEncoder', 'none');
        obs.setSetting(EOBSSettingsCategories.Output, 'RecFilePath', 'C:\\Test');

        await runReplayWithInvalidPath();
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

    // Test functions
    async function runStreamOutputTest() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo();
        
        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        await sleep(500);

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo();

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    }

    async function runRecordOutputTest() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        await sleep(500);

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));
    }

    async function runReplayBufferTest() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(500);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
    }

    async function runRecordAndStreamTest() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        await sleep(500);

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    }

    async function runReplayAndStreamTest() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(500);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    }

    async function runAllOutputsTest() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startStreaming();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.StreamOutput));

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            osn.NodeObs.OBS_service_stopRecording();
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(500);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopReplayBuffer(false);

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopRecording();
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        osn.NodeObs.OBS_service_stopRecording();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            osn.NodeObs.OBS_service_stopStreaming(false);
            throw Error(GetErrorMessage(ETestErrorMsg.RecordOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));

        osn.NodeObs.OBS_service_stopStreaming(false);

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.code != 0) {
            throw Error(GetErrorMessage(ETestErrorMsg.StreamOutputStoppedWithError, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.StreamOutput));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Streaming, GetErrorMessage(ETestErrorMsg.StreamOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate, GetErrorMessage(ETestErrorMsg.StreamOutput));
    }

    async function runRecordWithInvalidPath() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startRecording();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.Recording, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.RecordingOutput));
        expect(signalInfo.code).to.equal(-4, GetErrorMessage(ETestErrorMsg.RecordingOutput));
    }

    async function runReplayWithInvalidPath() {
        let signalInfo: IOBSOutputSignalInfo;

        osn.NodeObs.OBS_service_startReplayBuffer();

        signalInfo = await obs.getNextSignalInfo();

        if (signalInfo.signal == EOBSOutputSignal.Stop) {
            throw Error(GetErrorMessage(ETestErrorMsg.ReplayBufferDidNotStart, signalInfo.code.toString(), signalInfo.error));
        }

        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        await sleep(500);

        osn.NodeObs.OBS_service_processReplayBufferHotkey();

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing, GetErrorMessage(ETestErrorMsg.ReplayBuffer));

        signalInfo = await obs.getNextSignalInfo();
        expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
        expect(signalInfo.code).to.equal(-4, GetErrorMessage(ETestErrorMsg.ReplayBuffer));
    }
});
