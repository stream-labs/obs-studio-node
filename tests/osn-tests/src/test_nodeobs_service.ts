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

function setEnconderAndRecordingPath(mode: string, invalidPath: boolean = false) {
    const path = require('path');

    // Getting stream settings container
    const streamSettings = osn.NodeObs.OBS_settings_getSettings('Output');

    // Changing mode to simple or advanced
    streamSettings.find(category => {
        return category.nameSubCategory === 'Untitled';
    }).parameters.find(parameter => {
        return parameter.name === 'Mode';
    }).currentValue = mode;

    osn.NodeObs.OBS_settings_saveSettings('Output', streamSettings);

    let setEncoderAndRecordingFilePath = osn.NodeObs.OBS_settings_getSettings('Output');

    if (mode == 'Simple') {
        // Setting encoder to x264, AppVeyor does not have any hardware encoders
        setEncoderAndRecordingFilePath.find(category => {
            return category.nameSubCategory === 'Streaming';
        }).parameters.find(parameter => {
            return parameter.name === 'StreamEncoder';
        }).currentValue = 'x264';

        if (!invalidPath) {
            // Setting recording file path
            setEncoderAndRecordingFilePath.find(category => {
                return category.nameSubCategory === 'Recording';
            }).parameters.find(parameter => {
                return parameter.name === 'FilePath';
            }).currentValue = path.join(path.normalize(__dirname), '..', 'osnData');
        } else {
            // Setting invalid recording file path
            setEncoderAndRecordingFilePath.find(category => {
                return category.nameSubCategory === 'Recording';
            }).parameters.find(parameter => {
                return parameter.name === 'FilePath';
            }).currentValue = 'C:\\Test';
        }

        osn.NodeObs.OBS_settings_saveSettings('Output', setEncoderAndRecordingFilePath);
    } 
    
    if (mode == 'Advanced') {
        // Setting encoder to x264, AppVeyor does not have any hardware encoders
        setEncoderAndRecordingFilePath.find(category => {
            return category.nameSubCategory === 'Streaming';
        }).parameters.find(parameter => {
            return parameter.name === 'Encoder';
        }).currentValue = 'obs_x264';

        setEncoderAndRecordingFilePath.find(category => {
            return category.nameSubCategory === 'Recording';
        }).parameters.find(parameter => {
            return parameter.name === 'RecEncoder';
        }).currentValue = 'none';

        if (!invalidPath) {
            // Setting recording file path
            setEncoderAndRecordingFilePath.find(category => {
                return category.nameSubCategory === 'Recording';
            }).parameters.find(parameter => {
                return parameter.name === 'RecFilePath';
            }).currentValue = path.join(path.normalize(__dirname), '..', 'osnData');
        } else {
            // Setting invalid recording file path
            setEncoderAndRecordingFilePath.find(category => {
                return category.nameSubCategory === 'Recording';
            }).parameters.find(parameter => {
                return parameter.name === 'RecFilePath';
            }).currentValue = 'C:\\Test';
        }

        osn.NodeObs.OBS_settings_saveSettings('Output', setEncoderAndRecordingFilePath);
    }
}

