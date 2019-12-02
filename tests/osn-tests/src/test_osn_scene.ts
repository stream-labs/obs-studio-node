import 'mocha'
import * as osn from '../osn';
import { expect } from 'chai'
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums';

const testName = 'osn-scene';

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

    it('Create scene', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('create_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('create_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);

        scene.release();
    });

    it('Duplicate scene', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('duplicate_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('duplicate_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);

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

    it('Get scene from name', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('fromName_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('fromName_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);

        // Getting scene by its name
        const sceneFromName = osn.SceneFactory.fromName('fromName_test');

        // Checking if scene was returned
        expect(sceneFromName).to.not.equal(undefined);
        expect(sceneFromName.id).to.equal('scene');
        expect(sceneFromName.name).to.equal('fromName_test');
        expect(sceneFromName.type).to.equal(osn.ESourceType.Scene);

        scene.release();
    });

    it('Fail test - Get scene from name that don\'t exist ', () => {
        expect(function() {
            const failSceneFromName = osn.SceneFactory.fromName('does_not_exist');
        }).to.throw();
    });

    it('Add all source types (including scene as source) to scene', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('addSource_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('addSource_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);

        obs.inputTypes.forEach(function(inputType) {
            // Creating source
            const input = osn.InputFactory.create(inputType, 'input');

            // Checking if input source was created correctly
            expect(input.id).to.equal(inputType);
            expect(input.name).to.equal('input');

            // Adding input source to scene
            const sceneItem = scene.add(input);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(inputType);
            expect(sceneItem.source.name).to.equal('input');
        });

        // Creating source scene
        const sourceScene = osn.SceneFactory.create('source_scene'); 

        // Checking if scene was created correctly
        expect(sourceScene.id).to.equal('scene');
        expect(sourceScene.name).to.equal('source_scene');
        expect(sourceScene.type).to.equal(osn.ESourceType.Scene);

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

    it('Find scene item by id', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('findItem_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('findItem_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);

        // Creating source
        const input = osn.InputFactory.create(EOBSInputTypes.ImageSource, 'input');

        // Checking if input source was created correctly
        expect(input.id).to.equal(EOBSInputTypes.ImageSource);
        expect(input.name).to.equal('input');
        
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

    it('Fail test - Try to find scene that don\'t exist', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('findItem_fail_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('findItem_fail_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);

        // Getting scene item with id that does not exist
        expect(function () {
            const sceneItem = scene.findItem('does_not_exist');
        }).to.throw();

        scene.release();
    });

    it('Get all scene items in a scene', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('getItems_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('getItems_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);
        
        // Creating first input source
        const firstInput = osn.InputFactory.create(EOBSInputTypes.ImageSource, 'getItems_input1');

        // Checking if input source was created correctly
        expect(firstInput.id).to.equal(EOBSInputTypes.ImageSource);
        expect(firstInput.name).to.equal('getItems_input1');

        // Adding input source to existing scene
        const firstSceneItem = scene.add(firstInput);
        
        // Checking if scene item was returned correctly
        expect(firstSceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
        expect(firstSceneItem.source.name).to.equal('getItems_input1');

        // Creating second input source
        const secondInput = osn.InputFactory.create(EOBSInputTypes.ImageSource, 'getItems_input2');

        // Checking if input source was created correctly
        expect(secondInput.id).to.equal(EOBSInputTypes.ImageSource);
        expect(secondInput.name).to.equal('getItems_input2');

        // Adding input source to existing scene
        const secondSceneItem = scene.add(secondInput);
        
        // Checking if scene item was returned correctly
        expect(secondSceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
        expect(secondSceneItem.source.name).to.equal('getItems_input2');

        // Getting all scene items
        const sceneItems = scene.getItems();

        // Checking if sceneItems array has the correct scenes
        expect(sceneItems.length).to.equal(2);
        expect(sceneItems[0].source.name).to.equal('getItems_input1');
        expect(sceneItems[1].source.name).to.equal('getItems_input2');

        // Moving scene item
        scene.moveItem(1, 0);

        // Getting all scene items
        const movedSceneItems = scene.getItems();
        
        //Checking if scene items were moved
        expect(movedSceneItems.length).to.equal(2);
        expect(movedSceneItems[0].source.name).to.equal('getItems_input2');
        expect(movedSceneItems[1].source.name).to.equal('getItems_input1');

        firstSceneItem.source.release();
        firstSceneItem.remove();
        secondSceneItem.source.release();
        secondSceneItem.remove();
        scene.release();
    });

    it('Fail test - Try to access out of bounds', () => {
        // Creating scene
        const scene = osn.SceneFactory.create('outOfBounds_test'); 

        // Checking if scene was created correctly
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal('outOfBounds_test');
        expect(scene.type).to.equal(osn.ESourceType.Scene);
        
        expect(function() {
            scene.moveItem(3, 0);
        }).to.throw();

        scene.release();
    });
});
