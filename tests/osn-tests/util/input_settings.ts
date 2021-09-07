import { ISettings } from '../osn';

// Input settings
let imageSource: ISettings = { 
    unload: false
};
export {imageSource};

let colorSource: ISettings = {
    color: 4294967295,
    height: 400,
    width: 400
};
export {colorSource};

let slideshow: ISettings = {
    loop: true,
    playback_behavior: 'always_play',
    slide_mode: 'mode_auto',
    slide_time: 8000,
    transition: 'fade',
    transition_speed: 700,
    use_custom_size: 'Automatic'
};
export {slideshow};

let browserSource: ISettings = {
    css: 'body { background-color: rgba(0, 0, 0, ' +
    '0); margin: 0px auto; overflow: hidden; ' +
    '}',
    fps: 30,
    fps_custom: false,
    height: 600,
    is_media_flag: false,
    reroute_audio: false,
    restart_when_active: false,
    shutdown: false,
    url: 'https://obsproject.com/browser-source',
    width: 800
};
export {browserSource};

let ffmpegSource: ISettings = {
    buffering_mb: 2,
    caching: false,
    clear_on_media_end: true,
    is_local_file: true,
    linear_alpha: false,
    looping: false,
    restart_on_activate: true,
    speed_percent: 100
};
export {ffmpegSource};

let ndiSource: ISettings = {
    latency: 0,
    ndi_bw_mode: 0,
    ndi_fix_alpha_blending: false,
    ndi_sync: 1,
    yuv_colorspace: 2,
    yuv_range: 1
}
export {ndiSource};

let textGDIPlus: ISettings = {
    align: 'left',
    bk_color: 0,
    bk_opacity: 0,
    chatlog_lines: 6,
    color: 16777215,
    extents_cx: 100,
    extents_cy: 100,
    extents_wrap: true,
    font: { face: 'Arial', size: 36 },
    gradient_color: 16777215,
    gradient_dir: 90,
    gradient_opacity: 100,
    opacity: 100,
    outline_color: 16777215,
    outline_opacity: 100,
    outline_size: 2,
    valign: 'top'
};
export {textGDIPlus};

let textFT2Source: ISettings = {
    color1: 4294967295,
    color2: 4294967295,
    font: { face: 'Arial', size: 32 },
    log_lines: 6
};
export {textFT2Source};

let vlcSource: ISettings =
{
  loop: true,
  network_caching: 400,
  playback_behavior: 'stop_restart',
  shuffle: false,
  subtitle: 1,
  subtitle_enable: false,
  track: 1
}
export {vlcSource};

let monitorCapture: ISettings = { 
    capture_cursor: true,
    monitor: 0
};
export {monitorCapture};

let win32WindowCapture: ISettings = {
    compatibility: false,
    cursor: true
};
export {win32WindowCapture};

let macWindowCapture: ISettings = {
    show_empty_names: false,
    show_shadow: false,
    window: 0
};
export {macWindowCapture};

let gameCapture: ISettings = {
    allow_transparency: false,
    anti_cheat_hook: true,
    auto_capture_rules_path: '',
    auto_fit_to_output: true,
    auto_placeholder_image: "",
    auto_placeholder_message: 'Looking for a game to capture',
    capture_cursor: true,
    capture_mode: 'any_fullscreen',
    capture_overlays: false,
    force_scaling: false,
    hook_rate: 1,
    limit_framerate: false,
    priority: 2,
    scale_res: '0x0',
    sli_compatibility: false,
    user_placeholder_image: '',
    user_placeholder_use: false
};
export {gameCapture};

let simpleCapture: ISettings = {
    auto_capture_rules_path: '',
    auto_placeholder_image: '',
    auto_placeholder_message: 'Looking for a game to capture',
    capture_source_list: 'game:0',
    capture_source_prev: 'game:0',
    user_placeholder_use: false
};
export {simpleCapture};

let dshowInput: ISettings = {
  active: true,
  audio_output_mode: 0,
  color_range: 'default',
  color_space: 'default',
  frame_interval: -1,
  res_type: 0,
  video_format: 0
};
export {dshowInput};

let wasapi: ISettings = {
    device_id: 'default',
    use_device_timing: false
};
export {wasapi};

let avCaptureInput: ISettings = {
    color_space: -1,
    input_format: 4294967295,
    preset: 'AVCaptureSessionPreset1280x720',
    uid: '',
    use_preset: true,
    video_range: -1
};
export {avCaptureInput};

let coreaudio: ISettings = {
    device_id: 'default'
};
export {coreaudio};

let displayCapture: ISettings = {
    crop_mode: 0,
    display: 0,
    show_cursor: true,
    show_empty_names: false,
    window: 0
};
export {displayCapture};