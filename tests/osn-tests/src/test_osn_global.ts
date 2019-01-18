import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IInput, ISource } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler'

describe('osn_global', () => {
    let obs: OBSProcessHandler;
    
    before(function() {
        obs = new OBSProcessHandler();
        
        if (obs.startup() != true)
        {
            throw new Error("Could not start OBS process. Aborting!")
        }
    });

    after(function(done) {
        this.timeout(3000);
        obs.shutdown();
        obs = null;
        setTimeout(done, 2000);
    });

    context('# SetOutputSource', () => {
        it('should set source to output channel', () => {
            let source: IInput;

            try {
                source = osn.InputFactory.create('image_source', 'test_osn_global_source');
            } catch(e) {
                throw new Error("failed to create source");
            }

            expect(source.id).to.not.equal(undefined);

            try {
                osn.Global.setOutputSource(1, source);
            } catch(e) {
                throw new Error("failed to set source to output channel");
            }
        });
    });

    context('# GetOutputSource', () => {
        it('should get source from output channel', () => {
            let source: ISource;

            try {
                source = osn.Global.getOutputSource(1);
            } catch(e) {
                throw new Error("failed to set source to output channel");
            }
            
            expect(source.name).to.equal('test_osn_global_source');
        });
    });
});