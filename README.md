[Reasons](#reasons) |
[Getting Started](#quick-start) |
[Environment Variables](#environment-variables) |
[Issues](#issues)

__Reasons__
===========

__What happened to node-obs?__
------------------------------
node-obs proved to be difficult to work with. The following are just a few reasons why a new set of bindings were warranted:

- Lack of objects altogether making context harder to control
- What objects we do use, v8 cannot control their lifetimes
- Difficult to use over the Electron IPC (or any IPC)
- Requirement to constantly sync between frontend and backend
- Lack of flexibility and feature for the frontend
- An unintuitive interface that was difficult to use and error prone
- The lack of a scene item object which didn't allow for duplicate sources.
- And probably more I'm not thinking of right now.

__Why cmake?__
--------------
Because the alternative is gyp which is deprecated. Frankly, node-gyp shouldn't be used anymore since it's incredibly difficult to use outside of simple use-cases.

__Getting Started__
===================
I've tried to simplify the build system as much as possible to streamline going from nothing to a working library.

### __Requirements__ ###
- Currently only Windows is supported
  - Note that it shouldn't take much to add other platforms.
- VC++ 14 (2015)
- CMake 3.0 or higher - [Website](https://cmake.org)(https://obsproject.com/downloads/dependencies2013.zip)
- yarn  - [Website](https://yarnpkg.com)

The rest are handled through package management.

### __Building__ ###
Simply checkout the module and use `yarn` or `yarn install`. `npm install` should also work although I've not tested it. 

### __Building Documentation__ ###
Building the documentation couldn't be more simple. Make sure you've done the initial setup and do `yarn docs`. It will output static HTML inside of a folder called 'docs' in the root directory.

__Environment Variables__
=========================
The environment variables are only used if you build manually or if you set them as globals. I don't recommend either but these are used to get the build system working so changing them is also required to work. In particular, a useful variable is `OBS_BUILD_TYPE` which you can set to `Debug` to magically compile things in debug mode. Note that since index loads the node by name, changing `OSN_NODE_NAME` will current break the export script. 

- __ENABLE_DISTRIBUTION__ - \[ Default: `false` \]
- __OSN_DIST_DIR__ - \[ Default: `"distribute"` \]
- __OSN_NODE_NAME__ - \[ Default: `"obs_node"` \]
- __OBS_BUILD_TYPE__ - \[ Default: `Release` \]
- __OBS_STUDIO_PATH__ - \[ Default: `${CMAKE_SOURCE_DIR}/obs-build` \]
- __OBS_STUDIO_BUILD64__ - \[ Default: `${OBS_STUDIO_PATH}/build64` \]
- __OBS_STUDIO_DEPS64__ - \[ Default: `${OBS_STUDIO_PATH}/dependencies2015/win64` \]

__Issues__
==========
- Need to add support for other platforms.
- Need the code base to be used and practically tested. 