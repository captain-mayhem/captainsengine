# Captain's Game Engine [![build](https://github.com/captain-mayhem/captainsengine/actions/workflows/build.yml/badge.svg)](https://github.com/captain-mayhem/captainsengine/actions/workflows/build.yml)
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
Requirements: CMake 3.21 or newer and a C++20 compiler (Visual Studio 2022/2026, GCC 13). The Adventure also needs a Java JDK (found through `JAVA_HOME` or `PATH`).

`CMakePresets.json` has one configure preset per platform; each builds into `build/<preset>`:

| Preset | Platform | Needs |
|---|---|---|
| `default`, `debug` | Ninja, Release or Debug: Linux, or Windows from a developer prompt | Ninja |
| `windows` | Visual Studio 2026 solution, x64 | |
| `emscripten` | Web, single-threaded | `EMSDK` pointing to an emsdk (run `emsdk_env` first) |
| `emscripten-threads` | Web with pthreads and ASYNCIFY (`CGE_WEB_THREADS`) | the same |
| `android`, `android-x86_64` | Android (API 24) with the NDK's toolchain | `ANDROID_NDK_HOME` pointing to an NDK |

```
cmake --preset default
cmake --build --preset default
```

On Windows, the StoryDesigner's C# projects are built by MSBuild in custom targets (`CSharpZip`, `StoryDesigner`, `RemoteControl`), so `cmake --build` and the generated solution build everything. With Ninja, MSBuild must be in `PATH` (a developer prompt). Edit the C# projects in their own solution, `Adventure/Editor/StoryDesigner/StoryDesigner.sln`.

Paths that only exist on your machine (a JDK, an NDK) go into a `CMakeUserPresets.json` (not committed) with presets that inherit the ones above and set `environment` or `cacheVariables`.

### Unit tests
The engine's unit tests in `tests/` use googletest (`extern/googletest`) and need no window, so they also run on CI machines. They're built by default for desktop builds of the engine itself (`CGE_BUILD_TESTS`; off for Android, the web and when another project includes the engine). Run them with the test preset of the same name:

```
ctest --preset default      # or: ctest --preset windows
```

The test files are in `tests/data`; `tests/data/make_fixtures.py` writes them again. Each test run uses its own user folders (`UserDirs`) and removes them at the end.

### The web
The web build is WebAssembly with WebGL 2. Each application becomes a page: `<name>.html` (the engine's shell page, `Engine/web/shell.html`: the canvas over the whole browser window and a loading bar), `<name>.js`, `<name>.wasm` and `<name>.data` with its preloaded files. The engine is linked statically into it. Pages must be served over HTTP; for a quick test, `python -m http.server` in the folder with the files will do.

* **Two configurations:** single-threaded by default, which works on any web server. `CGE_WEB_THREADS=ON` (preset `emscripten-threads`) builds with pthreads and ASYNCIFY, for applications that wait for input in their own nested loop (`Engine::pumpFrame()` yields to the browser); its page must be served cross-origin isolated (the headers `Cross-Origin-Opener-Policy: same-origin` and `Cross-Origin-Embedder-Policy: require-corp`). Both run the engine's main loop on the browser's main thread, which must not block: there is no frame limiter (the browser paces the frames), and a single-threaded application can't wait in a nested loop.
* **Files:** the application's preloaded files are its assets (`Assets`); the user folders (`UserDirs`) are in IndexedDB, and the settings store is saved after each frame that changed it, since a page can be closed at any time.
* **Data packages:** files that several pages share or that are chosen when the page is opened, like the Adventure's games, go into a data package (`cge_web_package`). The page loads the packages named in its address before the application starts: `Adventure.html?data=demo` loads `demo.js` and `demo.data`.
* **Audio:** Web Audio; browsers start it only after the first click or tap on the page.
* **Input:** mouse, keyboard and touch (one finger acts as the left mouse button). The page keeps the browser's function of Ctrl+C/V/X, F5, F11 and F12. The clipboard holds the text copied last in the page; Ctrl+V brings the system clipboard's text with it.
* The Adventure runs in the browser: `Adventure.html?data=demo` with the demo game, whose package is installed next to it.

Notes:
* The engine is built as a shared library: `CGE.dll` on Windows, `libCGE.so` on Linux. Lua and Dear ImGui are part of it and exported, because applications use their APIs directly (and there must be one ImGui context). The other third-party libraries (zlib with minizip, libpng, libjpeg, giflib, FreeType, miniaudio, libogg and libvorbis, glad) are linked into it statically and not exported. The OpenGL backend plugin has its own copy of glad.
* Audio (`Engine/audio`, `Engine::getAudio()`) is miniaudio, with libvorbis for Ogg Vorbis: one-shot sounds, music, players, PCM streams for applications that decode themselves, and effect buses (reverb, echo, chorus/flanger, distortion). zlib, libpng, libjpeg, FreeType, Dear ImGui, miniaudio, libogg and libvorbis are the same copies as in guitads. googletest is only used by the unit tests.
* The UI is Dear ImGui: the engine draws ImGui on top of every frame, so applications call ImGui's functions (`#include <imgui.h>`) in their `render()`. While ImGui wants the keyboard or the mouse (a text field is active, the mouse is over an ImGui window), the application doesn't get the keys and characters, and `Input::Mouse::isGuiClick()` is true. The console (`consol`, toggled with `~`) is an ImGui window.
* The physics module (ODE) is a separate library, `CGE::Physics`. It is built when `CGE_WITH_PHYSICS` is on, which is the default when OSMRacer is built.
* The Adventure's sound goes through the engine's audio on every platform. Its videos (`playavi`, `playvideo`) aren't played: they were decoded with FFmpeg, which is gone. Flash animations (`playswf`) still play.

## Using the engine from another project
Add the engine with `add_subdirectory()` and link the `CGE::Engine` target. It carries the include directories, definitions and the C++20 requirement:

```cmake
add_subdirectory(../captainsengine cge)
add_executable(mygame WIN32 main.cpp)
target_link_libraries(mygame PRIVATE CGE::Engine)
cge_copy_runtime(mygame)                   # copies CGE.dll next to the .exe after building
cge_install_runtime(mygame mygame)         # installs the engine library with the application
# on the web: the page with the files the application reads (<source>@<path>); nothing elsewhere
cge_web_application(mygame PRELOAD engine.ini@engine.ini assets@assets)
```

The engine's own projects (Adventure, Heroquest, OSMRacer) are switched off when the engine isn't the top-level project. `cge_install_runtime` puts the engine library next to the executable on Windows, and into `lib/` (found through an `$ORIGIN/lib` RPATH) on Linux; on the web it installs the page's `.js`, `.wasm` and `.data` files. `cge_web_package(name <source>@<path>... [DESTINATION dir])` makes a data package for web pages.

Engine classes and functions that applications use must be marked `CGE_API` (from `<cge_export.h>`). On Linux the library is built with hidden visibility, so anything else isn't visible to applications.

## How to use the Engine
Have a look at my [Wiki](http://wiki.captain-online.de).
