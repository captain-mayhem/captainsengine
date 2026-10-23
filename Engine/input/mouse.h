#ifndef MOUSE_H
#define MOUSE_H

#include <cge_export.h>

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
#ifdef LINUX
#include <X11/Xlib.h>
#define MB_LEFT Button1
#define MB_MIDDLE Button2
#define MB_RIGHT Button3
#define MB_CTRL ControlMask
#define MB_SHIFT ShiftMask
#endif
#if defined(__EMSCRIPTEN__)
//button numbers and state bits at once, like Windows' MK_ values
#define MB_LEFT 1
#define MB_RIGHT 2
#define MB_SHIFT 4
#define MB_CTRL 8
#define MB_MIDDLE 16
#endif
#if defined(ANDROID)
//button numbers and state bits at once, as on the web
#define MB_LEFT 1
#define MB_RIGHT 2
#define MB_SHIFT 4
#define MB_CTRL 8
#define MB_MIDDLE 16
#endif

namespace Input{

  class CGE_API Mouse{
  public:
    ~Mouse();
    static void init(bool hasGraphics);
    inline static Mouse* instance() {return mouse_;}
    inline static void release() {SAFE_DELETE(mouse_);}
    void buttonDown(int x, int y, int button);
    void buttonUp(int x, int y, int button);
    void doubleClick(int x, int y, int button);
    void move(int x, int y, int buttons);
    void wheel(int x, int y, int delta);
    //! get the position of the last click in virtual coordinates
    inline CGE::Vector2D getClickPos() {return clickPos_;}
    //! reset click position
    inline void resetMousePos() {clickPos_ = CGE::Vector2D(-1,-1);}
    //! was the last click on an ImGui window? (the application gets it anyway)
    inline bool isGuiClick() {return gui_click_;}
    //! show the cursor
    void showCursor(bool visible);
    //! set the mouse position in virtual coordinates
    void setMousePos(int x, int y);
    //! is the mouse visible
    inline bool isMouseActive() {return mousePointer_;}
    //! is a button clicked
    bool isPressed(int button);
    //! get the position of the mouse in virtual coordinates
    inline CGE::Vector2D getMousePos() {return mousePos_;}
  protected:
    static Mouse* mouse_;
    Mouse();
    //! the position of the last click
    CGE::Vector2D clickPos_;
    //! the position of the mouse
    CGE::Vector2D mousePos_;
    //! was it a click on an ImGui window?
    bool gui_click_;
    //! is the mouse pointer present
    bool mousePointer_;
    //! is a mouse button pressed
    bool pressed_[3];
#ifdef LINUX
    //! app has graphics context
    bool graphics_;
    //! the invisible cursor
    Cursor invCursor_;
#endif
  };

}

#endif
