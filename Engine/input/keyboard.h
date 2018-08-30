#ifndef KEYBOARD_H
#define KEYBOARD_H

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
#ifdef QNX
#define KEY_ESCAPE 0
#define KEY_RETURN 1
#define KEY_BACKSPACE 2
#define KEY_UP 3
#define KEY_DOWN 4
#define KEY_LEFT 5
#define KEY_RIGHT 6
#define KEY_F1 7
#define KEY_F2 8
#define KEY_C 9
#define KEY_R 10
#define KEY_T 11

#define KEY_CTRL 19
#define KEY_ALT 20
#define KEY_DELETE 21
#define KEY_PAGEUP 22
#define KEY_PAGEDOWN 23

#define KEY_SPACE 32

#define KEY_0 48

#define KEY_CIRCUMFLEX 94

#define KEY_A 97

#endif

#include "../system/engine.h"

namespace Input{

class Keyboard{
public:
  static void init();
  inline static Keyboard* instance() {return key_;}
  inline static void release() {SAFE_DELETE(key_);}
  //! set keydown callback
  inline void setKeyDownCB(void (*proc)(int key)) {keyDownCB_ = proc;}
  //! set keyup callback
  inline void setKeyUpCB(void (*proc)(int key)) {keyUpCB_ = proc;}
  //! set keydown callback
  inline void setAsciiCB(void (*proc)(unsigned char key)) {asciiCB_ = proc;}
  //! set keypressed callback
  inline void setKeyPressedCB(void (*proc)(int key, float diffTime)) {keyPressedCB_ = proc;}
  void keyDown(int key);
  void keyUp(int key);
  void ascii(unsigned char key);
  void processPressedKeys(float diffTime);
  void enableBuitInConsole(bool doit) { mBuiltInConsole = doit; }
protected:
  static Keyboard* key_;
  Keyboard();
  //! keydown callback
  void (*keyDownCB_)(int key);
  //! keyup callback
  void (*keyUpCB_)(int key);
  //! ascii callback
  void (*asciiCB_)(unsigned char key);
  //! key pressed callback
  void (*keyPressedCB_)(int key, float diffTime);
  //! the key state
  std::list<int> pressedKeys_;
  //! enable built-in console
  bool mBuiltInConsole;
};

}

#endif