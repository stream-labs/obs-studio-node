OBS Studio Node.JS module
==================================

This module allows direct interaction from one or more Node.JS processes with one or more libobs/OBS Studio processes through the use of named pipes and kernel facing IPC.

__Why cmake?__
--------------
Because the alternative is gyp which is deprecated. Frankly, node-gyp shouldn't be used anymore since it's incredibly difficult to use outside of simple use-cases.

__Getting Started__
===================
I've tried to simplify the build system as much as possible to streamline going from nothing to a working library.

### __Requirements__ ###
- Currently only Windows is supported
- A compiler that works with your node runtime and CMake
- CMake 3.0 or higher - [Website](https://cmake.org)(https://obsproject.com/downloads/dependencies2013.zip)
- npm or yarn  - [Website](https://yarnpkg.com)

### __Building__ ###
I've tried to make the build process as simple as possible. 
Currently, we use node-cmake. It's a simple CMake module that wraps over our native Node.js module for us to handle anything specific to node. We can treat it as a normal CMake target in that regard. 

To build, use it as a normal CMake project. Doing something like the following is valid though may not meet all needs:
`mkdir build`
`cd build`
`cmake ..`
`cmake --build .`

### __Packaging__ ###
When you configure, you must specify an install prefix to cmake at configure time. Simply passing `-DCMAKE_INSTALL_PREFIX=<install_dir>` will tell cmake to install into the given directory. For example, `cmake --build build --target install --config Release` will create a node module in `build/distribute` that you can then gunzip (tar.gz) into an archive for distribution. CPack may also work though I've not tested it.

### __Building Documentation__ ###
Building the documentation couldn't be more simple. Make sure you've done the initial setup and do `yarn build:docs`. It will output static HTML inside of a folder called 'docs' in the root directory.

### __Structure of the Project__ ###

- `CMakeLists.txt` is where cmake and cmake-js logic is done. It's just a standard cmake script. 

- `js` is where both the typescript and the compiled result is stored. We use typescript definitions to describe the module which is then compiled down to a `.js` implementation and a `.d.ts` type definitions file. If a change is made to the original `.ts`, it must be compiled and the resulting files must also be uploaded with the changes. This removes a runtime dependency on Typescript when installing the module.

- `src/obspp` is the C++ wrapper over obs. It's somewhat specific to our needs but is 95% reusable. The 5% has to do with quirks and how objects are released/destroyed (via the shared pointer system in libobs). The weak/strong pointer wrappers are also pretty subpar as noted elsewhere. They were used to avoid misuse but didn't pan out so well.

- `src/nodeobs_*` files are from node-obs. Once upon a time, there was a separate project called node-obs. node-obs had some limitations so an alternative was born called obs-studio-node. Unfortunately, streamlabs-obs never quite weened off of node-obs and at some point, it made sense to merge the two until node-obs was ultimately removed. It has no concept of C++ objects encapsulated in javascript objects, has a lot of hardcoded structures, and a lot of legacy code that just isn't used anymore. These will hopefully eventually be removed slowly but surely. If you're creating a new project, I'd suggest not using anything from these files. They're also not described in the typescript definitions and must be used as raw javascript objects.

- Everything else is part of obs-studio-node. It tries to wrap libobs weak pointers into objects and tries to prevent bad data entry as much as possible. In practice, it has flaws. Weak pointers ended up adding more innefficiency and complication than it solved misuse. The objects are not serializable meaning you cannot use objects from this API over a serializing IPC (though other work is being done to hopefully negate this problem). It did nothing to solve the issue of callbacks, especially when used over an IPC. 

This module is far easier to use when using it in a single-process manner (or really, anything is for that matter) since it didn't take into account the need to be serializable and took for assumption that APIs like Electron's remote module would be far more efficient than they actually are.

### __Using obs-studio-node outside of streamlabs-obs__ ###
There are some quirks to how obs-studio-node currently runs though those quirks are disappearing over time, one by one. Currently, libobs is dynamically loaded at runtime, using delay load features of the linker (and will use lazy load features in GCC/Clang). So in order to have libobs load, obs-studio-node dynamically maps all dependencies into the process first and then maps obs.dll into the process as well. This allows us to load libobs from anywhere we want and to specify directories from virtually anywhere in a relatively safe manner. However, there are some hard-coded paths currently that have yet to be removed. You'll need to look at how the whitelisting of depencies is done in nodeobs_api.c. You can also posts issues on mantis or discord concerning any questions. 

Outside of that, using the module is relatively straight forward if you've used libobs before.

NOTE: The docs are currently a bit out of date. Generating the docs will still provide a good API tree to browse through but the notes on it might be old or wrong.

__Issues__
==========
- We have a few Windows specific issues, specifically the display code. With the upcoming IPC code, pretty much all of the IPC code is platform-specific unfortunately so it will be awhile to port to other platforms, if ever without outside efforts.

- The other is that we don't handle hotkeys through libobs which means source hotkeys can't be mapped to a hotkey. A new API will eventually need to be introduced to fix this to allow the frontend to provide input and to query available hotkey structures.

- The display code is specific to the streamlabs-obs frontend. Once upon a time, I added a way to dynamically load display plugins, as I called them, so the need to modify obs-studio-node directly was removed. This feature was removed due to lack of resources and time to dedicate to it, in addition to the lack of need of that interface in streamlabs-obs which was our primary effort. With a little bit of help, I don't mind adding that feature back in but it's not on my immediate todo list. You can use the display code if you want but it's very specific to the needs of streamlabs-obs.

- If any other issues arise, feel free to email me or file an issue. We have a tracker for streamlabs-obs but in the case people outside of that project wish to use obs-studio-node, I've opened up github issues once again for the project.