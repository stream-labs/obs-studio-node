import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { ISettings } from '../osn';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSInputTypes, basicDebugOBSInputTypes, basicOBSFilterTypes, basicOBSTransitionTypes } from '../util/general';

describe('osn-source', () => {
    let obs: OBSProcessHandler;
    let OBSInputTypes: string[];

    // Initialize OBS process
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }

        if (process.env.BUILD_REASON=="PullRequest") {
            OBSInputTypes = basicDebugOBSInputTypes;
        } else {
            OBSInputTypes = basicOBSInputTypes;
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
            expect(inputTypes).to.include.members(OBSInputTypes);

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
        it('Update settings of all inputs', () => {
            let settings: ISettings = {};

            // Getting all input source types
            const inputTypes = osn.InputFactory.types();

            // Checking if inputTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(OBSInputTypes);

            inputTypes.forEach(function(inputType) {
                // Creating input source
                const input = osn.InputFactory.create(inputType, 'input');
    
                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal('input');

                // Preparing settings object
                settings = input.settings;
                
                switch(inputType) {
                    case 'image_source': {
                        settings['unload'] = true;
                        break;
                    }
                    case 'color_source': {
                        settings['height'] = 500;
                        settings['width'] = 600;
                        break;
                    }
                    case 'slideshow': {
                        settings['loop'] = false;
                        settings['slide_time'] = 9000;
                        settings['transition_speed'] = 800;
                        break;
                    }
                    case 'browser_source': {
                        settings['fps'] = 60;
                        settings['height'] = 500;
                        settings['restart_when_active'] = true;
                        settings['shutdown'] = true;
                        break;
                    }
                    case 'ffmpeg_source': {
                        settings['buffering_mb'] = 3;
                        settings['clear_on_media_end'] = false;
                        settings['looping'] = true;
                        settings['speed_percent'] = 80;
                        settings['caching'] = true;
                        break;
                    }
                    case 'text_gdiplus': {
                        settings['align'] = 'right';
                        settings['extents_cx'] = 90;
                        settings['extents_cy'] = 90;
                        settings['valign'] = 'bottom';
                        break;
                    }
                    case 'text_ft2_source': {
                        settings['color1'] = 4294967296;
                        settings['color2'] = 5294967295;
                        break;
                    }
                    case 'monitor_capture': {
                        settings['capture_cursor'] = false;
                        settings['monitor'] = 1;
                        break;
                    }
                    case 'window_capture': {
                        settings['compatibility'] = true;
                        settings['cursor'] = false;
                        break;
                    }
                    case 'game_capture': {
                        settings['allow_transparency'] = true;
                        settings['force_scaling'] = true;
                        settings['hook_rate'] = 2;
                        break;
                    }
                    case 'dshow_input': {
                        settings['audio_output_mode'] = 1;
                        settings['res_type'] = 1;
                        settings['video_format'] = 2;
                        break;
                    }
                    case 'wasapi_input_capture': {
                        settings['use_device_timing'] = true;
                        break;
                    }
                    case 'wasapi_output_capture': {
                        settings['use_device_timing'] = false;
                        break;
                    }
                }
    
                // Updating settings of source
                input.update(settings);

                // Sending save signal to source
                expect(function() {
                    input.save();
                }).to.not.throw;

                // Checking if setting was added to source
                expect(input.settings).to.eql(settings);

                settings = {};
                input.release();
            });
        });

        it('Update settings of all filters', () => {
            let settings: ISettings = {};

            // Getting all filter types
            const filterTypes = osn.FilterFactory.types();

            // Checking if filterTypes array contains the basic obs filter types
            expect(filterTypes.length).to.not.equal(0);
            expect(filterTypes).to.include.members(basicOBSFilterTypes);

            filterTypes.forEach(function(filterType) {
                if (filterType != 'gpu_delay' &&
                    filterType != 'async_delay_filter' &&
                    filterType != 'invert_polarity_filter' &&
                    filterType != 'vst_filter') {
                    // Creating filter
                    const filter = osn.FilterFactory.create(filterType, 'filter');
        
                    // Checking if filter source was created correctly
                    expect(filter).to.not.equal(undefined);
                    expect(filter.id).to.equal(filterType);
                    expect(filter.name).to.equal('filter');

                    // Preparing settings object
                    settings = filter.settings;

                    switch(filterType) {
                        case 'mask_filter': {
                            settings['color'] = 26777215;
                            settings['opacity'] = 80;
                            break;
                        }
                        case 'crop_filter': {
                            settings['relative'] = false;
                            break;
                        }
                        case 'gain_filter': {
                            settings['db'] = 5;
                            break;
                        }
                        case 'color_filter': {
                            settings['brightness'] = 10;
                            settings['contrast'] = 15;
                            settings['saturation'] = 55;
                            break;
                        }
                        case 'scale_filter': {
                            settings['sampling'] = 'bilinear';
                            settings['undistort'] = true;
                            break;
                        }
                        case 'scroll_filter': {
                            settings['cx'] = 80;
                            settings['cy'] = 60;
                            settings['limit_size'] = true;
                            break;
                        }
                        case 'color_key_filter': {
                            settings['gamma'] = 60;
                            settings['key_color_type'] = 'red';
                            settings['similarity'] = 55;
                            break;
                        }
                        case 'clut_filter': {
                            settings['clut_amount'] = 2;
                            break;
                        }
                        case 'sharpness_filter': {
                            settings['sharpness'] = 0.05;
                            break;
                        }
                        case 'chroma_key_filter': {
                            settings['key_color'] = 68280;
                            settings['smoothness'] = 20;
                            settings['spill'] = 90;
                            break;
                        }
                        case 'noise_suppress_filter': {
                            settings['suppress_level'] = -10;
                            settings['']
                            break;
                        }
                        case 'noise_gate_filter': {
                            settings['hold_time'] = 120;
                            settings['open_threshold'] = -12;
                            settings['release_time'] = 80;
                            break;
                        }
                        case 'compressor_filter': {
                            settings['attack_time'] = 8;
                            settings['output_gain'] = 4;
                            settings['threshold'] = -9;
                            break;
                        }
                        case 'limiter_filter': {
                            settings['release_time'] = 30;
                            settings['threshold'] = -12;
                            break;
                        }
                        case 'expander_filter': {
                            settings['attack_time'] = 15;
                            settings['ratio'] = 5;
                            settings['threshold'] = -20;
                            break;
                        }
                    }
        
                    // Updating settings of filter
                    filter.update(settings);

                    // Sending save signal to filter
                    expect(function() {
                        filter.save();
                    }).to.not.throw;

                    // Checking if setting was added to filter
                    expect(filter.settings).to.eql(settings);
        
                    filter.release();
                }
            });
        });

        it('Update settings of all transitions', () => {
            let settings: ISettings = {};

            // Getting all transition types
            const transitionTypes = osn.TransitionFactory.types();

            // Checking if transition array contains the basic obs filter types
            expect(transitionTypes.length).to.not.equal(0);
            expect(transitionTypes).to.include.members(basicOBSTransitionTypes);

            transitionTypes.forEach(function(transitionType) {
                if(transitionType == 'fade_to_color_transition' ||
                   transitionType == 'wipe_transition') {
                    // Creating filter
                    const transition = osn.FilterFactory.create(transitionType, 'transition');
        
                    // Checking if filter source was created correctly
                    expect(transition).to.not.equal(undefined);
                    expect(transition.id).to.equal(transitionType);
                    expect(transition.name).to.equal('transition');

                    // Preparing settings object
                    settings = transition.settings;

                    switch(transitionType) {
                        case 'fade_to_color_transition': {
                            settings['color'] = 5278190080;
                            settings['switch_point'] = 80;
                            break;
                        }
                        case 'wipe_transition': {
                            settings['luma_invert'] = true;
                            settings['luma_softness'] = 0.05;
                            break;
                        }
                    }
        
                    // Updating settings of transition
                    transition.update(settings);

                    // Sending save signal to transition
                    expect(function() {
                        transition.save();
                    }).to.not.throw;

                    // Checking if setting was added to transition
                    expect(transition.settings).to.include(settings);
        
                    transition.release();
                }
            });
        });
    });

    context('# SetFlag and GetFlag', () => {
        it('Set flags and get them for all input source types', () => {
            // Getting all input source types
            const inputTypes = osn.InputFactory.types();

            // Checking if inputTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(OBSInputTypes);

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
            expect(inputTypes).to.include.members(OBSInputTypes);

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
