import { ISettings } from '../osn';

// Filter settings
let mask: ISettings = {
    color: 16777215,
    opacity: 100,
    type: 'mask_color_filter.effect'
}
export {mask};

let crop: ISettings = {
    relative: true
}
export {crop};

let gain: ISettings = {
    db: 0
}
export {gain};

let color: ISettings = {
    brightness: 0,
    color: 16777215,
    contrast: 0,
    gamma: 0,
    hue_shift: 0,
    opacity: 100,
    saturation: 0
}
export {color};

let scale: ISettings = {
    resolution: 'None',
    sampling: 'bicubic',
    undistort: false
}
export {scale};

let scroll: ISettings = {
    cx: 100,
    cy: 100, 
    limit_size: false
}
export {scroll};

let colorKey: ISettings = {
    brightness: 0,
    contrast: 0,
    gamma: 0,
    key_color: 65280,
    key_color_type: 'green',
    opacity: 100,
    similarity: 80,
    smoothness: 50
}
export {colorKey}

let clut: ISettings = {
    clut_amount: 1
}
export {clut};

let sharpness: ISettings = {
    sharpness: 0.08
}
export {sharpness};

let chromaKey: ISettings = {
    brightness: 0,
    contrast: 0,
    gamma: 0,
    key_color: 65280,
    key_color_type: 'green',
    opacity: 100,
    similarity: 400,
    smoothness: 80,
    spill: 100
}
export {chromaKey}

let noiseSuppress: ISettings = {
    suppress_level: -30
}
export {noiseSuppress}

let noiseGate: ISettings = {
    attack_time: 25,
    close_threshold: -32,
    hold_time: 200,
    open_threshold: -26,
    release_time: 150
}
export {noiseGate};

let compressor: ISettings = {
    attack_time: 6,
    output_gain: 0,
    ratio: 10,
    release_time: 60,
    sidechain_source: 'none',
    threshold: -18
}
export {compressor};

let limiter: ISettings = {
    release_time: 60,
    threshold: -6
}
export {limiter};

let expander: ISettings = {
    attack_time: 10,
    detector: 'RMS',
    output_gain: 0,
    presets: 'expander',
    ratio: 2,
    release_time: 50,
    threshold: -40
}
export {expander};

let lumaKey: ISettings = {
    luma_max: 1,
    luma_max_smooth: 0,
    luma_min: 0,
    luma_min_smooth: 0
}
export {lumaKey};

let ndi: ISettings = {
    ndi_filter_ndiname: 'Dedicated NDI Output'
}
export {ndi};

let hdrTonemap: ISettings = {
    hdr_input_maximum_nits: 4000,
    hdr_output_maximum_nits: 1000,
    sdr_white_level_nits: 300,
    transform: 0
}
export { hdrTonemap };

let basicEq: ISettings = {
    low: 0.0,
    mid: 0.0,
    high: 0.0
}
export { basicEq };

let upwardCompressor: ISettings = { "attack_time": 10, "detector": "RMS", "knee_width": 10, "output_gain": 0, "ratio": 0.5, "release_time": 50, "threshold": -20 }
export { upwardCompressor };