# libobs via node bindings
This library intends to provide bindings to obs-studio's internal library, named libobs accordingly, for the purpose of using it from a node runtime.
Currently, only Windows is supported.

## Why CMake?
CMake offers better compatibility with existing projects than node-gyp and comparable solutions. It's also capable of generating solution files for multiple different IDEs and compilers, which makes it ideal for a native module. Personally, I don't like gyp syntax or the build system surrounding it or the fact it requires you to install python.

# Building

## Prerequisites
You will need to have the following installed:

* Git
* [Node.js](https://nodejs.org/en/)
* [Yarn](https://yarnpkg.com/en/docs/install#windows-stable)
* [CMake](https://cmake.org/)

### Windows
Building on windows requires additional software:

* [Visual Studio 2017 or 2015](https://visualstudio.microsoft.com/)
* [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk) (may be installed with Visual Studio 2017 Installer)

### Example Build
We use a flexible cmake script to be as broad and generic as possible in order to prevent the need to constantly manage the cmake script for custom uses, while also providing sane defaults. It follows a pretty standard cmake layout and you may execute it however you want.

Example:
```
git submodule update --init --recursive
mkdir build
cd build
cmake .. -G"Visual Studio 15 2017" -A x64
cmake --build .
cpack -G ZIP
```

This will will download any required dependencies, build the module, and then place it in an archive compatible with npm or yarn that you may specify in a given package.json.

### Custom OBS Build
By default, we download a pre-built version of libobs if none is specified. However, this pre-built version may not be what you want to use or maybe you're testing a new obs feature.

You may specify a custom archive of your own. However, some changes need to be made to obs-studio's default configuration before building:

* `ENABLE_SCRIPTING` must be set to `false`
* `ENABLE_UI` must be set to `false`
* `QTDIR` should *not* be specified.

If you don't know how to build obs-studio from source, you may find instructions [here](https://github.com/obsproject/obs-studio/wiki/Install-Instructions#windows-build-directions).

Example (from root of obs-studio repository clone):
```
mkdir build
cd build
cmake .. -DENABLE_UI=false -DDepsPath="C:\Users\computerquip\Projectslibobs-deps\win64" -DENABLE_SCRIPTING=false -G"Visual Studio 15 2017" -A x64
cmake --build .
cpack -G ZIP
```

This will create an archive that's compatible with obs-studio-node. The destination of the archive will appear after cpack is finished executing.
Example:

> CPack: Create package using ZIP
>
> CPack: Install projects
>
> CPack: - Install project: obs-studio
>
> CPack: Create package
>
> CPack: - package: C:/Users/computerquip/Projects/obs-studio/build/obs-studio-x64-22.0.3-sl-7-13-g208cb2f5.zip generated.

This archive may then be specified as a cmake variable when building obs-studio-node like so:
```
cmake .. -G"Visual Studio 15 2017" -A x64 -DOSN_LIBOBS_URL="C:/Users/computerquip/Projects/obs-studio/build/obs-studio-x64-22.0.3-sl-7-13-g208cb2f5.zip"
cmake --build .
cpack -G ZIP
```

### Further Building
I don't specify every possible combination of variables. Here's a list of actively maintained variables that control how obs-studio-node is built:

* All configurable node-cmake variables found [here](https://github.com/cjntaylor/node-cmake/blob/dev/docs/NodeJSCmakeManual.md).
* `OSN_LIBOBS_URL` - Controls where to fetch the libobs archive. May be a directory, any compressed archive that cpack supports, or a URI of various types including FTP or HTTP/S.

If you find yourself unable to configure something about our build script or have any questions, please file a github issue!
