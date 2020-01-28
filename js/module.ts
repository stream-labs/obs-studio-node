const obs = require('./obs_studio_client.node');
import * as path from 'path';
import * as fs from 'fs';

/* Convenient paths to modules */
export const DefaultD3D11Path: string = 
    path.resolve(__dirname, `libobs-d3d11.dll`);

export const DefaultOpenGLPath: string = 
    path.resolve(__dirname, `libobs-opengl.dll`);

export const DefaultDrawPluginPath: string = 
    path.resolve(__dirname, `simple_draw.dll`);

export const DefaultBinPath: string = 
    path.resolve(__dirname);

export const DefaultDataPath: string =
    path.resolve(__dirname, `data`);

export const DefaultPluginPath: string = 
    path.resolve(__dirname, `obs-plugins`);

export const DefaultPluginDataPath: string = 
    path.resolve(__dirname, `data/obs-plugins/%module%`);

/**
 * To be passed to Input.flags
 */
export const enum ESourceFlags {
    Unbuffered = (1 << 0),
    ForceMono = (1 << 1)
}

export const enum EMonitoringType {
    None,
    MonitoringOnly,
    MonitoringAndOutput
}

export const enum EOrderMovement {
    Up,
    Down,
    Top,
    Bottom
}

export const enum EDeinterlaceFieldOrder {
    Top,
    Bottom
}

export const enum EVideoCodes {
	Success = 0,
	Fail = -1,
	NotSupported = -2,
	InvalidParam = -3,
	CurrentlyActive = -4,
	ModuleNotFound = -5	
}

export const enum EHotkeyObjectType {
	Frontend = 0,
	Source = 1,
	Output = 2,
	Encoder = 3,
	Service = 4
}

export const enum EDeinterlaceMode {
    Disable,
    Discard,
    Retro,
    Blend,
    Blend2X,
    Linear,
    Linear2X,
    Yadif,
    Yadif2X
}

export const enum EFontStyle {
  Bold = (1<<0),
  Italic = (1<<1),
  Underline = (1<<2),
  Strikeout = (1<<3),
}

/**
 * Enumeration describing the type of a property
 */
export const enum EPropertyType {
    Invalid,
    Boolean,
    Int,
    Float,
    Text,
    Path,
    List,
    Color,
    Button,
    Font,
    EditableList,
    FrameRate
}

export const enum EListFormat {
    Invalid,
    Int,
    Float,
    String
}

export const enum EEditableListType {
    Strings,
    Files,
    FilesAndUrls
}

export const enum EPathType {
    File,
    FileSave,
    Directory
}

export const enum ETextType {
    Default,
    Password,
    Multiline
}

export const enum ENumberType {
    Scroller,
    Slider
}

/**
 * A binary flag representing alignment
 */
export const enum EAlignment {
    Center = 0,
    Left = (1 << 0),
    Right = (1 << 1),
    Top = (1 << 2),
    Bottom = (1 << 3),
    TopLeft = (Top | Left),
    TopRight = (Top | Right),
    BottomLeft = (Bottom | Left),
    BottomRight = (Bottom | Right)
}

/**
 * A binary flag representing output capabilities
 * Apparently you can't fetch these for now (???)
 */
export const enum EOutputFlags {
    Video = (1<<0),
    Audio = (1<<1),
    AV = (Video | Audio),
    Encoded = (1<<2),
    Service = (1<<3),
    MultiTrack = (1<<4)
}

/**
 * A binary flag representing source output capabilities
 */
export const enum ESourceOutputFlags {
    Video = (1 << 0),
    Audio = (1 << 1),
    Async = (1 << 2), 
    AsyncVideo = Async | Video,
    CustomDraw = (1 << 3),
    Interaction = (1 << 5),
    Composite = (1 << 6),
    DoNotDuplicate = (1 << 7),
    Deprecated = (1 << 8),
    DoNotSelfMonitor = (1 << 9)
}

export const enum ESceneDupType {
    Refs,
    Copy,
    PrivateRefs,
    PrivateCopy
}

/**
 * Describes the type of source
 */
export const enum ESourceType {
    Input,
    Filter,
    Transition,
    Scene,
}

/**
 * Describes the type of encoder
 */
export const enum EEncoderType {
    Audio,
    Video
}

/**
 * Describes algorithm type to use for volume representation.
 */
export const enum EFaderType {
    Cubic,
    IEC /* IEC 60-268-18 */,
    Log /* Logarithmic */
}

export const enum EColorFormat {
	Unknown,
	A8,
	R8,
	RGBA,
	BGRX,
	BGRA,
	R10G10B10A2,
	RGBA16,
	R16,
	RGBA16F,
	RGBA32F,
	RG16F,
	RG32F,
	R16F,
	R32F,
	DXT1,
	DXT3,
	DXT5
}

export const enum EZStencilFormat {
	None,
	Z16,
	Z24_S8,
	Z32F,
	Z32F_S8X24
}

export const enum EScaleType {
    Default,
    Point,
    FastBilinear,
    Bilinear,
    Bicubic
}

