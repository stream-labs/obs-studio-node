import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { ISettings } from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import * as filterSettings from '../util/filter_settings';

describe('osn-filter', () => {
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

    it('Create all filter types', () => {
        // Create each filter type available
        obs.filterTypes.forEach(function(filterType) {
            const filter = osn.FilterFactory.create(filterType, 'filter');

            // Checking if filter was created correctly
            expect(filter).to.not.equal(undefined);
            expect(filter.id).to.equal(filterType);
            expect(filter.name).to.equal('filter');
            filter.release();
        });
    });

    it('Create all filter types with settings', () => {
        
        // Create each filter type availabe passing settings parameter
        obs.filterTypes.forEach(function(filterType) {
            let settings: ISettings = {};

            switch(filterType) {
                case 'mask_filter': {
                    settings = filterSettings.mask;
                    settings['opacity'] = 80;
                    break; 
                }
                case 'crop_filter': {
                    settings = filterSettings.crop;
                    settings['relative'] = false;
                    break;
                }
                case 'gain_filter': {
                    settings = filterSettings.gain;
                    settings['db'] = 10;
                    break;
                }
                case 'color_filter': {
                    settings = filterSettings.color;
                    settings['brightness'] = 50;
                    break;
                }
                case 'scale_filter': {
                    settings = filterSettings.scale;
                    settings['undistort'] = true;
                    break;
                }
                case 'scroll_filter': {
                    settings = filterSettings.scroll;
                    settings['cx'] = 200;
                    break;
                }
                case 'color_key_filter': {
                    settings = filterSettings.colorKey;
                    settings['smoothness'] = 80;
                    break;
                }
                case 'clut_filter': {
                    settings = filterSettings.clut;
                    settings['clut_amount'] = 2;
                    break;
                }
                case 'sharpness_filter': {
                    settings = filterSettings.sharpness;
                    settings['sharpness'] = 0.15;
                    break;
                }
                case 'chroma_key_filter': {
                    settings = filterSettings.chromaKey;
                    settings['spill'] = 75;
                    break;
                }
                case 'noise_suppress_filter': {
                    settings = filterSettings.noiseSuppress;
                    settings['suppress_level'] = -20;
                    break;
                }
                case 'noise_gate_filter': {
                    settings = filterSettings.noiseGate;
                    settings['hold_time'] = 300;
                    break; 
                }
                case 'compressor_filter': {
                    settings = filterSettings.compressor;
                    settings['ratio'] = 5;
                    break;
                }
                case 'limiter_filter': {
                    settings = filterSettings.limiter;
                    settings['threshold'] = -5;
                    break;
                }
                case 'expander_filter': {
                    settings = filterSettings.expander;
                    settings['attack_time'] = 20;
                    break;
                }
                case 'luma_key_filter': {
                    settings = filterSettings.lumaKey;
                    settings['luma_max'] = 2;
                    break;
                }
                case 'ndi_filter': 
                case 'ndi_audiofilter': {
                    settings = filterSettings.ndi;
                    settings['ndi_filter_ndiname'] = 'Test Output';
                }
            }

            const filter = osn.FilterFactory.create(filterType, 'filter', settings);

            // Checking if filter was created correctly
            expect(filter).to.not.equal(undefined);
            expect(filter.id).to.equal(filterType);
            expect(filter.name).to.equal('filter');
            expect(filter.settings).to.eql(settings);
            filter.release();
        });
    });
});
