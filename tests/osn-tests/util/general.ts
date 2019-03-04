import { ITimeSpec } from 'obs-studio-node';

const basicOBSInputTypes: string[] = ['image_source', 'color_source', 'slideshow', 'browser_source', 'ffmpeg_source', 'text_gdiplus', 'text_ft2_source', 
                                      'monitor_capture', 'window_capture', 'game_capture', 'dshow_input', 'wasapi_input_capture', 'wasapi_output_capture'];
export {basicOBSInputTypes};

let basicOBSFilterTypes: string[] = ['mask_filter', 'crop_filter', 'gain_filter', 'color_filter', 'scale_filter', 'scroll_filter', 'gpu_delay',
                                     'color_key_filter', 'clut_filter', 'sharpness_filter', 'chroma_key_filter', 'async_delay_filter', 'noise_suppress_filter',
                                     'invert_polarity_filter', 'noise_gate_filter', 'compressor_filter', "limiter_filter", 'expander_filter', 'vst_filter'];
export {basicOBSFilterTypes};

let basicOBSTransitionTypes: string[] = ['cut_transition', 'fade_transition', 'swipe_transition', 'slide_transition', 'fade_to_color_transition', 'wipe_transition', 'obs_stinger_transition'];
export {basicOBSTransitionTypes};

let basicOBSSettingsCategories: string[] = ['General', 'Stream', 'Output', 'Audio', 'Video', 'Hotkeys', 'Advanced'];
export {basicOBSSettingsCategories};

export function getTimeSpec(ms: number): ITimeSpec {
    return {
        sec: Math.floor(ms / 1000),
        nsec: Math.floor(ms % 1000) * 1000000,
    };
}

export function getCppErrorMsg(errorStack: any): string {
    return errorStack.stack.split("\n", 1).join("").substring(7);
}

export function deleteConfigFiles(): void {
    const fs = require('fs');
    const path = require('path');
    const configFolderPath = path.join(path.normalize(__dirname), '..', 'osnData/slobs-client');

    fs.readdir(configFolderPath, (error, files) => {
        if (error) throw error;
      
        for (const file of files) {
              fs.unlink(path.join(configFolderPath, file), err => {
              if (error) throw error;
          });
        }
    });
}