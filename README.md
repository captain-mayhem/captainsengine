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
* buildtools: CMake stuff for special builds like Android, emscripten, ...
* contrib: 3rd-party libraries used by the engine or its projects. Things like libjpeg, lua, etc.
* playground: Small code snippets, ideas or projects that were not continued. In short, all the stuff that is too useless to be put into the repository at top level but too interesting to be thrown away.

## How to build
Run cmake with trunk as source directory. If you are not doing a cross-platform build and have all dependencies available on your system (OpenGL/DirectX, OpenAL, ffmpeg, Java runtime), it does not need additional arguments. There are tons of it if something needs to be tweaked.
You can find more detailed instructions in my [Wiki](http://wiki.captain-online.de)

## How to use the Engine
Have a look at my [Wiki](http://wiki.captain-online.de).
