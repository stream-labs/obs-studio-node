import { expect } from 'chai';
import { RtmpTestSession } from './rtmp-test-server';

/** Asserts the exact audio track set, including a sequence header and encoded frames for each track. */
export function expectAudioTracks(session: RtmpTestSession, ids: number[]): void {
    const packets = session.snapshot();
    const label = `RTMP ${session.streamKey}, attempt ${session.attempt}`;
    expect(session.audioTrackIds(), `${label} audio tracks`).to.have.members(ids);
    for (const trackId of ids) {
        expect(packets.some(packet => packet.kind === 'audio' && packet.trackId === trackId && packet.sequenceHeader),
            `${label} audio track ${trackId} header`).to.equal(true);
        expect(packets.some(packet => packet.kind === 'audio' && packet.trackId === trackId && packet.packetType === 1),
            `${label} audio track ${trackId} frames`).to.equal(true);
    }
}

/** Asserts actual encoded video frames; sequence headers and end-of-sequence messages do not count. */
export function expectVideoFrames(session: RtmpTestSession): void {
    expect(session.snapshot().some(packet => packet.kind === 'video' && (packet.packetType === 1 || packet.packetType === 3)),
        `RTMP ${session.streamKey}, attempt ${session.attempt} video frames`).to.equal(true);
}

/** Returns one received AAC-LC track with ADTS framing for decoding by a media probe. */
export function getAacAudioTrack(session: RtmpTestSession, trackId: number): Buffer {
    const packets = session.snapshot().filter(packet => packet.kind === 'audio' && packet.trackId === trackId);
    const config = packets.find(packet => packet.sequenceHeader)?.audioData;
    if (!config || config.length < 2) throw new Error(`Missing AAC configuration for track ${trackId}`);
    const objectType = config[0] >> 3;
    const sampleRateIndex = ((config[0] & 7) << 1) | (config[1] >> 7);
    const channels = (config[1] >> 3) & 15;
    if (objectType !== 2 || sampleRateIndex >= 13 || channels === 0 || channels > 7 || packets.some(packet => packet.codec !== 'mp4a')) {
        throw new Error(`Unsupported AAC configuration for track ${trackId}`);
    }
    const frames = packets.filter(packet => packet.packetType === 1).map(packet => {
        const data = packet.audioData;
        if (!data?.length || data.length + 7 > 8191) throw new Error(`Invalid AAC frame for track ${trackId}`);
        const size = data.length + 7;
        return Buffer.concat([Buffer.from([
            0xff, 0xf1, ((objectType - 1) << 6) | (sampleRateIndex << 2) | (channels >> 2),
            ((channels & 3) << 6) | (size >> 11), (size >> 3) & 255, ((size & 7) << 5) | 31, 0xfc,
        ]), data]);
    });
    if (!frames.length) throw new Error(`Missing AAC frames for track ${trackId}`);
    return Buffer.concat(frames);
}
