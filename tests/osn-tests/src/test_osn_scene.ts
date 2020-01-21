import 'mocha'
import { expect } from 'chai'
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { EOBSInputTypes } from '../util/obs_enums';
import { ETestErrorMsg, GetErrorMessage } from '../util/error_messages';

const testName = 'osn-scene';

describe(testName, () => {
    let obs: OBSHandler;
    let hasTestFailed: boolean = false;
    
    // Initialize OBS process
    before(function() {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
    });

    // Shutdown OBS process
    after(async function() {
        obs.shutdown();

        if (hasTestFailed === true) {
            logInfo(testName, 'One or more test cases failed. Uploading cache');
            await obs.uploadTestCache();
        }

        obs = null;
        deleteConfigFiles();
        logInfo(testName, 'Finished ' + testName + ' tests');
        logEmptyLine();
    });

    afterEach(function() {
        if (this.currentTest.state == 'failed') {
            hasTestFailed = true;
        }
    });

    it('Create scene', () => {
        const sceneName = 'create_test_scene';

        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));

        scene.release();
    });

    it('Duplicate scene', () => {
        const sceneName = 'duplicate_test_scene';

        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));

        // Duplicating scene
        const duplicatedScene = scene.duplicate(sceneName, osn.ESceneDupType.Copy);

        // Checking if scene was duplicated correctly
        expect(duplicatedScene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.DuplicateScene, sceneName));
        expect(duplicatedScene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.DuplicateSceneId, sceneName));
        expect(duplicatedScene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.DuplicateSceneName, sceneName));
        expect(duplicatedScene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.DuplicateSceneType, sceneName));

        scene.release();
        duplicatedScene.release();
    });

    it('Get scene from name', () => {
        const sceneName = 'fromName_test_scene'

        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));

        // Getting scene by its name
        const sceneFromName = osn.SceneFactory.fromName(sceneName);

        // Checking if scene was returned
        expect(sceneFromName).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.SceneFromName, sceneName));
        expect(sceneFromName.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneFromNameId, sceneName));
        expect(sceneFromName.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneFromNameName, sceneName));
        expect(sceneFromName.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneFromNameType, sceneName));

        scene.release();
    });

    it('Add all source types (including scene as source) to scene', () => {
        const sceneName = 'addSource_test_scene';

        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));

        obs.inputTypes.forEach(function(inputType) {
            // Creating source
            const input = osn.InputFactory.create(inputType, 'input');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, inputType));
            expect(input.id).to.equal(inputType, GetErrorMessage(ETestErrorMsg.InputId, inputType));
            expect(input.name).to.equal('input', GetErrorMessage(ETestErrorMsg.InputName, inputType));

            // Adding input source to scene
            const sceneItem = scene.add(input);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.AddSourceToScene, inputType, sceneName));
            expect(sceneItem.source.id).to.equal(inputType, GetErrorMessage(ETestErrorMsg.SceneItemInputId, inputType));
            expect(sceneItem.source.name).to.equal('input', GetErrorMessage(ETestErrorMsg.SceneItemInputName, inputType));
        });

        // Creating source scene
        const sourceSceneName = 'source_scene';
        const sourceScene = osn.SceneFactory.create(sourceSceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sourceSceneName));

        // Adding a scene as source
        const sourceSceneItem = scene.add(sourceScene.source);

        // Checking if input source was added to the scene correctly
        expect(sourceSceneItem).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.AddSourceToScene, sourceScene.id, sourceSceneName));
        expect(sourceSceneItem.source.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneItemInputId, sourceScene.id));
        expect(sourceSceneItem.source.name).to.equal(sourceSceneName, GetErrorMessage(ETestErrorMsg.SceneItemInputName, sourceScene.id));

        scene.getItems().forEach(function(sceneItem) {
            sceneItem.source.release();
            sceneItem.remove();
        });

        scene.release();
    });

    it('Find scene item by id', () => {
        const sceneName = 'findItem_test_scene';

        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));

        // Creating source
        const input = osn.InputFactory.create(EOBSInputTypes.ImageSource, 'input');

        // Checking if input source was created correctly
        expect(input).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, EOBSInputTypes.ImageSource));
        expect(input.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.InputId, EOBSInputTypes.ImageSource));
        expect(input.name).to.equal('input', GetErrorMessage(ETestErrorMsg.InputName, EOBSInputTypes.ImageSource));
        
        // Adding input source to scene
        const sceneItem = scene.add(input);

        // Checking if input source was added to the scene correctly
        expect(sceneItem).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.AddSourceToScene, input.id, sceneName));
        expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.SceneItemInputId, input.id));
        expect(sceneItem.source.name).to.equal('input', GetErrorMessage(ETestErrorMsg.SceneItemInputName, input.id));

        // Getting scene item by id
        const sceneItemById = scene.findItem(1);

        // Checking if scene item was returned correctly
        expect(sceneItemById).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.SceneItemById, EOBSInputTypes.ImageSource));
        expect(sceneItemById.source.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.SceneItemInputId, EOBSInputTypes.ImageSource));
        expect(sceneItemById.source.name).to.equal('input', GetErrorMessage(ETestErrorMsg.SceneItemInputName, EOBSInputTypes.ImageSource));

        sceneItemById.source.release();
        sceneItemById.remove();
        scene.release();
    });

    it('Get all scene items in a scene', () => {
        const sceneName = 'getItems_test';
        const firstInputName = 'getItems_input1';
        const secondInputName = 'getItems_input2';

        // Creating scene
        const scene = osn.SceneFactory.create('getItems_test'); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));
        
        // Creating first input source
        const firstInput = osn.InputFactory.create(EOBSInputTypes.ImageSource, firstInputName);

        // Checking if input source was created correctly
        expect(firstInput).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, EOBSInputTypes.ImageSource));
        expect(firstInput.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.InputId, EOBSInputTypes.ImageSource));
        expect(firstInput.name).to.equal(firstInputName, GetErrorMessage(ETestErrorMsg.InputName, EOBSInputTypes.ImageSource));

        // Adding input source to existing scene
        const firstSceneItem = scene.add(firstInput);
        
        // Checking if scene item was returned correctly
        expect(firstSceneItem).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.AddSourceToScene, firstInput.id, sceneName));
        expect(firstSceneItem.source.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.SceneItemInputId, firstInput.id));
        expect(firstSceneItem.source.name).to.equal(firstInputName, GetErrorMessage(ETestErrorMsg.SceneItemInputName, firstInput.id));

        // Creating second input source
        const secondInput = osn.InputFactory.create(EOBSInputTypes.ImageSource, secondInputName);

        // Checking if input source was created correctly
        expect(secondInput).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateInput, EOBSInputTypes.ImageSource));
        expect(secondInput.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.InputId, EOBSInputTypes.ImageSource));
        expect(secondInput.name).to.equal(secondInputName, GetErrorMessage(ETestErrorMsg.InputName, EOBSInputTypes.ImageSource));

        // Adding input source to existing scene
        const secondSceneItem = scene.add(secondInput);
        
        // Checking if scene item was returned correctly
        expect(secondSceneItem).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.AddSourceToScene, secondInput.id, sceneName));
        expect(secondSceneItem.source.id).to.equal(EOBSInputTypes.ImageSource, GetErrorMessage(ETestErrorMsg.SceneItemInputId, secondInput.id));
        expect(secondSceneItem.source.name).to.equal(secondInputName, GetErrorMessage(ETestErrorMsg.SceneItemInputName, secondInput.id));

        // Getting all scene items
        const sceneItems = scene.getItems();

        // Checking if sceneItems array has the correct scenes
        expect(sceneItems.length).to.equal(2, GetErrorMessage(ETestErrorMsg.GetSceneItems, sceneName));
        expect(sceneItems[0].source.name).to.equal(firstInputName, ETestErrorMsg.SceneItemPosition);
        expect(sceneItems[1].source.name).to.equal(secondInputName, ETestErrorMsg.SceneItemPosition);

        // Moving scene item
        scene.moveItem(1, 0);

        // Getting all scene items
        const movedSceneItems = scene.getItems();
        
        //Checking if scene items were moved
        expect(movedSceneItems.length).to.equal(2, ETestErrorMsg.GetSceneItems);
        expect(movedSceneItems[0].source.name).to.equal(secondInputName, ETestErrorMsg.SceneItemPositionAfterMove);
        expect(movedSceneItems[1].source.name).to.equal(firstInputName, ETestErrorMsg.SceneItemPositionAfterMove);

        firstSceneItem.source.release();
        firstSceneItem.remove();
        secondSceneItem.source.release();
        secondSceneItem.remove();
        scene.release();
    });

    it('Fail test - Get scene from name that don\'t exist ', () => {
        expect(function() {
            const failSceneFromName = osn.SceneFactory.fromName('does_not_exist');
        }).to.throw();
    });

    it('Fail test - Try to find scene that don\'t exist', () => {
        const sceneName = 'findItem_fail_test';

        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));

        // Getting scene item with id that does not exist
        expect(function () {
            const sceneItem = scene.findItem('does_not_exist');
        }).to.throw();

        scene.release();
    });

    it('Fail test - Try to access out of bounds', () => {
        const sceneName = 'outOfBounds_test_scene';

        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined, GetErrorMessage(ETestErrorMsg.CreateScene, sceneName));
        expect(scene.id).to.equal('scene', GetErrorMessage(ETestErrorMsg.SceneId, sceneName));
        expect(scene.name).to.equal(sceneName, GetErrorMessage(ETestErrorMsg.SceneName, sceneName));
        expect(scene.type).to.equal(osn.ESourceType.Scene, GetErrorMessage(ETestErrorMsg.SceneType, sceneName));
        
        expect(function() {
            scene.moveItem(3, 0);
        }).to.throw();

        scene.release();
    });
});
