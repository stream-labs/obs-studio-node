# libobs via node bindings
This library intends to provide bindings to obs-studio's internal library, named libobs accordingly, for the purpose of using it from a node runtime.
Currently, only Windows is supported.

## Building and Tests
See [BUILDING.md](./BUILDING.md).

## Example Projects
- [obs-studio-node-example](https://github.com/Envek/obs-studio-node-example/), a minimalistic screen and webcam recording example
- [AGView](https://github.com/hrueger/AGView), a slide-based presenting software. It's a little more complex than [obs-studio-node-example](https://github.com/Envek/obs-studio-node-example/) becuase it uses TypeScript and Angular, but all the `obs-studio-node` code lives in [this file](https://github.com/hrueger/AGView/blob/master/src/worker/obs.ts).

## Usage
> **Important:** If used with Electron, everything shows here has to be done in the **main** process, not in the renderer process. If you get `Uncaught Error: Failed to host and connect.`, this could be the problem.

### Import `obs-studio-node`
TypeScript:
```TypeScript or ES2015
import * as osn from "obs-studio-node";
```
JavaScript:
```JavaScript
const osn = require("obs-studio-node");
```

### Initialization
```JavaScript
const { Subject } = require("rxjs");
// Usually some UUIDs go there
osn.NodeObs.IPC.host("obs-studio-node-example");
// set the working dir
osn.NodeObs.SetWorkingDirectory(path.join(__dirname, "../../node_modules/obs-studio-node"));
// OBS Studio configs and logs
const obsDataPath = path.join(__dirname, "../../osn-data"); 
// init api with locale, data path and version
const initResult = osn.NodeObs.OBS_API_initAPI("en-US", obsDataPath, "1.0.0");

if (initResult !== 0) {
  const errorReasons = {
    "-2": "DirectX could not be found on your system. Please install the latest version of DirectX for your machine here <https://www.microsoft.com/en-us/download/details.aspx?id=35?> and try again.",
    "-5": "Failed to initialize OBS. Your video drivers may be out of date, or Streamlabs OBS may not be supported on your system.",
  };

  const errorMessage = errorReasons[initResult.toString()] || `An unknown error #${initResult} was encountered while initializing OBS.`;

  console.error("OBS init failure", errorMessage);

  // see below for this function
  shutdown();

  throw Error(errorMessage);
}

const signals = new Subject()
osn.NodeObs.OBS_service_connectOutputSignals((signalInfo) => {
    signals.next(signalInfo);
});
```

### Configuration (for recording)
```JavaScript
function getAvailableValues(category, subcategory, parameter) {
  const categorySettings = osn.NodeObs.OBS_settings_getSettings(category).data;
  if (!categorySettings) {
      console.warn(`There is no category ${category} in OBS settings`);
      return [];
  }

  const subcategorySettings = categorySettings.find(
      (sub) => sub.nameSubCategory === subcategory,
  );
  if (!subcategorySettings) {
      console.warn(`There is no subcategory ${subcategory} for OBS settings category ${category}`);
      return [];
  }

  const parameterSettings = subcategorySettings.parameters.find(
      (param) => param.name === parameter,
  );
  if (!parameterSettings) {
      console.warn(`There is no parameter ${parameter} for OBS settings category ${category}.${subcategory}`);
      return [];
  }

  return parameterSettings.values.map((value) => Object.values(value)[0]);
}

function setSetting(category, parameter, value) {
  let oldValue;
  // Getting settings container
  const settings = osn.NodeObs.OBS_settings_getSettings(category).data;

  settings.forEach((subCategory) => {
    subCategory.parameters.forEach((param) => {
      if (param.name === parameter) {
        oldValue = param.currentValue;
        param.currentValue = value;
      }
    });
  });
  // Saving updated settings container
  if (value != oldValue) {
    osn.NodeObs.OBS_settings_saveSettings(category, settings);
  }
}

setSetting("Output", "Mode", "Simple");
const availableEncoders = getAvailableValues("Output", "Recording", "RecEncoder");
setSetting("Output", "RecEncoder", availableEncoders.slice(-1)[0] || "x264");
setSetting("Output", "FilePath", path.join(__dirname, "../videos"));
setSetting("Output", "RecFormat", "mkv");
setSetting("Output", "VBitrate", 10000); // 10 Mbps
setSetting("Video", "FPSCommon", 60);
```

### Scenes and Sources
#### Creating
You can create a scene like so:
```JavaScript
const scene = osn.SceneFactory.create("myScene");
```
Then, you can create a source and add it:
```JavaScript
const source = osn.InputFactory.create("image_source", "logo", { file: path.join(__dirname, "../assets/icons/favicon.png") });
const sceneItem = scene.add(source);
```

The `osn.InputFactory.create()` method needs the parameters `id`, `name` and optionally `settings`. The `id` could for example be `image_source`, `browser_source`, `ffmpeg_source` or `text_gdiplus`. In the settings object you can specify `is_local_file: true` and `local_file: "myPathToTheFile"`. For videos, you can say `looping: true`. The settings for `text_gdiplus` look different: `read_from_file: true, file: "myPathToTheFile"`.

#### Finding
If you know the name of any object (for example a scene or a source), you can get it with the `[factory].fromName(name)`. For example:
```JavaScript
const myScene = osn.SceneFactory.fromName("myScene");
```

#### Working with scene items
You can modify the properties of the scene item to for example move or scale the source:
```JavaScript
sceneItem.position = { x: 50, y: 50 };
sceneItem.scale = { x: 0.5, y: 0.7 };
```

#### Setting the scene as an output
```JavaScript
osn.Global.setOutputSource(0, this.scene);
```

### Transitions
To transition between scenes, you first need to create a transition and set it as output.
```JavaScript
const transition = osn.TransitionFactory.create(transitionType, "myTransition", {});
transition.set(scene);
osn.Global.setOutputSource(0, transition);
```
You can set the scene using the `transition.set(scene)` method and transition to another scene like this:
```JavaScript
transition.start(300, scene);
```
The `start` method needs the duration of the transition and the scene object to transition to.

### Recording
#### Start
```JavaScript

function getNextSignalInfo() {
  return new Promise((resolve, reject) => {
    signals.pipe(first()).subscribe(signalInfo => resolve(signalInfo));
    setTimeout(() => reject('Output signal timeout'), 30000);
  });
}
let signalInfo;
console.debug('Starting recording...');
osn.NodeObs.OBS_service_startRecording();
console.debug('Started?');
signalInfo = await getNextSignalInfo();
if (signalInfo.signal === 'Stop') {
  throw Error(signalInfo.error);
}
console.debug('Started signalInfo.type:', signalInfo.type, '(expected: "recording")');
console.debug('Started signalInfo.signal:', signalInfo.signal, '(expected: "start")');
console.debug('Started!');
```

#### Stop
```JavaScript
let signalInfo;
console.debug('Stopping recording...');
osn.NodeObs.OBS_service_stopRecording();
console.debug('Stopped?');
signalInfo = await getNextSignalInfo();
console.debug('On stop signalInfo.type:', signalInfo.type, '(expected: "recording")');
console.debug('On stop signalInfo.signal:', signalInfo.signal, '(expected: "stopping")');
signalInfo = await getNextSignalInfo();
console.debug('After stop signalInfo.type:', signalInfo.type, '(expected: "recording")');
console.debug('After stop signalInfo.signal:', signalInfo.signal, '(expected: "stop")');
console.debug('Stopped!');
```

### Displays
You can create a display like so:
```JavaScript
const displayId = "myDisplay";
const displayWidth = 960;
const displayHeight = 540;
const resized = () => {
    const { width, height } = previewWindow.getContentBounds();
    osn.NodeObs.OBS_content_resizeDisplay(displayId, width, height + 20);
    osn.NodeObs.OBS_content_setPaddingSize(displayId, 5);
};
previewWindow = new BrowserWindow({
    width: displayWidth,
    height: displayHeight,
    // if you use this, the window will automatically close
    // when the parent window is closed
    parent: parentWindow,
    useContentSize: true,
});
previewWindow.on("close", () => {
    osn.NodeObs.OBS_content_destroyDisplay(displayId);
    previewWindow = undefined;
});
previewWindow.on("resize", resized);

osn.NodeObs.OBS_content_createSourcePreviewDisplay(
    previewWindow.getNativeWindowHandle(),
    "", // or use camera source Id here
    displayId,
);
osn.NodeObs.OBS_content_setShouldDrawUI(displayId, false);
osn.NodeObs.OBS_content_setPaddingColor(displayId, 255, 255, 255);
resized();
```

You need a `BrowserWindow` and OBS will draw the display on top of it. Be sure to listen to all events and resize or move the distplay accordingly, as it will otherwise be on top of your other HTML elements which can be in the same `BrowserWindow`.

You can set a padding with
```JavaScript
osn.NodeObs.OBS_content_setPaddingSize(displayId, 5);
osn.NodeObs.OBS_content_setPaddingColor(displayId, 255, 255, 255);
```

### Shutting down
```JavaScript
function shutdown() {
  try {
    osn.NodeObs.OBS_service_removeCallback();
    osn.NodeObs.IPC.disconnect();
    this.obsInitialized = false;
  } catch (e) {
    throw Error(`Exception when shutting down OBS process${e}`);
  }
}
```
