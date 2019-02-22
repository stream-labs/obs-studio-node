import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSInputTypes, showHideInputHotkeys, slideshowHotkeys, ffmpeg_sourceHotkeys, game_captureHotkeys, dshow_wasapitHotkeys } from '../util/general';

interface IPerformanceState {
    CPU: number;
    numberDroppedFrames: number;
    percentageDroppedFrames: number;
    bandwidth: number;
    frameRate: number;
}

type OBSHotkey = {
    ObjectName: string;
    ObjectType: osn.EHotkeyObjectType;
    HotkeyName: string;
    HotkeyDesc: string;
    HotkeyId: number;
};

function createScene(sceneName: string): osn.IScene {
    // Creating scene
    const scene = osn.SceneFactory.create(sceneName); 

    // Checking if scene was created correctly
    expect(scene).to.not.equal(undefined);
    expect(scene.id).to.equal('scene');
    expect(scene.name).to.equal(sceneName);
    expect(scene.type).to.equal(osn.ESourceType.Scene);

    return scene;
}

function createSource(inputType: string, inputName: string): osn.IInput {
    // Creating source
    const input = osn.InputFactory.create(inputType, inputName);

    // Checking if input source was created correctly
    expect(input).to.not.equal(undefined);
    expect(input.id).to.equal(inputType);
    expect(input.name).to.equal(inputName);

    return input;
}

describe('nodeobs_api', function() {
    let obs: OBSProcessHandler;
    
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    after(function() {
        obs.shutdown();
        obs = null;
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
            let obsHotkeys: OBSHotkey[];

            // Creating scene
            const scene = createScene('scene');

            basicOBSInputTypes.forEach(function(inputType) {
                // Creating source
                const input = createSource(inputType, inputType);

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
