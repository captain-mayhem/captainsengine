#ifndef MOUSE_H
#define MOUSE_H

#include "../system/engine.h"
#include "../math/vector.h"

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
#ifdef UNIX
#include <X11/Xlib.h>
#define MB_LEFT Button1
#define MB_MIDDLE Button2
#define MB_RIGHT Button3
#define MB_CTRL ControlMask
#define MB_SHIFT ShiftMask
#endif

namespace Input{

  class Mouse{
  public:
    ~Mouse();
    static void init(bool hasGraphics);
    inline static Mouse* instance() {return mouse_;}
    inline static void release() {SAFE_DELETE(mouse_);}
    //! set button down callback
    inline void setButtonDownCB(void (*proc)(int x, int y, int button)) {buttonDownCB_ = proc;}
    //! set button up callback
    inline void setButtonUpCB(void (*proc)(int x, int y, int button)) {buttonUpCB_ = proc;}
    //! set mouse move callback
    inline void setMouseMoveCB(void (*proc)(int x, int y, int button)) {moveCB_ = proc;}
    void buttonDown(int x, int y, int button);
    void buttonUp(int x, int y, int button);
    void move(int x, int y, int buttons);
    //! get the position of the last click
    inline ::Math::Vector2D getClickPos() {return clickPos_;}
    //! reset click position
    inline void resetMousePos() {clickPos_ = ::Math::Vector2D(-1,-1);}
    //! was the last click a gui click
    inline bool isGuiClick() {return gui_click_;}
    //! show the cursor
    void showCursor(bool visible);
  protected:
    static Mouse* mouse_;
    Mouse();
    //! button down callback
    void (*buttonDownCB_)(int x, int y, int button);
    //! button up callback
    void (*buttonUpCB_)(int x, int y, int button);
    //! mouse move callback
    void (*moveCB_)(int x, int y, int button);
    //! the position of the last click
    ::Math::Vector2D clickPos_;
    //! was it a click on a gui element?
    bool gui_click_;
    //! is the mouse pointer present
    bool mousePointer_;
#ifdef UNIX
    //! app has graphics context
    bool graphics_;
    //! the invisible cursor
    Cursor invCursor_;
#endif
  };

}

#endif

