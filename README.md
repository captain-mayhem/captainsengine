# captainsengine
A cross-platform game engine with focus on classical point-and-click adventures

## The structure of the repository
On top level is the code of the engine and its main projects. Sorted by importance. Directories not mentioned below are candidates to be put into the playground
* **Engine**: The core engine, renderer, system, io, physics, etc. Everything that is not adventure or project specific
* **Adventure**: The adventure engine, based on the core engine. I know, I should rename directories below to something more useful:
  * AppBuilder/AppBuilder: The adventure engine written in C++.
  * Editor: the mighty adventure editor written in C# called StoryDesigner
* **heroquest**: A 3D round-based multiplayer dungeon crawler game. This is where the engine started out before its main focus turned on adventure games.
  * HQClient: The GUI-Client
  * HQServer: The server runtime
  * HQEditor: The Heroquest-specific level editor
* editor: A 3D-scene editor for the core engine
* Compiler: A java-like VM at an very early stage
* OSMRacer: A car racing game based on OpenStreetmap data
* extern: The libs from contrib unpacked and integrated into CMake buildsystem
* buildtools: CMake stuff for special builds like Android, emscripten, ...
* contrib: 3rd-party libraries used by the engine or its projects. Things like libjpeg, lua, etc.
* playground: Small code snippets, ideas or projects that were not continued. In short, all the stuff that is too useless to be put into trunk but too interesting to be thrown away.

## How to build
Run cmake with trunk as source directory. If you are not doing a cross-platform build and have all dependencies available (OpenGL/DirectX, OpenAL, ffmpeg, Java runtime) on your system, it does not need additional arguments. There are tons of it if something needs to be tweaked.
You can find more detailed instructions in my [Wiki](http://wiki.obfuscated.de)

## How to use the Engine
Have a look at my [Wiki](http://wiki.obfuscated.de).