export const enum ERangeType {
    Default,
    Partial,
    Full
}

export const enum EVideoFormat {
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

export const enum EBoundsType {
    None,
    Stretch,
    ScaleInner,
    ScaleOuter,
    ScaleToWidth,
    ScaleToHeight,
    MaxOnly
}

export const enum EColorSpace {
    Default,
    CS601,
    CS709
}

export const enum ESpeakerLayout {
    Unknown,
    Mono,
    Stereo,
    TwoOne,
    Quad,
    FourOne,
    FiveOne,
    FiveOneSurround,
    SevenOne,
    SevenOneSurround,
    Surround
}

export const enum ESceneSignalType {
    ItemAdd,
    ItemRemove,
    Reorder,
    ItemVisible,
    ItemSelect,
    ItemDeselect,
    ItemTransform
}

export const enum EOutputCode {
    Success = 0,
    BadPath = -1,
    ConnectFailed = -2,
    InvalidStream = -3,
    Error = -4,
    Disconnected = -5,
    Unsupported = -6,
    NoSpace = -7,
    EncoderError = -8,
    OutdatedDriver = -65,
}

export const enum ECategoryTypes {
    NODEOBS_CATEGORY_LIST = 0,
	NODEOBS_CATEGORY_TAB = 1
}

export const enum ERenderingMode {
    OBS_MAIN_RENDERING = 0,
	OBS_STREAMING_RENDERING = 1,
	OBS_RECORDING_RENDERING = 2
}

export const Global: IGlobal = obs.Global;
export const Video: IVideo = obs.Video;
export const OutputFactory: IOutputFactory = obs.Output;
export const AudioEncoderFactory: IAudioEncoderFactory = obs.AudioEncoder;
export const VideoEncoderFactory: IVideoEncoderFactory = obs.VideoEncoder;
export const ServiceFactory: IServiceFactory = obs.Service;
export const InputFactory: IInputFactory = obs.Input;
export const SceneFactory: ISceneFactory = obs.Scene;
export const FilterFactory: IFilterFactory = obs.Filter;
export const TransitionFactory: ITransitionFactory = obs.Transition;
export const DisplayFactory: IDisplayFactory = obs.Display;
export const VolmeterFactory: IVolmeterFactory = obs.Volmeter;
export const FaderFactory: IFaderFactory = obs.Fader;
export const AudioFactory: IAudioFactory = obs.Audio;
export const ModuleFactory: IModuleFactory = obs.Module;
export const IPC: IIPC = obs.IPC;

/**
 * Meta object in order to better describe settings
 */
export interface ISettings {
    [key: string]: any;
}

/**
 * Used for various 2-dimensional functions
 */
export interface IVec2 {
    readonly x: number;
    readonly y: number;
}

/**
 * Used to represented a time in nanoseconds
 * JS can't hold 64-bit integers thus can
 * easily overflow when representing time in ns.
 */
export interface ITimeSpec {
    readonly sec: number;
    readonly nsec: number;
}

/**
 * Interface describing the transform information in an item
 */
export interface ITransformInfo {
    readonly pos: IVec2;
    readonly rot: number;
    readonly scale: IVec2;
    readonly alignment: EAlignment;
    readonly boundsType: EBoundsType;
    readonly boundsAlignment: number;
    readonly bounds: IVec2;
}

/**
 * Interface describing the crop of an item.
 */
export interface ICropInfo {
    readonly left: number;
    readonly right: number;
    readonly top: number;
    readonly bottom: number;
}

export interface IVideoInfo {
    readonly graphicsModule: string;
    readonly fpsNum: number;
    readonly fpsDen: number;
    readonly baseWidth: number;
    readonly baseHeight: number;
    readonly outputWidth: number;
    readonly outputHeight: number;
    readonly outputFormat: EVideoFormat;
    readonly adapter: number;
    readonly gpuConversion: boolean;
    readonly colorspace: EColorSpace;
    readonly range: ERangeType;
    readonly scaleType: EScaleType;
}

export interface IAudioInfo {
    readonly samplesPerSec: number;
    readonly speakerLayout: ESpeakerLayout;
}

export interface IDisplayInit {
    width: number;
    height: number;
    format: EColorFormat;
    zsformat: EZStencilFormat;
}

/** 
 * Namespace representing the global libobs functionality
 */

export interface IIPC {
    /**
     * Set the path and optionally working directory for the IPC server binary.
     * @param binaryPath - Path to the binary file to be executed
     * @param workingDirectoryPath - Path to the directory where it is executed in.
	 * @throws SyntaxError if an invalid number of parameters is given.
	 * @throws TypeError if a parameter is of invalid type.
     */
	setServerPath(binaryPath: string, workingDirectoryPath?: string): void;
	
    /**
     * Connect to an existing server.
     * @param uri - URI for the server.
	 * @throws SyntaxError if an invalid number of parameters is given.
	 * @throws TypeError if a parameter is of invalid type.
	 * @throws Error if it failed to connect.
     */
	connect(uri: string): void;
	
    /**
     * Hosts a new server and connects to it.
     * @param uri - URI for the server.
	 * @throws SyntaxError if an invalid number of parameters is given.
	 * @throws TypeError if a parameter is of invalid type.
	 * @throws Error if it failed to host and connect.
     */
	host(uri: string): void;
	
    /**
     * Disconnect from a server.
     */
	disconnect(): void;
}
 
export interface IGlobal {
    /**
     * Initializes libobs global context
     * @param locale - Locale to be used within libobs
     * @param path - Data path of libobs
     */
    startup(locale: string, path?: string): void;

    /**
     * Uninitializes libobs global context
     */
    shutdown(): void;

    /**
     * @param id - String ID of the source
     * @returns - The output flags (capabilities) of the source type
     */
    getOutputFlagsFromId(id: string): number;

    /**
     * Output channels are useful in that we can attach multiple
     * sources for output. For the most part, you're generally only
     * going to use one channel for video. However, if you so wanted, 
     * you could assign more to be layered on top of other channels. 
     * 
     * This also accepts audio input sources which are automatically 
     * mixed into the audio output. This means you can have a standalone
     * input source that isn't attached to the scene being rendered. 
     * @param channel - The output channel to assign source
     * @param input - The source to assign to the output channel
     */
    setOutputSource(channel: number, input: ISource): void;

    /**
     * Obtains the source associated with a given output channel
     * @param channel - The output channel to fetch source of
     * @returns - The associated source or null if none was assigned to the given channel or channel was invalid.
     */
    getOutputSource(channel: number): ISource;

    /**
     * Number of total render frames
     */
    readonly totalFrames: number;

    /**
     * Number of total lost frames due to being short 
     * of rendering time.
     */
    readonly laggedFrames: number;

    /**
     * Current status of the global libobs context
     */
    readonly initialized: boolean;

    /**
     * Current locale of current libobs context
     */
    locale: string;

    /**
     * Rendering of current libobs context
     */
    multipleRendering: boolean;

    /**
     * Version of current libobs context.
     * Represented as a 32-bit unsigned integer.
     * First 2 bytes are major.
     * Second 2 bytes are minor.
     * Last 4 bytes are patch.
     */
    readonly version: number;
}

export interface IBooleanProperty extends IProperty {

}

export interface IColorProperty extends IProperty {

}

export interface IButtonProperty extends IProperty {
    /**
     * @param source An object containing context
     * used by the plugin. This is always the source
     * associated with the property. Right now, I 
     * just accept a generic object for forward
     * compatibility. 
     */
    buttonClicked(source: object): void;
}

export interface IFontProperty extends IProperty {

}

export interface IListProperty extends IProperty {
    readonly details: IListDetails;
}

export interface IListDetails {
    readonly format: EListFormat;

    /**
     * A list of options to be made available within the list.
     * You can determine if it's a string or number by testing
     * {@link IListProperty#format}
     */
    readonly items: { name: string, value: string | number }[];
}

export interface IEditableListProperty extends IProperty {
    readonly details: IEditableListDetails;
}

export interface IEditableListDetails extends IListDetails {
    readonly type: EEditableListType;

     /** String describing allowed valued */
    readonly filter: string;

    /** Default value for the editable box */
    readonly defaultPath: string;
}

export interface IPathProperty extends IProperty {
    readonly details: IPathDetails;
}

export interface IPathDetails {
    readonly type: EPathType;
    readonly filter: string;
    readonly defaultPath: string;
}

export interface ITextProperty extends IProperty {
    readonly details: ITextDetails;
}

export interface ITextDetails {
    readonly type: ETextType;
}

export interface INumberProperty extends IProperty {
    readonly details: INumberDetails;
}

export interface INumberDetails {
    readonly type: ENumberType;
    readonly min: number;
    readonly max: number;
    readonly step: number;
}

/**
 * Class representing an entry in a properties list (Properties).
 */
export interface IProperty {
    /** The validity of the current property instance */
    readonly status: number;

    /**
     * The name associated with the property
     * You can use this name to fetch from the source
     * settings object (see {@link ObsSource#settings})
     * if you need the value set on this property. 
     */
    readonly name: string;

    /** A short description of the property */
    readonly description: string;

    /** A long detailed description of the property */
    readonly longDescription: string;

    /** Whether or not the property is enabled */
    readonly enabled: boolean;

    /** Whether or not the property should be made visible */
    readonly visible: boolean;

    /** Type of the property */
    readonly type: EPropertyType;

    /** Current value of the property */
    readonly value: any;

    /**
     * Uses the current object to obtain the next
     * property in the properties list.
     * 
     * @returns If it's successful, returns true.
     * Otherwise or if end of the list, returns false. 
     */
    next(): IProperty;
    modified(): boolean;
}

/**
 * Object representing a list of properties. 
 * 
 * Use .properties member on an encoder, source, output, or service
 * to obtain an instance.
 */
export interface IProperties {

    /** Obtains the status of the list */
    readonly status: number;

    /** Obtains the first property in the list. */
    first(): IProperty;

    count(): number;

    /**
     * Obtains property matching name.
     * @param name The name of the property to fetch.
     * @returns - The property instance or null if not found
     */
    get(name: string): IProperty;
}

export interface IFactoryTypes {
    types(): string[];
}

export interface IReleasable {
    release(): void;
}

export interface IEncoder extends IConfigurable, IReleasable {
    name: string;
    readonly id: string;
    readonly type: EEncoderType;
    readonly caps: number;
    readonly codec: string;
    readonly active: boolean;
}

export interface IVideoEncoderFactory extends IFactoryTypes {
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): IVideoEncoder;
    fromName(name: string): IVideoEncoder;
}

