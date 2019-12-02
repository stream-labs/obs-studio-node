import 'mocha';
import * as osn from '../osn';
import { expect } from 'chai';
import { logInfo, logEmptyLine } from '../util/logger';
import { ISettings } from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { EOBSInputTypes, EOBSFilterTypes, EOBSTransitionTypes } from '../util/obs_enums'
import { deleteConfigFiles } from '../util/general';

const testName = 'osn-source';

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

    it('Get all osn-source info from all input types', () => {
        obs.inputTypes.forEach(function(inputType) {
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
        obs.filterTypes.forEach(function(filterType) {
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
        obs.transitionTypes.forEach(function(transitionType) {
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

    it('Update settings of all inputs', () => {
        let settings: ISettings = {};

        obs.inputTypes.forEach(function(inputType) {
            // Creating input source
            const input = osn.InputFactory.create(inputType, 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input');

            // Preparing settings object
            settings = input.settings;
            
            switch(inputType) {
                case EOBSInputTypes.ImageSource: {
                    settings['unload'] = true;
                    break;
                }
                case EOBSInputTypes.ColorSource: {
                    settings['height'] = 500;
                    settings['width'] = 600;
                    break;
                }
                case EOBSInputTypes.Slideshow: {
                    settings['loop'] = false;
                    settings['slide_time'] = 9000;
                    settings['transition_speed'] = 800;
                    break;
                }
                case EOBSInputTypes.BrowserSource: {
                    settings['fps'] = 60;
                    settings['height'] = 500;
                    settings['restart_when_active'] = true;
                    settings['shutdown'] = true;
                    break;
                }
                case EOBSInputTypes.FFMPEGSource: {
                    settings['buffering_mb'] = 3;
                    settings['clear_on_media_end'] = false;
                    settings['looping'] = true;
                    settings['speed_percent'] = 80;
                    settings['caching'] = true;
                    break;
                }
                case EOBSInputTypes.TextGDI: {
                    settings['align'] = 'right';
                    settings['extents_cx'] = 90;
                    settings['extents_cy'] = 90;
                    settings['valign'] = 'bottom';
                    break;
                }
                case EOBSInputTypes.TextFT2: {
                    settings['color1'] = 4294967296;
                    settings['color2'] = 5294967295;
                    break;
                }
                case EOBSInputTypes.MonitorCapture: {
                    settings['capture_cursor'] = false;
                    settings['monitor'] = 1;
                    break;
                }
                case EOBSInputTypes.WindowCapture: {
                    settings['compatibility'] = true;
                    settings['cursor'] = false;
                    break;
                }
                case EOBSInputTypes.GameCapture: {
                    settings['allow_transparency'] = true;
                    settings['force_scaling'] = true;
                    settings['hook_rate'] = 2;
                    break;
                }
                case EOBSInputTypes.DShowInput: {
                    settings['audio_output_mode'] = 1;
                    settings['res_type'] = 1;
                    settings['video_format'] = 2;
                    break;
                }
                case EOBSInputTypes.WASAPIInput: {
                    settings['use_device_timing'] = true;
                    break;
                }
                case EOBSInputTypes.WASAPIOutput: {
                    settings['use_device_timing'] = false;
                    break;
                }
            }

            // Updating settings of source
            input.update(settings);

            // Sending save signal to source
            expect(function() {
                input.save();
            }).to.not.throw();

            // Checking if setting was added to source
            expect(input.settings).to.eql(settings);

            settings = {};
            input.release();
        });
    });

    it('Update settings of all filters', () => {
        let settings: ISettings = {};

        obs.filterTypes.forEach(function(filterType) {
            if (filterType != EOBSFilterTypes.GPUDelay &&
                filterType != EOBSFilterTypes.AsyncDelay &&
                filterType != EOBSFilterTypes.InvertPolarity &&
                filterType != EOBSFilterTypes.PremultipliedAlpha &&
                filterType != EOBSFilterTypes.VST) {
                // Creating filter
                const filter = osn.FilterFactory.create(filterType, 'filter');
    
                // Checking if filter source was created correctly
                expect(filter).to.not.equal(undefined);
                expect(filter.id).to.equal(filterType);
                expect(filter.name).to.equal('filter');

                // Preparing settings object
                settings = filter.settings;

                switch(filterType) {
                    case EOBSFilterTypes.Mask: {
                        settings['color'] = 26777215;
                        settings['opacity'] = 80;
                        break;
                    }
                    case EOBSFilterTypes.Crop: {
                        settings['relative'] = false;
                        break;
                    }
                    case EOBSFilterTypes.Gain: {
                        settings['db'] = 5;
                        break;
                    }
                    case EOBSFilterTypes.Color: {
                        settings['brightness'] = 10;
                        settings['contrast'] = 15;
                        settings['saturation'] = 55;
                        break;
                    }
                    case EOBSFilterTypes.Scale: {
                        settings['sampling'] = 'bilinear';
                        settings['undistort'] = true;
                        break;
                    }
                    case EOBSFilterTypes.Scroll: {
                        settings['cx'] = 80;
                        settings['cy'] = 60;
                        settings['limit_size'] = true;
                        break;
                    }
                    case EOBSFilterTypes.ColorKey: {
                        settings['gamma'] = 60;
                        settings['key_color_type'] = 'red';
                        settings['similarity'] = 55;
                        break;
                    }
                    case EOBSFilterTypes.Clut: {
                        settings['clut_amount'] = 2;
                        break;
                    }
                    case EOBSFilterTypes.Sharpness: {
                        settings['sharpness'] = 0.05;
                        break;
                    }
                    case EOBSFilterTypes.ChromaKey: {
                        settings['key_color'] = 68280;
                        settings['smoothness'] = 20;
                        settings['spill'] = 90;
                        break;
                    }
                    case EOBSFilterTypes.NoiseSuppress: {
                        settings['suppress_level'] = -10;
                        settings['']
                        break;
                    }
                    case EOBSFilterTypes.NoiseGate: {
                        settings['hold_time'] = 120;
                        settings['open_threshold'] = -12;
                        settings['release_time'] = 80;
                        break;
                    }
                    case EOBSFilterTypes.Compressor: {
                        settings['attack_time'] = 8;
                        settings['output_gain'] = 4;
                        settings['threshold'] = -9;
                        break;
                    }
                    case EOBSFilterTypes.Limiter: {
                        settings['release_time'] = 30;
                        settings['threshold'] = -12;
                        break;
                    }
                    case EOBSFilterTypes.Expander: {
                        settings['attack_time'] = 15;
                        settings['ratio'] = 5;
                        settings['threshold'] = -20;
                        break;
                    }
                    case EOBSFilterTypes.NDI:
                    case EOBSFilterTypes.NDIAudio: {
                        settings['ndi_filter_name'] = 'Test Filter';
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

        obs.transitionTypes.forEach(function(transitionType) {
            if(transitionType == EOBSTransitionTypes.FadeToColor ||
                transitionType == EOBSTransitionTypes.Wipe) {
                // Creating filter
                const transition = osn.FilterFactory.create(transitionType, 'transition');
    
                // Checking if filter source was created correctly
                expect(transition).to.not.equal(undefined);
                expect(transition.id).to.equal(transitionType);
                expect(transition.name).to.equal('transition');

                // Preparing settings object
                settings = transition.settings;

                switch(transitionType) {
                    case EOBSTransitionTypes.FadeToColor: {
                        settings['color'] = 5278190080;
                        settings['switch_point'] = 80;
                        break;
                    }
                    case EOBSTransitionTypes.Wipe: {
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
                }).to.not.throw();

                // Checking if setting was added to transition
                expect(transition.settings).to.include(settings);
    
                transition.release();
            }
        });
    });

    it('Set flags and get them for all input source types', () => {
        obs.inputTypes.forEach(function(inputType) {
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

    it('Set muted and get it for all input source types', () => {
        obs.inputTypes.forEach(function(inputType) {
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

    it('Set enabled and get it for all filter types', () => {
        obs.filterTypes.forEach(function(filterType) {
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
