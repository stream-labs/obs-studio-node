import * as osn from '../osn';
import { logInfo, logWarning } from '../util/logger';
import { UserPoolHandler } from './user_pool_handler';
import { CacheUploader } from '../util/cache-uploader'
import { EOBSOutputType, EOBSOutputSignal, EOBSSettingsCategories} from '../util/obs_enums'
import { Subject } from 'rxjs';
import { first } from 'rxjs/operators';

// Interfaces
export interface IPerformanceState {
    CPU: number;
    numberDroppedFrames: number;
    percentageDroppedFrames: number;
    streamingBandwidth: number;
    streamingDataOutput: number;
    recordingBandwidth: number;
    recordingDataOutput: number;
    frameRate: number;
    averageTimeToRenderFrame: number;
    memoryUsage: number;
    diskSpaceAvailable: string;
}

export interface IOBSOutputSignalInfo {
    type: EOBSOutputType;
    signal: EOBSOutputSignal;
    code: osn.EOutputCode;
    error: string;
}

export interface IConfigProgress {
    event: TConfigEvent;
    description: string;
    percentage?: number;
    continent?: string;
}

export interface IVec2 {
    x: number;
    y: number;
}

export interface ICrop {
    top: number;
    bottom: number;
    left: number;
    right: number;
}

// Types
export type TOBSHotkey = {
    ObjectName: string;
    ObjectType: osn.EHotkeyObjectType;
    HotkeyName: string;
    HotkeyDesc: string;
    HotkeyId: number;
};

export type TConfigEvent = 'starting_step' | 'progress' | 'stopping_step' | 'error' | 'done';

// OBSHandler class
export class OBSHandler {
    private path = require('path');
    private uuid = require('uuid/v4');

    // Variables for obs initialization
    private workingDirectory: string = this.path.normalize(osn.wd);
    private language: string = 'en-US';
    private obsPath: string = this.path.join(this.path.normalize(__dirname), '..', 'osnData/slobs-client');
    private pipeName: string = 'osn-tests-pipe-'.concat(this.uuid());
    private version: string = '0.00.00-preview.0';

    // Other variables/objects
    private userPoolHandler: UserPoolHandler;
    private cacheUploader: CacheUploader;
    private hasUserFromPool: boolean = false;
    private osnTestName: string;
    private signals = new Subject<IOBSOutputSignalInfo>();
    private progress = new Subject<IConfigProgress>();
    inputTypes: string[];
    filterTypes: string[];
    transitionTypes: string[];
    os: string;

    constructor(testName: string) {
        this.os = process.platform;
        this.osnTestName = testName;
        this.cacheUploader = new CacheUploader(testName, this.obsPath);
        this.startup();
        this.inputTypes = osn.InputFactory.types();
        const index = this.inputTypes.indexOf('syphon-input', 0);
        if (index > -1) {
            this.inputTypes.splice(index, 1);
        }
        this.filterTypes = osn.FilterFactory.types();
        this.transitionTypes = osn.TransitionFactory.types();
    }

    startup() {
        let initResult: any;
        logInfo(this.osnTestName, 'Initializing OBS');

        try {
            logInfo(this.osnTestName, 'pipeName: ' + this.pipeName);
            logInfo(this.osnTestName, 'workingDirectory: ' + this.workingDirectory);
            logInfo(this.osnTestName, 'obsPath: ' + this.obsPath);
            logInfo(this.osnTestName, 'version: ' + this.version);
            osn.NodeObs.IPC.host(this.pipeName);
            osn.NodeObs.SetWorkingDirectory(this.workingDirectory);
            initResult = osn.NodeObs.OBS_API_initAPI(this.language, this.obsPath, this.version);
        } catch(e) {
            throw Error('Exception when initializing OBS process: ' + e);
        }

        if (initResult != osn.EVideoCodes.Success) {
            throw Error('OBS process initialization failed with code ' + initResult);
        }  

        logInfo(this.osnTestName, 'OBS started successfully');
    }

