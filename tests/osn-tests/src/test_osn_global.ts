import 'mocha'
import * as osn from '../osn';
import * as logger from '../util/logger';
import { expect } from 'chai'
import { ISource } from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';
import { EOBSInputTypes } from '../util/obs_enums';

const testName = 'osn-global';

describe(testName, () => {
    let obs: OBSHandler;
    
    // Initialize OBS process
    before(function() {
        logger.logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
        logger.logInfo(testName, 'Finished ' + testName + ' tests');
        logger.logEmptyLine();
    });

    it('Set source to output channel and get it', () => {
        // Creating input source
        const input = osn.InputFactory.create(EOBSInputTypes.ImageSource, 'test_osn_global_source');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, EOBSInputTypes.ImageSource));
        expect(input.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.InputId, EOBSInputTypes.ImageSource));
        expect(input.name).to.equal('test_osn_global_source', GetErrorMessage(ETestErrorMsg.InputName, EOBSInputTypes.ImageSource));

        // Setting input source to output channel
        osn.Global.setOutputSource(1, input);

        // Getting input source from output channel
        const channel = 1;
        const returnSource = osn.Global.getOutputSource(channel);

        // Checking if input source returned previously is correct
        expect(returnSource).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.NoInputInChannel, channel.toString()));
        expect(returnSource.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.InputFromChannelId));
        expect(returnSource.name).to.equal('test_osn_global_source', GetErrorMessage(ETestErrorMsg.InputFromChannelName));
        input.release();
    });

    it('Get flags (capabilities) of a source type', () => {
        let flags: number = undefined;

        // For each input type available get their flags and check if they are not undefined
        obs.inputTypes.forEach(inputType => {
            flags = osn.Global.getOutputFlagsFromId(inputType);
            expect(flags).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.GetOutputFlags, inputType));
            flags = undefined;
        });
    });

    it('Get lagged frames value', () => {
        let laggedFrames: number = undefined;

        // Getting lagged frames value
        laggedFrames = osn.Global.laggedFrames;

        // Checking if lagged frames was returned correctly
        expect(laggedFrames).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.LaggedFrames));
    });

    it('Get total frames value', () => {
        let totalFrames: number = undefined;

        // Getting total frames value
        totalFrames = osn.Global.totalFrames;

        // Checking if total frames was returned correctly
        expect(totalFrames).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.TotalFrames));
    });

    it('Set locale and get it', () => {
        let locale: string;

        // Setting locale
        osn.Global.locale = 'pt-BR';

        // Getting locale
        locale = osn.Global.locale;

        // Checking if locale was returned correctly
        expect(locale).to.equal('pt-BR', GetErrorMessage(ETestErrorMsg.Locale));
    });

    it('Fail test - Get source from empty output channel', () => {
        let input: ISource;
        let channel: number = 5;

        // Trying to get source from empty channel
        input = osn.Global.getOutputSource(channel);

        // Checking if source is undefined	            
        expect(input).to.equal(undefined, GetErrorMessage(ETestErrorMsg.ChannelNotEmpty, channel.toString()));
    });
});
