import * as osn from 'obs-studio-node';

export class OBSProcessHandler {
    startup(): osn.EVideoCodes {
        const path = require('path');
        const wd = path.join(path.normalize(__dirname), '..', 'node_modules', 'obs-studio-node');
        const pipeName = 'osn-tests-pipe';  

        try {
            osn.NodeObs.IPC.host(pipeName);
            osn.NodeObs.SetWorkingDirectory(wd);
            const initResult = osn.NodeObs.OBS_API_initAPI('en-US', path.join(path.normalize(__dirname), '..', 'osnData/slobs-client'));
            return initResult;
        } catch(e) {
            return osn.EVideoCodes.Fail;
        }
    }

    shutdown(): boolean {
        try {
            osn.NodeObs.OBS_API_destroyOBS_API();
            osn.NodeObs.IPC.disconnect();
        } catch(e) {
            return false;
        }

        return true;
    }
}