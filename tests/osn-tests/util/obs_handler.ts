import * as osn from '../osn';
import { Services } from '../util/services';
import { EOBSOutputType, EOBSOutputSignal} from '../util/obs_enums'
import { Subject } from 'rxjs';
import { first } from 'rxjs/operators';

// Interfaces
export interface IPerformanceState {
    CPU: number;
    numberDroppedFrames: number;
    percentageDroppedFrames: number;
    bandwidth: number;
    frameRate: number;
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
    services: Services;
    
    signals = new Subject<IOBSOutputSignalInfo>();
    progress = new Subject<IConfigProgress>();
    hasUserFromPool: boolean = false;
    inputTypes: string[];
    filterTypes: string[];
    transitionTypes: string[];

    constructor() {
        this.startup();
        this.inputTypes = osn.InputFactory.types();
        this.filterTypes = osn.FilterFactory.types();
        this.transitionTypes = osn.TransitionFactory.types();
    }

    startup() {
        const path = require('path');
        const uuid = require('uuid/v4');

        const wd = path.normalize(osn.wd);
        const pipeName = 'osn-tests-pipe'.concat(uuid());  

        try {
            osn.NodeObs.IPC.host(pipeName);
            osn.NodeObs.SetWorkingDirectory(wd);
            const initResult = osn.NodeObs.OBS_API_initAPI('en-US', path.join(path.normalize(__dirname), '..', 'osnData/slobs-client'), '0.00.00-preview.0');

            if (initResult != osn.EVideoCodes.Success) {
                throw Error('OBS process initialization failed with code ' + initResult);
            }  
        } catch(e) {
            throw Error('Exception when initializing OBS process' + e);
        }
    }

    shutdown(): boolean {
        try {
            osn.NodeObs.OBS_service_removeCallback();
            osn.NodeObs.IPC.disconnect();
        } catch(e) {
            return false;
        }

        return true;
    }

    instantiateUserPool() {
        this.services = new Services();
    }

    async reserveUser() {
        let streamKey: string = "";

        try {
            streamKey = await this.services.getStreamKey();
            this.hasUserFromPool = true;
        } catch(e) {
            streamKey = process.env.SLOBS_BE_STREAMKEY;
            this.hasUserFromPool = false;
        }

        this.setSetting('Stream', 'key', streamKey);
    }

    async releaseUser() {
        if (this.hasUserFromPool) {
            await this.services.releaseUser();
            this.hasUserFromPool = false;
        }
    }

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

    connectOutputSignals() {
        osn.NodeObs.OBS_service_connectOutputSignals((signalInfo: IOBSOutputSignalInfo) => {
            this.signals.next(signalInfo);
        });
    }

    getNextSignalInfo(): Promise<IOBSOutputSignalInfo> {
        return new Promise((resolve, reject) => {
            this.signals.pipe(first()).subscribe(signalInfo => resolve(signalInfo));
            setTimeout(() => reject('Output signal timeout'), 30000);
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
            setTimeout(() => reject( autoconfigStep + ' step timeout'), 50000);
        });
    }
}
