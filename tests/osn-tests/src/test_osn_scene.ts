import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IScene, ISceneItem, IInput } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler'


describe('osn_scene', () => {
    let obs: OBSProcessHandler;
    let scene: IScene;
    let duplicatedScene: IScene;
    let privateScene: IScene;
    let sceneFromName: IScene;
    let sceneItems: ISceneItem[];
    
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() != true)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    after(function(done) {
        this.timeout(5000);
        obs.shutdown();
        obs = null;
        setTimeout(done, 3000);
    });

    context('# Create', () => {
        it('should create scene', () => {
            try {
                scene = osn.SceneFactory.create('test_osn_scene'); 
            } catch(e) {
                throw new Error("failed to create scene");
            }

            expect(scene.id).to.equal('scene');
            expect(scene.name).to.equal('test_osn_scene');
            expect(scene.type).to.equal(osn.ESourceType.Scene);
        });
    });

    context('# Duplicate', () => {
        it('should duplicate scene', () => {
            try {
                duplicatedScene = scene.duplicate('test_osn_scene_duplicate', osn.ESceneDupType.Copy);
            } catch(e) {
                throw new Error("failed to duplicate scene");
            }

            expect(duplicatedScene.id).to.equal('scene');
            expect(duplicatedScene.name).to.equal('test_osn_scene_duplicate');
            expect(duplicatedScene.type).to.equal(osn.ESourceType.Scene);
        });
    });

    context('# CreatePrivate', () => {
        it('should create private scene', () => {
            try {
                privateScene = osn.SceneFactory.createPrivate('test_osn_scene_private'); 
            } catch(e) {
                throw new Error("failed to create private scene");
            }

            expect(privateScene.id).to.equal('scene');
            expect(privateScene.name).to.equal('test_osn_scene_private');
            expect(privateScene.type).to.equal(osn.ESourceType.Scene);
        });
    });

    context('# FromName', () => {
        it('should get scene from name', () => {
            try {
                sceneFromName = osn.SceneFactory.fromName('test_osn_scene');
            } catch (e) {
                throw new Error("failed to get scene from name");
            }

            expect(sceneFromName.id).to.equal('scene');
            expect(sceneFromName.name).to.equal('test_osn_scene');
            expect(sceneFromName.type).to.equal(osn.ESourceType.Scene);
        });

        it('should fail if scene name does not exist', () => {
            let failSceneFromName: IScene;

            expect(function() {
                failSceneFromName = osn.SceneFactory.fromName('does_not_exist');
            }).to.throw();
        });
    });

    context('# AddSource', () => {
        it('should add source to scene', () => {
            let source: IInput; 
            let sceneItem: ISceneItem;
            
            try {
                source = osn.InputFactory.create('image_source', 'test_osn_scene_source1');
            } catch(e) {
                throw new Error("failed to create source");
            }
            
            expect(source).to.not.equal(undefined);

            try{
                sceneItem = scene.add(source);
            } catch(e) {
                throw new Error("failed to add source to scene");
            }

            expect(sceneItem).to.not.equal(undefined);
        });
    });

    context('# FindItem', () => {
        it('should find scene item by id', () => {
            let sceneItem: ISceneItem;

            try {
                sceneItem = scene.findItem(1);
            } catch(e) {
                throw new Error("failed to get scene item by id");
            }

            expect(sceneItem).to.not.equal(undefined);
        });

        it('should return undefined if a scene item does not exist (source name)', () => {
            let sceneItem: ISceneItem;

            try {
                sceneItem = scene.findItem('this_scene_does_not_exist');
            } catch(e) {
                throw new Error("failed to get scene item by id");
            }

            expect(sceneItem).to.equal(undefined);
        });
    });

    context('# GetItems', () => {
        it('should return all scene items in a scene', () => {
            let source: IInput; 
            let sceneItem: ISceneItem;

            try {
                source = osn.InputFactory.create('image_source', 'test_osn_scene_source2');
            } catch(e) {
                throw new Error("failed to create source");
            }
            
            expect(source).to.not.equal(undefined);

            try {
                sceneItem = scene.add(source);
            } catch(e) {
                throw new Error("failed to add source to scene");
            }

            expect(sceneItem).to.not.equal(undefined);

            try {
                sceneItems = scene.getItems();
            } catch(e)
            {
                throw new Error("failed to get all scene items");
            }

            expect(sceneItems.length).to.equal(2);
        });
    });

    context('# MoveItem', () => {
        it('should exchange scene item ids', () => {
            try {
                scene.moveItem(1, 0);
            } catch(e) {
                throw new Error("failed to move scene item");
            }

            sceneItems = [];

            try {
                sceneItems = scene.getItems();
            } catch(e)
            {
                throw new Error("failed to get all scene items");
            }
            
            expect(sceneItems[0].source.name).to.equal('test_osn_scene_source2');
        });
    });

    context('# Release', () => {
        it('should release all the scenes created in this test', () => {
            try {
                sceneFromName.release();
            } catch(e) {
                throw new Error("failed to release from name scene");
            }
            
            try {
                privateScene.release();
            } catch(e) {
                throw new Error("failed to release private scene");
            }

            try {
                duplicatedScene.release();
            } catch(e) {
                throw new Error("failed to release duplicated scene");
            }
        });
    });
});