export interface IVideoEncoder extends IEncoder {
    setVideo(video: IVideo): void;
    getVideo(): IVideo;

    getHeight(): number;
    getWidth(): number;

    setScaledSize(width: number, height: number): void;
    setPreferredFormat(format: EVideoFormat): void;
    getPreferredFormat(): EVideoFormat;
}

export interface IAudioEncoderFactory extends IFactoryTypes {
    create(id: string, name: string, settings?: ISettings, track?: number, hotkeys?: ISettings): IAudioEncoder;
    fromName(name: string): IAudioEncoder;
}

export interface IAudioEncoder extends IEncoder {
    setAudio(video: IAudio): void;
    getAudio(): IAudio;

    getSampleRate(): number;
}

export interface IOutput extends IConfigurable, IReleasable {
    setMedia(video: IVideo, audio: IAudio): void;
    getVideo(): IVideo;
    getAudio(): IAudio;

    /** 32-bit integer representing mixer track.
      * Note that the backend treats this as size_t.
      * The bindings only accept a 32-bit integer
      * to prevent complication.  */
    mixer: number;

    getVideoEncoder(): IVideoEncoder;
    setVideoEncoder(encoder: IVideoEncoder): void;

    getAudioEncoder(idx: number): IAudioEncoder;
    setAudioEncoder(encoder: IAudioEncoder, idx: number): void;

