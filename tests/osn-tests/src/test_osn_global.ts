import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { IInput, ISource } from '../osn';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSInputTypes } from '../util/general';

describe('osn-global', () => {
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

    context('# SetOutputSource and GetOutputSource', () => {
        let input: IInput;
        let returnSource: ISource;

        it('Set source to output channel and get it', () => {
            // Creating input source
            input = osn.InputFactory.create('image_source', 'test_osn_global_source');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal('image_source');
            expect(input.name).to.equal('test_osn_global_source');

            // Setting input source to output channel
            osn.Global.setOutputSource(1, input);

            // Getting input source from output channel
            returnSource = osn.Global.getOutputSource(1);

            // Checking if input source returned previously is correct
            expect(returnSource).to.not.equal(undefined);
            expect(returnSource.id).to.equal('image_source');
            expect(returnSource.name).to.equal('test_osn_global_source');
            input.release();
        });
        
        it('FAIL TEST: Get source from empty output channel', () => {
            let source: ISource;

            // Trying to get source from empty channel
            expect(function () {
                source = osn.Global.getOutputSource(5);
            }).to.throw();
        });
    });

    context('# GetOutputFlagsFromId', () => {
        it('Get flags (capabilities) of a source type', () => {
            let inputType: string;
            let inputTypes: string[];
            let flags: number = undefined;

            // Getting all input source types
            inputTypes = osn.InputFactory.types();

            // Checking if sourceTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(basicOBSInputTypes);

            // For each input type available get their flags and check if they are not undefined
            for (inputType of inputTypes)
            {
                flags = osn.Global.getOutputFlagsFromId(inputType);
                expect(flags).to.not.equal(undefined);
                flags = undefined;
            }
        });
    });

    context('# LaggedFrames', () => {
        it('Get lagged frames value', () => {
            let laggedFrames: number = undefined;

            // Getting lagged frames value
            laggedFrames = osn.Global.laggedFrames;

            // Checking if lagged frames was returned correctly
            expect(laggedFrames).to.not.equal(undefined);
        });
    });

    context('# TotalFrames', () => {
        it('Get total frames value', () => {
            let totalFrames: number = undefined;

            // Getting total frames value
            totalFrames = osn.Global.totalFrames;

            // Checking if total frames was returned correctly
            expect(totalFrames).to.not.equal(undefined);
        });
    });

    context('# SetLocale and GetLocale', () => {
        it('Set locale and get it', () => {
            let locale: string;

            // Setting locale
            osn.Global.locale = 'pt-BR';

            // Getting locale
            locale = osn.Global.locale;

            // Checking if locale was returned correctly
            expect(locale).to.equal('pt-BR');
        });
    });
});
