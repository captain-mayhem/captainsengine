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
// On Android and the web, the primary finger of a touch screen is ImGui's
// mouse (Input::Touch), and an active text field brings up the on-screen
// keyboard (Platform::startTextInput; again with a tap into the field, if the
// user closed it).

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
//! the mouse events that follow come from a touch screen (true) or a mouse
CGE_API void ImGui_ImplCGE_Platform_AddMouseSource(bool touch);
//! wheel notches; positive y scrolls up, positive x to the left
CGE_API void ImGui_ImplCGE_Platform_AddMouseWheel(float x, float y);
//! a key in the engine's codes (input/keyboard.h: virtual keys on Windows, KeySyms on X11, keyCodes on the web)
CGE_API void ImGui_ImplCGE_Platform_AddKey(int key, bool down);
//! the state of the modifier keys
CGE_API void ImGui_ImplCGE_Platform_AddModifiers(bool ctrl, bool shift, bool alt, bool super);
//! a typed character as a Unicode code point
CGE_API void ImGui_ImplCGE_Platform_AddChar(unsigned int ch);
//! the window got or lost the keyboard focus
CGE_API void ImGui_ImplCGE_Platform_AddFocus(bool focused);

//! the ImGui key of an engine key code, ImGuiKey_None if there is none
CGE_API ImGuiKey ImGui_ImplCGE_Platform_ToImGuiKey(int key);

#endif
