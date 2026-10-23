#include "imgui_impl_cge_platform.h"

#include <cfloat>
#include <chrono>
#include <string>

#include "../window/window.h"
#include "../platform/services.h"
#include "../platform/platform_internal.h"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(LINUX)
#include <X11/keysym.h>
#elif defined(ANDROID)
#include <android/keycodes.h>
#endif

using namespace CGE;

namespace{

//! the backend's state in io.BackendPlatformUserData
struct BackendData{
  AppWindow* window = nullptr;
  std::chrono::steady_clock::time_point time;
  //! the content scale the style is scaled with
  float scale = 1;
  //! the cursor shown instead of the application's, -1 for none
  int cursor = -1;
  //! the text copied last, for platforms without a clipboard service
  std::string clipboard;
};

BackendData* getData(){
  return ImGui::GetCurrentContext() ? (BackendData*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

const char* getClipboardText(ImGuiContext*){
  BackendData* bd = getData();
  std::string text = Platform::getClipboardText();
  if (!text.empty())
    bd->clipboard = text;
  return bd->clipboard.c_str();
}

void setClipboardText(ImGuiContext*, const char* text){
  BackendData* bd = getData();
  bd->clipboard = text;
  Platform::setClipboardText(text);
}

//! the platform cursor of an ImGui cursor
int toPlatformCursor(ImGuiMouseCursor cursor){
  switch (cursor){
    case ImGuiMouseCursor_TextInput: return Platform::CURSOR_IBEAM;
    case ImGuiMouseCursor_ResizeAll: return Platform::CURSOR_MOVE;
    case ImGuiMouseCursor_ResizeNS: return Platform::CURSOR_RESIZE_V;
    case ImGuiMouseCursor_ResizeEW: return Platform::CURSOR_RESIZE_H;
    //the platform services have no diagonal cursors
    case ImGuiMouseCursor_ResizeNESW: return Platform::CURSOR_MOVE;
    case ImGuiMouseCursor_ResizeNWSE: return Platform::CURSOR_MOVE;
    case ImGuiMouseCursor_Hand: return Platform::CURSOR_HAND;
    case ImGuiMouseCursor_Wait: return Platform::CURSOR_WAIT;
    case ImGuiMouseCursor_Progress: return Platform::CURSOR_WAIT;
    case ImGuiMouseCursor_NotAllowed: return Platform::CURSOR_FORBIDDEN;
    default: return Platform::CURSOR_ARROW;
  }
}

void updateCursor(BackendData& bd){
  ImGuiIO& io = ImGui::GetIO();
  if (!bd.window || (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
    return;
  //ImGui's cursor only over its windows; elsewhere the application's
  int cursor = io.WantCaptureMouse ? toPlatformCursor(ImGui::GetMouseCursor()) : -1;
  if (cursor == bd.cursor)
    return;
  bd.cursor = cursor;
  PlatformInternal::setOverrideCursor(cursor);
}

//! scales the style's sizes and the fonts to the content scale
void updateScale(BackendData& bd){
  if (!bd.window)
    return;
  float scale = bd.window->getContentScale();
  if (scale <= 0 || scale == bd.scale)
    return;
  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(scale/bd.scale);
  style.FontScaleDpi = scale;
  bd.scale = scale;
}

bool isModifier(ImGuiKey key, ImGuiKey& mod){
  switch (key){
    case ImGuiKey_LeftCtrl: case ImGuiKey_RightCtrl: mod = ImGuiMod_Ctrl; return true;
    case ImGuiKey_LeftShift: case ImGuiKey_RightShift: mod = ImGuiMod_Shift; return true;
    case ImGuiKey_LeftAlt: case ImGuiKey_RightAlt: mod = ImGuiMod_Alt; return true;
    case ImGuiKey_LeftSuper: case ImGuiKey_RightSuper: mod = ImGuiMod_Super; return true;
    default: return false;
  }
}

}

bool ImGui_ImplCGE_Platform_Init(AppWindow* window){
  ImGuiIO& io = ImGui::GetIO();
  IM_ASSERT(io.BackendPlatformUserData == nullptr && "the platform backend is already initialised");
  BackendData* bd = new BackendData();
  bd->window = window;
  bd->time = std::chrono::steady_clock::now();
  io.BackendPlatformUserData = bd;
  io.BackendPlatformName = "imgui_impl_cge_platform";
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
  platformIO.Platform_GetClipboardTextFn = getClipboardText;
  platformIO.Platform_SetClipboardTextFn = setClipboardText;
  //ImGui's IME support on Windows positions the candidate window with it
  if (window)
    ImGui::GetMainViewport()->PlatformHandleRaw = window->getHandle();
  updateScale(*bd);
  return true;
}

void ImGui_ImplCGE_Platform_Shutdown(){
  BackendData* bd = getData();
  if (!bd)
    return;
  if (bd->cursor >= 0)
    PlatformInternal::setOverrideCursor(-1);
  ImGuiIO& io = ImGui::GetIO();
  io.BackendPlatformName = nullptr;
  io.BackendPlatformUserData = nullptr;
  io.BackendFlags &= ~ImGuiBackendFlags_HasMouseCursors;
  ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
  platformIO.Platform_GetClipboardTextFn = nullptr;
  platformIO.Platform_SetClipboardTextFn = nullptr;
  ImGui::GetMainViewport()->PlatformHandleRaw = nullptr;
  delete bd;
}

void ImGui_ImplCGE_Platform_NewFrame(){
  BackendData* bd = getData();
  IM_ASSERT(bd != nullptr && "ImGui_ImplCGE_Platform_Init wasn't called");
  ImGuiIO& io = ImGui::GetIO();
  //without a window, the caller sets io.DisplaySize
  if (bd->window)
    io.DisplaySize = ImVec2((float)bd->window->getWidth(), (float)bd->window->getHeight());
  io.DisplayFramebufferScale = ImVec2(1, 1);
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  float dt = std::chrono::duration<float>(now - bd->time).count();
  io.DeltaTime = dt > 0 ? dt : 1e-5f;
  bd->time = now;
  updateScale(*bd);
  updateCursor(*bd);
}

void ImGui_ImplCGE_Platform_AddMousePos(float x, float y){
  if (getData())
    ImGui::GetIO().AddMousePosEvent(x, y);
}

void ImGui_ImplCGE_Platform_AddMouseLeave(){
  if (getData())
    ImGui::GetIO().AddMousePosEvent(-FLT_MAX, -FLT_MAX);
}

void ImGui_ImplCGE_Platform_AddMouseButton(int button, bool down){
  if (getData() && button >= 0 && button < ImGuiMouseButton_COUNT)
    ImGui::GetIO().AddMouseButtonEvent(button, down);
}

void ImGui_ImplCGE_Platform_AddMouseWheel(float x, float y){
  if (getData())
    ImGui::GetIO().AddMouseWheelEvent(x, y);
}

void ImGui_ImplCGE_Platform_AddKey(int key, bool down){
  if (!getData())
    return;
  ImGuiKey imKey = ImGui_ImplCGE_Platform_ToImGuiKey(key);
  if (imKey == ImGuiKey_None)
    return;
  ImGuiIO& io = ImGui::GetIO();
  //a modifier key also changes the modifier state (X11 reports the state before the key)
  ImGuiKey mod;
  if (isModifier(imKey, mod))
    io.AddKeyEvent(mod, down);
  io.AddKeyEvent(imKey, down);
  io.SetKeyEventNativeData(imKey, key, -1);
}

void ImGui_ImplCGE_Platform_AddModifiers(bool ctrl, bool shift, bool alt, bool super){
  if (!getData())
    return;
  ImGuiIO& io = ImGui::GetIO();
  io.AddKeyEvent(ImGuiMod_Ctrl, ctrl);
  io.AddKeyEvent(ImGuiMod_Shift, shift);
  io.AddKeyEvent(ImGuiMod_Alt, alt);
  io.AddKeyEvent(ImGuiMod_Super, super);
}

void ImGui_ImplCGE_Platform_AddChar(unsigned int ch){
  if (!getData() || ch < 0x20 || ch == 0x7f)
    return;
  ImGuiIO& io = ImGui::GetIO();
#if defined(WIN32)
  //the window is an ANSI window, so its characters are in the ANSI code page
  if (ch >= 0x80){
    char byte = (char)ch;
    wchar_t wide = 0;
    if (MultiByteToWideChar(CP_ACP, 0, &byte, 1, &wide, 1) == 1)
      io.AddInputCharacterUTF16((ImWchar16)wide);
    return;
  }
#endif
  //Latin-1 on X11, which are the first 256 Unicode code points; code points on the web
  io.AddInputCharacter(ch);
}

void ImGui_ImplCGE_Platform_AddFocus(bool focused){
  if (getData())
    ImGui::GetIO().AddFocusEvent(focused);
}

ImGuiKey ImGui_ImplCGE_Platform_ToImGuiKey(int key){
#if defined(WIN32)
  if (key >= '0' && key <= '9')
    return (ImGuiKey)(ImGuiKey_0 + (key - '0'));
  if (key >= 'A' && key <= 'Z')
    return (ImGuiKey)(ImGuiKey_A + (key - 'A'));
  if (key >= VK_F1 && key <= VK_F24)
    return (ImGuiKey)(ImGuiKey_F1 + (key - VK_F1));
  if (key >= VK_NUMPAD0 && key <= VK_NUMPAD9)
    return (ImGuiKey)(ImGuiKey_Keypad0 + (key - VK_NUMPAD0));
  switch (key){
    case VK_TAB: return ImGuiKey_Tab;
    case VK_LEFT: return ImGuiKey_LeftArrow;
    case VK_RIGHT: return ImGuiKey_RightArrow;
    case VK_UP: return ImGuiKey_UpArrow;
    case VK_DOWN: return ImGuiKey_DownArrow;
    case VK_PRIOR: return ImGuiKey_PageUp;
    case VK_NEXT: return ImGuiKey_PageDown;
    case VK_HOME: return ImGuiKey_Home;
    case VK_END: return ImGuiKey_End;
    case VK_INSERT: return ImGuiKey_Insert;
    case VK_DELETE: return ImGuiKey_Delete;
    case VK_BACK: return ImGuiKey_Backspace;
    case VK_SPACE: return ImGuiKey_Space;
    case VK_RETURN: return ImGuiKey_Enter;
    case VK_ESCAPE: return ImGuiKey_Escape;
    case VK_OEM_7: return ImGuiKey_Apostrophe;
    case VK_OEM_COMMA: return ImGuiKey_Comma;
    case VK_OEM_MINUS: return ImGuiKey_Minus;
    case VK_OEM_PERIOD: return ImGuiKey_Period;
    case VK_OEM_2: return ImGuiKey_Slash;
    case VK_OEM_1: return ImGuiKey_Semicolon;
    case VK_OEM_PLUS: return ImGuiKey_Equal;
    case VK_OEM_4: return ImGuiKey_LeftBracket;
    case VK_OEM_5: return ImGuiKey_Backslash;
    case VK_OEM_6: return ImGuiKey_RightBracket;
    case VK_OEM_3: return ImGuiKey_GraveAccent;
    case VK_CAPITAL: return ImGuiKey_CapsLock;
    case VK_SCROLL: return ImGuiKey_ScrollLock;
    case VK_NUMLOCK: return ImGuiKey_NumLock;
    case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
    case VK_PAUSE: return ImGuiKey_Pause;
    case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
    case VK_DIVIDE: return ImGuiKey_KeypadDivide;
    case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
    case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
    case VK_ADD: return ImGuiKey_KeypadAdd;
    //WM_KEYDOWN reports these without the side
    case VK_SHIFT: return ImGuiKey_LeftShift;
    case VK_CONTROL: return ImGuiKey_LeftCtrl;
    case VK_MENU: return ImGuiKey_LeftAlt;
    case VK_LSHIFT: return ImGuiKey_LeftShift;
    case VK_LCONTROL: return ImGuiKey_LeftCtrl;
    case VK_LMENU: return ImGuiKey_LeftAlt;
    case VK_LWIN: return ImGuiKey_LeftSuper;
    case VK_RSHIFT: return ImGuiKey_RightShift;
    case VK_RCONTROL: return ImGuiKey_RightCtrl;
    case VK_RMENU: return ImGuiKey_RightAlt;
    case VK_RWIN: return ImGuiKey_RightSuper;
    case VK_APPS: return ImGuiKey_Menu;
    default: return ImGuiKey_None;
  }
#elif defined(LINUX)
  //the KeySyms of the unshifted keys
  if (key >= XK_0 && key <= XK_9)
    return (ImGuiKey)(ImGuiKey_0 + (key - XK_0));
  if (key >= XK_a && key <= XK_z)
    return (ImGuiKey)(ImGuiKey_A + (key - XK_a));
  if (key >= XK_A && key <= XK_Z)
    return (ImGuiKey)(ImGuiKey_A + (key - XK_A));
  if (key >= XK_F1 && key <= XK_F24)
    return (ImGuiKey)(ImGuiKey_F1 + (key - XK_F1));
  if (key >= XK_KP_0 && key <= XK_KP_9)
    return (ImGuiKey)(ImGuiKey_Keypad0 + (key - XK_KP_0));
  switch (key){
    case XK_Tab: return ImGuiKey_Tab;
    case XK_ISO_Left_Tab: return ImGuiKey_Tab;
    case XK_Left: return ImGuiKey_LeftArrow;
    case XK_Right: return ImGuiKey_RightArrow;
    case XK_Up: return ImGuiKey_UpArrow;
    case XK_Down: return ImGuiKey_DownArrow;
    case XK_Page_Up: return ImGuiKey_PageUp;
    case XK_Page_Down: return ImGuiKey_PageDown;
    case XK_Home: return ImGuiKey_Home;
    case XK_End: return ImGuiKey_End;
    case XK_Insert: return ImGuiKey_Insert;
    case XK_Delete: return ImGuiKey_Delete;
    case XK_BackSpace: return ImGuiKey_Backspace;
    case XK_space: return ImGuiKey_Space;
    case XK_Return: return ImGuiKey_Enter;
    case XK_Escape: return ImGuiKey_Escape;
    case XK_apostrophe: return ImGuiKey_Apostrophe;
    case XK_comma: return ImGuiKey_Comma;
    case XK_minus: return ImGuiKey_Minus;
    case XK_period: return ImGuiKey_Period;
    case XK_slash: return ImGuiKey_Slash;
    case XK_semicolon: return ImGuiKey_Semicolon;
    case XK_equal: return ImGuiKey_Equal;
    case XK_bracketleft: return ImGuiKey_LeftBracket;
    case XK_backslash: return ImGuiKey_Backslash;
    case XK_bracketright: return ImGuiKey_RightBracket;
    case XK_grave: return ImGuiKey_GraveAccent;
    case XK_Caps_Lock: return ImGuiKey_CapsLock;
    case XK_Scroll_Lock: return ImGuiKey_ScrollLock;
    case XK_Num_Lock: return ImGuiKey_NumLock;
    case XK_Print: return ImGuiKey_PrintScreen;
    case XK_Pause: return ImGuiKey_Pause;
    case XK_KP_Decimal: return ImGuiKey_KeypadDecimal;
    case XK_KP_Divide: return ImGuiKey_KeypadDivide;
    case XK_KP_Multiply: return ImGuiKey_KeypadMultiply;
    case XK_KP_Subtract: return ImGuiKey_KeypadSubtract;
    case XK_KP_Add: return ImGuiKey_KeypadAdd;
    case XK_KP_Enter: return ImGuiKey_KeypadEnter;
    case XK_KP_Equal: return ImGuiKey_KeypadEqual;
    case XK_Shift_L: return ImGuiKey_LeftShift;
    case XK_Control_L: return ImGuiKey_LeftCtrl;
    case XK_Alt_L: return ImGuiKey_LeftAlt;
    case XK_Super_L: return ImGuiKey_LeftSuper;
    case XK_Shift_R: return ImGuiKey_RightShift;
    case XK_Control_R: return ImGuiKey_RightCtrl;
    case XK_Alt_R: return ImGuiKey_RightAlt;
    case XK_ISO_Level3_Shift: return ImGuiKey_RightAlt;
    case XK_Super_R: return ImGuiKey_RightSuper;
    case XK_Menu: return ImGuiKey_Menu;
    default: return ImGuiKey_None;
  }
#elif defined(__EMSCRIPTEN__)
  //KeyboardEvent.keyCode; the same values as Windows' virtual keys
  if (key >= '0' && key <= '9')
    return (ImGuiKey)(ImGuiKey_0 + (key - '0'));
  if (key >= 'A' && key <= 'Z')
    return (ImGuiKey)(ImGuiKey_A + (key - 'A'));
  if (key >= 112 && key <= 135)
    return (ImGuiKey)(ImGuiKey_F1 + (key - 112));
  if (key >= 96 && key <= 105)
    return (ImGuiKey)(ImGuiKey_Keypad0 + (key - 96));
  switch (key){
    case 9: return ImGuiKey_Tab;
    case 37: return ImGuiKey_LeftArrow;
    case 39: return ImGuiKey_RightArrow;
    case 38: return ImGuiKey_UpArrow;
    case 40: return ImGuiKey_DownArrow;
    case 33: return ImGuiKey_PageUp;
    case 34: return ImGuiKey_PageDown;
    case 36: return ImGuiKey_Home;
    case 35: return ImGuiKey_End;
    case 45: return ImGuiKey_Insert;
    case 46: return ImGuiKey_Delete;
    case 8: return ImGuiKey_Backspace;
    case 32: return ImGuiKey_Space;
    case 13: return ImGuiKey_Enter;
    case 27: return ImGuiKey_Escape;
    case 222: return ImGuiKey_Apostrophe;
    case 188: return ImGuiKey_Comma;
    case 189: return ImGuiKey_Minus;
    case 173: return ImGuiKey_Minus; //Firefox
    case 190: return ImGuiKey_Period;
    case 191: return ImGuiKey_Slash;
    case 186: return ImGuiKey_Semicolon;
    case 59: return ImGuiKey_Semicolon; //Firefox
    case 187: return ImGuiKey_Equal;
    case 61: return ImGuiKey_Equal; //Firefox
    case 219: return ImGuiKey_LeftBracket;
    case 220: return ImGuiKey_Backslash;
    case 221: return ImGuiKey_RightBracket;
    case 192: return ImGuiKey_GraveAccent;
    case 20: return ImGuiKey_CapsLock;
    case 145: return ImGuiKey_ScrollLock;
    case 144: return ImGuiKey_NumLock;
    case 44: return ImGuiKey_PrintScreen;
    case 19: return ImGuiKey_Pause;
    case 110: return ImGuiKey_KeypadDecimal;
    case 111: return ImGuiKey_KeypadDivide;
    case 106: return ImGuiKey_KeypadMultiply;
    case 109: return ImGuiKey_KeypadSubtract;
    case 107: return ImGuiKey_KeypadAdd;
    //keyCode doesn't tell the sides apart
    case 16: return ImGuiKey_LeftShift;
    case 17: return ImGuiKey_LeftCtrl;
    case 18: return ImGuiKey_LeftAlt;
    case 91: return ImGuiKey_LeftSuper;
    case 92: return ImGuiKey_RightSuper;
    case 93: return ImGuiKey_Menu;
    default: return ImGuiKey_None;
  }
#elif defined(ANDROID)
  //AKEYCODE_
  if (key >= AKEYCODE_0 && key <= AKEYCODE_9)
    return (ImGuiKey)(ImGuiKey_0 + (key - AKEYCODE_0));
  if (key >= AKEYCODE_A && key <= AKEYCODE_Z)
    return (ImGuiKey)(ImGuiKey_A + (key - AKEYCODE_A));
  if (key >= AKEYCODE_F1 && key <= AKEYCODE_F12)
    return (ImGuiKey)(ImGuiKey_F1 + (key - AKEYCODE_F1));
  if (key >= AKEYCODE_NUMPAD_0 && key <= AKEYCODE_NUMPAD_9)
    return (ImGuiKey)(ImGuiKey_Keypad0 + (key - AKEYCODE_NUMPAD_0));
  switch (key){
    case AKEYCODE_TAB: return ImGuiKey_Tab;
    case AKEYCODE_DPAD_LEFT: return ImGuiKey_LeftArrow;
    case AKEYCODE_DPAD_RIGHT: return ImGuiKey_RightArrow;
    case AKEYCODE_DPAD_UP: return ImGuiKey_UpArrow;
    case AKEYCODE_DPAD_DOWN: return ImGuiKey_DownArrow;
    case AKEYCODE_PAGE_UP: return ImGuiKey_PageUp;
    case AKEYCODE_PAGE_DOWN: return ImGuiKey_PageDown;
    case AKEYCODE_MOVE_HOME: return ImGuiKey_Home;
    case AKEYCODE_MOVE_END: return ImGuiKey_End;
    case AKEYCODE_INSERT: return ImGuiKey_Insert;
    case AKEYCODE_FORWARD_DEL: return ImGuiKey_Delete;
    case AKEYCODE_DEL: return ImGuiKey_Backspace;
    case AKEYCODE_SPACE: return ImGuiKey_Space;
    case AKEYCODE_ENTER: return ImGuiKey_Enter;
    case AKEYCODE_ESCAPE: return ImGuiKey_Escape;
    case AKEYCODE_APOSTROPHE: return ImGuiKey_Apostrophe;
    case AKEYCODE_COMMA: return ImGuiKey_Comma;
    case AKEYCODE_MINUS: return ImGuiKey_Minus;
    case AKEYCODE_PERIOD: return ImGuiKey_Period;
    case AKEYCODE_SLASH: return ImGuiKey_Slash;
    case AKEYCODE_SEMICOLON: return ImGuiKey_Semicolon;
    case AKEYCODE_EQUALS: return ImGuiKey_Equal;
    case AKEYCODE_LEFT_BRACKET: return ImGuiKey_LeftBracket;
    case AKEYCODE_BACKSLASH: return ImGuiKey_Backslash;
    case AKEYCODE_RIGHT_BRACKET: return ImGuiKey_RightBracket;
    case AKEYCODE_GRAVE: return ImGuiKey_GraveAccent;
    case AKEYCODE_CAPS_LOCK: return ImGuiKey_CapsLock;
    case AKEYCODE_SCROLL_LOCK: return ImGuiKey_ScrollLock;
    case AKEYCODE_NUM_LOCK: return ImGuiKey_NumLock;
    case AKEYCODE_SYSRQ: return ImGuiKey_PrintScreen;
    case AKEYCODE_BREAK: return ImGuiKey_Pause;
    case AKEYCODE_NUMPAD_DOT: return ImGuiKey_KeypadDecimal;
    case AKEYCODE_NUMPAD_DIVIDE: return ImGuiKey_KeypadDivide;
    case AKEYCODE_NUMPAD_MULTIPLY: return ImGuiKey_KeypadMultiply;
    case AKEYCODE_NUMPAD_SUBTRACT: return ImGuiKey_KeypadSubtract;
    case AKEYCODE_NUMPAD_ADD: return ImGuiKey_KeypadAdd;
    case AKEYCODE_NUMPAD_ENTER: return ImGuiKey_KeypadEnter;
    case AKEYCODE_NUMPAD_EQUALS: return ImGuiKey_KeypadEqual;
    case AKEYCODE_SHIFT_LEFT: return ImGuiKey_LeftShift;
    case AKEYCODE_CTRL_LEFT: return ImGuiKey_LeftCtrl;
    case AKEYCODE_ALT_LEFT: return ImGuiKey_LeftAlt;
    case AKEYCODE_META_LEFT: return ImGuiKey_LeftSuper;
    case AKEYCODE_SHIFT_RIGHT: return ImGuiKey_RightShift;
    case AKEYCODE_CTRL_RIGHT: return ImGuiKey_RightCtrl;
    case AKEYCODE_ALT_RIGHT: return ImGuiKey_RightAlt;
    case AKEYCODE_META_RIGHT: return ImGuiKey_RightSuper;
    case AKEYCODE_MENU: return ImGuiKey_Menu;
    default: return ImGuiKey_None;
  }
#else
  (void)key;
  return ImGuiKey_None;
#endif
}
