# Captain's Game Engine [![Build Status](https://travis-ci.org/captain-mayhem/captainsengine.svg?branch=master)](https://travis-ci.org/captain-mayhem/captainsengine)
A cross-platform game engine with focus on classical point-and-click adventures

## The structure of the repository
On top level is the code of the engine and its main projects. In this Readme, they are sorted by importance:
* **Engine**: The core engine, renderer, system, io, physics, etc. Everything that is not adventure or project specific
* **Adventure**: The adventure engine, based on the core engine.
  * AdvEngine/Engine: The adventure engine written in C++.
  * Editor: the mighty adventure editor written in C# (StoryDesigner)
* **heroquest**: A 3D round-based multiplayer dungeon crawler game. This is where the engine started out before its main focus turned on adventure games.
  * HQClient: The GUI-Client
  * HQServer: The server runtime
  * HQEditor: The Heroquest-specific level editor
* editor: A very basic 3D scene editor for the core engine
* Compiler: A java-like VM at a very early stage
* OSMRacer: A car racing game based on OpenStreetMap data
* extern: The libs from contrib unpacked and integrated into CMake buildsystem
* contrib: 3rd-party libraries used by the engine or its projects. Things like libjpeg, lua, etc.
* playground: Small code snippets, ideas or projects that were not continued. In short, all the stuff that is too useless to be put into the repository at top level but too interesting to be thrown away.

## How to build
Requirements: CMake 3.21 or newer and a C++20 compiler (Visual Studio 2022/2026, GCC 13). The Adventure also needs a Java JDK (found through `JAVA_HOME` or `PATH`) and, on Windows, OpenAL (`OPENAL_LIBRARY`, `OPENAL_INCLUDE_DIR`).

`CMakePresets.json` has one configure preset per platform; each builds into `build/<preset>`:

| Preset | Platform | Needs |
|---|---|---|
| `default`, `debug` | Ninja, Release or Debug: Linux, or Windows from a developer prompt | Ninja |
| `windows` | Visual Studio 2026 solution, x64 | |
| `emscripten` | Web | `EMSDK` pointing to an emsdk (run `emsdk_env` first). The engine doesn't build for the web yet |
| `android`, `android-x86_64` | Android (API 24) with the NDK's toolchain | `ANDROID_NDK_HOME` pointing to an NDK |

```
cmake --preset default
cmake --build --preset default
```

On Windows, the StoryDesigner's C# projects are built by MSBuild in custom targets (`CSharpZip`, `StoryDesigner`, `RemoteControl`), so `cmake --build` and the generated solution build everything. With Ninja, MSBuild must be in `PATH` (a developer prompt). Edit the C# projects in their own solution, `Adventure/Editor/StoryDesigner/StoryDesigner.sln`.

Paths that only exist on your machine (a JDK, an OpenAL build, an NDK) go into a `CMakeUserPresets.json` (not committed) with presets that inherit the ones above and set `environment` or `cacheVariables`.

Notes:
* The engine is built as a shared library: `CGE.dll` on Windows, `libCGE.so` on Linux. Lua is part of it and exported, because applications use the Lua API directly. The other third-party libraries (zlib, libpng, libjpeg, giflib, GLEW) are linked into it statically and not exported.
* The physics module (ODE) is a separate library, `CGE::Physics`. It is built when `CGE_WITH_PHYSICS` is on, which is the default when OSMRacer is built.
* On Linux the Adventure is built without sound by default (`DISABLE_SOUND`): its sound code needs the FFmpeg 2.x API, which current distributions don't ship.

## Using the engine from another project
Add the engine with `add_subdirectory()` and link the `CGE::Engine` target. It carries the include directories, definitions and the C++20 requirement:

```cmake
add_subdirectory(../captainsengine cge)
add_executable(mygame WIN32 main.cpp)
target_link_libraries(mygame PRIVATE CGE::Engine)
cge_copy_runtime(mygame)                   # copies CGE.dll next to the .exe after building
cge_install_runtime(mygame mygame)         # installs the engine library with the application
```

The engine's own projects (Adventure, Heroquest, OSMRacer) are switched off when the engine isn't the top-level project. `cge_install_runtime` puts the engine library next to the executable on Windows, and into `lib/` (found through an `$ORIGIN/lib` RPATH) on Linux.

Engine classes and functions that applications use must be marked `CGE_API` (from `<cge_export.h>`). On Linux the library is built with hidden visibility, so anything else isn't visible to applications.

## How to use the Engine
Have a look at my [Wiki](http://wiki.captain-online.de).
