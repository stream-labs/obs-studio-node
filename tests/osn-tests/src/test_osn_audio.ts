import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';

const testName = 'osn-audio';

describe(testName, () => {
    let obs: OBSHandler;
    let hasTestFailed: boolean = false;

    // Initialize OBS process
    before(function() {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
    });

    // Shutdown OBS process
    after(async function() {
        obs.shutdown();

        if (hasTestFailed === true) {
            logInfo(testName, 'One or more test cases failed. Uploading cache');
            await obs.uploadTestCache();
        }

        obs = null;
        deleteConfigFiles();
        logInfo(testName, 'Finished ' + testName + ' tests');
        logEmptyLine();
    });

    afterEach(function() {
        if (this.currentTest.state == 'failed') {
            hasTestFailed = true;
        }
    });

    it('Get and set audio context', () => {
        let currentAudio = osn.Audio.get();

        // Check if the current video context correctly returns the default values
        expect(currentAudio.sampleRate).to.equal(44100, GetErrorMessage(ETestErrorMsg.AudioDefaultSampleRate));
        expect(currentAudio.speakers).to.equal(osn.ESpeakerLayout.Stereo, GetErrorMessage(ETestErrorMsg.AudioDefaultSpeakers));

        const newAudioContext: osn.AudioContext = {
            sampleRate: 48000,
            speakers: osn.ESpeakerLayout.SevenOne
        }
        osn.Audio.set(newAudioContext);

        currentAudio = osn.Audio.get();
        expect(currentAudio.sampleRate).to.equal(48000, GetErrorMessage(ETestErrorMsg.AudioSampleRate));
        expect(currentAudio.speakers).to.equal(osn.ESpeakerLayout.SevenOne, GetErrorMessage(ETestErrorMsg.AudioSpeakers));
    });
});
