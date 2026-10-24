# Notes for Claude

Captain's Game Engine (CGE), LGPL 2.1. The engine library is `Engine/`; the other folders are projects built on it (Adventure, heroquest/HQClient, editor, OSMRacer, playground) and its unit tests (`tests/`).

## The modernisation
- The engine is being modernised on the branch **`modernize`**, driven by the game jewelhunter. The plan and the status of every step (what was done, what was checked and how, what wasn't, and "Worth knowing" notes) are in **`../jewelhunter/PLAN.md`** (GitHub `captain-mayhem/jewelhunter`), Phase 1. Read the step you're working on there, and record a finished step there, not here.
- The rule from the plan: existing projects keep working by being ported, not by keeping old APIs. Every change is checked against all projects (the regression check below).
- Other consumers that the plan takes code or lessons from: `../guitads` (guit3: fonts, the web) and `../tads-runner`.
- Don't commit or push unless asked.

## Code style
- Match the surrounding file: `//!` Doxygen comments on declarations, `//` comments starting lowercase and saying why, `{` on the same line, two-space indentation, `name_` members in newer code.
- Some files have CRLF line endings, others LF: keep each file's endings (check with `file`; the Edit tool keeps them, scripts must too).
- Platforms: `#if defined(WIN32)`, `LINUX` (X11), `ANDROID` and `__EMSCRIPTEN__`. The web defines `UNIX` but not `LINUX`, and the engine is a static library there; elsewhere it's a shared library (`CGE_API`, `cge_export.h`).
- Where things are: the windows per platform are `window/native*.cpp` (`nativeLinux` is X11; the X11 event loop is `Engine::processEvents` in `system/engine.cpp`); input is `input/`; the platform services (clipboard, cursors, text input, URLs, settings, user folders, assets) are `platform/`, with the table of what exists where in `platform/services.h` and engine-internal hooks in `platform/platform_internal.h`; the ImGui backends are `ui/imgui_impl_cge_*`; the renderer is `renderer/` with the GL backend in `renderer/GL`; text is `renderer/font.h` (FreeType through the `SpriteBatch`) and `renderer/screentext.h` (the old print queue, `Engine::getScreenText`), the system font lookup `platform/systemfonts.h`, and the engine's fonts are `Engine/fonts/`. The Android activity's Java code is `Engine/android/java` (`CgeActivity`, `TextInputView`); the web shell page is `Engine/web/shell.html`.

## Building
Release builds; the presets are in `CMakePresets.json`, the machine-specific ones (`windows-local`, `android-local`) in `CMakeUserPresets.json`, which git ignores. Logs go to `build/*.log`, named by step (e.g. `build_1c5.log`); grep them for `warning|error`.

| What | Command |
|---|---|
| Windows, all projects | `cmake --build --preset windows-local` |
| Tests, Windows | `ctest --preset windows-local` |
| Install (Windows) | `cmake --install build/windows-local --config Release` (into `build/install/windows-local`; the Adventure demo, HQClient and the editor are run from there) |
| Linux (WSL), all projects | `wsl -e bash -lc "cd /mnt/c/Projects/captainsengine && cmake --build build/default -j8"` |
| Tests, Linux | `wsl -e bash -lc "cd /mnt/c/Projects/captainsengine && ctest --test-dir build/default"` |
| Web | PowerShell: `. C:\Projects\emsdk\emsdk_env.ps1; cmake --build build/emscripten` (and `build/emscripten-threads`) |
| Android | `cmake --build --preset android-local` |

**Regression check** (expected for every step): the Windows build of all projects with no errors and no warnings; the Linux build with only the known warnings (`tmpnam` in the vendored Lua, and `getcwd` in HQServer, which only appears when it's recompiled); the unit tests passing on Windows and Linux (161 on Windows and 160 on Linux as of Phase 1c step 6); no new warnings on the web and Android; and the projects a change touches started and tried (jewelhunter, the Adventure demo, HQClient, the editor).

Built as part of jewelhunter (`CGE_DIR`), the engine's own projects and tests are switched off.

## Testing on the platforms
- **Windows:** drive applications with PowerShell (`SendInput`/`mouse_event`; VK keys need their scan code from `MapVirtualKey`, `KEYEVENTF_UNICODE` types any character), screenshot with `System.Drawing` `CopyFromScreen` (after `SetProcessDPIAware`), use `Get-Clipboard`/`Set-Clipboard`. `SendKeys` goes to whatever has the focus.
- **Linux:** WSLg. Drive with `xdotool` (the engine's X11 window is found by `--name "Captains Game Engine"`, not by `--pid`). WSLg windows are Windows windows, so screenshots come from PowerShell's `CopyFromScreen` when the window is in front; when other windows cover it (`PrintWindow` gives black), a small X11 client that saves `XGetImage` of the window as PPM does it. WSLg bridges the X11 CLIPBOARD to Windows' clipboard, but only for the focused client. `sudo` needs a password, so nothing can be installed with apt from a session; small test X11 clients build with `g++ ... -lX11`.
- **Web:** jewelhunter's `tools/webtest.mjs` drives headless Chrome (mouse, touch, keys, input-method text, screenshots). A debug build (`CMAKE_BUILD_TYPE=Debug`, `-sASSERTIONS=2`) gives function names in stack traces; `emsymbolizer <file>.wasm <address>` gives the source line.
- **Android:** the emulator `Medium_Phone_API_37.0` (x86_64), through jewelhunter's Gradle project. Logs: `adb logcat -s CGE`, crashes `adb logcat -b crash`.

## This machine's paths
Specific to the original machine; on another computer, write `CMakeUserPresets.json` with that machine's paths.
- `windows-local`: Visual Studio 2026; `JAVA_HOME` Android Studio's JDK `C:/Program Files/Android/Android Studio/jbr` (the Adventure's ANTLR parser is generated with a JDK); no OpenAL: it's optional, and without it the Adventure has no sound until its sound moves to miniaudio (Phase 1c step 7).
- `android-local`: NDK `%LOCALAPPDATA%\Android\Sdk\ndk\30.0.16248370`.
- emsdk 4.0.22 in `C:\Projects\emsdk` (`emsdk_env.ps1` sets `EMSDK`). Ninja from winget (`%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_*`).
- WSL (Ubuntu 26.04, GCC 15) with `build-essential cmake ninja-build libx11-dev libgl1-mesa-dev libglu1-mesa-dev libegl-dev libopenal-dev libfontconfig-dev default-jdk-headless xdotool`; its build is `build/default`.
