#ifndef CGE_VIRTUALSCREEN_H
#define CGE_VIRTUALSCREEN_H

#include <cge_export.h>

#include "../math/vector.h"

namespace CGE{

class Renderer;

//! The virtual resolution: the coordinate space of the GUI pass and the input events
/*! The virtual area of width x height units is scaled uniformly to the largest
    size that fits into the window, and centred. On a window with a different
    aspect ratio more than the virtual area is visible: to the left and right on
    a wider window, above and below on a narrower one. Coordinates there are
    negative or larger than the virtual size, so backgrounds can extend into them
    while everything important stays inside the virtual area.

    Virtual coordinates have their origin at the top left with y down, like
    window pixels, unless a function says otherwise. The size comes from
    virtualWidth/virtualHeight in engine.ini (default 1024x768); applications
    can change it with setSize. */
class CGE_API VirtualScreen{
public:
  VirtualScreen();
  //! sets the size of the virtual area
  void setSize(int width, int height);
  inline int getWidth() const {return width_;}
  inline int getHeight() const {return height_;}
  //! the window (the real screen) is width x height pixels
  void setWindowSize(int width, int height);
  inline int getWindowWidth() const {return windowWidth_;}
  inline int getWindowHeight() const {return windowHeight_;}
  //! window pixels per virtual unit
  inline float getScale() const {return scale_;}
  //! the top left corner of the visible area in virtual coordinates (<= 0)
  Vec2f getVisibleMin() const;
  //! the bottom right corner of the visible area in virtual coordinates (>= the virtual size)
  Vec2f getVisibleMax() const;
  //! maps window pixels to virtual coordinates
  Vec2i toVirtual(int x, int y) const;
  //! maps virtual coordinates to window pixels
  Vec2i toWindow(int x, int y) const;
  //! sets an orthographic projection that puts the virtual area at its place in the window
  /*! With yDown, the origin is at the top left with y down. Otherwise it's at
      the bottom left with y up, as in the engine's GUI pass. Covers the whole
      viewport, which is expected to be the whole window. */
  void ortho(Renderer* rend, bool yDown = false) const;
private:
  void update();
  int width_;
  int height_;
  int windowWidth_;
  int windowHeight_;
  float scale_;
  //! the window pixel of the virtual origin
  Vec2f offset_;
};

}

#endif