    service: IService;

    setReconnectOptions(retry_count: number, retry_sec: number): void;
    setPreferredSize(width: number, height: number): void;

    readonly width: number;
    readonly height: number;

    /** Statistic from 0.0f to 1.0f 
      * representing congestion */
    readonly congestion: number;

    /** Time output spent connecting its service */
    readonly connectTime: number;

    /** If it's in the state of reconnecting */
    readonly reconnecting: boolean;

    /** 
      * This will return a list of supported codec
      * standards, not the implementations. For instance
      * rtmp_output will return 'h264'.
      *
      * You may fetch what codecs are implemented by
      * iterating through the video encoders.
      */
    readonly supportedVideoCodecs: string[];

    /** 
      * This will return a list of supported codec
      * standards, not the implementations. For instance
      * rtmp_output will return 'aac'.
      *
      * You may fetch what codecs are implemented by
      * iterating through the audio encoders.
      */
    readonly supportedAudioCodecs: string[];

    /** Number of frames dropped total */
    readonly framesDropped: number;

    /** Total number of processed frames, 
      * including dropped frames */
    readonly totalFrames: number;

    /** Start outputing data. Please 
      * note that this doesn't mean the
      * output starts immediately. In order
      * to determine when the output 
      * actually starts, you must use signals.
      * libobs will provide a start signal
      * when it's actually started. */
    start(): void;

    /** Stop outputing data. Please 
      * note that this doesn't mean the
      * output stops immediately. In order
      * to determine when the output 
      * actually starts, you must use signals.
      * libobs will provide a stop signal
      * when it's actually stopped. */
    stop(): void;

    /** Set delay to output */
    setDelay(ms: number, flags: EDelayFlags): void;
    getDelay(): void;
    getActiveDelay(): void;
}

export interface IOutputFactory extends IFactoryTypes {
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): IOutput;
    fromName(name: string): IOutput;
}

export enum EDelayFlags {
    PreserveDelay = (1<<0)
}

/** 
 * A service represents the service we're generally streaming to.
 * Note that in order to use a service, you need an output that takes
 * advantage of a service. This makes sense since not all outputs will
 * use a service (think recording or NDI). */
export interface IServiceFactory extends IFactoryTypes {
    /**
     * Create a new instance of a Service
     * @param id - The type of service source to create, possibly from {@link types}
     * @param name - Name of the created input source
     * @param settings - Optional, settings to create input sourc with
     * @param hotkeys - Optional, hotkey data associated with input
     * @returns - Returns instance or null if failure
     */
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): IService;

    /**
     * Create a new instance of a Service that's private
     * Private in this context means any function that returns an 
     * Service will not return this source
     * @param id - The type of input source to create, possibly from {@link types}
     * @param name - Name of the created input source
     * @param settings - Optional, settings to create input source with
     * @returns - Returns instance or null if failure
     */
    createPrivate(id: string, name: string, settings?: ISettings): IService;

    fromName(name: string): IService;
}

