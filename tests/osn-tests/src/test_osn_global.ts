import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { IInput, ISource } from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';

const testName = 'osn-global';

describe(testName, () => {
    let obs: OBSHandler;
    
    // Initialize OBS process
    before(function() {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
        logInfo(testName, 'Finished ' + testName + ' tests');
        logEmptyLine();
    });

    context('# SetOutputSource and GetOutputSource', () => {
        it('Set source to output channel and get it', () => {
            // Creating input source
            const input = osn.InputFactory.create('image_source', 'test_osn_global_source');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal('image_source');
            expect(input.name).to.equal('test_osn_global_source');

            // Setting input source to output channel
            osn.Global.setOutputSource(1, input);

            // Getting input source from output channel
            const returnSource = osn.Global.getOutputSource(1);

            // Checking if input source returned previously is correct
            expect(returnSource).to.not.equal(undefined);
            expect(returnSource.id).to.equal('image_source');
            expect(returnSource.name).to.equal('test_osn_global_source');
            input.release();
        });
        
        it('FAIL TEST: Get source from empty output channel', () => {
            let input: ISource;

            // Trying to get source from empty channel
            input = osn.Global.getOutputSource(5);
            // Checking if source is undefined	            
            expect(input).to.equal(undefined);
        });
    });

    context('# GetOutputFlagsFromId', () => {
        it('Get flags (capabilities) of a source type', () => {
            let flags: number = undefined;

            // For each input type available get their flags and check if they are not undefined
            obs.inputTypes.forEach(inputType => {
                flags = osn.Global.getOutputFlagsFromId(inputType);
                expect(flags).to.not.equal(undefined);
                flags = undefined;
            });
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
