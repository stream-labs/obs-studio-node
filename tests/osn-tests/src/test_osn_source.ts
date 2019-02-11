import 'mocha';
import { expect } from 'chai';
import * as osn from 'obs-studio-node';
import { ISettings } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSInputTypes, basicOBSFilterTypes, basicOBSTransitionTypes } from '../util/general';

describe('osn-source', () => {
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

    context('# IsConfigurable, GetProperties, GetSettings, GetName, GetOutputFlags and GetId', () => {
        it('Get all osn-source info from all input types', () => {
            // Getting all input source types
            const inputTypes = osn.InputFactory.types();

            // Checking if inputTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(basicOBSInputTypes);

            inputTypes.forEach(function(inputType) {
                // Creating input source
                const input = osn.InputFactory.create(inputType, 'input');
    
                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);

                // Getting input id
                const id = input.id;

                // Checking if id was returned correctly
                expect(id).to.not.equal(undefined);
                expect(id).to.equal(inputType);

                // Getting input name
                const inputName = input.name;

                // Checking if name was returned correctly
                expect(inputName).to.not.equal(undefined);
                expect(inputName).to.equal('input');

                // Getting input configurable value
                const configurableValue = input.configurable;
    
                // Checking if configurable value was returned properly
                expect(configurableValue).to.not.equal(undefined);

                // Getting input property
                const properties = input.properties;

                // Checking if properties were returned properly
                expect(properties).to.not.equal(undefined);

                // Getting input settings
                const settings = input.settings;

                // Checking if settings were returned properly
                expect(settings).to.not.equal(undefined);

                // Getting output flags
                const outputFlags = input.outputFlags;

                // Checking if output flags were returned properly
                expect(outputFlags).to.not.equal(undefined);
    
                input.release();
            });
        });

        it('Get all osn-source info from all filter types', () => {
            // Getting all filter types
            const filterTypes = osn.FilterFactory.types();

            // Checking if filterTypes array contains the basic obs filter types
            expect(filterTypes.length).to.not.equal(0);
            expect(filterTypes).to.include.members(basicOBSFilterTypes);

            filterTypes.forEach(function(filterType) {
                // Creating filter
                const filter = osn.FilterFactory.create(filterType, 'filter');
    
                // Checking if filter source was created correctly
                expect(filter).to.not.equal(undefined);

                // Getting filter id
                const id = filter.id;

                // Checking if id was returned correctly
                expect(id).to.not.equal(undefined);
                expect(id).to.equal(filterType);

                // Getting filter name
                const filterName = filter.name;

                // Checking if name was returned correctly
                expect(filterName).to.not.equal(undefined);
                expect(filterName).to.equal('filter');
    
                // Getting filter configurable value
                const configurableValue = filter.configurable;
    
                // Checking if configurable value was returned properly
                expect(configurableValue).to.not.equal(undefined);

                // Getting filter property
                const properties = filter.properties;

                // Checking if properties were returned properly
                expect(properties).to.not.equal(undefined);

                // Getting filter settings
                const settings = filter.settings;

                // Checking if settings were returned properly
                expect(settings).to.not.equal(undefined);

                // Getting output flags
                const outputFlags = filter.outputFlags;

                // Checking if output flags were returned properly
                expect(outputFlags).to.not.equal(undefined);
    
                filter.release();
            });
        });

        it('Get all osn-source info from all transition types', () => {
            // Getting all transition types
            const transitionTypes = osn.TransitionFactory.types();

            // Checking if transition array contains the basic obs filter types
            expect(transitionTypes.length).to.not.equal(0);
            expect(transitionTypes).to.include.members(basicOBSTransitionTypes);

            transitionTypes.forEach(function(transitionType) {
                // Creating transition
                const transition = osn.FilterFactory.create(transitionType, 'transition');
    
                // Checking if transition was created correctly
                expect(transition).to.not.equal(undefined);

                // Getting transition id
                const id = transition.id;

                // Checking if id was returned correctly
                expect(id).to.not.equal(undefined);
                expect(id).to.equal(transitionType);

                // Getting transition name
                const transitionName = transition.name;

                // Checking if name was returned correctly
                expect(transitionName).to.not.equal(undefined);
                expect(transitionName).to.equal('transition');
    
                // Getting transition configurable value
                const configurableValue = transition.configurable;
    
                // Checking if configurable value was returned properly
                expect(configurableValue).to.not.equal(undefined);

                // Getting transition property
                const properties = transition.properties;

                // Checking if properties were returned properly
                expect(properties).to.not.equal(undefined);

                // Getting transition settings
                const settings = transition.settings;

                // Checking if settings were returned properly
                expect(settings).to.not.equal(undefined);

                // Getting output flags
                const outputFlags = transition.outputFlags;

                // Checking if output flags were returned properly
                expect(outputFlags).to.not.equal(undefined);
    
                transition.release();
            });
        });

        it('Get all osn-source info of a scene', () => {
            // Creating scene
            const scene = osn.SceneFactory.create('test_osn_scene'); 

            // Checking if scene was created correctly
            expect(scene).to.not.equal(undefined);

            // Getting scene id
            const id = scene.id;

            // Checking if id was returned correctly
            expect(id).to.not.equal(undefined);
            expect(id).to.equal('scene');

            // Getting scene name
            const sceneName = scene.name;

            // Checking if name was returned correctly
            expect(sceneName).to.not.equal(undefined);
            expect(sceneName).to.equal('test_osn_scene');

            // Getting scene configurable value
            const configurableValue = scene.configurable;
    
            // Checking if configurable value was returned properly
            expect(configurableValue).to.not.equal(undefined);

            // Getting scene properties
            const properties = scene.properties;

            // Checking if properties were returned properly
            expect(properties).to.not.equal(undefined);

            // Getting scene settings
            const settings = scene.settings;

            // Checking if settings were returned properly
            expect(settings).to.not.equal(undefined);

            // Getting output flags
            const outputFlags = scene.outputFlags;

            // Checking if output flags were returned properly
            expect(outputFlags).to.not.equal(undefined);

            scene.release();
        });
    });

    context('# Update and Save', () => {
        let settings: ISettings = {};
        settings['test'] = 1;

        it('Update settings of all inputs', () => {
            // Getting all input source types
            const inputTypes = osn.InputFactory.types();

            // Checking if inputTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(basicOBSInputTypes);

            inputTypes.forEach(function(inputType) {
                // Creating input source
                const input = osn.InputFactory.create(inputType, 'input');
    
                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal('input');
    
                // Updating settings of source
                input.update(settings);

                // Sending save signal to source
                expect(function() {
                    input.save();
                }).to.not.throw;

                // Checking if setting was added to source
                expect(input.settings).to.include(settings);

                input.release();
            });
        });

        it('Update settings of all filters', () => {
            // Getting all filter types
            const filterTypes = osn.FilterFactory.types();

            // Checking if filterTypes array contains the basic obs filter types
            expect(filterTypes.length).to.not.equal(0);
            expect(filterTypes).to.include.members(basicOBSFilterTypes);

            filterTypes.forEach(function(filterType) {
                // Creating filter
                const filter = osn.FilterFactory.create(filterType, 'filter', settings);
    
                // Checking if filter source was created correctly
                expect(filter).to.not.equal(undefined);
                expect(filter.id).to.equal(filterType);
                expect(filter.name).to.equal('filter');
    
                // Updating settings of filter
                filter.update(settings);

                // Sending save signal to filter
                expect(function() {
                    filter.save();
                }).to.not.throw;

                // Checking if setting was added to filter
                expect(filter.settings).to.include(settings);
    
                filter.release();
            });
        });

        it('Update settings of all transitions', () => {
            // Getting all transition types
            const transitionTypes = osn.TransitionFactory.types();

            // Checking if transition array contains the basic obs filter types
            expect(transitionTypes.length).to.not.equal(0);
            expect(transitionTypes).to.include.members(basicOBSTransitionTypes);

            transitionTypes.forEach(function(filterType) {
                // Creating filter
                const filter = osn.FilterFactory.create(filterType, 'filter', settings);
    
                // Checking if filter source was created correctly
                expect(filter).to.not.equal(undefined);
                expect(filter.id).to.equal(filterType);
                expect(filter.name).to.equal('filter');
    
                // Updating settings of filter
                filter.update(settings);

                // Sending save signal to filter
                expect(function() {
                    filter.save();
                }).to.not.throw;

                // Checking if setting was added to filter
                expect(filter.settings).to.include(settings);
    
                filter.release();
            });
        });

        it('Update settings of a scene', () => {
            // Creating scene
            const scene = osn.SceneFactory.create('test_osn_scene'); 

            // Checking if scene was created correctly
            expect(scene).to.not.equal(undefined);
            expect(scene.id).to.equal('scene');
            expect(scene.name).to.equal('test_osn_scene');

            // Updating settings of scene
            scene.update(settings);

            // Checking if setting was added to scene
            expect(scene.settings).to.include(settings);

            scene.release();
        });
    });

    context('# SetFlag and GetFlag', () => {
        it('Set flags and get them for all input source types', () => {
            // Getting all input source types
            const inputTypes = osn.InputFactory.types();

            // Checking if inputTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(basicOBSInputTypes);

            inputTypes.forEach(function(inputType) {
                // Creating input source
                const input = osn.InputFactory.create(inputType, 'input');
    
                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal('input');
    
                // Setting input source flags
                input.flags = osn.ESourceFlags.ForceMono;

                // Getting input source flags
                const flags = input.flags;

                // Checking if flags were returned correctly
                expect(flags).to.not.equal(undefined);
                expect(flags).to.equal(osn.ESourceFlags.ForceMono);

                input.release();
            });
        });
    });

    context('# SetMuted and GetMuted', () => {
        it('Set muted and get it for all input source types', () => {
            // Getting all input source types
            const inputTypes = osn.InputFactory.types();

            // Checking if inputTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(basicOBSInputTypes);

            inputTypes.forEach(function(inputType) {
                // Creating input source
                const input = osn.InputFactory.create(inputType, 'input');
    
                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal('input');
    
                // Setting input source flags
                input.muted = true;

                // Getting input source flags
                const muted = input.muted;

                // Checking if flags were returned correctly
                expect(muted).to.not.equal(undefined);
                expect(muted).to.equal(true);

                input.release();
            });
        });
    });

    context('# SetEnabled and GetEnabled', () => {
        it('Set enabled and get it for all filter types', () => {
            // Getting all filter types
            const filterTypes = osn.FilterFactory.types();

            // Checking if filterTypes array contains the basic obs filter types
            expect(filterTypes.length).to.not.equal(0);
            expect(filterTypes).to.include.members(basicOBSFilterTypes);

            filterTypes.forEach(function(filterType) {
                // Creating filter
                const filter = osn.FilterFactory.create(filterType, 'filter');
    
                // Checking if filter source was created correctly
                expect(filter).to.not.equal(undefined);
                expect(filter.id).to.equal(filterType);
                expect(filter.name).to.equal('filter');
    
                // Setting enabled
                filter.enabled = true;

                // Getting enabled
                const enabled = filter.enabled;

                // Checking if enabled was returnd correctly
                expect(enabled).to.not.equal(undefined);
                expect(enabled).to.equal(true);
    
                filter.release();
            });
        });
    });
});