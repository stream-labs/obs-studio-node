import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { IScene, ISceneItem, IInput } from '../osn';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { basicOBSInputTypes, basicDebugOBSInputTypes, deleteConfigFiles } from '../util/general';

function createScene(sceneName: string): IScene {
    // Creating scene
    const scene = osn.SceneFactory.create(sceneName); 

    // Checking if scene was created correctly
    expect(scene).to.not.equal(undefined);
    expect(scene.id).to.equal('scene');
    expect(scene.name).to.equal(sceneName);
    expect(scene.type).to.equal(osn.ESourceType.Scene);

    return scene;
}

function createSource(inputType: string, inputName: string): IInput {
    // Creating source
    const input = osn.InputFactory.create(inputType, inputName);

    // Checking if input source was created correctly
    expect(input).to.not.equal(undefined);
    expect(input.id).to.equal(inputType);
    expect(input.name).to.equal(inputName);

    return input;
}

describe('osn-scene', () => {
    let obs: OBSProcessHandler;
    let OBSInputTypes: string[];
    
    // Initialize OBS process
    before(function() {
        deleteConfigFiles();
        obs = new OBSProcessHandler();
        
        if (obs.startup() !== osn.EVideoCodes.Success)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }

        if (process.env.RELEASE_NAME == "debug") {
            OBSInputTypes = basicDebugOBSInputTypes;
        } else {
            OBSInputTypes = basicOBSInputTypes;
        }
    });

    // Shutdown OBS process
    after(function() {
        obs.shutdown();
        obs = null;
        deleteConfigFiles();
    });

    context('# Create', () => {
        it('Create scene', () => {
            // Creating scene
            const scene = createScene('create_test');
            scene.release();
        });
    });

    context('# Duplicate', () => {
        it('Duplicate scene', () => {
            // Creating scene
            const scene = createScene('duplicate_test');

            // Duplicating scene
            const duplicatedScene = scene.duplicate('duplicated_scene', osn.ESceneDupType.Copy);

            // Checking if scene was duplicated correctly
            expect(duplicatedScene).to.not.equal(undefined);
            expect(duplicatedScene.id).to.equal('scene');
            expect(duplicatedScene.name).to.equal('duplicated_scene');
            expect(duplicatedScene.type).to.equal(osn.ESourceType.Scene);

            scene.release();
            duplicatedScene.release();
        });
    });

    context('# FromName', () => {
        it('Get scene from name', () => {
            // Creating scene
            const scene = createScene('fromName_test');

            // Getting scene by its name
            const sceneFromName = osn.SceneFactory.fromName('fromName_test');

            // Checking if scene was returned
            expect(sceneFromName).to.not.equal(undefined);
            expect(sceneFromName.id).to.equal('scene');
            expect(sceneFromName.name).to.equal('fromName_test');
            expect(sceneFromName.type).to.equal(osn.ESourceType.Scene);

            scene.release();
        });

        it('FAIL TEST: Get scene from name that don\'t exist ', () => {
            expect(function() {
                const failSceneFromName = osn.SceneFactory.fromName('does_not_exist');
            }).to.throw();
        });
    });

    context('# AddSource and AsSource', () => {
        it('Add all source types (including scene as source) to scene', () => {
            // Creating scene
            const scene = createScene('addSource_test');

            // Getting all input source types
            const inputTypes = osn.InputFactory.types();

            // Checking if inputTypes array contains the basic obs input types
            expect(inputTypes.length).to.not.equal(0);
            expect(inputTypes).to.include.members(OBSInputTypes);

            inputTypes.forEach(function(inputType) {
                const input = createSource(inputType, 'input');

                // Adding input source to scene
                const sceneItem = scene.add(input);

                // Checking if input source was added to the scene correctly
                expect(sceneItem).to.not.equal(undefined);
                expect(sceneItem.source.id).to.equal(inputType);
                expect(sceneItem.source.name).to.equal('input');
            });

            // Creating source scene
            const sourceScene = createScene('source_scene');

            // Adding a scene as source
            const sourceSceneItem = scene.add(sourceScene.source);

            // Checking if input source was added to the scene correctly
            expect(sourceSceneItem).to.not.equal(undefined);
            expect(sourceSceneItem.source.id).to.equal('scene');
            expect(sourceSceneItem.source.name).to.equal('source_scene');

            scene.getItems().forEach(function(sceneItem) {
                sceneItem.source.release();
                sceneItem.remove();
            });

            scene.release();
        });
    });

    context('# FindItem', () => {
        it('Find scene item by id', () => {
            // Creating scene
            const scene = createScene('findItem_test');

            const input = createSource('image_source', 'input');
            
            // Adding input source to scene
            const sceneItem = scene.add(input);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal('image_source');
            expect(sceneItem.source.name).to.equal('input');

            // Getting scene item by id
            const sceneItemById = scene.findItem(1);

            // Checking if scene item was returned correctly
            expect(sceneItemById).to.not.equal(undefined);
            expect(sceneItemById.source.id).to.equal('image_source');
            expect(sceneItemById.source.name).to.equal('input');

            sceneItemById.source.release();
            sceneItemById.remove();
            scene.release();
        });

        it('FAIL TEST: Try to find scene that don\'t exist', () => {
            const scene = createScene('findItem_fail_test');

            // Getting scene item with id that does not exist
            expect(function () {
                const sceneItem = scene.findItem('does_not_exist');
            }).to.throw();

            scene.release();
        });
    });

    context('# GetItems and MoveItem', () => {
        it('Get all scene items in a scene', () => {
            // Creating scene
            const scene = createScene('addSource_test');

            // Creating new input source
            const firstInput = createSource('image_source', 'getItems_test1');

            // Adding input source to existing scene
            const firstSceneItem = scene.add(firstInput);
           
            // Checking if scene item was returned correctly
            expect(firstSceneItem).to.not.equal(undefined);
            expect(firstSceneItem.source.id).to.equal('image_source');
            expect(firstSceneItem.source.name).to.equal('getItems_test1');

            // Creating new input source
            const secondInput = createSource('image_source', 'getItems_test2');

            // Checking if input source was created correctly
            expect(secondInput).to.not.equal(undefined);
            expect(secondInput.id).to.equal('image_source');
            expect(secondInput.name).to.equal('getItems_test2');

            // Adding input source to existing scene
            const secondSceneItem = scene.add(secondInput);
           
            // Checking if scene item was returned correctly
            expect(secondSceneItem).to.not.equal(undefined);
            expect(secondSceneItem.source.id).to.equal('image_source');
            expect(secondSceneItem.source.name).to.equal('getItems_test2');

            // Getting all scene items
            const sceneItems = scene.getItems();

            // Checking if sceneItems array has the correct scenes
            expect(sceneItems.length).to.equal(2);
            expect(sceneItems[0].source.name).to.equal('getItems_test1');
            expect(sceneItems[1].source.name).to.equal('getItems_test2');

            // Moving scene item
            scene.moveItem(1, 0);

            // Getting all scene items
            const movedSceneItems = scene.getItems();
            
            //Checking if scene items were moved
            expect(movedSceneItems.length).to.equal(2);
            expect(movedSceneItems[0].source.name).to.equal('getItems_test2');
            expect(movedSceneItems[1].source.name).to.equal('getItems_test1');

            firstSceneItem.source.release();
            firstSceneItem.remove();
            secondSceneItem.source.release();
            secondSceneItem.remove();
            scene.release();
        });

        it('FAIL TEST: Try to access out of bounds', () => {
            // Creating scene
            const scene = createScene('addSource_test');
            
            expect(function() {
                scene.moveItem(3, 0);
            }).to.throw();

            scene.release();
        });
    });
});
