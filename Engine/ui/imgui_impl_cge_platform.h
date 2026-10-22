#ifndef CGE_IMGUI_IMPL_CGE_PLATFORM_H
#define CGE_IMGUI_IMPL_CGE_PLATFORM_H

#include <cge_export.h>

#include <imgui.h>

namespace CGE{
class AppWindow;
}

// The engine's Dear ImGui platform backend: the display size, the time, the
// window's content scale (DPI), the mouse cursor and the clipboard (through
// the platform services) for ImGui. The engine's window code passes the input
// events on with the ImGui_ImplCGE_Platform_Add* functions, before the engine
// hands them to the application (Input::Keyboard, Input::Mouse), which doesn't
// get the events ImGui wants (io.WantCaptureKeyboard, io.WantTextInput).
//
// The engine sets it up for its window (Engine::startup). Mouse positions are
// window pixels, so ImGui works in window pixels, not in virtual coordinates.
// Windows and Linux (X11) for now; touch and the web come in Phase 1c.

//! window: the window ImGui is shown in
CGE_API bool ImGui_ImplCGE_Platform_Init(CGE::AppWindow* window);
CGE_API void ImGui_ImplCGE_Platform_Shutdown();
//! before ImGui::NewFrame
CGE_API void ImGui_ImplCGE_Platform_NewFrame();

//! the mouse is at (x, y) window pixels
CGE_API void ImGui_ImplCGE_Platform_AddMousePos(float x, float y);
//! the mouse left the window
CGE_API void ImGui_ImplCGE_Platform_AddMouseLeave();
//! button is an ImGuiMouseButton (0 left, 1 right, 2 middle)
CGE_API void ImGui_ImplCGE_Platform_AddMouseButton(int button, bool down);
//! wheel notches; positive y scrolls up, positive x to the left
CGE_API void ImGui_ImplCGE_Platform_AddMouseWheel(float x, float y);
//! a key in the engine's codes (input/keyboard.h: virtual keys on Windows, KeySyms on X11)
CGE_API void ImGui_ImplCGE_Platform_AddKey(int key, bool down);
//! the state of the modifier keys
CGE_API void ImGui_ImplCGE_Platform_AddModifiers(bool ctrl, bool shift, bool alt, bool super);
//! a typed character in the window system's encoding (the ANSI code page on Windows, Latin-1 on X11)
CGE_API void ImGui_ImplCGE_Platform_AddChar(unsigned int ch);
//! the window got or lost the keyboard focus
CGE_API void ImGui_ImplCGE_Platform_AddFocus(bool focused);

//! the ImGui key of an engine key code, ImGuiKey_None if there is none
CGE_API ImGuiKey ImGui_ImplCGE_Platform_ToImGuiKey(int key);

#endif
