import 'mocha';
import { expect } from 'chai';
import * as osn from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';

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

describe('osn-volmeter', () => {
    let obs: OBSProcessHandler;
    let inputTypes: string[];

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

    context('# Create and Attach', () => {
        it('Create volmeter and attach it to a audio source', () => {
            // Creating audio source
            const input = osn.InputFactory.create('wasapi_input_capture', 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal('wasapi_input_capture');
            expect(input.name).to.equal('input');

            // Creating volmeter
            const volmeter = osn.VolmeterFactory.create(osn.EFaderType.IEC);

            // Checking if volmeter was created correctly
            expect(volmeter).to.not.equal(undefined);

            // Attach volmeter to input source
            volmeter.attach(input);
            input.release();
        });
    });

    context('# AddCallback and RemoveCallback', () => {
        it('Add callback to volmeter and remove it', () => {
            // Creating audio source
            const input = osn.InputFactory.create('wasapi_input_capture', 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal('wasapi_input_capture');
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
            volmeter.removeCallback(cb);
        });
    });
});