import 'mocha';
import { expect } from 'chai';
import * as osn from 'obs-studio-node';
import { IProperties, ISettings } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSInputTypes, basicOBSFilterTypes } from '../util/general';

describe('osn-video', () => {
    let obs: OBSProcessHandler;

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
    });

    context('# GetGlobal', () => {
        it('Get video inteface', () => {
            // Getting video interface
            const video = osn.VideoFactory.getGlobal();

            // Checking if video was returned properly
            expect(video).to.not.equal(undefined);
        });
    });

    context('# GetSkippedFrames', () => {
        it('Get skipped frames value', () => {
            // Getting skipped frames
            const skippedFrames = osn.VideoFactory.getGlobal().skippedFrames;

            // Checking if skipped frames was returned properly
            expect(skippedFrames).to.not.equal(undefined);
            expect(skippedFrames).to.equal(0);
        });
    });

    context('# GetTotalFrames', () => {
        it('Get total frames value', () => {
            // Getting total frames value
            const totalFrames = osn.VideoFactory.getGlobal().totalFrames;

            // Checking if total frames was returned properly
            expect(totalFrames).to.not.equal(undefined);
            expect(totalFrames).to.equal(0);
        });
    });
});