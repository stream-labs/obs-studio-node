import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { getCppErrorMsg } from '../util/general';

interface IPerformanceState {
    CPU: number;
    numberDroppedFrames: number;
    percentageDroppedFrames: number;
    bandwidth: number;
    frameRate: number;
}

type OBSHotkey = {
    ObjectName: string;
    ObjectType: osn.EHotkeyObjectType;
    HotkeyName: string;
    HotkeyDesc: string;
    HotkeyId: number;
};

describe('nodeobs_api', () => {
    let obs: OBSProcessHandler;
    let obsHotkeys: OBSHotkey[];
    
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    after(function() {
        obs.shutdown();
        obs = null;
    });

    context('# OBS_API_getPerformanceStatistics', () => {
        it('Fill stats object', () => {
            let stats: IPerformanceState;

            try {
                stats = osn.NodeObs.OBS_API_getPerformanceStatistics();
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }
            
            expect(stats.CPU).to.not.equal(undefined);
            expect(stats.bandwidth).to.not.equal(undefined);
            expect(stats.frameRate).to.not.equal(undefined);
            expect(stats.numberDroppedFrames).to.not.equal(undefined);
            expect(stats.percentageDroppedFrames).to.not.equal(undefined);
        });
    });

    context('# OBS_API_QueryHotkeys', () => {
        it('Get all hotkeys', () => {
            try {
                obsHotkeys = osn.NodeObs.OBS_API_QueryHotkeys();
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }
        });
    });

    context('# OBS_API_ProcessHotkeyStatus', () => {
        it('Process all hot keys gotten previously', () => {
            let hotkeyId: string;
            let isKeyDown: boolean = true;

            for (hotkeyId in obsHotkeys) {
                try {
                    osn.NodeObs.OBS_API_ProcessHotkeyStatus(+hotkeyId, isKeyDown);
                } catch(e) {
                    throw new Error(getCppErrorMsg(e));
                }
            }
        });
    });
});
