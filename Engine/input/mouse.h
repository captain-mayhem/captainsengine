#ifndef MOUSE_H
#define MOUSE_H

#include "../system/engine.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef MB_RIGHT
#define MB_LEFT MK_LBUTTON
#define MB_MIDDLE MK_MBUTTON
#define MB_RIGHT MK_RBUTTON
#define MB_CTRL MK_CONTROL
#define MB_SHIFT MK_SHIFT
#endif

namespace Input{

  class Mouse{
  public:
    static void init();
    inline static Mouse* instance() {return mouse_;}
    inline static void release() {SAFE_DELETE(mouse_);}
    void buttonDown(int x, int y, int button);
    void buttonUp(int x, int y, int button);
    void move(int x, int y, int buttons);
  protected:
    static Mouse* mouse_;
    Mouse();
  };

}

#endif