export interface IService extends IConfigurable, IReleasable {
    readonly url: string;
    readonly key: string;
    readonly username: string;
    readonly password: string;
    readonly name: string;
    readonly id: string;
}

export interface IFilterFactory extends IFactoryTypes {
    /**
     * Create an instance of an ObsFilter
     * @param id - ID of the filter, possibly returned from types()
     * @param name - Name of the filter
     * @param settings - Optional, settings to create the filter with
     * @returns - Created instance of ObsFilter or null if failure
     */
    create(id: string, name: string, settings?: ISettings): IFilter;
}

/**
 * Class representing a filter
 */
export interface IFilter extends ISource {
}

export interface IInputFactory extends IFactoryTypes {
    /**
     * Create a new instance of an ObsInput
     * @param id - The type of input source to create, possibly from {@link types}
     * @param name - Name of the created input source
     * @param settings - Optional, settings to create input sourc with
     * @param hotkeys - Optional, hotkey data associated with input
     * @returns - Returns instance or null if failure
     */
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): IInput;

    /**
     * Create a new instance of an ObsInput that's private
     * Private in this context means any function that returns an 
     * ObsInput will not return this source
     * @param id - The type of input source to create, possibly from {@link types}
     * @param name - Name of the created input source
     * @param settings - Optional, settings to create input source with
     * @returns - Returns instance or null if failure
     */
    createPrivate(id: string, name: string, settings?: ISettings): IInput;

    /**
     * Create an instance of an ObsInput by fetching the source by name.
     * @param name - Name of the source to look for
     * @returns - Returns instance or null if it failed to find the source
     */
    fromName(name: string): IInput;

    /**
     * Fetches a list of all public input sources available.
     */
    getPublicSources(): IInput[];
}


export const enum EInteractionFlags {
	None         = 0,
	CapsKey      = 1,
	ShiftKey     = 1 << 1,
	ControlKey   = 1 << 2,
	AltKey       = 1 << 3,
	MouseLeft    = 1 << 4,
	MouseMiddle  = 1 << 5,
	MouseRight   = 1 << 6,
	CommandKey   = 1 << 7,
	Numlock_Key  = 1 << 8,
	IsKeyPad     = 1 << 9,
	IsLeft       = 1 << 10,
	IsRight      = 1 << 11
};

export const enum EMouseButtonType {
	Left,
	Middle,
	Right
};

export interface IMouseEvent {
	modifiers: EInteractionFlags;
	x: number;
	y: number;
};

export interface IKeyEvent {
	modifiers: EInteractionFlags;
	text: string;
	nativeModifiers: number;
	nativeScancode: number;
	nativeVkey: number;
};

export interface ISceneItemInfo {
    name: string,
    crop: ICropInfo,
    scaleX: number,
    scaleY: number,
    visible: boolean,
    x: number,
    y: number,
    rotation: number
}

/**
 * Class representing a source
 * 
 * An input source can be either an audio or video or even both. 
 * So some of these don't make sense right now. For instance, there's
 * no reason tot call volume on a source that only provides video input. 
 */
export interface IInput extends ISource {
    volume: number;
    syncOffset: ITimeSpec;
    showing: boolean;
    audioMixers: number;
    monitoringType: EMonitoringType;
    deinterlaceFieldOrder: EDeinterlaceFieldOrder;
    deinterlaceMode: EDeinterlaceMode;

    /**
     * Create a new instance using the current instance. 
     * If no parameters are provide, an instance is created
     * using the current instance as if it were new. 
     * @param name - Name of new source
     * @param isPrivate - Whether or not the new source is private
     */
    duplicate(name?: string, isPrivate?: boolean): IInput;

    /**
     * Find a filter associated with the input source by name.
     * @param name - Name of filter to find
     * @returns - Returns the filter instance or null if it couldn't find the filter
     */
    findFilter(name: string): IFilter;

    /**
     * Attach a filter instance to this input source
     * @param filter - The filter instance to attach to this input source.
     */
    addFilter(filter: IFilter): void;

    /**
     * Remove a filter instance from this input source
     * @param filter - The filter instance to remove from this input source.
     */
    removeFilter(filter: IFilter): void;

    sendMouseClick(eventData: IMouseEvent, type: EMouseButtonType, mouseUp: boolean, clickCount: number): void
    sendMouseMove(eventData: IMouseEvent, mouseLeave: boolean): void;
    sendMouseWheel(eventData: IMouseEvent, x_delta: number, y_delta: number): void;
    sendFocus(focus: boolean): void;
    sendKeyClick(eventData: IKeyEvent, keyUp: boolean): void;

    /**
     * Move a filter up, down, top, or bottom in the filter list.
     * @param filter - The filter to move within the input source.
     * @param movement - The movement to make within the list.
     */
    setFilterOrder(filter: IFilter, movement: EOrderMovement): void;

