#ifndef CGE_TOUCH_H
#define CGE_TOUCH_H

#include <cge_export.h>

#include <vector>

#include "../system/engine.h"
#include "../math/vector.h"

namespace Input{

//! The fingers on a touch screen (Android and the web)
/*! The window code reports every finger with its id, which the platform
    keeps for the finger while it touches (a later finger may get the id of
    one that was lifted); positions are window pixels. The application gets
    them in virtual coordinates (IApplication::onTouchDown/Move/Up/Cancel).

    The first finger on an empty screen is the primary finger, until it is
    lifted; fingers put down meanwhile aren't, and none is until all are up.
    ImGui always sees the primary finger as its mouse. With mouse emulation
    (the default; touchAsMouse in engine.ini, or setMouseEmulation), the
    primary finger is also the left mouse button for the application, so
    projects that only need clicks and drags need no touch code: a tap moves
    the mouse there and clicks, two taps soon after another are a double
    click. Applications that handle the touches themselves turn it off. */
class CGE_API Touch{
public:
  //! a finger on the screen
  struct Finger{
    int id;
    //! its position in virtual coordinates
    CGE::Vec2i pos;
    //! its position in window pixels
    CGE::Vec2i windowPos;
  };

  static void init();
  inline static Touch* instance() {return touch_;}
  inline static void release() {SAFE_DELETE(touch_);}

  //! a finger touched the screen at window pixel (x, y)
  void down(int id, int x, int y);
  //! a finger moved; nothing happens if its position didn't change
  void move(int id, int x, int y);
  //! a finger was lifted
  void up(int id, int x, int y);
  //! the system took a finger over (e.g. for a gesture): it's gone without a tap
  void cancel(int id);
  //! all fingers are gone without a tap (e.g. the window lost them)
  void cancelAll();

  //! the fingers on the screen, in the order they were put down
  inline const std::vector<Finger>& getFingers() const {return fingers_;}
  //! is the finger on the screen?
  bool isDown(int id) const;
  //! the primary finger's id, -1 for none
  inline int getPrimary() const {return primary_;}

  //! the primary finger acts as the left mouse button (from its next touch)
  inline void setMouseEmulation(bool emulate) {mouseEmulation_ = emulate;}
  inline bool isMouseEmulation() const {return mouseEmulation_;}
protected:
  static Touch* touch_;
  Touch();
  //! the finger's index in fingers_, -1 if it isn't there
  int find(int id) const;
  //! the finger at index leaves the screen; cancelled: without a tap
  void remove(int index, bool cancelled);
  //! the fingers on the screen
  std::vector<Finger> fingers_;
  //! the primary finger, -1 for none
  int primary_;
  //! the primary finger acts as the left mouse button
  bool mouseEmulation_;
  //! the emulated left button is down (it's released even if the
  //! emulation was turned off meanwhile)
  bool mousePressed_;
};

}

#endif
