import * as osn from 'obs-studio-node';

export class OBSProcessHandler {
    startup(): boolean {
        const path = require('path');
        const wd = path.join(__dirname, '..', 'node_modules', 'obs-studio-node');
        const pipeName = 'osn-tests-pipe';  

        try {
            osn.NodeObs.IPC.ConnectOrHost(pipeName);
            osn.NodeObs.SetWorkingDirectory(wd);
            osn.NodeObs.OBS_API_initAPI('en-US', path.join(__dirname, '..', 'AppData'));
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