import 'mocha';
import * as osn from '../osn';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { Services } from '../util/services';
import { deleteConfigFiles } from '../util/general';

type TConfigEvent = 'starting_step' | 'progress' | 'stopping_step' | 'error' | 'done';

interface IConfigProgress {
    event: TConfigEvent;
    description: string;
    percentage?: number;
    continent?: string;
}

type TConfigProgressCallback = (progress: IConfigProgress) => void;

function handleProgress(progress: IConfigProgress) {
    if (progress.event === 'stopping_step') {
        if (progress.description === 'bandwidth_test') {
            osn.NodeObs.StartStreamEncoderTest();
        } else if (progress.description === 'streamingEncoder_test') {
            osn.NodeObs.StartRecordingEncoderTest();
        } else if (progress.description === 'recordingEncoder_test') {
            osn.NodeObs.StartCheckSettings();
        } else if (progress.description === 'checking_settings') {
            osn.NodeObs.StartSaveStreamSettings();
        } else if (progress.description === 'saving_service') {
            osn.NodeObs.StartSaveSettings();
        } else if (progress.description === 'setting_default_settings') {
            osn.NodeObs.StartSaveStreamSettings();
        }
    }

    if (progress.event === 'error') {
        osn.NodeObs.StartSetDefaultSettings();
    }

    if (progress.event === 'done') {
        osn.NodeObs.TerminateAutoConfig();
    }
}

function start(cb: TConfigProgressCallback) {
    cb({
        event: 'stopping_step',
        description: 'location_found',
    });

    osn.NodeObs.InitializeAutoConfig((progress: IConfigProgress) => {
        handleProgress(progress);
        cb(progress);
    },
    {
        service_name: 'Twitch',
    });

    osn.NodeObs.StartBandwidthTest();
}

function saveStreamKey(key: string) {
    // Getting stream settings container
    const streamSettings = osn.NodeObs.OBS_settings_getSettings('Stream').data;

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

describe('nodeobs_autoconfig', () => {
    let obs: OBSProcessHandler;
    let services: Services;
    let hasUserFromPool: boolean;

    // Initialize OBS process
    before(function() {
        obs = new OBSProcessHandler();
        services = new Services();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    // Shutdown OBS process
    after(async function() {
        if (hasUserFromPool) {
            await services.releaseUser();
        }
        
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
    });

    context('# Full auto config run (all functions)', () => {
        let hasStepFailed: boolean = false;

        it('Run all auto config steps (error)', function(done) {
            // Starting auto configuration processes
            start(progress => {
                if (progress.event === 'done') {
                    if (!hasStepFailed) {
                        done(new Error('Autoconfig completed successfully. An error was expected.'));
                    } else {
                        done();
                    }
                } else if (progress.event === 'error') {
                    hasStepFailed = true;
                }
            });
        });

        it('Run all auto config steps (success)', function(done) {
            hasStepFailed = false;

            // Getting stream key from user pool
            services.getStreamKey().then(key => {
                // Saving stream key
                saveStreamKey(key);
                hasUserFromPool = true;

                // Starting auto configuration processes
                start(progress => {
                    if (progress.event === 'done') {
                        if (hasStepFailed) {
                            done(new Error('An autoconfig step has failed.'));
                        } else {
                            done();
                        }
                        
                    } else if (progress.event === 'error') {
                        hasStepFailed = true;
                    }
                });
            // If unable to get stream key use environment variable
            }).catch(function() {
                // Saving stream key
                saveStreamKey(process.env.SLOBS_BE_STREAMKEY);
                hasUserFromPool = false;

                // Starting auto configuration processes
                start(progress => {
                    if (progress.event === 'done') {
                        if (hasStepFailed) {
                            done(new Error('An autoconfig step has failed.'));
                        } else {
                            done();
                        }  
                    } else if (progress.event === 'error') {
                        hasStepFailed = true;
                    }
                });
            });            
        });
    });
});
