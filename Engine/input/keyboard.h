#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <cge_export.h>

#include <cstddef>
#include <list>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define KEY_BACKSPACE VK_BACK
#define KEY_UP VK_UP
#define KEY_DOWN VK_DOWN
#define KEY_LEFT VK_LEFT
#define KEY_RIGHT VK_RIGHT
#define KEY_ESCAPE VK_ESCAPE
#define KEY_RETURN VK_RETURN
#define KEY_DELETE VK_DELETE
#define KEY_SPACE VK_SPACE
#define KEY_CTRL VK_CONTROL
#define KEY_ALT VK_MENU
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_A 'A'
#define KEY_C 'C'
#define KEY_D 'D'
#define KEY_I 'I'
#define KEY_J 'J'
#define KEY_K 'K'
#define KEY_L 'L'
#define KEY_R 'R'
#define KEY_S 'S'
#define KEY_T 'T'
#define KEY_W 'W'
#define KEY_0 '0'
#define KEY_1 '1'
#define KEY_2 '2'
#define KEY_CIRCUMFLEX VK_OEM_5
#define KEY_PAGEUP VK_PRIOR
#define KEY_PAGEDOWN VK_NEXT
#endif
#ifdef LINUX
#include <X11/keysym.h>
#define KEY_BACKSPACE XK_BackSpace
#define KEY_UP XK_Up
#define KEY_DOWN XK_Down
#define KEY_LEFT XK_Left
#define KEY_RIGHT XK_Right
#define KEY_ESCAPE XK_Escape
#define KEY_RETURN XK_Return
#define KEY_DELETE XK_Delete
#define KEY_SPACE XK_space
#define KEY_CTRL XK_Control_L
#define KEY_ALT XK_Alt_L
#define KEY_F1 XK_F1
#define KEY_F2 XK_F2
#define KEY_A XK_a
#define KEY_C XK_c
#define KEY_D XK_d
#define KEY_I XK_i
#define KEY_J XK_j
#define KEY_K XK_k
#define KEY_L XK_l
#define KEY_R XK_r
#define KEY_S XK_s
#define KEY_T XK_t
#define KEY_W XK_w
#define KEY_0 XK_0
#define KEY_1 XK_1
#define KEY_2 XK_2
#define KEY_CIRCUMFLEX XK_dead_circumflex
#define KEY_PAGEUP XK_Page_Up
#define KEY_PAGEDOWN XK_Page_Down
#endif
#if defined(ANDROID)
#include <android/keycodes.h>
//Android's key codes; the back button comes as KEY_ESCAPE
#define KEY_ESCAPE AKEYCODE_ESCAPE
#define KEY_RETURN AKEYCODE_ENTER
#define KEY_BACKSPACE AKEYCODE_DEL
#define KEY_UP AKEYCODE_DPAD_UP
#define KEY_DOWN AKEYCODE_DPAD_DOWN
#define KEY_LEFT AKEYCODE_DPAD_LEFT
#define KEY_RIGHT AKEYCODE_DPAD_RIGHT
#define KEY_F1 AKEYCODE_F1
#define KEY_F2 AKEYCODE_F2
#define KEY_CTRL AKEYCODE_CTRL_LEFT
#define KEY_ALT AKEYCODE_ALT_LEFT
#define KEY_DELETE AKEYCODE_FORWARD_DEL
#define KEY_PAGEUP AKEYCODE_PAGE_UP
#define KEY_PAGEDOWN AKEYCODE_PAGE_DOWN
#define KEY_SPACE AKEYCODE_SPACE
#define KEY_0 AKEYCODE_0
#define KEY_1 AKEYCODE_1
#define KEY_2 AKEYCODE_2
#define KEY_CIRCUMFLEX AKEYCODE_GRAVE
#define KEY_A AKEYCODE_A
#define KEY_C AKEYCODE_C
#define KEY_D AKEYCODE_D
#define KEY_I AKEYCODE_I
#define KEY_J AKEYCODE_J
#define KEY_K AKEYCODE_K
#define KEY_L AKEYCODE_L
#define KEY_R AKEYCODE_R
#define KEY_S AKEYCODE_S
#define KEY_T AKEYCODE_T
#define KEY_W AKEYCODE_W
#endif
#if defined(__EMSCRIPTEN__)
//the browser's KeyboardEvent.keyCode, which has the values of Windows' virtual keys
#define KEY_BACKSPACE 8
#define KEY_RETURN 13
#define KEY_CTRL 17
#define KEY_ALT 18
#define KEY_ESCAPE 27
#define KEY_SPACE 32
#define KEY_PAGEUP 33
#define KEY_PAGEDOWN 34
#define KEY_LEFT 37
#define KEY_UP 38
#define KEY_RIGHT 39
#define KEY_DOWN 40
#define KEY_DELETE 46
#define KEY_0 '0'
#define KEY_1 '1'
#define KEY_2 '2'
#define KEY_A 'A'
#define KEY_C 'C'
#define KEY_D 'D'
#define KEY_I 'I'
#define KEY_J 'J'
#define KEY_K 'K'
#define KEY_L 'L'
#define KEY_R 'R'
#define KEY_S 'S'
#define KEY_T 'T'
#define KEY_W 'W'
#define KEY_F1 112
#define KEY_F2 113
//the key left of 1 on a German keyboard, like VK_OEM_5
#define KEY_CIRCUMFLEX 220
#endif

#include "../system/engine.h"

namespace Input{

class CGE_API Keyboard{
public:
  static void init();
  inline static Keyboard* instance() {return key_;}
  inline static void release() {SAFE_DELETE(key_);}
  void keyDown(int key);
  void keyUp(int key);
  //! a typed character (a Unicode code point); control characters are ignored
  void character(unsigned int ch);
  //! typed text in UTF-8: character() for each of its code points (an input
  //! method may commit several at once); invalid sequences are skipped
  void text(const char* utf8, size_t length);
  //! is the key held down?
  bool isPressed(int key) const;
  void enableBuitInConsole(bool doit) { mBuiltInConsole = doit; }
protected:
  static Keyboard* key_;
  Keyboard();
  //! the key state
  std::list<int> pressedKeys_;
  //! enable built-in console
  bool mBuiltInConsole;
};

}

#endif