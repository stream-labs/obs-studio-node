import 'mocha'
import { expect } from 'chai'
import * as osn from 'obs-studio-node';
import { IInput, ISource } from 'obs-studio-node';
import { OBSProcessHandler } from '../util/obs_process_handler';
import { getCppErrorMsg } from '../util/general';

describe('osn-global', () => {
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
        let source: IInput;

        it('should set source to output channel', () => {
            try {
                source = osn.InputFactory.create('image_source', 'test_osn_global_source');
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }

            expect(source.id).to.not.equal(undefined);

            try {
                osn.Global.setOutputSource(1, source);
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }
        });

        it('should fail if trying to set source to output channel that does not exist', () => {
            expect(function() {
                osn.Global.setOutputSource(99, source);
            }).to.throw();
        });
    });

    context('# GetOutputSource', () => {
        it('should get source from output channel', () => {
            let source: ISource;

            try {
                source = osn.Global.getOutputSource(1);
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }
            
            expect(source.name).to.equal('test_osn_global_source');
        });

        it('should fail if trying to get from empty output channel', () => {
            let source: ISource;

            try {
                source = osn.Global.getOutputSource(5);
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }

            expect(source).to.equal(undefined);
        });
    });

    context('# GetOutputFlagsFromId', () => {
        it('should get flags (capabilities) of a source type', () => {
            let sourceType: string;
            let sourceTypes: string[];
            let flags: number = 0;

            try {
                sourceTypes = osn.InputFactory.types();
            } catch(e) {
                throw new Error(getCppErrorMsg(e));
            }

            expect(sourceTypes.length).to.not.equal(0);

            for (sourceType of sourceTypes)
            {
                try {
                    flags = osn.Global.getOutputFlagsFromId(sourceType);
                } catch(e) {
                    throw new Error(getCppErrorMsg(e));
                }

                expect(flags).to.not.equal(0);
                flags = 0;
            }
        });
    });

    context('# LaggedFrames', () => {
        it('should get lagged frames value', () => {
            let laggedFrames: number;

            expect(function() {
                laggedFrames = osn.Global.laggedFrames;
            }).to.not.throw();
        });
    });

    context('# TotalFrames', () => {
        it('should get total frames value', () => {
            let totalFrames: number = undefined;

            expect(function() {
                totalFrames = osn.Global.totalFrames;
            }).to.not.throw();

            expect(totalFrames).to.not.equal(undefined);
        });
    });

    context('# SetLocale', () => {
        it('shoud set locale', () => {
            expect(function() {
                osn.Global.locale = 'pt-BR';
            }).to.not.throw();
        });
    });

    context('# GetLocale', () => {
        it('shoud get locale', () => {
            let locale: string;

            expect(function() {
                locale = osn.Global.locale;
            }).to.not.throw();

            expect(locale).to.equal('pt-BR');
        });
    });
});