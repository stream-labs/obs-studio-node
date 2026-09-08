import 'mocha';
import { expect } from 'chai';
import * as fs from 'fs';
import * as path from 'path';
import * as osn from '../osn';
import { OBSHandler } from '../util/obs_handler';
import { deleteConfigFiles, sleep } from '../util/general';
import { EOBSOutputSignal, EOBSOutputType, EOBSSettingsCategories } from '../util/obs_enums';
import { getAudioStreamBitrates, getVideoKeyframes } from '../util/media_probe';
import { logInfo, logEmptyLine } from '../util/logger';

const testName = 'osn-encoder-settings';
const configPath = path.join(__dirname, '..', 'osnData', 'slobs-client');
const outputCategory = EOBSSettingsCategories.Output;

// Broadband stereo audio makes the measured AAC bitrate meaningful. Silence
// and a single sine wave can encode far below the requested bitrate.
function createTestAudio(filePath: string) {
    const sampleRate = 48000;
    const sampleCount = sampleRate * 8;
    const wav = Buffer.alloc(44 + sampleCount * 4);
    wav.write('RIFF');
    wav.writeUInt32LE(wav.length - 8, 4);
    wav.write('WAVEfmt ', 8);
    wav.writeUInt32LE(16, 16);
    wav.writeUInt16LE(1, 20);
    wav.writeUInt16LE(2, 22);
    wav.writeUInt32LE(sampleRate, 24);
    wav.writeUInt32LE(sampleRate * 4, 28);
    wav.writeUInt16LE(4, 32);
    wav.writeUInt16LE(16, 34);
    wav.write('data', 36);
    wav.writeUInt32LE(wav.length - 44, 40);
    let seed = 12345;
    for (let i = 0; i < sampleCount * 2; i++) {
        seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
        wav.writeInt16LE(Math.floor((seed / 0x100000000 - 0.5) * 16000), 44 + i * 2);
    }
    fs.writeFileSync(filePath, wav);
}

