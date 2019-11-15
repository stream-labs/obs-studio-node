import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { IInput, ISettings, ITimeSpec } from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { getTimeSpec, deleteConfigFiles} from '../util/general';
import * as inputSettings from '../util/input_settings';

describe('osn-input', () => {
    let obs: OBSHandler;

    // Initialize OBS process
    before(function() {
        deleteConfigFiles();
        obs = new OBSHandler();
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
    });

    it('# Create all types of input', () => {
        // Create all input sources available
        obs.inputTypes.forEach(function(inputType) {
            const input = osn.InputFactory.create(inputType, 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input');
            input.release();
        });
    });

    it('# Create all types of input with settings parameter', () => {
        // Create all input sources available
        obs.inputTypes.forEach(function(inputType) {
            let settings: ISettings = {};

            switch(inputType) {
                case 'image_source': {
                    settings = inputSettings.imageSource;
                    settings['unload'] = true;
                    break;
                }
                case 'color_source': {
                    settings = inputSettings.colorSource;
                    settings['height'] = 500;
                    break;
                }
                case 'slideshow': {
                    settings = inputSettings.slideshow;
                    settings['loop'] = false;
                    break;
                }
                case 'browser_source': {
                    settings = inputSettings.browserSource;
                    settings['restart_when_active'] = true;
                    break;
                }
                case 'ffmpeg_source': {
                    settings = inputSettings.ffmpegSource;
                    settings['speed_percent'] = 80;
                    break;
                }
                case 'ndi_source': {
                    settings = inputSettings.ndiSource;
                    settings['yuv_colorspace'] = 1;
                    break;
                }
                case 'text_gdiplus': {
                    settings = inputSettings.textGDIPlus;
                    settings['align'] = 'right';
                    break;
                }
                case 'text_ft2_source': {
                    settings = inputSettings.textFT2Source;
                    settings['log_lines'] = 5;
                    break;
                }
                case 'vlc_source': {
                    settings = inputSettings.vlcSource;
                    settings['shuffle'] = true;
                    break;
                }
                case 'monitor_capture': {
                    settings = inputSettings.monitorCapture;
                    settings['capture_cursor'] = false;
                    break;
                }
                case 'window_capture': {
                    settings = inputSettings.windowCapture;
                    settings['compatibility'] = true;
                    break;
                }
                case 'game_capture': {
                    settings = inputSettings.gameCapture;
                    settings['allow_transparency'] = true;
                    break;
                }
                case 'dshow_input': {
                    settings = inputSettings.dshowInput;
                    settings['video_format'] = 1;
                    break;
                }
                case 'wasapi_input_capture': 
                case 'wasapi_output_capture': {
                    settings = inputSettings.wasapi;
                    settings['use_device_timing'] = true;
                    break;
                }
            }

            const input = osn.InputFactory.create(inputType, 'input', settings);

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input');
            expect(input.settings).to.eql(settings);
            input.release();
        });
    });

    it('# Create an instance of an input by getting it by name', () => {
        let inputFromName: IInput;

        obs.inputTypes.forEach(function(inputType) {
            // Creating input source
            const input = osn.InputFactory.create(inputType, 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input');

            // Getting input source by its name
            inputFromName = osn.InputFactory.fromName('input');

            // Checking if returned input source is correct
            expect(inputFromName).to.not.equal(undefined);
            expect(inputFromName.id).to.equal(inputType);
            expect(inputFromName.name).to.equal('input');
            input.release();
        });
    });
    
    it('# FAIL TEST: Try to find an input that does not exist', () => {
        let inputFromName: IInput;

        expect(function () {
            inputFromName = osn.InputFactory.fromName('doesNotExist');
        }).to.throw();
    });

    it('# Get volume value from input source', () => {
        let volume: number = undefined;

        // Creating input source
        const input = osn.InputFactory.create('ffmpeg_source', 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined);
        expect(input.id).to.equal('ffmpeg_source');
        expect(input.name).to.equal('input');

        // Getting input source volume
        volume = input.volume;

        // Checking if volume value is what was expected
        expect(volume).to.not.equal(undefined);
        input.release();
    });

    it('# Set sync offset and get it', () => {
        let returnedSyncOffset: ITimeSpec;

        // Creating input source
        const input = osn.InputFactory.create('wasapi_input_capture', 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined);
        expect(input.id).to.equal('wasapi_input_capture');
        expect(input.name).to.equal('input');

        // Setting input sync offset
        input.syncOffset = getTimeSpec(5000);

        // Getting input sync offset
        returnedSyncOffset = input.syncOffset;

        // Checking if sync offset value was returned correctly
        expect(returnedSyncOffset).to.eql(getTimeSpec(5000));
        input.release();
    });

    it('# Set audio mixers value and get it', () => {
        let returnedAudioMixers: number;

        // Creating input source
        const input = osn.InputFactory.create('wasapi_output_capture', 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined);
        expect(input.id).to.equal('wasapi_output_capture');
        expect(input.name).to.equal('input');

        // Setting input audio mixers value
        input.audioMixers = 3;

        // Getting input audio mixers value
        returnedAudioMixers = input.audioMixers;

        // Checking if audio mixers value was returned correctly
        expect(returnedAudioMixers).to.equal(3);
        input.release();
    });

    it('# Set audio mixers value and get it', () => {
        let returnedMonitoringType: osn.EMonitoringType;

        // Creating input source
        const input = osn.InputFactory.create('wasapi_output_capture', 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined);
        expect(input.id).to.equal('wasapi_output_capture');
        expect(input.name).to.equal('input');

        // Setting input audio mixers value
        input.monitoringType = osn.EMonitoringType.MonitoringAndOutput;

        // Getting input audio mixers value
        returnedMonitoringType = input.monitoringType;

        // Checking if audio mixers value was returned correctly
        expect(returnedMonitoringType).to.equal(osn.EMonitoringType.MonitoringAndOutput);
        input.release();
    });

    it('# Add video filter to video sources', () => {
        let videoFilters: string[] = [];
        let addedFilters: string[] = [];

        // Getting all video filter types
        osn.FilterFactory.types().forEach(function(filterType) {
            const video = !!(osn.ESourceOutputFlags.Video & osn.Global.getOutputFlagsFromId(filterType));

            // Filling videoFilters array
            if (video) {
                videoFilters.push(filterType);
            }
        });

        // Creating all video sources available
        obs.inputTypes.forEach(function(inputType) {
            const videoSource = !!(osn.ESourceOutputFlags.Video & osn.Global.getOutputFlagsFromId(inputType));

            if (videoSource) {
                // Creating video source
                const input = osn.InputFactory.create(inputType, inputType);

                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal(inputType);

                videoFilters.forEach(function(filterType) {
                    if (filterType === 'ndi_filter') {
                        if (inputType === 'ndi_source') {
                            const filter = osn.FilterFactory.create(filterType, filterType);

                            // Checking if filter was created correctly
                            expect(filter).to.not.equal(undefined);

                            // Adding ndi filter to ndi source
                            input.addFilter(filter);

                            // Adding filter to addedFilters array
                            addedFilters.push(filter.id);
                        }
                    } else if (filterType === 'async_delay_filter') {
                        if (inputType === 'ffmpeg_source' ||
                            inputType === 'dshow_input') {
                            const filter = osn.FilterFactory.create(filterType, filterType);

                            // Checking if filter was created correctly
                            expect(filter).to.not.equal(undefined);

                            // Adding ndi filter to ndi source
                            input.addFilter(filter);

                            // Adding filter to addedFilters array
                            addedFilters.push(filter.id);
                        }
                    } else {
                        const filter = osn.FilterFactory.create(filterType, filterType);

                        // Checking if filter was created correctly
                        expect(filter).to.not.equal(undefined);

                        // Adding filter to source
                        input.addFilter(filter);
                    
                        // Adding filter to addedFilters array
                        addedFilters.push(filter.id);
                    }
                });

                // Finding each filter and removing all of them
                input.filters.forEach(function(filter) {
                    const foundFilter = input.findFilter(filter.name);

                    // Checking if source has all video filters
                    expect(addedFilters).to.include(foundFilter.id);

                    input.removeFilter(foundFilter);
                    filter.release();
                });

                // Checking if all filters where removed
                expect(input.filters.length).to.equal(0);

                input.release();
            }
        });
    });

    it('# Add async filters to async sources', () => {
        let asyncFilters: string[] = [];
        let addedFilters: osn.ESourceType[] = [];

        // Getting all filter types and separating them between video, audio and async
        osn.FilterFactory.types().forEach(function(filterType) {
            const async = !!(osn.ESourceOutputFlags.Async & osn.Global.getOutputFlagsFromId(filterType));

            // Filling asyncFilters array
            if (async) {
                asyncFilters.push(filterType);
            }
        });

        // Create all async sources available
        obs.inputTypes.forEach(function(inputType) {
            const asyncSource = !!(osn.ESourceOutputFlags.Async & osn.Global.getOutputFlagsFromId(inputType));

            if (asyncSource) {
                // Creating async source
                const input = osn.InputFactory.create(inputType, inputType);

                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal(inputType);

                asyncFilters.forEach(function(filterType) {
                    const filter = osn.FilterFactory.create(filterType, filterType);

                    // Checking if filter was created correctly
                    expect(filter).to.not.equal(undefined);
                    
                    // Adding filter to source
                    input.addFilter(filter);
                    
                    // Adding filter to addedFilters array
                    addedFilters.push(filter.type);
                });

                // Finding each filter and removing all of them
                input.filters.forEach(function(filter) {
                    const foundFilter = input.findFilter(filter.name);

                    // Checking if source has all video filters
                    expect(addedFilters).to.include(foundFilter.type);

                    input.removeFilter(filter);
                    filter.release();
                });

                // Checking if all filters where removed
                expect(input.filters.length).to.equal(0);

                input.release();
            }
        });
    });

    it('# Add audio filters to audio sources', () => {
        let audioFilters: string[] = [];
        let addedFilters: osn.ESourceType[] = [];

        // Getting all audio filter types
        osn.FilterFactory.types().forEach(function(filterType) {
            const audio = !!(osn.ESourceOutputFlags.Audio & osn.Global.getOutputFlagsFromId(filterType));

            // Filling audioFilters array
            if (audio) {
                audioFilters.push(filterType);
            }
        });

        // Creating all audio sources available
        obs.inputTypes.forEach(function(inputType) {
            const audioSource = !!(osn.ESourceOutputFlags.Audio & osn.Global.getOutputFlagsFromId(inputType));

            if (audioSource) {
                // Creating audio source
                const input = osn.InputFactory.create(inputType, 'input');

                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal('input');

                audioFilters.forEach(function(filterType) {
                    const filter = osn.FilterFactory.create(filterType, filterType);

                    // Checking if filter was created correctly
                    expect(filter).to.not.equal(undefined);
                    
                    // Adding filter to source
                    input.addFilter(filter);
                    
                    // Adding filter to addedFilters index
                    addedFilters.push(filter.type);
                });

                // Finding each filter and removing all of them
                input.filters.forEach(function(filter) {
                    const foundFilter = input.findFilter(filter.name);

                    // Checking if source has all video filters
                    expect(addedFilters).to.include(foundFilter.type);

                    input.removeFilter(foundFilter);
                    filter.release();
                });

                // Checking if all filters where removed
                expect(input.filters.length).to.equal(0);
                
                input.release();
            }
        });
    });

    it('# Change the order of filters in the list', () => {
        // Creating source
        const input = osn.InputFactory.create('game_capture', 'test_source');
        
        // Checking if source was created correctly
        expect(input).to.not.equal(undefined);
        expect(input.id).to.equal('game_capture');
        expect(input.name).to.equal('test_source');

        // Creating filters
        const filter1 = osn.FilterFactory.create('color_filter', 'filter1');
        const filter2 = osn.FilterFactory.create('crop_filter', 'filter2');
        const filter3 = osn.FilterFactory.create('gpu_delay', 'filter3');

        // Checking if filters were created correctly
        expect(filter1).to.not.equal(undefined);
        expect(filter2).to.not.equal(undefined);
        expect(filter3).to.not.equal(undefined);
                    
        // Adding filters to source
        input.addFilter(filter1);
        input.addFilter(filter2);
        input.addFilter(filter3);

        // Changing filter order down
        input.setFilterOrder(filter1, osn.EOrderMovement.Down);

        // Checking if filter is in the right position
        expect(input.filters[1].name).to.equal('filter1');

        // Changing filter order up
        input.setFilterOrder(filter3, osn.EOrderMovement.Up);

        // Checking if filter is in the right position
        expect(input.filters[1].name).to.equal('filter3');

        // Changing filter order to bottom
        input.setFilterOrder(filter2, osn.EOrderMovement.Bottom);

        // Checking if filter is in the right position
        expect(input.filters[2].name).to.equal('filter2');

        // Changing filter order to top
        input.setFilterOrder(filter2, osn.EOrderMovement.Top);

        // Checking if filter is in the right position
        expect(input.filters[0].name).to.equal('filter2');

        // Finding each filter and removing all of them
        input.filters.forEach(function(filter) {
            const foundFilter = input.findFilter(filter.name);
            input.removeFilter(foundFilter);
            filter.release();
        });

        input.release();
    });
});