    /**
     * Move a filter up, down, top, or bottom in the filter list.
     * @param filter - The filter to move within the input source.
     * @param movement - The movement to make within the list.
     */
    setFilterOrder(filter: IFilter, movement: EOrderMovement): void;


    /**
     * Obtain a list of all filters associated with the input source
     */
    readonly filters: IFilter[];

    /**
     * Width of the underlying source
     */
    readonly width: number;

    /**
     * Height of the underlying source
     */
    readonly height: number;
}

export interface ISceneFactory {
    /**
     * Create a new scene instance
     * @param name - Name of the scene to create
     * @returns - Returns the instance or null on failure
     */
    create(name: string): IScene;

    /**
     * Create a new scene instance that's private
     * @param name - Name of the scene to create
     * @returns - Returns the instance or null on failure
     */
    createPrivate(name: string): IScene;

    /**
     * Create a new scene instance by fetching it by name
     * @param name - Name of the scene to look for
     * @returns - Returns the instance or null on failure to find the scene
     */
    fromName(name: string): IScene;
}

/**
 * Class representing a scene
 */
export interface IScene extends ISource {
    /**
     * Create a new instance of a scene using the current scene
     * @param name - New name of the duplicated scene
     * @param type - Method of scene item duplication
     */
    duplicate(name: string, type: ESceneDupType): IScene;

    /**
     * Add an input source to the scene, creating a scene item.
     * @param source - Input source to add to the scene
     * @returns - Return the sceneitem or null on failure
     */
    add(source: IInput, transform?: ISceneItemInfo): ISceneItem;
    
    /**
     * A scene may be used as an input source (even though its type
     * will still be of the value Scene). To use a scene as an ObsInput,
     * simply fetch this property.
     */
    readonly source: IInput;

    /**
     * Orders an item from the old index to the new index (from bottom to top)
     * @param oldIndex - Item index
     * @param newIndex - Index where you want to move item
     */
    moveItem(oldIndex: number, newIndex: number): void;

    /**
     * Find an item within a scene
     * 
     * @param id - A string representing the name of the
     * underlying source of the item to search for or an integer specifying 
     * the id assigned to the item when it was created.
     * 
     * @returns - The found item instance or null
     */
    findItem(id: string | number): ISceneItem;

    /**
     * Find an item within a scene by index
     * 
     * @param idx - An integer representing the index the item sits at within the scene
     * @returns - The item instance or null if the index was bad
     */
    getItemAtIdx(idx: number): ISceneItem;

    /**
     * Fetch all items within the scene
     * @returns - The array of item instances
     */
    getItems(): ISceneItem[];

    /**
     * Connect a callback to a particular signal 
     * associated with this scene. 
     */
    connect(sigType: ESceneSignalType, cb: (info: ISettings) => void): ICallbackData;

    /**
     * Disconnect the signal registered with connect()
     */
    disconnect(data: ICallbackData): void;
}

/**
 * Class representing an item within a scene. 
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
export interface ISceneItem {
    /** The underlying input source associated with this item */
    readonly source: IInput;

    /** The scene this item is in */
    readonly scene: IScene;

    /** The id assigned to this item when its created */
    readonly id: number;

    /** A flag determining whether the item is selected */
    selected: boolean;

    /** Position of the item */
    position: IVec2;

    /** Rotation of the in degrees */
    rotation: number;

    /** Scale of the item, with 1 being to scale */
    scale: IVec2;

    alignment: EAlignment;
    boundsAlignment: number;
    bounds: IVec2;

    /** How to apply bounds */
    boundsType: EBoundsType;

    /** How to apply scale */
    scaleFilter: EScaleType;

    /** Whether or not the item is visible */
    visible: boolean;

    /** Whether or not the item is visible on the streaming output */
    streamVisible: boolean;

    /** Whether or not the item is visible on the recording output */
    recordingVisible: boolean;

    /**
     * Transform information on the item packed into
     * a single convenient object
     */
    readonly transformInfo: ITransformInfo;

    /** Current crop applied to the item */
    crop: ICropInfo;

    /** Move the item towards the top-most item one spot */
    moveUp(): void;

    /** Move the item towards the bottom-most item one spot */
    moveDown(): void;

    /** Make the item the top-most item */
    moveTop(): void;

    /** Make the item the bottom-most item */
    moveBottom(): void;

    /**
     * Move the item to the specified position.
     * @param position Position relative to the bottom-most item
     */
    move(position: number): void;

    /** Remove the item from the scene it's attached to (destroys the item!) */
    remove(): void;

    /** Prevent updating of the item to prevent data races */
    deferUpdateBegin(): void;

    /** Allow updating of the item after calling {@link deferUpdateBegin} */
    deferUpdateEnd(): void;
}

