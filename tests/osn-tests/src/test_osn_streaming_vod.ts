import 'mocha';
import { expect } from 'chai';
import * as fs from 'fs';
import * as path from 'path';
import * as osn from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles } from '../util/general';
import { RtmpTestServer } from '../util/rtmp-test-server';
import { expectAudioTracks, expectVideoFrames, getAacAudioTrack } from '../util/rtmp-assertions';
import { getMeanVolumeDb } from '../util/media_probe';
import { createStreamingOutput, ITestStreamingOutput, TStreamingMode } from '../util/streaming_output';

const testName = 'osn-streaming-vod';
const mediaCounts = { audioPackets: 25, videoPackets: 15 };

describe(testName, function () {
    this.timeout(30000);
    let obs: OBSHandler;
    let horizontal: osn.IVideo;
    let vertical: osn.IVideo;
    let receiver: RtmpTestServer;
    let outputs: ITestStreamingOutput[];
    let services: osn.IService[];
    const toneFile = path.join(__dirname, '..', 'osnData', 'vod-routing-tone.wav');
    const audioFile = path.join(__dirname, '..', 'osnData', 'vod-routing-capture.aac');

    before(() => {
        deleteConfigFiles();
        obs = new OBSHandler(testName, false);
        const videoInfo = {
            fpsNum: 30, fpsDen: 1, baseWidth: 320, baseHeight: 180, outputWidth: 320, outputHeight: 180,
            outputFormat: osn.EVideoFormat.NV12, colorspace: osn.EColorSpace.CS709,
            range: osn.ERangeType.Partial, scaleType: osn.EScaleType.Bilinear, fpsType: osn.EFPSType.Fractional,
        };
        horizontal = osn.VideoFactory.create();
        horizontal.video = videoInfo;
        vertical = osn.VideoFactory.create();
        vertical.video = { ...videoInfo, baseWidth: 180, baseHeight: 320, outputWidth: 180, outputHeight: 320 };
        for (const index of [1, 2, 6]) osn.AudioTrackFactory.setAtIndex(osn.AudioTrackFactory.create(160, `Track ${index}`), index);
        const sampleRate = 48000;
        const pcm = Buffer.alloc(sampleRate * 2);
        for (let index = 0; index < sampleRate; index++) pcm.writeInt16LE(Math.round(12000 * Math.sin(2 * Math.PI * 440 * index / sampleRate)), index * 2);
        const header = Buffer.alloc(44);
        header.write('RIFF'); header.writeUInt32LE(pcm.length + 36, 4); header.write('WAVEfmt ', 8);
        header.writeUInt32LE(16, 16); header.writeUInt16LE(1, 20); header.writeUInt16LE(1, 22);
        header.writeUInt32LE(sampleRate, 24); header.writeUInt32LE(sampleRate * 2, 28);
        header.writeUInt16LE(2, 32); header.writeUInt16LE(16, 34); header.write('data', 36); header.writeUInt32LE(pcm.length, 40);
        fs.writeFileSync(toneFile, Buffer.concat([header, pcm]));
    });

    beforeEach(async () => {
        outputs = [];
        services = [];
        receiver = await RtmpTestServer.start();
    });

    afterEach(async () => {
        try {
            for (const output of outputs) await output.destroy();
            services.forEach(service => osn.ServiceFactory.destroy(service));
            receiver.assertHealthy();
        } finally { await receiver.close(); }
    });

    after(() => {
        if (vertical) vertical.destroy();
        if (horizontal) horizontal.destroy();
        if (obs) obs.shutdown();
        for (const file of [toneFile, audioFile]) if (fs.existsSync(file)) fs.unlinkSync(file);
    });

    function createService(id: 'rtmp_common' | 'rtmp_custom', key: string, settings: osn.ISettings = {}): osn.IService {
        const service = osn.ServiceFactory.create(id, key, settings);
        services.push(service);
        // Preserve the original create-then-merge trigger when the test supplies a stale Twitch label.
        service.update({ server: receiver.url, key, streamType: id });
        return service;
    }

    it('Simple VOD start and cleanup preserve source routing used by another output', async () => {
        const source = osn.InputFactory.create('ffmpeg_source', 'shared-desktop-audio');
        source.audioMixers = 63;
        osn.Global.setOutputSource(1, source);
        const first = createStreamingOutput({
            mode: 'Simple', video: horizontal, name: 'first-twitch',
            service: createService('rtmp_common', 'first-twitch', { service: 'Twitch' }),
        });
        outputs.push(first);
        const companion = createStreamingOutput({
            mode: 'Simple', video: vertical, name: 'second-twitch',
            service: createService('rtmp_common', 'second-twitch', { service: 'Twitch' }),
        });
        outputs.push(companion);
        first.stream.enableTwitchVOD = true;
        companion.stream.enableTwitchVOD = true;
        try {
            await first.start();
            const session = await receiver.waitForPublish('first-twitch');
            await session.waitForMedia(mediaCounts);
            await companion.start();
            await (await receiver.waitForPublish('second-twitch')).waitForMedia(mediaCounts);
            const mixers = source.audioMixers;
            expect(mixers).to.equal(63);
            await first.stop();
            first.stream.enableTwitchVOD = false;
            await first.start();
            const restarted = await receiver.waitForPublish('first-twitch', { afterAttempt: session.attempt });
            await restarted.waitForMedia(mediaCounts);
            await first.stop();
            expectAudioTracks(restarted, [0]);
            expectVideoFrames(restarted);
            expect(source.audioMixers).to.equal(mixers);
        } finally {
            await first.stop();
            await companion.stop();
            osn.Global.setOutputSource(1, null);
            source.release();
        }
    });

    for (const mode of ['Simple', 'Advanced'] as TStreamingMode[]) {
        it(`${mode}: VOD respects source exclusions on ${mode === 'Simple' ? 'fixed track 2' : 'selected track 6'}`, async () => {
            const source = osn.InputFactory.create('ffmpeg_source', `${mode}-vod-tone`, {
                is_local_file: true, local_file: toneFile, looping: true,
            });
            const vodMixer = mode === 'Simple' ? 1 : 5;
            const otherMixer = mode === 'Simple' ? 5 : 1;
            const output = createStreamingOutput({
                mode, video: horizontal, name: `${mode}-vod-routing`,
                service: createService('rtmp_common', `${mode}-vod-routing`, { service: 'Twitch' }),
            });
            outputs.push(output);
            output.stream.enableTwitchVOD = true;
            if (mode === 'Advanced') (output.stream as osn.IAdvancedStreaming).twitchTrack = 6;
            osn.Global.setOutputSource(1, source);
            let previousAttempt = 0;
            try {
                for (const included of [true, false]) {
                    const mixers = 1 | (1 << (included ? vodMixer : otherMixer));
                    source.audioMixers = mixers;
                    await output.start();
                    const session = await receiver.waitForPublish(`${mode}-vod-routing`, { afterAttempt: previousAttempt });
                    await session.waitForMedia({ audioPackets: 120, videoPackets: 15 });
                    await output.stop();
                    expectAudioTracks(session, [0, 1]);
                    expectVideoFrames(session);
                    expect(source.audioMixers).to.equal(mixers);
                    for (const trackId of [0, 1]) {
                        fs.writeFileSync(audioFile, getAacAudioTrack(session, trackId));
                        const volume = getMeanVolumeDb(audioFile);
                        if (trackId === 0 || included) expect(volume, `${mode} track ${trackId} audible tone`).to.be.greaterThan(-50);
                        else expect(volume, `${mode} VOD excludes the tone`).to.be.lessThan(-80);
                    }
                    previousAttempt = session.attempt;
                }
            } finally {
                await output.stop();
                osn.Global.setOutputSource(1, null);
                source.release();
            }
        });
    }

    for (const mode of ['Simple', 'Advanced'] as TStreamingMode[]) {
        for (const inherited of [false, true]) {
            it(`${mode}: custom RTMP sends one track with VOD enabled${inherited ? ' and inherited Twitch settings' : ''}`, async () => {
                const output = createStreamingOutput({
                    mode, video: vertical, name: 'custom',
                    service: createService('rtmp_custom', 'custom', inherited ? { service: 'Twitch' } : {}),
                });
                outputs.push(output);
                output.stream.enableTwitchVOD = true;
                await output.start();
                const session = await receiver.waitForPublish('custom');
                await session.waitForMedia(mediaCounts);
                await output.stop();
                expectAudioTracks(session, [0]);
                expectVideoFrames(session);
            });
        }

        it(`${mode}: retained Twitch output removes and restores VOD when the preference changes`, async () => {
            const output = createStreamingOutput({
                mode, video: horizontal, name: 'twitch',
                service: createService('rtmp_common', 'twitch', { service: 'Twitch' }),
            });
            outputs.push(output);
            let previousAttempt = 0;
            // Repeat both transitions on the retained output to catch stale VOD tracks after removal and re-addition.
            for (const enabled of [true, false, true, false]) {
                output.stream.enableTwitchVOD = enabled;
                await output.start();
                const session = await receiver.waitForPublish('twitch', { afterAttempt: previousAttempt });
                await session.waitForMedia(mediaCounts);
                await output.stop();
                expectAudioTracks(session, enabled ? [0, 1] : [0]);
                expectVideoFrames(session);
                previousAttempt = session.attempt;
            }
        });

        it(`${mode}: retained output clears VOD after switching from Twitch to custom RTMP`, async () => {
            const output = createStreamingOutput({
                mode, video: horizontal, name: 'switch-provider',
                service: createService('rtmp_common', 'switch-provider', { service: 'Twitch' }),
            });
            outputs.push(output);
            output.stream.enableTwitchVOD = true;
            await output.start();
            const twitch = await receiver.waitForPublish('switch-provider');
            await twitch.waitForMedia(mediaCounts);
            await output.stop();
            expectAudioTracks(twitch, [0, 1]);
            expectVideoFrames(twitch);
            output.stream.service = createService('rtmp_custom', 'switch-provider', { service: 'Twitch' });
            await output.start();
            const custom = await receiver.waitForPublish('switch-provider', { afterAttempt: twitch.attempt });
            await custom.waitForMedia(mediaCounts);
            await output.stop();
            expectAudioTracks(custom, [0]);
            expectVideoFrames(custom);
        });

        for (const swapped of [false, true]) {
            it(`${mode}: simultaneous Twitch and custom outputs isolate VOD (${swapped ? 'Twitch vertical' : 'Twitch horizontal'})`, async () => {
                const twitch = createStreamingOutput({
                    mode, video: swapped ? vertical : horizontal, name: 'twitch',
                    service: createService('rtmp_common', 'twitch', { service: 'Twitch' }),
                });
                outputs.push(twitch);
                const custom = createStreamingOutput({
                    mode, video: swapped ? horizontal : vertical, name: 'custom',
                    service: createService('rtmp_custom', 'custom', { service: 'Twitch' }),
                });
                outputs.push(custom);
                twitch.stream.enableTwitchVOD = true;
                custom.stream.enableTwitchVOD = true;
                await twitch.start();
                const twitchSession = await receiver.waitForPublish('twitch');
                await twitchSession.waitForMedia(mediaCounts);
                await custom.start();
                const customSession = await receiver.waitForPublish('custom');
                await customSession.waitForMedia(mediaCounts);
                await custom.stop();
                await twitch.stop();
                expectAudioTracks(twitchSession, [0, 1]);
                expectVideoFrames(twitchSession);
                expectAudioTracks(customSession, [0]);
                expectVideoFrames(customSession);
            });
        }
    }
});
