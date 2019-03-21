import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { Subject } from 'rxjs';
import { Services } from '../util/services';

enum EOBSOutputType {
    Streaming = 'streaming',
    Recording = 'recording',
    ReplayBuffer = 'replay-buffer',
}
  
enum EOBSOutputSignal {
    Starting = 'starting',
    Start = 'start',
    Activate = 'activate',
    Stopping = 'stopping',
    Stop = 'stop',
    Deactivate = 'deactivate',
    Reconnect = 'reconnect',
    ReconnectSuccess = 'reconnect_success',
    Writing = 'writing',
    Wrote = 'wrote',
    WriteError = 'writing_error',
}

interface IOBSOutputSignalInfo {
    type: EOBSOutputType;
    signal: EOBSOutputSignal;
    code: osn.EOutputCode;
    error: string;
}

function saveStreamKey(key: string) {
    // Getting stream settings container
    const streamSettings = osn.NodeObs.OBS_settings_getSettings('Stream');

    // Setting stream service and stream key
    streamSettings.forEach(subCategory => {
        subCategory.parameters.forEach(parameter => {
            if (parameter.name === 'service') {
                parameter.currentValue = 'Twitch';
            }

            if (parameter.name === 'key') {
                parameter.currentValue = key;
            }
        });
    });

    osn.NodeObs.OBS_settings_saveSettings('Stream', streamSettings);
}

function setEnconderAndRecordingPath() {
    const path = require('path');

    // Getting stream settings container
    const streamSettings = osn.NodeObs.OBS_settings_getSettings('Output');

    // Changing mode to simple
    streamSettings.find(category => {
        return category.nameSubCategory === 'Untitled';
    }).parameters.find(parameter => {
        return parameter.name === 'Mode';
    }).currentValue = 'Simple';

    osn.NodeObs.OBS_settings_saveSettings('Output', streamSettings);

    let setEncoderAndRecordingFilePath = osn.NodeObs.OBS_settings_getSettings('Output');

    // Setting encoder to x264, AppVeyor does not have any hardware encoders
    setEncoderAndRecordingFilePath.find(category => {
        return category.nameSubCategory === 'Streaming';
    }).parameters.find(parameter => {
        return parameter.name === 'StreamEncoder';
    }).currentValue = 'x264';

    // Setting recording file path
    setEncoderAndRecordingFilePath.find(category => {
        return category.nameSubCategory === 'Recording';
    }).parameters.find(parameter => {
        return parameter.name === 'FilePath';
    }).currentValue = path.join(path.normalize(__dirname), '..', 'osnData');

    osn.NodeObs.OBS_settings_saveSettings('Output', setEncoderAndRecordingFilePath);
}

