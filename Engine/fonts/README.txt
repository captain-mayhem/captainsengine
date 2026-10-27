The engine's fonts (the same files as in guitads' imgui/misc/fonts).

DejaVuSans.ttf              DejaVu Sans 2.37, the engine's default font
                            (Font::load with an empty path, ImGui). Latin,
                            Greek, Cyrillic, Armenian, Georgian, Hebrew,
                            Arabic and many symbols. License: DejaVu-LICENSE.txt
                            (Bitstream Vera / DejaVu, free).
LiberationMono-Regular.ttf  Liberation Mono 2.1.5, the monospace font of the
                            engine's screen texts (ScreenText). License:
                            Liberation-LICENSE.txt (SIL Open Font License 1.1).

Applications ship the fonts they use: the desktop install copies this folder
next to the application (cge_install_runtime), web pages preload
fonts/DejaVuSans.ttf, Android packs it as an asset.
