import { ITimeSpec } from 'obs-studio-node';

const basicOBSInputTypes: string[] = ['image_source', 'color_source', 'slideshow', 'browser_source', 'ffmpeg_source', 'text_gdiplus', 'text_ft2_source', 
                                      'monitor_capture', 'window_capture', 'game_capture', 'dshow_input', 'wasapi_input_capture', 'wasapi_output_capture'];
export {basicOBSInputTypes};

let basicOBSFilterTypes: string[] = ['face_mask_filter', 'mask_filter', 'crop_filter', 'gain_filter', 'color_filter', 'scale_filter', 'scroll_filter', 'gpu_delay',
                                     'color_key_filter', 'clut_filter', 'sharpness_filter', 'chroma_key_filter', 'async_delay_filter', 'noise_suppress_filter',
                                     'noise_gate_filter', 'compressor_filter', 'vst_filter'];
export {basicOBSFilterTypes};

export function getTimeSpec(ms: number): ITimeSpec {
    return {
        sec: Math.floor(ms / 1000),
        nsec: Math.floor(ms % 1000) * 1000000,
    };
}

export function getCppErrorMsg(errorStack: any): string {
    return errorStack.stack.split("\n", 1).join("").substring(7);
}