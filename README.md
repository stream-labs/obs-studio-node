# OBS Studio for Node.Js
`obs-studio-node` is a Node.Js compatible wrapper around [https://github.com/stream-labs/obs-studio](OBS Studio). It works by wrapping libOBS inside a secondary process that is talked to using [https://github.com/stream-labs/lib-streamlabs-ipc](lib-streamlabs-ipc), which results in better stability and performance.

The goal of the project is to provide a fully featured working Node.Js and electron wrapper around libOBS (OBS Studio). 

## Why CMake?
CMake offers better compatibility with existing projects than node-gyp and comparable solutions. It's also capable of generating solution files for multiple different IDEs and compilers, which makes it ideal for a native module. It is also the build system of obs-studio, which we depend on for any functionality.

# Building The Project
The project is for the most part automated, so there is very little to actually do.

## Prerequisites
You will need to have the following installed:

* Node.JS (LTS or latest)
* Yarn (Run `npm install -g yarn` inside a shell with node.js access)
* [CMake](https://cmake.org/), Version 3.1 or higher

### Windows
Building on windows requires additional software:

* Visual Studio 2015 or 2017 with the latest update installed.
* Windows SDK (latest version)

## First Steps
1. Clone the project and check out master.
2. Recursively initialize all submodules to the commit you checked out.
3. Run `yarn install` in a node.js enabled shell inside the project directory.

## Configuration
1. Open CMake-GUI.
2. Set "Where is the source code:" to the cloned project directory.
3. Set "where to build the binaries:" to the build directory inside the project directory (you might have to create it first).
4. Click "Configure".
5. Specify the generator that you want to use. "... Win64" are 64-bit capable generators.
6. Optional: If you have a 64-bit system, type in `host=x64` into the "Optional toolset to use" box.
7. Leave it at default native compilers.
8. Click Finish.
9. Click Generate once the button is no longer grey.

## Building
1. Open the project (if you still have CMake-GUI open, just click Open Project)
2. Build the "ALL_BUILD" project.

## Installing and Distribution
1. Open CMake-GUI
2. Change the "Where to build the binaries:" directory to the configured directory. CMake should automatically update.
3. Modify the CMAKE_INSTALL_PREFIX variable to the directory of your liking. Visual Studio will need to have _write & execute_ access to that location.
4. Click Configure.
5. Click Generate.
6. Open the project and build the INSTALL project. This will create the proper node module structure in the directory you specified to CMAKE_INSTALL_PREFIX

# Building The Documentation
First make sure that you've configured and built obs-studio-node. Once you have done that, do the following:

1. Open a node.js enabled shell inside the project directory.
2. Run `yarn build:docs` and wait for the command to be done.
3. Documentation will be inside the `/docs/` directory as a static HTML file.

# Contributing To The Project
See the [Guidelines for contributing](https://github.com/stream-labs/obs-studio-node/blob/staging/CONTRIBUTING.md).
