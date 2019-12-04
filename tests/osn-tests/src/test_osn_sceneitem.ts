import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { logInfo, logEmptyLine } from '../util/logger';
import { OBSHandler, IVec2, ICrop } from '../util/obs_handler';
import { EOBSInputTypes } from '../util/obs_enums'
import { deleteConfigFiles } from '../util/general';

const testName = 'osn-sceneitem';

describe(testName, () => {
    let obs: OBSHandler;
    let sceneName: string = 'test_scene';
    let sourceName: string = 'test_source';

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

    beforeEach(function() {
        // Creating scene
        const scene = osn.SceneFactory.create(sceneName); 

        // Checking if scene was created correctly
        expect(scene).to.not.equal(undefined);
        expect(scene.id).to.equal('scene');
        expect(scene.name).to.equal(sceneName);
        expect(scene.type).to.equal(osn.ESourceType.Scene);

        // Creating input source
        const source = osn.InputFactory.create(EOBSInputTypes.ImageSource, sourceName);

        // Checking if source was created correctly
        expect(source).to.not.equal(undefined);
        expect(source.id).to.equal(EOBSInputTypes.ImageSource);
        expect(source.name).to.equal(sourceName);
    });

    afterEach(function() {
        const scene = osn.SceneFactory.fromName(sceneName);
        scene.release();
    });

    context('# GetSource', () => {
        it('Get source associated with a scene item', () => {
            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Getting source from scene item to create scene item
            const returnedSource = sceneItem.source;

            // Checking if source was returned correctly
            expect(returnedSource).to.not.equal(undefined);
            expect(returnedSource.id).to.equal(EOBSInputTypes.ImageSource);
            expect(returnedSource.name).to.equal(sourceName);

            sceneItem.source.release();
            sceneItem.remove();
            scene.release();
        });
    });

    context('# GetScene', () => {
        it('Get scene associated with a scene item', () => {
            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Getting scene associated with scene item
            const returnedScene = sceneItem.scene;

            // Checking if scene was returned correctly
            expect(returnedScene).to.not.equal(undefined);
            expect(returnedScene.id).to.equal('scene');
            expect(returnedScene.name).to.equal(sceneName);
            expect(returnedScene.type).to.equal(osn.ESourceType.Scene);
            
            sceneItem.source.release();
            sceneItem.remove();
            scene.release();
        });
    });

    context('# SetVisible and IsVisible', () => {
        it('Set scene item as visible and not visible and check it', () => {
            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Setting scene item as visible
            sceneItem.visible = true;

            // Checking if scene item is visible
            expect(sceneItem.visible).to.equal(true);

            // Setting scene item as not visible
            sceneItem.visible = false;

            // Checking if scene item is not visible
            expect(sceneItem.visible).to.equal(false);
            sceneItem.source.release();
            sceneItem.remove();
        });
    });

    context('# SetSelected and IsSelected', () => {
        it('Set scene item as selected or not selected and check it', () => {
            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Setting scene item as selected
            sceneItem.selected = true;
            
            // Checking if scene item is selected
            expect(sceneItem.selected).to.equal(true);

            // Setting scene item as not selected
            sceneItem.selected = false;

            // Checking if scene item is selected
            expect(sceneItem.selected).to.equal(false);
            sceneItem.source.release();
            sceneItem.remove();
        });
    });

    context('# SetPosition and GetPosition', () => {
        it('Set scene item position and get it', () => {
            let position: IVec2 = {x: 1,y: 2};

            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Setting position of scene item
            sceneItem.position = position;

            // Getting position of scene item
            const returnedPosition = sceneItem.position;

            // Checking if position was set properly
            expect(sceneItem.position.x).to.equal(returnedPosition.x);
            expect(sceneItem.position.y).to.equal(returnedPosition.y);
            sceneItem.source.release();
            sceneItem.remove();
        });
    });

    context('# SetRotation and GetRotation', () => {
        it('Set scene item rotation and get it', () => {
            let rotation: number = 180;

            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Setting scene item rotation
            sceneItem.rotation = rotation;

            // Getting scene item rotation
            const returnedRotation = sceneItem.rotation;

            // Checkin if rotation was set properly
            expect(sceneItem.rotation).to.equal(returnedRotation);
            sceneItem.source.release();
            sceneItem.remove();
        });
    });

    context('# SetScale and Get Scale', () => {
        it('Set scene item scale and get it', () => {
            let scale: IVec2 = {x: 20, y:30};

            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Setting scene item scale
            sceneItem.scale = scale;

            // Getting scene item scale
            const returnedScale = sceneItem.scale;

            // Checking if scale was set properly
            expect(sceneItem.scale.x).to.equal(returnedScale.x);
            expect(sceneItem.scale.y).to.equal(returnedScale.y);
            sceneItem.source.release();
            sceneItem.remove();
        });
    });

    context('# SetCrop and GetCrop', () => {
        it('Set crop value and get it', () => {
            let crop: ICrop = {top: 5, bottom: 5, left: 3, right:3};

            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Setting crop
            sceneItem.crop = crop;

            // Getting crop
            const returnedCrop = sceneItem.crop;

            // Checking if crop was set properly
            expect(sceneItem.crop.top).to.equal(returnedCrop.top);
            expect(sceneItem.crop.bottom).to.equal(returnedCrop.bottom);
            expect(sceneItem.crop.left).to.equal(returnedCrop.left);
            expect(sceneItem.crop.right).to.equal(returnedCrop.right);
            sceneItem.source.release();
            sceneItem.remove();
        });
    });

    context('# GetId', () => {
        it('Get scene item id', () => {
            let sceneItemId: number = undefined;

            // Getting scene
            const scene = osn.SceneFactory.fromName(sceneName);

            // Getting source
            const source = osn.InputFactory.fromName(sourceName);

            // Adding input source to scene to create scene item
            const sceneItem = scene.add(source);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal(EOBSInputTypes.ImageSource);
            expect(sceneItem.source.name).to.equal(sourceName);

            // Getting scene item id
            sceneItemId = sceneItem.id;

            // Checking
            expect(sceneItemId).to.not.equal(undefined);
            sceneItem.source.release();
            sceneItem.remove();
        });
    });
});
