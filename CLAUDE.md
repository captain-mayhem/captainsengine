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
- Where things are: the windows per platform are `window/native*.cpp` (`nativeLinux` is X11; the X11 event loop is `Engine::processEvents` in `system/engine.cpp`); input is `input/`; the platform services (clipboard, cursors, text input, URLs, settings, user folders, assets) are `platform/`, with the table of what exists where in `platform/services.h` and engine-internal hooks in `platform/platform_internal.h`; the ImGui backends are `ui/imgui_impl_cge_*`; the renderer is `renderer/` with the GL backend in `renderer/GL`; text is `renderer/font.h` (FreeType through the `SpriteBatch`) and `renderer/screentext.h` (the old print queue, `Engine::getScreenText`), the system font lookup `platform/systemfonts.h`, and the engine's fonts are `Engine/fonts/`. Audio is `audio/` (miniaudio; `audio_internal.h` keeps miniaudio out of the public headers); video is `video/` (pl_mpeg and libtheora; `CGE::Video` plays, `CGE::VideoDecoder` decodes). The StoryDesigner's video converter is `Adventure/Editor/VideoConverter` (a 32-bit ExternalProject with Visual Studio). The Android activity's Java code is `Engine/android/java` (`CgeActivity`, `TextInputView`); the web shell page is `Engine/web/shell.html`.

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
| Android | `cmake --build --preset android-local` (configured with `-DPROJECT_HEROQUEST=OFF -DPROJECT_OSMRACER=OFF`; those are desktop programs) |
| Adventure APKs | PowerShell, in `Adventure/android`: `$env:JAVA_HOME = "C:\Program Files\Android\Android Studio\jbr"; .\gradlew.bat assembleDebug` (flavors `player` and `game`, see the README) |

**Regression check** (expected for every step): the Windows build of all projects with no errors and no warnings; the Linux build with only the known warnings (`tmpnam` in the vendored Lua, and `getcwd` in HQServer, which only appears when it's recompiled); the unit tests passing on Windows and Linux (185 on Windows and 184 on Linux as of Phase 1c step 9); no new warnings on the web and Android; and the projects a change touches started and tried (jewelhunter, the Adventure demo, HQClient, the editor).

Built as part of jewelhunter (`CGE_DIR`), the engine's own projects and tests are switched off.

## Testing on the platforms
- **Windows:** drive applications with PowerShell (`SendInput`/`mouse_event`; VK keys need their scan code from `MapVirtualKey`, `KEYEVENTF_UNICODE` types any character), screenshot with `System.Drawing` `CopyFromScreen` (after `SetProcessDPIAware`), use `Get-Clipboard`/`Set-Clipboard`. `SendKeys` goes to whatever has the focus.
- **Linux:** WSLg. Drive with `xdotool` (the engine's X11 window is found by `--name "Captains Game Engine"`, not by `--pid`). WSLg windows are Windows windows, so screenshots come from PowerShell's `CopyFromScreen` when the window is in front; when other windows cover it (`PrintWindow` gives black), a small X11 client that saves `XGetImage` of the window as PPM does it. WSLg bridges the X11 CLIPBOARD to Windows' clipboard, but only for the focused client. `sudo` needs a password, so nothing can be installed with apt from a session; small test X11 clients build with `g++ ... -lX11`.
- **Web:** jewelhunter's `tools/webtest.mjs` drives headless Chrome (mouse, touch, keys, input-method text, screenshots). A debug build (`CMAKE_BUILD_TYPE=Debug`, `-sASSERTIONS=2`) gives function names in stack traces; `emsymbolizer <file>.wasm <address>` gives the source line.
- **Android:** the emulator `Medium_Phone_API_37.0` (x86_64), through jewelhunter's Gradle project or the Adventure's (`Adventure/android`; the game APK `de.captain_online.adventure.demo` has the demo, the player `de.captain_online.adventure` adds games with the system's file picker, which `uiautomator dump` and taps on the found texts drive). Logs: `adb logcat -s CGE`, crashes `adb logcat -b crash`. In the Adventure, Ctrl+R (`adb shell input keycombination KEYCODE_CTRL_LEFT KEYCODE_R`) restarts the render device; `run-as <app id> ls files/bin` shows the demo's saves.
- **Audio:** the unit tests mix without a device (`Audio::Desc::device = false`, `Audio::render`). What the applications really play: jewelhunter's `tools/loopback` records Windows' output (WASAPI loopback), which also carries WSLg's Linux sound and the Android emulator's; `tools/audio_levels.py` prints the levels over time. In the browser, an `AnalyserNode` on miniaudio's `scriptNode` (jewelhunter's `tools/webtest_audio.json`).

## This machine's paths
Specific to the original machine; on another computer, write `CMakeUserPresets.json` with that machine's paths.
- `windows-local`: Visual Studio 2026; `JAVA_HOME` Android Studio's JDK `C:/Program Files/Android/Android Studio/jbr` (the Adventure's ANTLR parser is generated with a JDK).
- `android-local`: NDK `%LOCALAPPDATA%\Android\Sdk\ndk\30.0.16248370`.
- emsdk 4.0.22 in `C:\Projects\emsdk` (`emsdk_env.ps1` sets `EMSDK`). Ninja from winget (`%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_*`).
- WSL (Ubuntu 26.04, GCC 15) with `build-essential cmake ninja-build libx11-dev libgl1-mesa-dev libglu1-mesa-dev libegl-dev libfontconfig-dev default-jdk-headless xdotool`; its build is `build/default`.
