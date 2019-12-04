import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler, IPerformanceState, TOBSHotkey } from '../util/obs_handler';
import { showHideInputHotkeys, slideshowHotkeys, ffmpeg_sourceHotkeys,
    game_captureHotkeys, dshow_wasapitHotkeys, deleteConfigFiles } from '../util/general';

const testName = 'nodeobs_api';

describe(testName, function() {
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

    context('# OBS_API_getPerformanceStatistics', function() {
        it('Fill stats object', function() {
            let stats: IPerformanceState;

            // Getting performance statistics
            stats = osn.NodeObs.OBS_API_getPerformanceStatistics();
            
            // Checking if performance statistics values returned correctly
            expect(stats.CPU).to.not.equal(undefined);
            expect(stats.bandwidth).to.not.equal(undefined);
            expect(stats.frameRate).to.not.equal(undefined);
            expect(stats.numberDroppedFrames).to.not.equal(undefined);
            expect(stats.percentageDroppedFrames).to.not.equal(undefined);
        });
    });

    context('# OBS_API_QueryHotkeys and OBS_API_ProcessHotkeyStatus', function() {
        it('Get hotkeys of sources that have them and process them all', function() {
            let obsHotkeys: TOBSHotkey[];

            // Creating scene
            const scene = osn.SceneFactory.create('scene'); 

            // Checking if scene was created correctly
            expect(scene).to.not.equal(undefined);
            expect(scene.id).to.equal('scene');
            expect(scene.name).to.equal('scene');
            expect(scene.type).to.equal(osn.ESourceType.Scene);

            obs.inputTypes.forEach(inputType => {
                // Creating source
                const input = osn.InputFactory.create(inputType, inputType);

                // Checking if input source was created correctly
                expect(input).to.not.equal(undefined);
                expect(input.id).to.equal(inputType);
                expect(input.name).to.equal(inputType);

                // Adding input source to scene
                const sceneItem = scene.add(input);

                // Checking if input source was added to the scene correctly
                expect(sceneItem).to.not.equal(undefined);
                expect(sceneItem.source.id).to.equal(inputType);
                expect(sceneItem.source.name).to.equal(inputType);
            });

            // Getting all hotkeys
            obsHotkeys = osn.NodeObs.OBS_API_QueryHotkeys();

            // Check if hotkeys exists and process them
            obsHotkeys.forEach(function(hotkey) {
                switch(hotkey.ObjectName) {
                    case 'scene': {
                        expect(hotkey.HotkeyName).to.be.oneOf(showHideInputHotkeys);
                        break;
                    }
                    case 'slideshow': {
                        expect(hotkey.HotkeyName).to.be.oneOf(slideshowHotkeys);
                        break;
                    }
                    case 'ffmpeg_source': {
                        expect(hotkey.HotkeyName).to.be.oneOf(ffmpeg_sourceHotkeys);
                        break;
                    }
                    case 'game_capture': {
                        expect(hotkey.HotkeyName).to.be.oneOf(game_captureHotkeys);
                        break;
                    }
                    case 'dshow_input': {
                        expect(hotkey.HotkeyName).to.be.oneOf(dshow_wasapitHotkeys);
                        break;
                    }
                    case 'wasapi_input_capture': {
                        expect(hotkey.HotkeyName).to.be.oneOf(dshow_wasapitHotkeys);
                        break;
                    }
                    case 'wasapi_output_capture': {
                        expect(hotkey.HotkeyName).to.be.oneOf(dshow_wasapitHotkeys);
                        break;
                    }
                    default: {
                        break;
                    }
                }

                expect(function() {
                    osn.NodeObs.OBS_API_ProcessHotkeyStatus(hotkey.HotkeyId, true);
                }).to.not.throw();
            });

            // Checking if hotkeys returned properly
            expect(obsHotkeys.length).to.not.equal(0);

            scene.getItems().forEach(function(sceneItem) {
                sceneItem.source.release();
                sceneItem.remove();
            });

            scene.release();
        });
    });

    context('# StopCrashHandler', function() {
        it('Stop crash handler', function() {
            // Stopping crash handler as a last test case
            expect(function() {
                osn.NodeObs.StopCrashHandler();
            }).to.not.throw();
        });
    });
});
