export namespace ObsGlobal {
    function startup(locale: string, path?: string): void;
    function shutdown(): void;
    const status: number;
    const locale: string;
    const version: number;
}

/**
 * Used for various 2-dimensional functions
 * @property {number} x
 * @property {number} y
 */
export interface IVec2 {
    x: number;
    y: number;
}

export class ObsSource {
    release(): void;
    remove(): void;

    readonly status: number;
    readonly id: string;
    readonly configurable: boolean;
    readonly width: number;
    readonly height: number;

    name: string;
}

export class ObsModule {
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

export class ObsFilter extends ObsSource {

}

export class ObsTransition extends ObsSource {
    constructor(id: string, name: string, settings?: object);
    start(ms: number, input: ObsInput | ObsScene): void;
}

export class ObsInput extends ObsSource {
    constructor(id: string, name: string, hotkeys?: object, settings?: object);
    volume: number;
    sync_offset: number;
    showing: boolean;
    flags: number;
    audio_mixers: number;

    readonly filters: ObsFilter[];
}

export class ObsScene extends ObsSource {
    constructor(name: string);
    add(source: ObsInput): ObsSceneItem;
}

enum EOrderMovement {
    MoveUp,
    MoveDown,
    MoveTop,
    MoveBottom
}

export class ObsSceneItem {
    readonly source: ObsInput;
    readonly scene: ObsScene;
    readonly id: number;

    selected: boolean;
    position: IVec2;
    rotation: number;
    scale: IVec2;
    alignment: number;
    bounds_alignment: number;
    bounds: IVec2;
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

export namespace module {
    function add_path(bin_path: string, data_path: string): void;
    function load_all(): void;
    function log_loaded(): void;
}

export namespace video {
    const enum EScaleType {
        Default,
        Point,
        FastBilinear,
        Bilinear,
        Bicubic
    }

    const enum EBoundsType {
        None,
        Stretch,
        ScaleInner,
        ScaleOuter,
        ScaleToWidth,
        ScaleToHeight,
        MaxOnly
    }

    const enum EColorSpace {
        Default,
        CS601,
        CS709
    }

    const enum ERangeType {
        Default,
        Partial,
        Full
    }

    const enum EFormat {
        None,
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

    interface IVideoInfo {
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

    function reset(info: IVideoInfo): void;
}