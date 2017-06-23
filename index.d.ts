// Type definitions for obs-studio-nan
// Project: obs-studio-nan
// Definitions by: Zachary Lund <admin@computerquip.com>

declare namespace obs {
    export enum EOrderMovement {
        MOVE_UP,
        MOVE_DOWN,
        MOVE_TOP,
        MOVE_BOTTOM
    }

    function startup(locale: string, path?: string): void;
    function shutdown(): void;
    
    const status: number;
    const locale: string;
    const version: number;

    class TVec2 {
        x: number;
        y: number;
    }

    class ISource {
        release(): void;
        remove(): void;

        readonly status: number;
        readonly id: string;
        readonly configurable: boolean;
        readonly width: number;
        readonly height: number;

        name: string;
    }

    class Module {
        constructor(bin_path: string, data_path: string);
        initialize(): void;
        file_name(): string;
        name(): string;
        author(): string;
        description(): string;
        bin_path(): string;
        data_path(): string;
        status(): number;
    }

    class Filter extends ISource {

    }

    class Input extends ISource {
        constructor(id: string, name: string, hotkeys?: object, settings?: object);
        volume: number;
        sync_offset: number;
        showing: boolean;
        flags: number;
        audio_mixers: number;

        readonly filters: Filter[];
    }

    class Scene extends ISource {
        constructor(name: string);
        add(source: Input): SceneItem;
    }

    class SceneItem {
        readonly source: Input;
        readonly scene: Scene;
        readonly id: number;

        selected: boolean;
        position: TVec2;
        rotation: number;
        scale: TVec2;
        alignment: number;
        bounds_alignment: number;
        bounds: TVec2;
        bounds_type: video.EBoundsType;
        scale_filter: video.EScaleType;
        // transform_info: TTransformInfo;
        // crop: TCropInfo;

        order(movement: EOrderMovement): void;
        order_position(pos: number): void;

        remove(): void;
        defer_update_begin(): void;
        defer_update_end(): void;
    }
}

declare namespace obs.module {
    function add_path(bin_path: string, data_path: string): void;
    function load_all(): void;
    function log_loaded(): void;
}

declare namespace obs.video {
    export enum EScaleType {
        DEFAULT,
        POINT,
        FAST_BILINEAR,
        BILINEAR,
        BICUBIC
    }

    export enum EBoundsType {
        NONE,
        STRETCH,
        SCALE_INNER,
        SCALE_OUTER,
        SCALE_TO_WIDTH,
        SCALE_TO_HEIGHT,
        MAX_ONLY
    }

    export enum EColorSpace {
        DEFAULT,
        CS601,
        CS709
    }

    export enum ERangeType {
        DEFAULT,
        PARTIAL,
        FULL
    }

    export enum EFormat {
        NONE,
        I420,
        NV12,
        YVYU,
        YUY2,
        UYVY,
        RGBA,
        BGRA,
        BGRX,
        Y800,
        I444
    }

    export interface IInfo {
        graphics_module: string,
        fps_num: number,
        fps_den: number,
        base_width: number,
        base_height: number,
        output_width: number,
        output_height: number,
        output_format: EFormat,
        adapter: number,
        gpu_conversion: boolean,
        colorspace: EColorSpace,
        range: ERangeType,
        scale_type: EScaleType
    }

    function reset(info: IInfo): void;
}

