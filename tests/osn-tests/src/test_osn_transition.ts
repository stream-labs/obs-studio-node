import 'mocha'
import * as osn from '../osn';
import * as transitionSettings from '../util/transition_settings';
import * as logger from '../util/logger';
import { expect } from 'chai'
import { IScene, ITransition, ISettings, ISource } from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { EOBSTransitionTypes } from '../util/obs_enums';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';

const testName = 'osn-transition';

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

    it('Create all transition types', () => {
        const transitionName: string = 'test_osn_transition_create';

        // Create each transition type available
        obs.transitionTypes.forEach(transitionType => {
            const transition = osn.TransitionFactory.create(transitionType, transitionName);

            // Checking if transition was created correctly
            expect(transition).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateTransition, transitionType));
            expect(transition.id).to.equal(transitionType, GetErrorMessage(ETestErrorMsg.TransitionId, transitionType));
            expect(transition.name).to.equal(transitionName, GetErrorMessage(ETestErrorMsg.TransitionName, transitionType));
            transition.release();
        });
    });

    it('Create all transition types with settings', () => {
        const transitionName: string = 'test_osn_transition_create_settings';

        // Create each transition type availabe passing settings parameter
        obs.transitionTypes.forEach(transitionType => {
            let settings: ISettings = {};

            switch(transitionType) {
                case EOBSTransitionTypes.FadeToColor: {
                    settings = transitionSettings.fadeToColor;
                    settings['switch_point'] = 60;
                    break;
                }
                case EOBSTransitionTypes.Wipe: {
                    settings = transitionSettings.wipe;
                    settings['luma_invert'] = true;
                    break;
                }
            }

            const transition = osn.TransitionFactory.create(transitionType, transitionName, settings);

            // Checking if transition was created correctly
            expect(transition).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateTransition, transitionType));
            expect(transition.id).to.equal(transitionType, GetErrorMessage(ETestErrorMsg.TransitionId, transitionType));
            expect(transition.name).to.equal(transitionName, GetErrorMessage(ETestErrorMsg.TransitionName, transitionType));
            expect(transition.settings).to.include(settings, GetErrorMessage(ETestErrorMsg.TransitionSetting, transitionType));
            transition.release();
        });
    });

    it('Set source, get it and clear it', () => {
        let transition: ITransition;
        let scene: IScene;
        let source: ISource;
        let sceneName: string = 'test_osn_scene';
        
        transition = osn.TransitionFactory.create(EOBSTransitionTypes.Cut, 'transition');            
        scene = osn.SceneFactory.create(sceneName); 

        transition.set(scene);

        source = transition.getActiveSource();
        expect(source).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.GetActiveSource, EOBSTransitionTypes.Cut));
        expect(source.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));

        transition.clear();

        expect(function() {
            source = transition.getActiveSource();
        }).to.throw();

        transition.release();
        scene.release();         
    });

    it('Start transition to scene', () => {
        let transition: ITransition;
        let scene: IScene;
        let source: ISource;
        let sceneName: string = 'test_osn_scene';
        
        transition = osn.TransitionFactory.create(EOBSTransitionTypes.Cut, 'transition');

        scene = osn.SceneFactory.create(sceneName); 

        transition.start(0,scene);
        source = transition.getActiveSource();
        expect(source).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.GetActiveSource, EOBSTransitionTypes.Cut));
        expect(source.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));

        transition.release();
        scene.release();         
    });

    it('Fail test - Try to get source from transition without setting in to transition', () => {
        let source: ISource;
        let transition: ITransition;
        transition = osn.TransitionFactory.create(EOBSTransitionTypes.Cut, 'transition');  
            
        expect(function () {
            source = transition.getActiveSource();
        }).to.throw();

        transition.release();
    });

});