describe('nodeobs_service', function() {
    let obs: OBSProcessHandler;
    let services: Services;
    let hasUserFromPool: boolean;

    let simpleStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isStreamingSimple: boolean = false;

    let advStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isStreamingAdv: boolean = false;

    let simpleRecordingSignals = new Subject<IOBSOutputSignalInfo>();
    let isRecordingSimple: boolean = false;

    let advRecordingSignals = new Subject<IOBSOutputSignalInfo>();
    let isRecordingAdv: boolean = false;

    let simpleReplaybufferSignals = new Subject<IOBSOutputSignalInfo>();
    let isReplayBufferSimple: boolean = false;

    let advReplaybufferSignals = new Subject<IOBSOutputSignalInfo>();
    let isReplayBufferAdv: boolean = false;

    let simpleRecordWhileStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isRecordingWhileStreamingSimple: boolean = false;

    let advRecordWhileStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isRecordingWhileStreamingAdv: boolean = false;

    let simpleReplayWhileStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isReplayWhileStreamingSimple: boolean = false;

    let advReplayWhileStreamingSignals = new Subject<IOBSOutputSignalInfo>();
    let isReplayWhileStreamingAdv: boolean = false;

    let simpleAllSignals = new Subject<IOBSOutputSignalInfo>();
    let isAllRecordingWhileStreamingSimple: boolean = false;

    let advAllSignals = new Subject<IOBSOutputSignalInfo>();
    let isAllRecordingWhileStreamingAdv: boolean = false;

    let invalidKeySignals = new Subject<IOBSOutputSignalInfo>();
    let isStreamingWithInvalidKey: boolean = false;

    let invalidPathRecordSignalsSimple = new Subject<IOBSOutputSignalInfo>();
    let isRecordingWithInvalidPathSimple: boolean = false;

    let invalidPathRecordSignalsAdv = new Subject<IOBSOutputSignalInfo>();
    let isRecordingWithInvalidPathAdv: boolean = false;

    let invalidPathReplaySignalsSimple = new Subject<IOBSOutputSignalInfo>();
    let isReplayWithInvalidPathSimple: boolean = false;

    let invalidPathReplaySignalsAdv = new Subject<IOBSOutputSignalInfo>();
    let isReplayWithInvalidPathAdv: boolean = false;

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
            if (isStreamingSimple == true) {
                if (signalInfo.type === EOBSOutputType.Streaming) {
                    simpleStreamingSignals.next(signalInfo);
                }
            }

            if (isStreamingAdv == true) {
                if (signalInfo.type === EOBSOutputType.Streaming) {
                    advStreamingSignals.next(signalInfo);
                }
            }

            if (isRecordingSimple == true) {
                if (signalInfo.type === EOBSOutputType.Recording) {
                    simpleRecordingSignals.next(signalInfo);
                }
            }

            if (isRecordingAdv == true) {
                if (signalInfo.type === EOBSOutputType.Recording) {
                    advRecordingSignals.next(signalInfo);
                }
            }

            if (isReplayBufferSimple == true) {
                if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                    simpleReplaybufferSignals.next(signalInfo);
                }
            }

            if (isReplayBufferAdv == true) {
                if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                    advReplaybufferSignals.next(signalInfo);
                }
            }

            if (isRecordingWhileStreamingSimple == true) {
                if (signalInfo.type === EOBSOutputType.Streaming ||
                    signalInfo.type === EOBSOutputType.Recording) {
                    simpleRecordWhileStreamingSignals.next(signalInfo);
                }
            }

            if (isRecordingWhileStreamingAdv == true) {
                if (signalInfo.type === EOBSOutputType.Streaming ||
                    signalInfo.type === EOBSOutputType.Recording) {
                    advRecordWhileStreamingSignals.next(signalInfo);
                }
            }
            
            if (isReplayWhileStreamingSimple == true) {
                if (signalInfo.type === EOBSOutputType.Streaming ||
                    signalInfo.type === EOBSOutputType.ReplayBuffer) {
                    simpleReplayWhileStreamingSignals.next(signalInfo);
                }
            }

            if (isReplayWhileStreamingAdv == true) {
                if (signalInfo.type === EOBSOutputType.Streaming ||
                    signalInfo.type === EOBSOutputType.ReplayBuffer) {
                    advReplayWhileStreamingSignals.next(signalInfo);
                }
            }

            if (isAllRecordingWhileStreamingSimple == true) {
                simpleAllSignals.next(signalInfo);
            }

            if (isAllRecordingWhileStreamingAdv == true) {
                advAllSignals.next(signalInfo);
            }

            if (isStreamingWithInvalidKey == true) {
                if (signalInfo.type === EOBSOutputType.Streaming) {
                    invalidKeySignals.next(signalInfo);
                }
            }

            if (isRecordingWithInvalidPathSimple == true) {
                if (signalInfo.type === EOBSOutputType.Recording) {
                    invalidPathRecordSignalsSimple.next(signalInfo);
                }
            }

            if (isRecordingWithInvalidPathAdv == true) {
                if (signalInfo.type === EOBSOutputType.Recording) {
                    invalidPathRecordSignalsAdv.next(signalInfo);
                }
            }

            if (isReplayWithInvalidPathSimple == true) {
                if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                    invalidPathReplaySignalsSimple.next(signalInfo);
                }
            }

            if (isReplayWithInvalidPathAdv == true) {
                if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                    invalidPathReplaySignalsAdv.next(signalInfo);
                }
            }
        });
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
        it('Set stream key', async function() {
            let streamKey: string = "";
            let updatedStreamKey: string = "";

            try {
                streamKey = await services.getStreamKey();
                hasUserFromPool = true;
            } catch(e) {
                streamKey = process.env.SLOBS_BE_STREAMKEY;
                hasUserFromPool = false;
            }

            console.log(streamKey);
                
            saveStreamKey(streamKey);

            const updatedStreamSettings = osn.NodeObs.OBS_settings_getSettings('Stream');

            updatedStreamSettings.forEach(subCategory => {
                subCategory.parameters.forEach(parameter => {
                    if (parameter.name === 'key') {
                        updatedStreamKey = parameter.currentValue;
                    }
                });
            });

            expect(streamKey).to.equal(updatedStreamKey);
        });

        context('* Simple mode', function() {
            it('Start and stop streaming', function(done) {
                let signalCode: number = 0;
                let outputType: string = "";
    
                // Setting encoder and recording path
                setEnconderAndRecordingPath('Simple', false);
    
                isStreamingSimple = true;
    
                simpleStreamingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isStreamingSimple = false;
    
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isStreamingSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
            });
    
            it('Start recording and stop', function(done) {
                isRecordingSimple = true;
    
                simpleRecordingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Stopping recording
                                    osn.NodeObs.OBS_service_stopRecording();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isRecordingSimple = false;
    
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    done(new Error(signalInfo.type + ' stop signal returned with code ' + signalInfo.code));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isRecordingSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startRecording();
            });
    
            it('Start replay buffer, save replay and stop', function(done) {
                let lastReplay: string = "";
    
                isReplayBufferSimple = true;
    
                simpleReplaybufferSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                                lastReplay = osn.NodeObs.OBS_service_getLastReplay();
    
                                // Stopping replay buffer
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isReplayBufferSimple = false;
    
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    done(new Error(signalInfo.type + ' stop signal returned with code ' + signalInfo.code));
                                } else if (lastReplay == undefined || lastReplay.length == 0) {
                                    done(new Error('Failed to get last replay'));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isReplayBufferSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startReplayBuffer();
            });
    
            it('Record while streaming', function(done) {
                let signalCode: number = 0;
                let outputType: string = "";
                isRecordingWhileStreamingSimple = true;
    
                simpleRecordWhileStreamingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isRecordingWhileStreamingSimple = false;
                                
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else {
                                    done();
                                }
                            }
                        } else if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Stopping recording
                                    osn.NodeObs.OBS_service_stopRecording();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
    
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }
                        } else {
                            isRecordingWhileStreamingSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
                osn.NodeObs.OBS_service_startRecording();
            });
    
            it('Record replay while streaming and save', function(done) {
                let lastReplay: string = "";
                let signalCode: number = 0;
                let outputType: string = "";
                
                isReplayWhileStreamingSimple = true;
    
                simpleReplayWhileStreamingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isReplayWhileStreamingSimple = false;
    
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else if (lastReplay == undefined || lastReplay.length == 0) {
                                    done(new Error('Failed to get last replay'));
                                } else {
                                    done();
                                }
                            }
                        } else if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                                lastReplay = osn.NodeObs.OBS_service_getLastReplay();
    
                                // Stopping replay buffer
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                                //osn.NodeObs.OBS_service_stopStreaming(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
    
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }
                        } else {
                            isReplayWhileStreamingSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
                osn.NodeObs.OBS_service_startReplayBuffer();
            });
    
            it('Record and use replay buffer while streaming', function(done) {
                let lastReplay: string = "";
                let signalCode: number = 0;
                let outputType: string = "";
    
                isAllRecordingWhileStreamingSimple = true;
    
                simpleAllSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isAllRecordingWhileStreamingSimple = false;
                                
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else if (lastReplay == undefined || lastReplay.length == 0) {
                                    done(new Error('Failed to get last replay'));
                                } else {
                                    done();
                                }
                            }
                        } else if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Stopping recording
                                    osn.NodeObs.OBS_service_stopRecording();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            }  
                        } else if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                                lastReplay = osn.NodeObs.OBS_service_getLastReplay()
    
                                // Stopping replay buffer
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
    
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }
                        } else {
                            isAllRecordingWhileStreamingSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
                osn.NodeObs.OBS_service_startRecording();
                osn.NodeObs.OBS_service_startReplayBuffer();
            });
        });
        
        context('* Advanced mode' , function() {
            it('Start and stop streaming', function(done) {
                let signalCode: number = 0;
                let outputType: string = "";
    
                // Setting encoder and recording path
                setEnconderAndRecordingPath('Advanced', false);
    
                isStreamingAdv = true;
    
                advStreamingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isStreamingAdv = false;
    
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isStreamingAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
            });
    
            it('Start recording and stop', function(done) {
                isRecordingAdv = true;
    
                advRecordingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Stopping recording
                                    osn.NodeObs.OBS_service_stopRecording();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isRecordingAdv = false;
    
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    done(new Error(signalInfo.type + ' stop signal returned with code ' + signalInfo.code));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isRecordingAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    },
                    done
                );
    
                osn.NodeObs.OBS_service_startRecording();
            });
    
            it('Start replay buffer, save replay and stop', function(done) {
                let lastReplay: string = "";
    
                isReplayBufferAdv = true;
    
                advReplaybufferSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                                lastReplay = osn.NodeObs.OBS_service_getLastReplay();
    
                                // Stopping replay buffer
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isReplayBufferAdv = false;
    
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    done(new Error(signalInfo.type + ' stop signal returned with code ' + signalInfo.code));
                                } else if (lastReplay == undefined || lastReplay.length == 0) {
                                    done(new Error('Failed to get last replay'));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isReplayBufferAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startReplayBuffer();
            });

            it('Record while streaming', function(done) {
                let signalCode: number = 0;
                let outputType: string = "";
    
                // Setting encoder and recording path
                setEnconderAndRecordingPath('Advanced', false);
                
                isRecordingWhileStreamingAdv = true;
    
                advRecordWhileStreamingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isRecordingWhileStreamingAdv = false;
                                
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else {
                                    done();
                                }
                            }
                        } else if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Stopping recording
                                    osn.NodeObs.OBS_service_stopRecording();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
    
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }
                        } else {
                            isRecordingWhileStreamingAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
                osn.NodeObs.OBS_service_startRecording();
            });

            it('Record replay while streaming and save', function(done) {
                let lastReplay: string = "";
                let signalCode: number = 0;
                let outputType: string = "";
                
                isReplayWhileStreamingAdv = true;
    
                advReplayWhileStreamingSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isReplayWhileStreamingAdv = false;
    
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else if (lastReplay == undefined || lastReplay.length == 0) {
                                    done(new Error('Failed to get last replay'));
                                } else {
                                    done();
                                }
                            }
                        } else if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                                // Getting saved replay file name
                                lastReplay = osn.NodeObs.OBS_service_getLastReplay();
    
                                // Stopping replay buffer
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
    
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }
                        } else {
                            isReplayWhileStreamingAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
                osn.NodeObs.OBS_service_startReplayBuffer();
            });

            it('Record and use replay buffer while streaming', function(done) {
                let lastReplay: string = "";
                let signalCode: number = 0;
                let outputType: string = "";
    
                isAllRecordingWhileStreamingAdv = true;
    
                advAllSignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            } else if (signalInfo.signal === EOBSOutputSignal.Deactivate) {
                                isAllRecordingWhileStreamingAdv = false;
                                
                                if (signalCode != 0 && outputType != "") {
                                    done(new Error(outputType + ' stop signal returned with code ' + signalCode));
                                } else if (lastReplay == undefined || lastReplay.length == 0) {
                                    done(new Error('Failed to get last replay'));
                                } else {
                                    done();
                                }
                            }
                        } else if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Stopping recording
                                    osn.NodeObs.OBS_service_stopRecording();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
                            }  
                        } else if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Wrote) {
                                // Getting saved replay file name
                                lastReplay = osn.NodeObs.OBS_service_getLastReplay();
    
                                // Stopping replay buffer
                                osn.NodeObs.OBS_service_stopReplayBuffer(false);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                // If signal code is different than 0, something went wrong
                                if (signalInfo.code != 0) {
                                    signalCode = signalInfo.code;
                                    outputType = signalInfo.type;
                                }
    
                                // Stopping stream
                                osn.NodeObs.OBS_service_stopStreaming(false);
                            }
                        } else {
                            isAllRecordingWhileStreamingAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
                osn.NodeObs.OBS_service_startRecording();
                osn.NodeObs.OBS_service_startReplayBuffer();
            });
        });

        context('* Fail tests', function() {
            it('Record with invalid path in simple mode', function(done) {
                // Setting encoder and recording path
                setEnconderAndRecordingPath('Simple', true);

                isRecordingWithInvalidPathSimple = true;

                invalidPathRecordSignalsSimple.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isRecordingWithInvalidPathSimple = false;

                                if (signalInfo.code != osn.EOutputCode.Error) {
                                    done(new Error('Received wrong signal code. Was expecting -4.'));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isRecordingWithInvalidPathSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );

                osn.NodeObs.OBS_service_startRecording();
            });

            it('Start replay buffer with invalid path in simple mode', function(done) {
                isReplayWithInvalidPathSimple = true;

                invalidPathReplaySignalsSimple.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isReplayWithInvalidPathSimple = false;

                                if (signalInfo.code != osn.EOutputCode.Error) {
                                    done(new Error('Received wrong signal code (). Was expecting -4'));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isReplayWithInvalidPathSimple = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );

                osn.NodeObs.OBS_service_startReplayBuffer();
            });

            it('Record with invalid path in advanced mode', function(done) {
                // Setting encoder and recording path
                setEnconderAndRecordingPath('Advanced', true);

                isRecordingWithInvalidPathAdv = true;

                invalidPathRecordSignalsAdv.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Recording) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isRecordingWithInvalidPathAdv = false;

                                if (signalInfo.code != osn.EOutputCode.Error) {
                                    done(new Error('Received wrong signal code. Was expecting -4'));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isRecordingWithInvalidPathAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );

                osn.NodeObs.OBS_service_startRecording();
            });

            it('Start replay buffer with invalid path in advanced mode', function(done) {
                isReplayWithInvalidPathAdv = true;

                invalidPathReplaySignalsAdv.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.ReplayBuffer) {
                            if (signalInfo.signal === EOBSOutputSignal.Start) {
                                setTimeout(function() {
                                    // Saving replay
                                    osn.NodeObs.OBS_service_processReplayBufferHotkey();
                                }, 500);
                            } else if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isReplayWithInvalidPathAdv = false;

                                if (signalInfo.code != osn.EOutputCode.Error) {
                                    done(new Error('Received wrong signal code. Expecting -4'));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isReplayWithInvalidPathAdv = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );

                osn.NodeObs.OBS_service_startReplayBuffer();
            });

            it('Stream with invalid stream key', function(done) {
                saveStreamKey('invalid');
    
                isStreamingWithInvalidKey = true;
    
                invalidKeySignals.subscribe(
                    signalInfo => {
                        if (signalInfo.type === EOBSOutputType.Streaming) {
                            if (signalInfo.signal === EOBSOutputSignal.Stop) {
                                isStreamingWithInvalidKey = false;

                                if (signalInfo.code != osn.EOutputCode.InvalidStream) {
                                    done(new Error('Received signal code. Was expecting -3.'));
                                } else {
                                    done();
                                }
                            }
                        } else {
                            isStreamingWithInvalidKey = false;
                            done(new Error('No output signals received.'));
                        }
                    }
                );
    
                osn.NodeObs.OBS_service_startStreaming();
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
        })
    });
});
