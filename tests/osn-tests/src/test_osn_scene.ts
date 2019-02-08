import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IScene, ISceneItem, IInput } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';

describe('osn-scene', () => {
    let obs: OBSProcessHandler;
    let scene: IScene;
    let duplicatedScene: IScene;
    let sceneFromName: IScene;
    
    
    // Initialize OBS process
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() != true)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    // Shutdown OBS process
    after(function(done) {
        this.timeout(3000);
        obs.shutdown();
        obs = null;
        setTimeout(done, 3000);
    });

    context('# Create', () => {
        it('Create scene', () => {
            // Creating scene
            scene = osn.SceneFactory.create('test_osn_scene'); 

            // Checking if scene was created correctly
            expect(scene).to.not.equal(undefined);
            expect(scene.id).to.equal('scene');
            expect(scene.name).to.equal('test_osn_scene');
            expect(scene.type).to.equal(osn.ESourceType.Scene);
        });
    });

    context('# Duplicate', () => {
        it('Duplicate scene', () => {
            // Duplicating scene
            duplicatedScene = scene.duplicate('test_osn_scene_duplicate', osn.ESceneDupType.Copy);

            // Checking if scene was duplicated correctly
            expect(duplicatedScene).to.not.equal(undefined);
            expect(duplicatedScene.id).to.equal('scene');
            expect(duplicatedScene.name).to.equal('test_osn_scene_duplicate');
            expect(duplicatedScene.type).to.equal(osn.ESourceType.Scene);
        });
    });

    context('# FromName', () => {
        it('Get scene from name', () => {
            // Getting scene by its name
            sceneFromName = osn.SceneFactory.fromName('test_osn_scene');

            // Checking if scene was returned
            expect(sceneFromName).to.not.equal(undefined);
            expect(sceneFromName.id).to.equal('scene');
            expect(sceneFromName.name).to.equal('test_osn_scene');
            expect(sceneFromName.type).to.equal(osn.ESourceType.Scene);
        });

        it('FAIL TEST: Get scene from name that don\'t exist ', () => {
            let failSceneFromName: IScene;

            expect(function() {
                failSceneFromName = osn.SceneFactory.fromName('does_not_exist');
            }).to.throw();
        });
    });

    context('# AddSource', () => {
        it('Add source to scene', () => {
            let input: IInput; 
            let sceneItem: ISceneItem;
            
            // Creating input source
            input = osn.InputFactory.create('image_source', 'test_osn_scene_source1');
            
            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal('image_source');
            expect(input.name).to.equal('test_osn_scene_source1');

            // Adding input source to scene
            sceneItem = scene.add(input);

            // Checking if input source was added to the scene correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal('image_source');
            expect(sceneItem.source.name).to.equal('test_osn_scene_source1');
        });
    });

    context('# FindItem', () => {
        it('Find scene item by id', () => {
            let sceneItem: ISceneItem;

            // Getting scene item by id
            sceneItem = scene.findItem(1);

            // Checking if scene item was returned correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal('image_source');
            expect(sceneItem.source.name).to.equal('test_osn_scene_source1');
        });

        it('FAIL TEST: Try to find scene that don\'t exist', () => {
            let sceneItem: ISceneItem;

            // Getting scene item by non existant id
            sceneItem = scene.findItem('this_scene_does_not_exist');

            // Checking if scene item is undefined
            expect(sceneItem).to.equal(undefined);
        });
    });

    context('# GetItems', () => {
        it('Get all scene items in a scene', () => {
            let input: IInput; 
            let sceneItem: ISceneItem;
            let sceneItems: ISceneItem[];

            // Creating new input source
            input = osn.InputFactory.create('image_source', 'test_osn_scene_source2');

            // Checking if input source was created correctly
            expect(input).to.not.equal(undefined);
            expect(input.id).to.equal('image_source');
            expect(input.name).to.equal('test_osn_scene_source2');

            // Adding input source to existing scene
            sceneItem = scene.add(input);
           
            // Checking if scene item was returned correctly
            expect(sceneItem).to.not.equal(undefined);
            expect(sceneItem.source.id).to.equal('image_source');
            expect(sceneItem.source.name).to.equal('test_osn_scene_source2');

            // Getting all scene items
            sceneItems = scene.getItems();

            // Checking if sceneItems array has the correct scenes
            expect(sceneItems.length).to.equal(2);
            expect(sceneItems[0].source.name).to.equal('test_osn_scene_source1');
            expect(sceneItems[1].source.name).to.equal('test_osn_scene_source2');
        });
    });

    context('# MoveItem', () => {
        it('Reorder scene item in obs scene item container', () => {
            let sceneItems: ISceneItem[];
            
            // Moving scene item
            scene.moveItem(1, 0);

            // Getting all scene items
            sceneItems = scene.getItems();
            
            //Checking if scene items were moved
            expect(sceneItems.length).to.equal(2);
            expect(sceneItems[0].source.name).to.equal('test_osn_scene_source2');
            expect(sceneItems[1].source.name).to.equal('test_osn_scene_source1');
        });

        it('FAIL TEST: Try to access out of bounds', () => {
            expect(function() {
                scene.moveItem(3, 0);
            }).to.throw();
        })
    });

    context('# Release', () => {
        it('Release all the scenes created in this test', () => {
            // Releasing first created scene
            scene.release();

            // Release duplicated scene
            duplicatedScene.release();
        });
    });
});