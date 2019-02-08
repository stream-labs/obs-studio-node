import * as osn from 'obs-studio-node';

export class OBSProcessHandler {
    startup(): boolean {
        const path = require('path');
        const uuid = require('uuid/v4');

        const wd = path.join(path.normalize(__dirname), '..', 'node_modules', 'obs-studio-node');
        const pipeName = 'osn-tests-pipe'.concat(uuid());  

        try {
            osn.NodeObs.IPC.host(pipeName);
            osn.NodeObs.SetWorkingDirectory(wd);
            osn.NodeObs.OBS_API_initAPI('en-US', path.join(path.normalize(__dirname), '..', 'osnData/slobs-client'));
        } catch(e) {
            return false;
        }

        return true;
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