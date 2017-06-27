export namespace ObsGlobal {
    /**
     * Initializes libobs global context
     * @param locale The locale used globally
     * @param path Path to libobs data files
     */
    function startup(locale: string, path?: string): void;

    /**
     * Uninitializes libobs global context
     */
    function shutdown(): void;

    /**
     * Current status of the global libobs context
     */
    const status: number;

    /**
     * Current locale of current libobs context
     */
    const locale: string;

    /**
     * Version of current libobs context.
     * Represented as a 32-bit unsigned integer.
     * First 2 bytes are major.
     * Second 2 bytes are minor.
     * Last 4 bytes are patch.
     */
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

/**
 * Base class for Filter, Transition, Scene, and Input.
 * Should not be created directly.
 * 
 * @property {number} status The validitty of the source
 * @property {string} id The corresponding id of the source
 * @property {boolean} configurable Whether the source has properties or not
 * @property {number} width width of the source. 0 if audio only
 * @property {number} height height of the source. 0 if audio only
 * @property {string} name Name of the source when referencing it
 * @property {number} flags Unsigned bit-field concerning various flags.
 * @method release Release the underlying reference
 * @method remove Send remove signal to other holders of the current reference.
 */

export interface ObsSource {
    release(): void;
    remove(): void;

    readonly status: number;
    readonly id: string;
    readonly configurable: boolean;
    readonly width: number;
    readonly height: number;

    name: string;
    flags: number;
}

/**
 * Object representing a filter.
 * 
 * You must call release() before all references run out. 
 */
export class ObsFilter implements ObsSource {
    private constructor();
    static create(id: string, name: string, settings?: object): ObsFilter;

    //Source
    release(): void;
    remove(): void;

    readonly status: number;
    readonly id: string;
    readonly configurable: boolean;
    readonly width: number;
    readonly height: number;

    name: string;
    flags: number;
}

/**
 * Object representing a transition.
 * Should be set to an output at all times. 
 * You must call release() before all references run out. 
 * 
 * @method start Begins a transition into another scene/input source
 */
export class ObsTransition implements ObsSource {
    private constructor();
    static create(id: string, name: string, settings?: object): ObsTransition;
    start(ms: number, input: ObsInput | ObsScene): void;

    //Source
    release(): void;
    remove(): void;

    readonly status: number;
    readonly id: string;
    readonly configurable: boolean;
    readonly width: number;
    readonly height: number;

    name: string;
    flags: number;
}

/**
 * Object representing an input source.
 * An input source can be either an audio or video or even both. 
 * So some of these don't make sense right now. For instance, there's
 * no reason tot call volume on a source that only provides video input. 
 * 
 * You can check for audio/video by using flags() but we'll be adding
 * properties to make this a bit easier. 
 * 
 * You must call release() before all references run out. 
 */
export class ObsInput implements ObsSource {
    private constructor();
    static create(id: string, name: string, hotkeys?: object, settings?: object): ObsInput;
    volume: number;
    sync_offset: number;
    showing: boolean;
    audio_mixers: number;

    readonly filters: ObsFilter[];

    //Source
    release(): void;
    remove(): void;

    readonly status: number;
    readonly id: string;
    readonly configurable: boolean;
    readonly width: number;
    readonly height: number;

    name: string;
    flags: number;
}

/**
 * Object representing a scene.
 * 
 * A scene can also be used as an input source. 
 * You can grab it's input reference by calling source()
 * on an instance of ObsScene. 
 * 
 * You must call release() before all reference run out. 
 */
export class ObsScene implements ObsSource {
    static create(name: string): ObsScene;
    private constructor();
    add(source: ObsInput): ObsSceneItem;

    //Source
    release(): void;
    remove(): void;

    readonly status: number;
    readonly id: string;
    readonly configurable: boolean;
    readonly width: number;
    readonly height: number;

    name: string;
    flags: number;
}

export const enum EOrderMovement {
    MoveUp,
    MoveDown,
    MoveTop,
    MoveBottom
}

/**
 * Object representing an item within a scene. 
 * 
 * When you add an input source to a scene, a few things
 * happen. If the input source provides video, it allocates
 * rendering structures for it. If it provides audio, it 
 * provides audio sampling structures for it. All actual
 * rendering information is held by the scene item. This
 * is so two scene items can be different even if they use
 * the same underlying source. 
 * 
 * Changing any of the properties will change how the 
 * input source is rendered for that particular item.
 */
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
    bounds_type: number;
    scale_filter: number;
    // transform_info: TTransformInfo;
    // crop: TCropInfo;

    order(movement: EOrderMovement): void;
    order_position(pos: number): void;


    defer_update_begin(): void;
    defer_update_end(): void;
}

/**
 * Object representing an entry in a properties list (Properties).
 */
export class ObsProperty {
    status: number;
    name: string;
    description: string;
    long_description: string;
    type: number;
    enabled: boolean;
    visible: boolean;

    /**
     * Uses the current object to obtain the next
     * property in the properties list. 
     * 
     * Check the status property in order to make
     * sure the property is still valid after using.
     */
    next(): void;
}

/**
 * Object representing a list of properties. 
 * 
 * Use .properties method on an encoder, source, output, or service
 * to obtain an instance. 
 */
export class ObsProperties {

    /**
     * Obtains the status of the list.
     */
    status: number;

    /**
     * Obtains the first property in the list.
     */
    first(): ObsProperty;

    /**
     * Obtains property matching name.
     * @param name The name of the property to fetch.
     */
    get(name: string): ObsProperty;

    /**
     * This applies settings associated with the object
     * the settings were obtained from. For instance,
     * if you obtained the settings from an instance
     * of an encoder, it would be applied to that instance.
     * 
     * NOTE: Settings are a WIP
     * @param settings Settings to apply
     */
    apply(settings: object): void;
}


// export class ObsModule {
//     constructor(bin_path: string, data_path: string);
//     initialize(): void;
//     file_name(): string;
//     name(): string;
//     author(): string;
//     description(): string;
//     bin_path(): string;
//     data_path(): string;
//     status(): number;
// }

// /**
//  * @namespace module 
//  * A namespace meant for functions interacting with global context 
//  * conerning modules. 
//  */
// export namespace module {
//     function add_path(bin_path: string, data_path: string): void;
//     function load_all(): void;
//     function log_loaded(): void;
// }

// /**
//  * @namespace video
//  * A namespace meant for functions interacting with global context 
//  * conerning modules. 
//  */
// export namespace video {
//     const enum EScaleType {
//         Default,
//         Point,
//         FastBilinear,
//         Bilinear,
//         Bicubic
//     };

//     const enum EBoundsType {
//         None,
//         Stretch,
//         ScaleInner,
//         ScaleOuter,
//         ScaleToWidth,
//         ScaleToHeight,
//         MaxOnly
//     };

//     const enum EColorSpace {
//         Default,
//         CS601,
//         CS709
//     };

//     const enum ERangeType {
//         Default,
//         Partial,
//         Full
//     };

//     const enum EFormat {
//         None,
//         I420,
//         NV12,
//         YVYU,
//         YUY2,
//         UYVY,
//         RGBA,
//         BGRA,
//         BGRX,
//         Y800,
//         I444
//     };

//     interface IVideoInfo {
//         graphics_module: string,
//         fps_num: number,
//         fps_den: number,
//         base_width: number,
//         base_height: number,
//         output_width: number,
//         output_height: number,
//         output_format: EFormat,
//         adapter: number,
//         gpu_conversion: boolean,
//         colorspace: EColorSpace,
//         range: ERangeType,
//         scale_type: EScaleType
//     };

//     function reset(info: IVideoInfo): void;
// }