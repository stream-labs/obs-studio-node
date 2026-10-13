import 'mocha';
import { expect } from 'chai';
import * as osn from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';

describe('nodeobs-simple-vod-settings', function () {
    this.timeout(30000);
    let obs: OBSHandler;

    before(() => {
        deleteConfigFiles();
        obs = new OBSHandler('nodeobs-simple-vod-settings');
    });
    after(() => {
        if (obs) obs.shutdown();
        deleteConfigFiles();
    });

    function fields(category: string): any[] {
        return osn.NodeObs.OBS_settings_getSettings(category).data.reduce((all, section) => all.concat(section.parameters), []);
    }
    function field(category: string, name: string): any {
        return fields(category).find(parameter => parameter.name === name);
    }
    function save(category: string, values: { [key: string]: any }) {
        const data = osn.NodeObs.OBS_settings_getSettings(category).data;
        for (const name of Object.keys(values)) {
            const parameter = data.reduce((all, section) => all.concat(section.parameters), []).find(parameter => parameter.name === name);
            expect(parameter, `${category}.${name}`).to.not.equal(undefined);
            parameter.currentValue = values[name];
        }
        osn.NodeObs.OBS_settings_saveSettings(category, data);
    }

    beforeEach(() => {
        save('Stream', { streamType: 'rtmp_common' });
        save('Stream', { service: 'Twitch', server: 'auto', key: '' });
        save('Output', { Mode: 'Simple' });
        save('Output', { UseAdvanced: false });
    });

    it('exposes Simple VOD only for common Twitch with advanced encoder settings enabled', () => {
        expect(field('Output', 'VodTrackEnabled')).to.equal(undefined);
        save('Output', { UseAdvanced: true });
        expect(field('Output', 'VodTrackEnabled')).to.include({ type: 'OBS_PROPERTY_BOOL', visible: true });
        save('Stream', { service: 'YouTube - RTMPS' });
        expect(field('Output', 'VodTrackEnabled')).to.equal(undefined);
        save('Stream', { service: 'Twitch' });
        save('Stream', { streamType: 'rtmp_custom' });
        expect(field('Output', 'VodTrackEnabled')).to.equal(undefined);
    });

    it('persists Simple VOD across restart while preserving Advanced track selections', () => {
        save('Output', { Mode: 'Advanced' });
        save('Output', { VodTrackEnabled: true });
        save('Output', { TrackIndex: '3', VodTrackIndex: '6' });
        save('Output', { Mode: 'Simple' });
        save('Output', { UseAdvanced: true });
        save('Output', { VodTrackEnabled: true });
        obs.shutdown();
        obs = new OBSHandler('nodeobs-simple-vod-settings');
        expect(field('Output', 'VodTrackEnabled').currentValue).to.equal(true);
        save('Output', { UseAdvanced: false });
        expect(field('Output', 'VodTrackEnabled')).to.equal(undefined);
        save('Output', { UseAdvanced: true });
        expect(field('Output', 'VodTrackEnabled').currentValue).to.equal(true);
        save('Output', { VodTrackEnabled: false });
        save('Output', { Mode: 'Advanced' });
        expect(field('Output', 'VodTrackEnabled').currentValue).to.equal(true);
        expect(field('Output', 'TrackIndex').currentValue).to.equal('3');
        expect(field('Output', 'VodTrackIndex').currentValue).to.equal('6');
    });
});