export interface ITransitionFactory extends IFactoryTypes {
    /**
     * Create a new instance of an ObsTransition
     * @param id - The type of transition source to create, possibly from {@link types}
     * @param name - Name of the created transition source
     * @param settings - Optional, settings to create transition source with
     * @param hotkeys - Optional, hotkey data associated with transition
     * @returns - Returns instance or null if failure
     */
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): ITransition;

    /**
     * Create a new instance of an ObsTransition that's private
     * Private in this context means any function that returns an 
     * ObsTransition will not return this source
     * @param id - The type of transition source to create, possibly from {@link types}
     * @param name - Name of the created inptransitionut source
     * @param settings - Optional, settings to create transition source with
     * @returns - Returns instance or null if failure
     */
    createPrivate(id: string, name: string, settings?: ISettings): ITransition;

    fromName(name: string): ITransition;
}

/**
 * Class representing a transition
 */
export interface ITransition extends ISource {
    /**
     * Returns a list of available filter types for creation
     */

    /**
     * Obtain currently set input source.
     */
    getActiveSource(): ISource;

    /**
     * Clear the currently set input source
     */
    clear(): void;

    /**
     * Set a new input without transitioning.
     * @param input - Source to transition to
     */
    set(input: ISource): void;

    /**
     * Begins a transition into another scene/input source
     * @param ms - Length of time transition to new scene should take
     * @param input - Source to transition to
     */
    start(ms: number, input: ISource): void;
}

export interface IConfigurable { 
    /**
     * Update the settings of the source instance
     * correlating to the values held within the
     * object passed. 
     */
    update(settings: ISettings): void;

    /**
     * Whether the source has properties or not
     */
    readonly configurable: boolean;

    /**
     * The properties of the source
     */
    readonly properties: IProperties;

    /**
     * Object holding current settings of the source
     */
    readonly settings: ISettings;
}

/**
 * Base class for Filter, Transition, Scene, and Input
 */
export interface ISource extends IConfigurable, IReleasable {
    /**
     * Send remove signal to other holders of the current reference.
     */
    remove(): void;

    /**
     * Send a save signal to sources themselves. 
     * This should always be called before saving to disk 
     * as it allows the source to know it needs to update 
     * its settings.
     */
     save(): void;

    /**
     * The validity of the source
     */
    readonly status: number;

    /**
     * Type of the source
     */
    readonly type: ESourceType;

    /**
     * The corresponding id of the source
     */
    readonly id: string;
    
    /**
     * Not to be confused with flags. This set
     * of flags provides the capabilities in the
     * output associated with the source. See
     * EOutputFlags for possible options. Is
     * represented as 32-bit binary flag.
     */
    readonly outputFlags: ESourceOutputFlags;

    /**
     * Name of the source when referencing it
     */
    name: string;

    /**
     * Unsigned bit-field concerning various flags
     */
    flags: ESourceFlags;

    /** 
     * Muted flag, separate of the current volume
     */
    muted: boolean;

    /**
     * Whether or not the source is disabled.
     * Easy way to disable a filter.
     */
    enabled: boolean;
}

export interface IFaderFactory {
    /**
     * Create an instance of a fader object
     * @param type - What algorithm to use for new fader.
     */
    create(type: EFaderType): IFader;
}

/**
 * Class representing a fader control corresponding to a source.
 */
export interface IFader {
    /**
     * Negative float representing volume using decibels.
     */
    db: number;

    /**
     * Percentage representing level of volume from 0% to 100%
     */
    deflection: number;

    /**
     * Multiplier representing volume levels
     */
    mul: number;

    /**
     * Attach to a source to monitor the volume of
     * @param source Input source to attach to
     */
    attach(source: IInput): void;

    /**
     * Detach from currently attached source.
     * Otherwise, is a no-op.
     */
    detach(): void;

    /**
     * Add a callback to the fader. Callback will be called
     * each time volume associated with the attached source changes. 
     * @param cb - A callback that occurs when volume changes.
     */
    addCallback(cb: (db: number) => void): ICallbackData;

    /**
     * Remove a callback to prevent events from occuring immediately. 
     * @param cbData - Object passed back from a call to {@link ObsFader#addCallback}
     */
    removeCallback(cbData: ICallbackData): void;
}

export interface IVolmeterFactory {
    /**
     * Create an instance of a volmeter object
     * @param type - What algorithm to use for new fader.
     */
    create(type: EFaderType): IVolmeter;
}

/**
 * Object representing a volmeter control corresponding to a source.
 */
export interface IVolmeter {
    /**
     * The interval at which the volmeter will call the callback.
     */
    updateInterval: number;

    /**
     * Attaches to the volmeter object to a source
     * @param source Source to monitor the volume of
     */
    attach(source: IInput): void;

    /**
     * Detaches the currently attached source from the volmeter object
     */
    detach(): void;

    /**
     * Add a callback to the volmeter. Callback will be called
     * each time volume associated with the attached source changes. 
     * @param cb - A callback that occurs when volume changes.
     */
    addCallback(
        cb: (magnitude: number[],
             peak: number[],
             inputPeak: number[]) => void): ICallbackData;

    /**
     * Remove a callback to prevent events from occuring immediately. 
     * @param cbData - Object passed back from a call to {@link ObsVolmeter#addCallback}
     */
    removeCallback(cbData: ICallbackData): void;
}