describe('nodeobs_service', function() {
    let obs: OBSProcessHandler;
    let services: Services;
    let hasUserFromPool: boolean;

    let streamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isStreaming: boolean = false;

    let recordWhileStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isRecordingWhileStreaming: boolean = false;

    let replayWhileStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isReplayWhileStreaming: boolean = false;

    let allSignals = new Subject<IOBSOutputSignalInfo>();
    let isRecordingAndReplayWhileStreaming: boolean = false;

    // Initialize OBS process
    before(function() {
        obs = new OBSProcessHandler();
        services = new Services();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }

        // Connecting output signals
        osn.NodeObs.OBS_service_connectOutputSignals((signalInfo: IOBSOutputSignalInfo) => {
            if (isStreaming == true) {
                if (signalInfo.type === EOBSOutputType.Streaming) {
                    streamingSignals.next(signalInfo);
                }
            }

            if (isRecordingWhileStreaming == true) {
                if (signalInfo.type === EOBSOutputType.Streaming ||
                    signalInfo.type === EOBSOutputType.Recording) {
                    recordWhileStreamingSignals.next(signalInfo);
                }
            }
            
            if (isReplayWhileStreaming == true) {
                if (signalInfo.type === EOBSOutputType.Streaming ||
                    signalInfo.type === EOBSOutputType.ReplayBuffer) {
                    replayWhileStreamingSignals.next(signalInfo);
                }
            }

            if (isRecordingAndReplayWhileStreaming == true) {
                allSignals.next(signalInfo);
            }
        });

        // Setting encoder and recording path
        setEnconderAndRecordingPath();
    });

    // Shutdown OBS process
    after(async function() {
        if (hasUserFromPool) {
            await services.releaseUser();
        }

        osn.NodeObs.OBS_service_removeCallback();
        obs.shutdown();
        obs = null;
    });

    context('# OBS_service_startStreaming, OBS_service_stopStreaming and recording functions', function() {
        it('Start and stop streaming (Twitch)', function(done) {
            // Getting stream key from user pool
            services.getStreamKey('twitch').then(key => {
                // Saving stream key
                saveStreamKey(key);
                hasUserFromPool = true;

            // If unable to get stream key use environment variable
            }).catch(function() {
                // Saving stream key
                saveStreamKey(process.env.SLOBS_BE_STREAMKEY);
                hasUserFromPool = false;
            });

            isStreaming = true;

            streamingSignals.subscribe(
                signalInfo => {
                    if (signalInfo.type === EOBSOutputType.Streaming) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Streaming for 5 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }, 5000);
                        } else if (signalInfo.signal === EOBSOutputSignal.Stopping) {
                            osn.NodeObs.OBS_service_stopStreaming(true);
                        } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                            isStreaming = false;
                            done();
                        }
                    } else {
                        isStreaming = false;
                        done(new Error('No output signals received.'));
                    }
                },
                done
            );

            osn.NodeObs.OBS_service_startStreaming();
        });

        it('Record while streaming', function(done) {
            isRecordingWhileStreaming = true;

            recordWhileStreamingSignals.subscribe(
                signalInfo => {
                    if (signalInfo.type === EOBSOutputType.Streaming) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Streaming for 5 secods
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }, 5000);

                            // Start recording after 1 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_startRecording();
                            }, 1000);
                        } else if (signalInfo.signal === EOBSOutputSignal.Stopping) {
                            osn.NodeObs.OBS_service_stopStreaming(true);
                        } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                            isRecordingWhileStreaming = false;
                            done();
                        }
                    } else if (signalInfo.type === EOBSOutputType.Recording) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Recording for 3 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopRecording();
                            }, 3000);
                        }
                    } else {
                        isRecordingWhileStreaming = false;
                        done(new Error('No output signals received.'));
                    }
                },
                done
            );

            osn.NodeObs.OBS_service_startStreaming();
        });

        it('Record replay while streaming and save it', function(done) {
            let lastReplay: string = "";
            isReplayWhileStreaming = true;

            replayWhileStreamingSignals.subscribe(
                signalInfo => {
                    if (signalInfo.type === EOBSOutputType.Streaming) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Streaming for 8 secods
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }, 8000);

                             // Starting replay buffer after 1 second
                             setTimeout(function() {
                                osn.NodeObs.OBS_service_startReplayBuffer();
                            }, 1000);
                        } else if (signalInfo.signal === EOBSOutputSignal.Stopping) {
                            osn.NodeObs.OBS_service_stopStreaming(true);
                        } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                            isReplayWhileStreaming = false;
                            done();
                        }
                    } else if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Recording replay for 5 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                            }, 5000);
    
                            // Saving replay after 2 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_processReplayBufferHotkey();
                            }, 2000);
                        } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                            // Getting saved replay file name
                            lastReplay = osn.NodeObs.OBS_service_getLastReplay();
                        } else if (signalInfo.signal === EOBSOutputSignal.Stopping) {
                            osn.NodeObs.OBS_service_stopReplayBuffer(true);
                        } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                            // Getting saved replay file name
                            lastReplay = osn.NodeObs.OBS_service_getLastReplay();

                            // Checking if last replay returned correctly
                            try {
                                expect(lastReplay).to.not.equal(undefined);
                                expect(lastReplay.length).to.not.equal(0);
                            } catch (e) {
                                // Failed to get last replay
                                isReplayWhileStreaming = false;
                                done(new Error(e));
                            }
                        }
                    } else {
                        isReplayWhileStreaming = false;
                        done(new Error('No output signals received.'));
                    }
                },
                done
            );

            osn.NodeObs.OBS_service_startStreaming();
        });

        it('Record and use replay buffer while streaming', function(done) {
            let lastReplay: string = "";
            isRecordingAndReplayWhileStreaming = true;

            allSignals.subscribe(
                signalInfo => {
                    if (signalInfo.type === EOBSOutputType.Streaming) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Streaming for 12 secods
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }, 12000);

                            // Start recording after 1 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_startRecording();
                            }, 1000);

                             // Starting replay buffer after 2 second
                             setTimeout(function() {
                                osn.NodeObs.OBS_service_startReplayBuffer();
                            }, 2000);
                        } else if (signalInfo.signal === EOBSOutputSignal.Stopping) {
                            osn.NodeObs.OBS_service_stopStreaming(true);
                        } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                            isRecordingAndReplayWhileStreaming = false;
                            done();
                        }
                    } else if (signalInfo.type === EOBSOutputType.Recording) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Recording for 8 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopRecording();
                            }, 8000);
                        }
                    } else if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                        if (signalInfo.signal === EOBSOutputSignal.Start) {
                            // Recording replay for 6 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                            }, 6000);
    
                            // Saving replay after 3 seconds
                            setTimeout(function() {
                                osn.NodeObs.OBS_service_processReplayBufferHotkey();
                            }, 3000);
                        } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                            // Getting saved replay file name
                            lastReplay = osn.NodeObs.OBS_service_getLastReplay();
                        } else if (signalInfo.signal === EOBSOutputSignal.Stopping) {
                            osn.NodeObs.OBS_service_stopReplayBuffer(true);
                        } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                            // Getting saved replay file name
                            lastReplay = osn.NodeObs.OBS_service_getLastReplay();

                            // Checking if last replay returned correctly
                            try {
                                expect(lastReplay).to.not.equal(undefined);
                                expect(lastReplay.length).to.not.equal(0);
                            } catch (e) {
                                // Failed to get last replay
                                isRecordingAndReplayWhileStreaming = false;
                                done(new Error(e));
                            }
                        }
                    } else {
                        isRecordingAndReplayWhileStreaming = false;
                        done(new Error('No output signals received.'));
                    }
                },
                done
            );

            osn.NodeObs.OBS_service_startStreaming();
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
        })
    });
});