    shutdown() {
        logInfo(this.osnTestName, 'Shutting down OBS');

        try {
            osn.NodeObs.OBS_service_removeCallback();
            osn.NodeObs.IPC.disconnect();
        } catch(e) {
            throw Error('Exception when shutting down OBS process: ' + e);
        }

        logInfo(this.osnTestName, 'OBS shutdown successfully');
    }
	
	instantiateUserPool(testName: string) {
        this.userPoolHandler = new UserPoolHandler(testName);
    }

    async reserveUser() {
        let streamKey: string = "";

        try {
            logInfo(this.osnTestName, 'Getting stream key from user pool');
            streamKey = await this.userPoolHandler.getStreamKey();
            this.hasUserFromPool = true;
        } catch(e) {
            logWarning(this.osnTestName, e);
            logWarning(this.osnTestName, 'Using predefined stream key');
            streamKey = process.env.SLOBS_BE_STREAMKEY;
            this.hasUserFromPool = false;
        }

        logInfo(this.osnTestName, 'Saving stream key');
        this.setSetting(EOBSSettingsCategories.Stream, 'key', streamKey);

        let savedStreamKey = this.getSetting(EOBSSettingsCategories.Stream, 'key');
        if (savedStreamKey == streamKey) {
            logInfo(this.osnTestName, 'Stream key saved successfully');
        } else {
            throw Error('Failed to save stream key');
        }
    }

    async releaseUser() {
        if (this.hasUserFromPool) {
            await this.userPoolHandler.releaseUser();
            this.hasUserFromPool = false;
        }
    }

    async uploadTestCache() {
        try {
            await this.cacheUploader.uploadCache();
        } catch(e) {
            logWarning(this.osnTestName, e);
        }
    };

    setSetting(category: string, parameter: string, value: any) {
        let oldValue: any;

        // Getting settings container
        const settings = osn.NodeObs.OBS_settings_getSettings(category).data;

        settings.forEach(subCategory => {
            subCategory.parameters.forEach(param => {
                if (param.name === parameter) {
                    oldValue = param.currentValue;
                    param.currentValue = value;
                }
            });
        });

        // Saving updated settings container
        if (value != oldValue) {
            osn.NodeObs.OBS_settings_saveSettings(category, settings);
        }
    }
	
    getSetting(category: string, parameter: string): any {
        let value: any;

        // Getting settings container
        const settings = osn.NodeObs.OBS_settings_getSettings(category).data;

        // Getting parameter value
        settings.forEach(subCategory => {
            subCategory.parameters.forEach(param => {
                if (param.name === parameter) {
                    value = param.currentValue;
                }
            });
        });

        return value;
    }

    setSettingsContainer(category: string, settings: any) {
        osn.NodeObs.OBS_settings_saveSettings(category, settings);
    }

    getSettingsContainer(category: string): any {
        return osn.NodeObs.OBS_settings_getSettings(category).data;
    }

    connectOutputSignals() {
        osn.NodeObs.OBS_service_connectOutputSignals((signalInfo: IOBSOutputSignalInfo) => {
            this.signals.next(signalInfo);
        });
    }

    getNextSignalInfo(output: string, signal: string): Promise<IOBSOutputSignalInfo> {
        return new Promise((resolve, reject) => {
            this.signals.pipe(first()).subscribe(signalInfo => resolve(signalInfo));
            setTimeout(() => reject(new Error(output.replace(/^\w/, c => c.toUpperCase()) + ' ' + signal + ' signal timeout')), 30000);
        });
    }

    startAutoconfig() {
        osn.NodeObs.InitializeAutoConfig((progressInfo: IConfigProgress) => {
            if (progressInfo.event == 'stopping_step' || progressInfo.event == 'done' || progressInfo.event == 'error') {
                this.progress.next(progressInfo);
            }
        },
        {
            service_name: 'Twitch',
        });
    }

    getNextProgressInfo(autoconfigStep: string): Promise<IConfigProgress> {
        return new Promise((resolve, reject) => {
            this.progress.pipe(first()).subscribe(progressInfo => resolve(progressInfo));
            setTimeout(() => reject(new Error(autoconfigStep + ' step timeout')), 50000);
        });
    }
}
