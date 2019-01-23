import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IInput } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSInputTypes } from '../util/general';

describe('lost ipc connection when creating multiple sources', () => {
    let obs: OBSProcessHandler;

    // Initialize OBS process
    before(function () {
        obs = new OBSProcessHandler();

        if (obs.startup() != true) {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    // Shutdown OBS process
    after(function (done) {
        this.timeout(5000);
        obs.shutdown();
        obs = null;
        setTimeout(done, 3000);
    });

    it('Create all types of input', function () {
        let inputTypes: string[];
        let input: IInput;

        // Getting all input source types
        inputTypes = osn.InputFactory.types();

        // Checking if inputTypes array contains the basic obs input types
        expect(inputTypes.length).to.not.equal(0);
        expect(inputTypes).to.include.members(basicOBSInputTypes);

        inputTypes.forEach(function (inputType) {
            console.log('creating input source ' + inputType + '...');
            input = osn.InputFactory.create(inputType, 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input');
            input.release();
        });

        inputTypes.forEach(function (inputType) {
            console.log('creating input source ' + inputType + ' a second time...');
            input = osn.InputFactory.create(inputType, 'input2');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input2');
            input.release();
        });

        inputTypes.forEach(function (inputType) {
            console.log('creating input source ' + inputType + ' a third time...');
            input = osn.InputFactory.create(inputType, 'input3');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input3');
            input.release();
        });

        inputTypes.forEach(function (inputType) {
            console.log('creating input source ' + inputType + ' a fourth time...');
            input = osn.InputFactory.create(inputType, 'input4');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input4');
            input.release();
        });

        inputTypes.forEach(function (inputType) {
            console.log('creating input source ' + inputType + ' a fifth time...');
            input = osn.InputFactory.create(inputType, 'input5');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input5');
            input.release();
        });
    });
});