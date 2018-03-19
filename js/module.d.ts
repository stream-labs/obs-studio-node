export declare const DefaultD3D11Path: string;
export declare const DefaultOpenGLPath: string;
export declare const DefaultDrawPluginPath: string;
export declare const DefaultBinPath: string;
export declare const DefaultDataPath: string;
export declare const DefaultPluginPath: string;
export declare const DefaultPluginDataPath: string;
export declare const enum ESourceFlags {
    Unbuffered = 1,
    ForceMono = 2,
}
export declare const enum EMonitoringType {
    None = 0,
    MonitoringOnly = 1,
    MonitoringAndOutput = 2,
}
export declare const enum EDeinterlaceFieldOrder {
    Top = 0,
    Bottom = 1,
}
export declare const enum EDeinterlaceMode {
    Disable = 0,
    Discard = 1,
    Retro = 2,
    Blend = 3,
    Blend2X = 4,
    Linear = 5,
    Linear2X = 6,
    Yadif = 7,
    Yadif2X = 8,
}
export declare const enum EFontStyle {
    Bold = 1,
    Italic = 2,
    Underline = 4,
    Strikeout = 8,
}
export declare const enum EPropertyType {
    Invalid = 0,
    Boolean = 1,
    Int = 2,
    Float = 3,
    Text = 4,
    Path = 5,
    List = 6,
    Color = 7,
    Button = 8,
    Font = 9,
    EditableList = 10,
    FrameRate = 11,
}
export declare const enum EListFormat {
    Invalid = 0,
    Int = 1,
    Float = 2,
    String = 3,
}
export declare const enum EEditableListType {
    Strings = 0,
    Files = 1,
    FilesAndUrls = 2,
}
export declare const enum EPathType {
    File = 0,
    FileSave = 1,
    Directory = 2,
}
export declare const enum ETextType {
    Default = 0,
    Password = 1,
    Multiline = 2,
}
export declare const enum ENumberType {
    Scroller = 0,
    Slider = 1,
}
export declare const enum EAlignment {
    Center = 0,
    Left = 1,
    Right = 2,
    Top = 4,
    Bottom = 8,
    TopLeft = 5,
    TopRight = 6,
    BottomLeft = 9,
    BottomRight = 10,
}
export declare const enum EOutputFlags {
    Video = 1,
    Audio = 2,
    AV = 3,
    Encoded = 4,
    Service = 8,
    MultiTrack = 16,
}
export declare const enum ESourceOutputFlags {
    Video = 1,
    Audio = 2,
    Async = 4,
    AsyncVideo = 5,
    CustomDraw = 8,
    Interaction = 32,
    Composite = 64,
    DoNotDuplicate = 128,
    Deprecated = 256,
    DoNotSelfMonitor = 512,
}
export declare const enum ESceneDupType {
    Refs = 0,
    Copy = 1,
    PrivateRefs = 2,
    PrivateCopy = 3,
}
export declare const enum EObjectType {
    Source = 0,
    Encoder = 1,
    Service = 2,
    Output = 3,
}
export declare const enum ESourceType {
    Input = 0,
    Filter = 1,
    Transition = 2,
    Scene = 3,
}
export declare const enum EEncoderType {
    Audio = 0,
    Video = 1,
}
export declare const enum EFaderType {
    Cubic = 0,
    IEC = 1,
    Log = 2,
}
export declare const enum EColorFormat {
    Unknown = 0,
    A8 = 1,
    R8 = 2,
    RGBA = 3,
    BGRX = 4,
    BGRA = 5,
    R10G10B10A2 = 6,
    RGBA16 = 7,
    R16 = 8,
    RGBA16F = 9,
    RGBA32F = 10,
    RG16F = 11,
    RG32F = 12,
    R16F = 13,
    R32F = 14,
    DXT1 = 15,
    DXT3 = 16,
    DXT5 = 17,
}
export declare const enum EZStencilFormat {
    None = 0,
    Z16 = 1,
    Z24_S8 = 2,
    Z32F = 3,
    Z32F_S8X24 = 4,
}
export declare const enum EScaleType {
    Default = 0,
    Point = 1,
    Bicubic = 2,
    Bilinear = 3,
    Lanczos = 4,
}
export declare const enum ERangeType {
    Default = 0,
    Partial = 1,
    Full = 2,
}
export declare const enum EVideoFormat {
    None = 0,
    I420 = 1,
    NV12 = 2,
    YVYU = 3,
    YUY2 = 4,
    UYVY = 5,
    RGBA = 6,
    BGRA = 7,
    BGRX = 8,
    Y800 = 9,
    I444 = 10,
}
export declare const enum EBoundsType {
    None = 0,
    Stretch = 1,
    ScaleInner = 2,
    ScaleOuter = 3,
    ScaleToWidth = 4,
    ScaleToHeight = 5,
    MaxOnly = 6,
}
export declare const enum EColorSpace {
    Default = 0,
    CS601 = 1,
    CS709 = 2,
}
export declare const enum ESpeakerLayout {
    Unknown = 0,
    Mono = 1,
    Stereo = 2,
    TwoOne = 3,
    Quad = 4,
    FourOne = 5,
    FiveOne = 6,
    SevenOne = 8,
}
export declare const enum ESceneSignalType {
    ItemAdd = 0,
    ItemRemove = 1,
    Reorder = 2,
    ItemVisible = 3,
    ItemSelect = 4,
    ItemDeselect = 5,
    ItemTransform = 6,
}
export declare const enum EOutputCode {
    Success = 0,
    BadPath = -1,
    ConnectFailed = -2,
    InvalidStream = -3,
    Error = -4,
    Disconnected = -5,
    Unsupported = -6,
    NoSpace = -7,
}
export declare const Global: IGlobal;
export declare const OutputFactory: IOutputFactory;
export declare const AudioEncoderFactory: IAudioEncoderFactory;
export declare const VideoEncoderFactory: IVideoEncoderFactory;
export declare const ServiceFactory: IServiceFactory;
export declare const InputFactory: IInputFactory;
export declare const SceneFactory: ISceneFactory;
export declare const FilterFactory: IFilterFactory;
export declare const TransitionFactory: ITransitionFactory;
export declare const DisplayFactory: IDisplayFactory;
export declare const VolmeterFactory: IVolmeterFactory;
export declare const FaderFactory: IFaderFactory;
export declare const AudioFactory: IAudioFactory;
export declare const VideoFactory: IVideoFactory;
export declare const ModuleFactory: IModuleFactory;
export interface ISettings {
    [key: string]: any;
}
export interface IVec2 {
    readonly x: number;
    readonly y: number;
}
export interface ITimeSpec {
    readonly sec: number;
    readonly nsec: number;
}
export interface ITransformInfo {
    readonly pos: IVec2;
    readonly rot: number;
    readonly scale: IVec2;
    readonly alignment: EAlignment;
    readonly boundsType: EBoundsType;
    readonly boundsAlignment: number;
    readonly bounds: IVec2;
}
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
export interface IGlobal {
    startup(locale: string, path?: string): void;
    shutdown(): void;
    getOutputFlagsFromId(id: string): number;
    setOutputSource(channel: number, input: ISource): void;
    getOutputSource(channel: number): ISource;
    getProperties(id: string, type: EObjectType): IProperties;
    readonly totalFrames: number;
    readonly laggedFrames: number;
    readonly initialized: boolean;
    readonly locale: string;
    readonly version: number;
}
export interface IBooleanProperty extends IProperty {
}
export interface IColorProperty extends IProperty {
}
export interface IButtonProperty extends IProperty {
    buttonClicked(source: object): void;
}
export interface IFontProperty extends IProperty {
}
export interface IListProperty extends IProperty {
    readonly details: IListDetails;
}
export interface IListDetails {
    readonly format: EListFormat;
    readonly items: {
        name: string;
        value: string | number;
    }[];
}
export interface IEditableListProperty extends IProperty {
    readonly details: IEditableListDetails;
}
export interface IEditableListDetails extends IListDetails {
    readonly type: EEditableListType;
    readonly filter: string;
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
export interface IProperty {
    readonly status: number;
    readonly name: string;
    readonly description: string;
    readonly longDescription: string;
    readonly enabled: boolean;
    readonly visible: boolean;
    readonly type: EPropertyType;
    next(): IProperty;
    modified(): boolean;
}
export interface IProperties {
    readonly status: number;
    first(): IProperty;
    count(): number;
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
    readonly congestion: number;
    readonly connectTime: number;
    readonly reconnecting: boolean;
    readonly supportedVideoCodecs: string[];
    readonly supportedAudioCodecs: string[];
    readonly framesDropped: number;
    readonly totalFrames: number;
    start(): void;
    stop(): void;
    setDelay(ms: number, flags: EDelayFlags): void;
    getDelay(): void;
    getActiveDelay(): void;
}
export interface IOutputFactory extends IFactoryTypes {
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): IOutput;
    fromName(name: string): IOutput;
}
export declare enum EDelayFlags {
    PreserveDelay = 1,
}
export interface IServiceFactory extends IFactoryTypes {
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): IService;
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
    create(id: string, name: string, settings?: ISettings): IFilter;
}
export interface IFilter extends ISource {
}
export interface IInputFactory extends IFactoryTypes {
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): IInput;
    createPrivate(id: string, name: string, settings?: ISettings): IInput;
    fromName(name: string): IInput;
    getPublicSources(): IInput[];
}
export interface IInput extends ISource {
    volume: number;
    syncOffset: ITimeSpec;
    showing: boolean;
    audioMixers: number;
    monitoringType: EMonitoringType;
    deinterlaceFieldOrder: EDeinterlaceFieldOrder;
    deinterlaceMode: EDeinterlaceMode;
    duplicate(name?: string, isPrivate?: boolean): IInput;
    findFilter(name: string): IFilter;
    addFilter(filter: IFilter): void;
    removeFilter(filter: IFilter): void;
    readonly filters: IFilter[];
    readonly width: number;
    readonly height: number;
}
export interface ISceneFactory {
    create(name: string): IScene;
    createPrivate(name: string): IScene;
    fromName(name: string): IScene;
}
export interface IScene extends ISource {
    duplicate(name: string, type: ESceneDupType): IScene;
    add(source: IInput): ISceneItem;
    readonly source: IInput;
    moveItem(oldIndex: number, newIndex: number): void;
    findItem(id: string | number): ISceneItem;
    getItemAtIdx(idx: number): ISceneItem;
    getItems(): ISceneItem[];
    connect(sigType: ESceneSignalType, cb: (info: ISettings) => void): ICallbackData;
    disconnect(data: ICallbackData): void;
}
export interface ISceneItem {
    readonly source: IInput;
    readonly scene: IScene;
    readonly id: number;
    selected: boolean;
    position: IVec2;
    rotation: number;
    scale: IVec2;
    alignment: EAlignment;
    boundsAlignment: number;
    bounds: IVec2;
    boundsType: EBoundsType;
    scaleFilter: EScaleType;
    visible: boolean;
    readonly transformInfo: ITransformInfo;
    crop: ICropInfo;
    moveUp(): void;
    moveDown(): void;
    moveTop(): void;
    moveBottom(): void;
    move(position: number): void;
    remove(): void;
    deferUpdateBegin(): void;
    deferUpdateEnd(): void;
}
export interface ITransitionFactory extends IFactoryTypes {
    create(id: string, name: string, settings?: ISettings, hotkeys?: ISettings): ITransition;
    createPrivate(id: string, name: string, settings?: ISettings): ITransition;
    fromName(name: string): ITransition;
}
export interface ITransition extends ISource {
    getActiveSource(): ISource;
    clear(): void;
    set(input: ISource): void;
    start(ms: number, input: ISource): void;
}
export interface IConfigurable {
    update(settings: ISettings): void;
    readonly configurable: boolean;
    readonly properties: IProperties;
    readonly settings: ISettings;
}
export interface ISource extends IConfigurable, IReleasable {
    remove(): void;
    save(): void;
    readonly status: number;
    readonly type: ESourceType;
    readonly id: string;
    readonly outputFlags: ESourceOutputFlags;
    name: string;
    flags: ESourceFlags;
    muted: boolean;
    enabled: boolean;
}
export interface IFaderFactory {
    create(type: EFaderType): IFader;
}
export interface IFader {
    db: number;
    deflection: number;
    mul: number;
    attach(source: IInput): void;
    detach(): void;
    addCallback(cb: (db: number) => void): ICallbackData;
    removeCallback(cbData: ICallbackData): void;
}
export interface IVolmeterFactory {
    create(type: EFaderType): IVolmeter;
}
export interface IVolmeter {
    updateInterval: number;
    attach(source: IInput): void;
    detach(): void;
    addCallback(cb: (magnitude: number[], peak: number[], inputPeak: number[]) => void): ICallbackData;
    removeCallback(cbData: ICallbackData): void;
}
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
export interface IVideo {
    readonly totalFrames: number;
    readonly skippedFrames: number;
}
export interface IVideoFactory {
    reset(info: IVideoInfo): number;
    getGlobal(): IVideo;
}
export interface IAudio {
}
export interface IAudioFactory {
    reset(info: IAudioInfo): boolean;
    getGlobal(): IAudio;
}
export interface IModuleFactory {
    create(binPath: string, dataPath: string): IModule;
    loadAll(): void;
    addPath(path: string, dataPath: string): void;
    logLoaded(): void;
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
export interface ISceneItemInfo {
    name: string;
    crop: ICropInfo;
    scaleX: number;
    scaleY: number;
    visible: boolean;
    x: number;
    y: number;
    rotation: number;
}
export declare function addItems(scene: IScene, sceneItems: ISceneItemInfo[]): ISceneItem[];
export interface FilterInfo {
    name: string;
    type: string;
    settings: ISettings;
    enabled: boolean;
}
export interface SourceInfo {
    filters: FilterInfo[];
    muted: boolean;
    name: string;
    settings: ISettings;
    type: string;
    volume: number;
}
export declare function createSources(sources: SourceInfo[]): IInput[];
export interface ISourceSize {
    name: string;
    width: number;
    height: number;
    outputFlags: number;
}
export declare function getSourcesSize(sourcesNames: string[]): ISourceSize[];
export declare const NodeObs: any;