/**
 * This is simply used to type check
 * objects passed back that hold internal
 * information when dealing with callbacks.
 */
export interface ICallbackData {
}

export interface IDisplayFactory {
    create(source?: IInput): IDisplay;
}

export interface IDisplay {
    destroy(): void;

    setPosition(x: number, y: number): void;
    getPosition(): IVec2;

    setSize(x: number, y: number): void;
    getSize(): IVec2;

    getPreviewOffset(): IVec2;
    getPreviewSize(x: number, y: number): void;

    shouldDrawUI: boolean;
    paddingSize: number;

    setPaddingColor(r: number, g: number, b: number, a: number): void;
    setBackgroundColor(r: number, g: number, b: number, a: number): void;
    setOutlineColor(r: number, g: number, b: number, a: number): void;
    setGuidelineColor(r: number, g: number, b: number, a: number): void;
    setResizeBoxOuterColor(r: number, g: number, b: number, a: number): void;
    setResizeBoxInnerColor(r: number, g: number, b: number, a: number): void;
}

/**
 * This represents a video_t structure from within libobs
 * For now, only the global context functions are implemented
 */
export interface IVideo {
	
	/**
     * Number of total skipped frames
     */
    readonly skippedFrames: number;
	
    /**
     * Number of total encoded frames
     */
    readonly encodedFrames: number;
}

/**
 * This represents a audio_t structure from within libobs
 * For now, only the global context functions are implemented
 */
export interface IAudio {

}

export interface IAudioFactory {
    reset(info: IAudioInfo): boolean;
    getGlobal(): IAudio;
}


export interface IModuleFactory extends IFactoryTypes {
    open(binPath: string, dataPath: string): IModule;
    loadAll(): void;
    addPath(path: string, dataPath: string): void;
    logLoaded(): void;
    modules(): String[];
}

export interface IModule {
    initialize(): void;
    filename(): string;
    name(): string;
    author(): string;
    description(): string;
    binPath(): string;
    dataPath(): string;
    status(): number;
}

export function addItems(scene: IScene, sceneItems: ISceneItemInfo[]): ISceneItem[] {
    const items: ISceneItem[] = [];
    if (Array.isArray(sceneItems)) {
        sceneItems.forEach(function(sceneItem) {
            const source = obs.Input.fromName(sceneItem.name);
            const item = scene.add(source, sceneItem);
            items.push(item);
        });
    }
    return items;
}
export interface FilterInfo {
    name: string,
    type: string,
    settings: ISettings,
    enabled: boolean
}
export interface SyncOffset {
    sec: number,
    nsec: number
}
export interface SourceInfo {
    filters: FilterInfo[],
    muted: boolean,
    name: string,
    settings: ISettings,
    type: string,
    volume: number,
    syncOffset: SyncOffset
}
export function createSources(sources: SourceInfo[]): IInput[] {
    const items: IInput[] = [];
    if (Array.isArray(sources)) {
        sources.forEach(function (source) {
            const newSource = obs.Input.create(source.type, source.name, source.settings);
            if (newSource.audioMixers) {
                newSource.muted = (source.muted != null) ? source.muted : false;
                newSource.volume = (source.volume != null) ? source.volume : 1;
                newSource.syncOffset =
                (source.syncOffset != null) ? source.syncOffset : {sec: 0, nsec: 0};
            }
            items.push(newSource);
            const filters = source.filters;
            if (Array.isArray(filters)) {
                    filters.forEach(function (filter) {
                    const ObsFilter = obs.Filter.create(filter.type, filter.name, filter.settings);
                    ObsFilter.enabled = (filter.enabled != null) ? filter.enabled : true;
                    newSource.addFilter(ObsFilter);
                    ObsFilter.release();
                });
            }
        });
    }
    return items;
}
export interface ISourceSize {
    name: string,
    width: number,
    height: number,
    outputFlags: number,
}
export function getSourcesSize(sourcesNames: string[]): ISourceSize[] {
    const sourcesSize: ISourceSize[] = [];
    if (Array.isArray(sourcesNames)) {
        sourcesNames.forEach(function (sourceName) {
            const ObsInput = obs.Input.fromName(sourceName);
            if(ObsInput) {
                sourcesSize.push({ name: sourceName, height: ObsInput.height, width: ObsInput.width, outputFlags: ObsInput.outputFlags });
            }
        });
    }
    return sourcesSize;
}

// Initialization and other stuff which needs local data.
if (fs.existsSync(path.resolve(__dirname, `obs64.exe`).replace('app.asar', 'app.asar.unpacked'))) {
	obs.IPC.setServerPath(path.resolve(__dirname, `obs64.exe`).replace('app.asar', 'app.asar.unpacked'), path.resolve(__dirname).replace('app.asar', 'app.asar.unpacked'));
} else {
	obs.IPC.setServerPath(path.resolve(__dirname, `obs32.exe`).replace('app.asar', 'app.asar.unpacked'), path.resolve(__dirname).replace('app.asar', 'app.asar.unpacked'));
}
export const NodeObs = obs;
