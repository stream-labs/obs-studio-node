import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IScene, ITransition, ISettings, ISource } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSTransitionTypes } from '../util/general';

describe('osn-transition', () => {
    let obs: OBSProcessHandler;
    let transitionTypes: string[];

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

    context('# types', () => {
        it('Get all transition types', () => {
            // Gettin all transition types
            transitionTypes = osn.TransitionFactory.types();

            // Checking if transitionTypes array contains the basic obs transition types
            expect(transitionTypes.length).to.not.equal(0);
            expect(transitionTypes).to.include.members(basicOBSTransitionTypes);
        });
    });

    context('# create', () => {
        it('Create all transition types', () => {
            let transitionType: string;
            let transition: ITransition;
            const transitionName: string = 'test_osn_transition_create';
            // Create each transition type available
            for (transitionType of transitionTypes)
            {
                transition = osn.TransitionFactory.create(transitionType, transitionName);

                // Checking if transition was created correctly
                expect(transition).to.not.equal(undefined);
                expect(transition.id).to.equal(transitionType);
                expect(transition.name).to.equal(transitionName);
                transition.release();
            }
        });

        it('Create all transition types with settings', () => {
            let transitionType: string;
            let transition: ITransition;
            let settings: ISettings = {};
            settings['test'] = 1;
            const transitionName: string = 'test_osn_transition_create_settings';

            // Create each transition type availabe passing settings parameter
            for (transitionType of transitionTypes)
            {
                transition = osn.TransitionFactory.create(transitionType, transitionName, settings);

                // Checking if transition was created correctly
                expect(transition).to.not.equal(undefined);
                expect(transition.id).to.equal(transitionType);
                expect(transition.name).to.equal(transitionName);
                expect(transition.settings).to.include(settings);
                transition.release();
            }
        });

    });

    context('# getActiveSource, set, clear', () => {
        it('Set source, get it and clear it', () => {
            let transition: ITransition;
            let scene: IScene;
            let source: ISource;
            
            transition = osn.TransitionFactory.create(basicOBSTransitionTypes[0], 'transition');            
            scene = osn.SceneFactory.create('test_osn_scene'); 

            transition.set(scene);

            source = transition.getActiveSource();
            expect(source).to.not.equal(undefined);
            expect(source.name).to.equal(scene.name);

            transition.clear();

            expect(function() {
                source = transition.getActiveSource();
            }).to.throw();

            transition.release();
            scene.release();         
        });

        it('FAIL TEST: Try to get source from transition without setting in to transition', () => {
            let source: ISource;
            let transition: ITransition;
            transition = osn.TransitionFactory.create(basicOBSTransitionTypes[0], 'transition');  
               
            expect(function () {
                source = transition.getActiveSource();
            }).to.throw;
        });        
    });

    context('# start', () => {
        it('Start transition to scene', () => {
            let transition: ITransition;
            let scene: IScene;
            let source: ISource;
            
            transition = osn.TransitionFactory.create(basicOBSTransitionTypes[0], 'transition');

            scene = osn.SceneFactory.create('test_osn_scene'); 
 
            transition.start(0,scene);
            source = transition.getActiveSource();
            expect(source).to.not.equal(undefined);
            expect(source.name).to.equal(scene.name);

            transition.release();
            scene.release();         
        });
    });
});