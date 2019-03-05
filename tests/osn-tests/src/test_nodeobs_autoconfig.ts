import 'mocha';
import * as osn from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { deleteConfigFiles } from '../util/general';

type TConfigEvent = 'starting_step' | 'progress' | 'stopping_step' | 'error' | 'done';

interface IConfigStep {
    startMethod: string;
    identifier: string;
}

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

describe('nodeobs_autoconfig', () => {
    let obs: OBSProcessHandler;

    interface IConfigStepPresentation {
        description: string;
        summary: string;
        percentage?: number;
    }

    // Initialize OBS process
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
    });

    context('# Full auto config run (all functions)', () => {
        let hasStepFailed: boolean = false;

        it('Run all auto config steps (error)', function(done) {
            let stepInfo: IConfigStepPresentation[] = [];

            // Starting auto configuration processes
            start(progress => {
                if ( progress.event === 'starting_step' ||
                    progress.event === 'progress' ||
                    progress.event === 'stopping_step' ) {
                        const step = stepInfo.find(step => {
                        return step.description === progress.description;
                    });
        
                    if (step) {
                        step.percentage = progress.percentage;
                    } else {
                        stepInfo.push({ description: progress.description,
                                        summary: '',
                                        percentage: progress.percentage, });
                    }
                } else if (progress.event === 'done') {
                    if (!hasStepFailed)
                    {
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

            // Getting stream settings container
            const streamSettings = osn.NodeObs.OBS_settings_getSettings('Stream');

            // Setting stream service and stream key
            streamSettings.forEach(subCategory => {
                subCategory.parameters.forEach(parameter => {
                    if (parameter.name === 'service') {
                        parameter.currentValue = 'Twitch';
                    }

                    if (parameter.name === 'key') {
                        parameter.currentValue = process.env.SLOBS_BE_STREAMKEY;
                      }
                });
            });

            osn.NodeObs.OBS_settings_saveSettings('Stream', streamSettings);

            let stepInfo: IConfigStepPresentation[] = [];

            // Starting auto configuration processes
            start(progress => {
                if ( progress.event === 'starting_step' ||
                    progress.event === 'progress' ||
                    progress.event === 'stopping_step' ) {
                        const step = stepInfo.find(step => {
                        return step.description === progress.description;
                    });
        
                    if (step) {
                        step.percentage = progress.percentage;
                    } else {
                        stepInfo.push({ description: progress.description,
                                        summary: '',
                                        percentage: progress.percentage,
                        });
                    }
                } else if (progress.event === 'done') {
                    if (hasStepFailed)
                    {
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