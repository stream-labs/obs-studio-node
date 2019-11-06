import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { OBSHandler, IOBSOutputSignalInfo } from '../util/obs_handler';
import { deleteConfigFiles, sleep } from '../util/general';
import { EOBSOutputType, EOBSOutputSignal } from '../util/obs_enums';

describe('nodeobs_service', function() {
    let obs: OBSHandler;
    const path = require('path');

    before(async function() {
        deleteConfigFiles();
        obs = new OBSHandler();

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
    });

    context('# OBS_service_startStreaming, OBS_service_stopStreaming and recording functions', function() {
        context('* Simple mode', function() {
            this.beforeEach(function() {
                // Preparing environment
                obs.setSetting('Output', 'Mode', 'Simple');
                obs.setSetting('Output', 'StreamEncoder', 'x264');
                obs.setSetting('Output', 'FilePath', path.join(path.normalize(__dirname), '..', 'osnData'));
            });

            it('Start and stop streaming', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
    
            it('Start recording and stop', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_stopRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
            });
    
            it('Start replay buffer, save replay and stop', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startReplayBuffer();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_processReplayBufferHotkey();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote);

                osn.NodeObs.OBS_service_stopReplayBuffer(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
            });
    
            it('Record while streaming', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_stopRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
    
            it('Record replay while streaming and save', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startReplayBuffer();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_processReplayBufferHotkey();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote);

                osn.NodeObs.OBS_service_stopReplayBuffer(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
    
            it('Record and use replay buffer while streaming', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startReplayBuffer();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_processReplayBufferHotkey();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote);

                osn.NodeObs.OBS_service_stopReplayBuffer(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
        });
        
        context('* Advanced mode', function() {
            this.beforeEach(function() {
                // Preparing environment
                obs.setSetting('Output', 'Mode', 'Advanced');
                obs.setSetting('Output', 'Encoder', 'obs_x264');
                obs.setSetting('Output', 'RecEncoder', 'none');
                obs.setSetting('Output', 'RecFilePath', path.join(path.normalize(__dirname), '..', 'osnData'));
            });

            it('Start and stop streaming', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
    
            it('Start recording and stop', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_stopRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
            });
    
            it('Start replay buffer, save replay and stop', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startReplayBuffer();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_processReplayBufferHotkey();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote);

                osn.NodeObs.OBS_service_stopReplayBuffer(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
            });
    
            it('Record while streaming', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_stopRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
    
            it('Record replay while streaming and save', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startReplayBuffer();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_processReplayBufferHotkey();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote);

                osn.NodeObs.OBS_service_stopReplayBuffer(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
    
            it('Record and use replay buffer while streaming', async function() {
                let signalInfo: IOBSOutputSignalInfo;

                osn.NodeObs.OBS_service_startStreaming();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Starting);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Activate);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                osn.NodeObs.OBS_service_startReplayBuffer();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                await sleep(500);

                osn.NodeObs.OBS_service_processReplayBufferHotkey();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Wrote);

                osn.NodeObs.OBS_service_stopReplayBuffer(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopRecording();

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                osn.NodeObs.OBS_service_stopStreaming(false);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stopping);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);

                signalInfo = await obs.getNextSignalInfo();
                expect(signalInfo.type).to.equal(EOBSOutputType.Streaming);
                expect(signalInfo.signal).to.equal(EOBSOutputSignal.Deactivate);
            });
        });

        context('* Fail tests', function() {
            it('Stream with invalid stream key', async function() {
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

            context('* Simple mode', function() {
                this.beforeEach(function() {
                    // Preparing environment
                    obs.setSetting('Output', 'Mode', 'Simple');
                    obs.setSetting('Output', 'StreamEncoder', 'x264');
                    obs.setSetting('Output', 'FilePath', 'C:\\Test');
                });

                it('Record with invalid path', async function() {
                    let signalInfo: IOBSOutputSignalInfo;

                    osn.NodeObs.OBS_service_startRecording();

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
                    expect(signalInfo.code).to.equal(-4);
                });

                it('Start replay buffer with invalid path', async function() {
                    let signalInfo: IOBSOutputSignalInfo;

                    osn.NodeObs.OBS_service_startReplayBuffer();

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                    await sleep(500);

                    osn.NodeObs.OBS_service_processReplayBufferHotkey();

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
                    expect(signalInfo.code).to.equal(-4);
                });
            });

            context('* Advanced mode', function() {
                this.beforeEach(function() {
                    // Preparing environment
                    obs.setSetting('Output', 'Mode', 'Advanced');
                    obs.setSetting('Output', 'Encoder', 'obs_x264');
                    obs.setSetting('Output', 'RecEncoder', 'none');
                    obs.setSetting('Output', 'RecFilePath', 'C:\\Test');
                });

                it('Record with invalid path', async function() {
                    let signalInfo: IOBSOutputSignalInfo;

                    osn.NodeObs.OBS_service_startRecording();

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.Recording);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
                    expect(signalInfo.code).to.equal(-4);
                });

                it('Start replay buffer with invalid path', async function() {
                    let signalInfo: IOBSOutputSignalInfo;

                    osn.NodeObs.OBS_service_startReplayBuffer();

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Start);

                    await sleep(500);

                    osn.NodeObs.OBS_service_processReplayBufferHotkey();

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Writing);

                    signalInfo = await obs.getNextSignalInfo();
                    expect(signalInfo.type).to.equal(EOBSOutputType.ReplayBuffer);
                    expect(signalInfo.signal).to.equal(EOBSOutputSignal.Stop);
                    expect(signalInfo.code).to.equal(-4);
                });
            });
        });
    });

    context('# OBS_service_resetVideoContext and OBS_service_resetAudioContext', function() {
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
});
