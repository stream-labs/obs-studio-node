# Contributing

## Project Structure
The project is structured into several pieces:

- `/ci/` contains Continuous Integration specific files, such as command shell scripts and similar
- `/cmake/` contains any CMake extras, like DownloadProject and the NodeJS helper code.
- `/js/` contains TypeScript and compiled JavaScript code. Changes to the .ts files have to be compiled and uploaded to the repository again.
- `/source/` contains shared source code that both server and client are using.
- `/obs-studio-server/` contains all code related to the server end of obs-studio-node, which is the part that interacts with libOBS (OBS Studio) and plugins.
- `/obs-studio-client/` contains all code related to the client end of obs-studio-node, which in this case is a Node.Js or Electron wrapper.
- `CMakeLists.txt` is the configuration file for CMake.

## Legacy Code
As of writing this file, there is a lot of legacy code back from when the project was in closed alpha and still called node-obs. The code has not yet been replaced by a more modern implementation, which limits the capabilities of this library significantly.
