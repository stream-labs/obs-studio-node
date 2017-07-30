const obs = require('./distribute/obs_node.node');
import * as path from 'path';

/* Convenient paths to modules */
export const DefaultD3D11Path: string = 
    path.resolve(__dirname, `distribute/libobs-d3d11.dll`);

export const DefaultDrawPluginPath: string = 
    path.resolve(__dirname, `distribute/simple_draw.dll`);

export const DefaultBinPath: string = 
    path.resolve(__dirname, `distribute`);

export const DefaultDataPath: string =
    path.resolve(__dirname, `distribute/data`);

export const DefaultPluginPath: string = 
    path.resolve(__dirname, `distribute/obs-plugins`);

export const DefaultPluginDataPath: string = 
    path.resolve(__dirname, `distribute/obs-plugins/%module%/data`);

export const ObsGlobal = obs.Global;
export const ObsInput = obs.Input;
export const ObsScene = obs.Scene;
export const ObsFilter = obs.Filter;
export const ObsTransition = obs.Transition;
export const ObsSceneItem = obs.SceneItem;
export const ObsProperties = obs.Properties;
export const ObsProperty = obs.Property;
export const ObsDisplay = obs.Display;
export const ObsSource = obs.ISource;
export const ObsVolmeter = obs.Volmeter;
export const ObsFader = obs.Fader;
export const ObsVideo = obs.Video;
export const ObsModule = obs.Module;

/**
 * Enumeration describing the type of a property
 */
export enum EPropertyType {
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

export enum EListFormat {
    Invalid,
    Int,
    Float,
    String
}

export enum EEditableListType {
    Strings,
    Files,
    FilesAndUrls
}

export enum EPathType {
    File,
    FileSave,
    Directory
}

export enum ETextType {
    Default,
    Password,
    Multiline
}

export enum ENumberType {
    Scroller,
    Slider
}

/**
 * A binary flag representing alignment
 */
export enum EAlignment {
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

export enum ESceneDupType {
    Refs,
    Copy,
    PrivateRefs,
    PrivateCopy
}

/**
 * Describes the type of source
 */
export enum ESourceType {
    Input,
    Filter,
    Transition,
    Scene,
}

/**
 * Describes algorithm type to use for volume representation.
 */
export enum EFaderType {
    Cubic,
    IEC /* IEC 60-268-18 */,
    Log /* Logarithmic */
}

export enum EColorFormat {
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

export enum EZStencilFormat {
	None,
	Z16,
	Z24_S8,
	Z32F,
	Z32F_S8X24
}

export enum EScaleType {
    Default,
    Point,
    FastBilinear,
    Bilinear,
    Bicubic
}

export enum ERangeType {
    Default,
    Partial,
    Full
}

export enum EOutputFormat {
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

export enum EBoundsType {
    None,
    Stretch,
    ScaleInner,
    ScaleOuter,
    ScaleToWidth,
    ScaleToHeight,
    MaxOnly
}

export enum EColorSpace {
    Default,
    CS601,
    CS709
}