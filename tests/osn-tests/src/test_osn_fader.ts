import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { OBSHandler } from '../util/obs_handler'
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums';

const testName = 'osn-fader';

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

    it('Create cubic fader and attach it to a audio source', () => {
        // Creating audio source
        const input = osn.InputFactory.create(EOBSInputTypes.WASAPIInput, 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, EOBSInputTypes.WASAPIInput));
        expect(input.id).to.equal(EOBSInputTypes.WASAPIInput, GetErrorMessage(ETestErrorMsg.InputId, EOBSInputTypes.WASAPIInput));
        expect(input.name).to.equal('input', GetErrorMessage(ETestErrorMsg.InputName, EOBSInputTypes.WASAPIInput));

        // Creating fader
        const cubicFader = osn.FaderFactory.create(osn.EFaderType.Cubic);

        // Checking if fader was created correctly
        expect(cubicFader).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateFader, 'cubic'));

        // Attach fader to input source
        expect(function() {
            cubicFader.attach(input);
        }).to.not.throw();
        
        input.release();
    });

    it('Create IEC fader and attach it to a audio source', () => {
        // Creating audio source
        const input = osn.InputFactory.create(EOBSInputTypes.WASAPIInput, 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, EOBSInputTypes.WASAPIInput));
        expect(input.id).to.equal(EOBSInputTypes.WASAPIInput, GetErrorMessage(ETestErrorMsg.InputId, EOBSInputTypes.WASAPIInput));
        expect(input.name).to.equal('input', GetErrorMessage(ETestErrorMsg.InputName, EOBSInputTypes.WASAPIInput));

        // Creating fader
        const iecFader = osn.FaderFactory.create(osn.EFaderType.IEC);

        // Checking if fader was created correctly
        expect(iecFader).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateFader, 'IEC'));

        // Attach fader to input source
        expect(function() {
            iecFader.attach(input);
        }).to.not.throw();
        
        input.release();
    });

    it('Create logarithmic fader and attach it to a audio source', () => {
        // Creating audio source
        const input = osn.InputFactory.create(EOBSInputTypes.WASAPIInput, 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, EOBSInputTypes.WASAPIInput));
        expect(input.id).to.equal(EOBSInputTypes.WASAPIInput, GetErrorMessage(ETestErrorMsg.InputId, EOBSInputTypes.WASAPIInput));
        expect(input.name).to.equal('input', GetErrorMessage(ETestErrorMsg.InputName, EOBSInputTypes.WASAPIInput));

        // Creating fader
        const logFader = osn.FaderFactory.create(osn.EFaderType.Log);

        // Checking if fader was created correctly
        expect(logFader).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateFader, 'log'));

        // Attach fader to input source
        expect(function() {
            logFader.attach(input);
        }).to.not.throw();
        
        input.release();
    });

    it('Get decibel, deflection and multiplier values', () => {
        let faderTypes: osn.EFaderType[] = [osn.EFaderType.Cubic, osn.EFaderType.IEC, osn.EFaderType.Log];
        let faderTypeStr: string;

        faderTypes.forEach(function(faderType) {
            switch(faderType) {
                case osn.EFaderType.Cubic: {
                    faderTypeStr = 'cubic';
                    break;
                }
                case osn.EFaderType.IEC: {
                    faderTypeStr = 'IEC';
                    break;
                }
                case osn.EFaderType.Log: {
                    faderTypeStr = 'log';
                    break;
                }
            }

            // Creating fader
            const fader = osn.FaderFactory.create(faderType);

            // Checking if fader was created correctly
            expect(fader).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateFader, faderTypeStr));

            // Getting fader decibel
            const decibel = fader.db;

            // Checking if decibel value was returned correctly
            expect(decibel).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.Decibel, faderTypeStr));
            expect(decibel).to.equal(0, GetErrorMessage(ETestErrorMsg.Decibel, faderTypeStr));

            // Getting fader deflection
            const deflection = fader.deflection;

            // Checking if deflection value was returned correctly
            expect(deflection).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.Deflection, faderTypeStr));
            expect(deflection).to.equal(1, GetErrorMessage(ETestErrorMsg.Deflection, faderTypeStr));

            // Getting multiplier decibel
            const multiplier = fader.mul;

            // Checking if multiplier value was returned correctly
            expect(multiplier).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.Multiplier, faderTypeStr));
            expect(multiplier).to.equal(1, GetErrorMessage(ETestErrorMsg.Multiplier, faderTypeStr));
        });
    });
});