describe(testName, function () {
    let obs: OBSHandler;
    let hasTestFailed = false;

    before(function () {
        logInfo(testName, 'Starting ' + testName + ' tests');
        deleteConfigFiles();
        obs = new OBSHandler(testName);
        obs.defaultVideoContext.video = {
            ...obs.defaultVideoContext.video,
            baseWidth: 320, baseHeight: 180, outputWidth: 320, outputHeight: 180,
        };
    });

    beforeEach(function () {
        obs.setSetting(outputCategory, 'Mode', 'Advanced');
        obs.setSetting(outputCategory, 'Encoder', 'obs_x264');
        obs.setSetting(outputCategory, 'RecEncoder', 'obs_x264');
        obs.setSetting(outputCategory, 'RecAEncoder', 'ffmpeg_aac');
        obs.setSetting(outputCategory, 'ApplyServiceSettings', false);
    });

    afterEach(function () {
        hasTestFailed = this.currentTest.state === 'failed' || hasTestFailed;
    });

    after(async function () {
        if (obs) {
            obs.shutdown();
            if (hasTestFailed) await obs.uploadTestCache();
        }
        // IPC disconnect returns before the server closes its media files.
        const deadline = Date.now() + 5000;
        while (true) {
            try {
                deleteConfigFiles();
                break;
            } catch (error) {
                if (Date.now() >= deadline) throw error;
                await sleep(100);
            }
        }
        logInfo(testName, 'Finished ' + testName + ' tests');
        logEmptyLine();
    });

    function saveSettings(values: osn.ISettings) {
        const settings = obs.getSettingsContainer(outputCategory);
        const parameters = settings.reduce((parameters, section) => parameters.concat(section.parameters), []);
        for (const name of Object.keys(values)) {
            const parameter = parameters.find(parameter => parameter.name === name);
            expect(parameter, `Missing output setting ${name}`).to.not.equal(undefined);
            parameter.currentValue = values[name];
        }
        obs.setSettingsContainer(outputCategory, settings);
    }

    function saveEncoderSettings(outputType: 'streaming' | 'recording', values: osn.ISettings) {
        const settings: osn.ISettings = {};
        for (const name of Object.keys(values)) {
            settings[outputType === 'recording' ? `Rec${name}` : name] = values[name];
        }
        saveSettings(settings);
    }

    it('Rejects invalid arguments, stale mode and mismatched encoder selections', function () {
        const getSettings = osn.NodeObs.OBS_settings_getEncoderSettings as (...args: any[]) => osn.ISettings;
        for (const args of [
            [], ['obs_x264', 'recording'], [null, 'recording', 'Advanced'],
            ['', 'recording', 'Advanced'], ['obs_x264', 'replay', 'Advanced'],
            ['obs_x264', 'recording', 'advanced'],
        ]) {
            expect(() => getSettings(...args)).to.throw(TypeError);
        }
        expect(() => getSettings('missing-encoder', 'recording', 'Advanced')).to.throw(Error);
        expect(() => getSettings('ffmpeg_aac', 'recording', 'Advanced')).to.throw(Error);
        expect(() => getSettings('obs_x264', 'recording', 'Simple')).to.throw(Error);

        const otherEncoder = osn.VideoEncoderFactory.types().find(id => id !== 'obs_x264');
        expect(otherEncoder, 'Expected an additional registered video encoder').to.not.equal(undefined);
        expect(() => getSettings(otherEncoder, 'recording', 'Advanced')).to.throw(Error);
    });

    for (const outputType of ['streaming', 'recording'] as const) {
        it(`Passes all saved advanced ${outputType} settings into a Factory encoder`, function () {
            const values = {
                rate_control: 'CRF', bitrate: 3100, crf: 18, keyint_sec: 1,
                preset: 'fast', profile: 'high', tune: '', x264opts: 'scenecut=0',
                use_bufsize: false, buffer_size: 0, repeat_headers: false,
            };
            saveEncoderSettings(outputType, values);
            const settingsPath = path.join(configPath, outputType === 'recording' ? 'recordEncoder.json' : 'streamEncoder.json');
            const saved = fs.readFileSync(settingsPath, 'utf8');
            const settings = osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', outputType, 'Advanced');
            expect(fs.readFileSync(settingsPath, 'utf8')).to.equal(saved);
            expect(settings).to.deep.include(values);
            expect(settings).to.not.have.property('RecEncoder');
            expect(settings).to.not.have.property('RecFilePath');
            expect(settings).to.not.have.property('Reckeyint_sec');

            const encoder = osn.VideoEncoderFactory.create('obs_x264', `saved-${outputType}`, settings);
            const defaultEncoder = osn.VideoEncoderFactory.create('obs_x264', `default-${outputType}`, {});
            try {
                expect(encoder.settings).to.deep.include(values);
                expect(defaultEncoder.settings).to.deep.equal({});
                settings.keyint_sec = 9;
                expect(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', outputType, 'Advanced'))
                    .to.have.property('keyint_sec', 1);
                expect(encoder.settings).to.have.property('keyint_sec', 1);
            } finally {
                encoder.release();
                defaultEncoder.release();
            }
        });
    }

    it('Uses the saved streaming settings when recording shares the stream encoder', function () {
        saveEncoderSettings('streaming', { keyint_sec: 2, preset: 'faster' });
        saveEncoderSettings('recording', { keyint_sec: 1, preset: 'fast' });
        obs.setSetting(outputCategory, 'RecEncoder', 'none');
        expect(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'recording', 'Advanced'))
            .to.deep.equal(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'streaming', 'Advanced'));
    });

    it('Reads the complete backup without changing files and restores defaults when settings are absent', function () {
        const filePath = path.join(configPath, 'recordEncoder.json');
        const backupPath = `${filePath}.bak`;
        const original = fs.readFileSync(filePath);
        const originalBackup = fs.existsSync(backupPath) ? fs.readFileSync(backupPath) : undefined;
        const backup = JSON.stringify({
            keyint_sec: 3, custom_boolean: false, custom_integer: 0,
            custom_number: 2.5, custom_string: '', custom_object: { enabled: false },
            custom_array: [{ value: 0 }, { value: '' }],
        });
        try {
            fs.writeFileSync(filePath, '{invalid json');
            fs.writeFileSync(backupPath, backup);
            const settings = osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'recording', 'Advanced');
            expect(settings).to.deep.include(JSON.parse(backup));
            expect(settings).to.have.property('preset', 'veryfast');
            expect(fs.readFileSync(filePath, 'utf8')).to.equal('{invalid json');
            expect(fs.readFileSync(backupPath, 'utf8')).to.equal(backup);

            fs.writeFileSync(backupPath, '{invalid backup');
            expect(() => osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'recording', 'Advanced'))
                .to.throw(Error);
            expect(fs.readFileSync(filePath, 'utf8')).to.equal('{invalid json');
            expect(fs.readFileSync(backupPath, 'utf8')).to.equal('{invalid backup');

            fs.unlinkSync(filePath);
            fs.unlinkSync(backupPath);
            const defaults = osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'recording', 'Advanced');
            expect(defaults).to.include({ keyint_sec: 0, preset: 'veryfast', crf: 23 });
            expect(fs.existsSync(filePath)).to.equal(false);
            expect(fs.existsSync(backupPath)).to.equal(false);
        } finally {
            fs.writeFileSync(filePath, original);
            if (originalBackup) fs.writeFileSync(backupPath, originalBackup);
            else if (fs.existsSync(backupPath)) fs.unlinkSync(backupPath);
        }
    });

    it('Maps simple streaming settings and leaves standalone recording quality to the output', function () {
        obs.setSetting(outputCategory, 'Mode', 'Simple');
        obs.setSetting(outputCategory, 'StreamEncoder', 'x264');
        obs.setSetting(outputCategory, 'UseAdvanced', true);
        saveSettings({ VBitrate: 3100, Preset: 'faster', x264Settings: 'scenecut=0' });
        expect(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'streaming', 'Simple'))
            .to.include({ bitrate: 3100, preset: 'faster', x264opts: 'scenecut=0', rate_control: 'CBR' });

        obs.setSetting(outputCategory, 'RecQuality', 'Stream');
        expect(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'recording', 'Simple'))
            .to.deep.equal(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'streaming', 'Simple'));

        obs.setSetting(outputCategory, 'RecQuality', 'HQ');
        obs.setSetting(outputCategory, 'RecEncoder', 'x264');
        expect(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'recording', 'Simple'))
            .to.include({ keyint_sec: 0, preset: 'veryfast', crf: 23 });

        obs.setSetting(outputCategory, 'UseAdvanced', false);
        expect(osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', 'streaming', 'Simple'))
            .to.include({ bitrate: 3100, preset: 'veryfast', x264opts: '' });
    });

    it('Keeps the configured AMD preset when simple streaming encoders start', async function () {
        const encoderId = 'h264_texture_amf';
        if (osn.VideoEncoderFactory.types().indexOf(encoderId) === -1) this.skip();
        obs.setSetting(outputCategory, 'Mode', 'Simple');
        obs.setSetting(outputCategory, 'StreamEncoder', 'amd');
        obs.setSetting(outputCategory, 'UseAdvanced', true);
        saveSettings({ AMDPreset: 'speed' });
        const settings = osn.NodeObs.OBS_settings_getEncoderSettings(encoderId, 'streaming', 'Simple');
        const encoder = osn.VideoEncoderFactory.create(encoderId, 'simple-amd-preset', settings);
        const audioEncoder = osn.AudioEncoderFactory.create('ffmpeg_aac', 'simple-amd-audio');
        const service = osn.ServiceFactory.create('rtmp_custom', 'simple-amd-service');
        const streaming = osn.SimpleStreamingFactory.create();
        const recording = osn.SimpleRecordingFactory.create();
        try {
            streaming.videoEncoder = encoder;
            streaming.audioEncoder = audioEncoder;
            streaming.service = service;
            streaming.video = obs.defaultVideoContext;
            streaming.useAdvanced = true;
            streaming.enforceServiceBitrate = false;
            recording.path = configPath;
            recording.fileFormat = 'simple-amd-preset';
            recording.overwrite = true;
            recording.video = obs.defaultVideoContext;
            recording.quality = osn.ERecordingQuality.Stream;
            recording.streaming = streaming;
            recording.signalHandler = signal => obs.signals.push(signal);
            recording.start();
            const started = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);
            expect(started.signal, started.error).to.equal(EOBSOutputSignal.Start);
            expect(started.code, started.error).to.equal(0);
            expect(encoder.settings).to.have.property('preset', 'speed');
            await sleep(500);
            recording.stop();
            const wrote = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Wrote);
            expect(wrote.signal, wrote.error).to.equal(EOBSOutputSignal.Wrote);
            expect(wrote.code, wrote.error).to.equal(0);
        } finally {
            osn.SimpleRecordingFactory.destroy(recording);
            osn.SimpleStreamingFactory.destroy(streaming);
            osn.ServiceFactory.destroy(service);
            audioEncoder.release();
            encoder.release();
        }
    });

    for (const outputType of ['streaming', 'recording'] as const) {
        it(`Encodes saved ${outputType} keyframes and track bitrates after settings change`, async function () {
            this.timeout(80000);
            const audioPath = path.join(configPath, `encoder-settings-${outputType}.wav`);
            createTestAudio(audioPath);
            const source = osn.InputFactory.create('ffmpeg_source', 'encoder-settings-audio', {
                local_file: audioPath, is_local_file: true, looping: true,
                restart_on_activate: true, close_when_inactive: false,
            });
            source.audioMixers = 3;
            const scene = osn.SceneFactory.create('encoder-settings-scene');
            const sceneItem = scene.add(source);
            osn.Global.setOutputSource(1, scene);
            const tracks = [osn.AudioTrackFactory.create(160, 'track1'), osn.AudioTrackFactory.create(160, 'track2')];
            tracks.forEach((track, index) => osn.AudioTrackFactory.setAtIndex(track, index + 1));
            try {
                await sleep(500);
                for (const keyint of [1, 2]) {
                    saveEncoderSettings(outputType, {
                        rate_control: 'CBR', bitrate: 2500, keyint_sec: keyint,
                        preset: 'fast', profile: 'high', x264opts: 'scenecut=0',
                    });
                    const bitrates = keyint === 1 ? [320, 160] : [160, 320];
                    saveSettings({ Track1Bitrate: bitrates[0], Track2Bitrate: bitrates[1] });
                    tracks.forEach((track, index) => {
                        track.bitrate = obs.getSetting(outputCategory, `Track${index + 1}Bitrate`);
                    });
                    const settings = osn.NodeObs.OBS_settings_getEncoderSettings('obs_x264', outputType, 'Advanced');
                    const encoder = osn.VideoEncoderFactory.create('obs_x264', `record-${outputType}-${keyint}`, settings);
                    const recording = osn.AdvancedRecordingFactory.create();
                    const streaming = outputType === 'streaming' ? osn.AdvancedStreamingFactory.create() : undefined;
                    recording.path = configPath;
                    recording.fileFormat = `${outputType}-${keyint}`;
                    recording.format = osn.ERecordingFormat.MP4;
                    recording.overwrite = true;
                    recording.video = obs.defaultVideoContext;
                    recording.videoEncoder = encoder;
                    recording.mixer = 3;
                    recording.useStreamEncoders = !!streaming;
                    recording.signalHandler = signal => obs.signals.push(signal);
                    if (streaming) {
                        streaming.videoEncoder = encoder;
                        streaming.video = obs.defaultVideoContext;
                        streaming.enforceServiceBitrate = false;
                        recording.streaming = streaming;
                    }
                    try {
                        expect(encoder.settings).to.include({ keyint_sec: keyint, preset: 'fast', profile: 'high' });
                        recording.start();
                        const started = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Start);
                        expect(started.signal, started.error).to.equal(EOBSOutputSignal.Start);
                        expect(started.code, started.error).to.equal(0);
                        await sleep(6200);
                        recording.stop();
                        const stopped = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Stop);
                        expect(stopped.code, stopped.error).to.equal(0);
                        const wrote = await obs.getNextSignalInfo(EOBSOutputType.Recording, EOBSOutputSignal.Wrote);
                        expect(wrote.signal, wrote.error).to.equal(EOBSOutputSignal.Wrote);
                        expect(wrote.code, wrote.error).to.equal(0);

                        const mediaFile = recording.lastFile();
                        const keyframes = getVideoKeyframes(mediaFile);
                        expect(keyframes.frameRate).to.equal(60);
                        expect(keyframes.duration).to.be.greaterThan(5.5);
                        expect(keyframes.frameCount).to.be.greaterThan(300);
                        expect(keyframes.times.length).to.be.at.least(keyint === 1 ? 6 : 3);
                        const gaps = keyframes.times.slice(1).map((time, index) => time - keyframes.times[index]);
                        expect(Math.max(...gaps)).to.be.at.most(keyint + 1 / 60);
                        // This scene has no cuts: the second recording must also reflect the new interval.
                        expect(Math.max(...gaps)).to.be.at.least(keyint - 1 / 60);
                        const audioStreams = getAudioStreamBitrates(mediaFile);
                        expect(audioStreams.length).to.equal(2);
                        audioStreams.forEach((audio, index) => {
                            expect(audio.codec).to.equal('aac');
                            expect(audio.bitrate).to.be.closeTo(bitrates[index] * 1000, bitrates[index] * 150);
                        });
                    } finally {
                        osn.AdvancedRecordingFactory.destroy(recording);
                        if (streaming) osn.AdvancedStreamingFactory.destroy(streaming);
                        encoder.release();
                    }
                }
            } finally {
                osn.Global.setOutputSource(1, null);
                sceneItem.remove();
                scene.release();
                source.release();
            }
        });
    }
});
