import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums'

interface Dictionary<TItemType> {
    [key: string]: TItemType;
}

interface IAudioSourceData {
    fader?: osn.IFader;
    volmeter?: osn.IVolmeter;
    callbackInfo?: osn.ICallbackData;
}

interface IVolmeter {
    magnitude: number[];
    peak: number[];
    inputPeak: number[];
}

const testName = 'osn-volmeter';

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

    context('# Create and Attach', () => {
        it('Create volmeter and attach it to a audio source', () => {
            // Creating audio source
            const input = osn.InputFactory.create(EOBSInputTypes.WASAPIInput, 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(EOBSInputTypes.WASAPIInput);
            expect(input.name).to.equal('input');

            // Creating volmeter
            const volmeter = osn.VolmeterFactory.create(osn.EFaderType.IEC);

            // Checking if volmeter was created correctly
            expect(volmeter).to.not.equal(undefined);

            // Attach volmeter to input source
            expect(function() {
                volmeter.attach(input);
            }).to.not.throw();

            input.release();
        });
    });

    context('# AddCallback and RemoveCallback', () => {
        it('Add callback to volmeter and remove it', () => {
            // Creating audio source
            const input = osn.InputFactory.create(EOBSInputTypes.WASAPIInput, 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(EOBSInputTypes.WASAPIInput);
            expect(input.name).to.equal('input');

            // Creating volmeter
            const volmeter = osn.VolmeterFactory.create(osn.EFaderType.IEC);

            // Checking if volmeter was created correctly
            expect(volmeter).to.not.equal(undefined);

            // Attaching volmeter to source
            volmeter.attach(input);

            // Adding callback to volmeter
            const cb = volmeter.addCallback((magnitude: number[], peak: number[], inputPeak: number[]) => {});

            // Checking if callback was added correctly
            expect(cb).to.not.equal(undefined);

            // Removing callback from volmeter
            const rmResult = volmeter.removeCallback(cb);

            // Checking if callback was removed correctly
            expect(rmResult).to.equal(true)

            input.release();
        });
    });
});
