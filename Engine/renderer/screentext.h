#ifndef CGE_SCREENTEXT_H
#define CGE_SCREENTEXT_H

#include <cge_export.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "material.h"

namespace CGE{

class Font;

namespace Gfx{
class SpriteBatch;
}

//! Texts printed onto the screen for a while, the engine's status and debug lines
/*! Engine::getScreenText gives the engine's ones, which it draws over each
    frame. print doesn't draw at once but queues the text; render draws the
    queue and counts down the texts' durations.
    The texts are drawn in a monospace font 16 units high, with about 10
    units per character, in the virtual screen with y up (as the engine's old
    bitmap font did): fonts/LiberationMono-Regular.ttf of the assets, or else
    the system's monospace font, or the engine's font. Set 1 is bold and
    slanted. The fonts are loaded on the first render. */
class CGE_API ScreenText{
public:
  ScreenText();
  ~ScreenText();
  ScreenText(const ScreenText&) = delete;
  ScreenText& operator=(const ScreenText&) = delete;

  //! queues a text (UTF-8) with its bottom left at (x, y), y up from the bottom of the virtual screen
  /*! \param set 0 for normal, 1 for bold slanted text
      \param duration how many seconds it stays; 0 for one frame */
  void print(int x, int y, const char* str, short set, float duration = 0.00f);
  //! the colour of the texts printed next
  void setColor(float r, float g, float b);
  void setColor(const Color& c);
  //! an id for the next text printed, to delete it with deleteText
  void setId(short id) {id_ = id;}
  //! draws the texts into the batch and counts down their durations by interval seconds
  /*! The batch's projection is the virtual screen's with y down
      (VirtualScreen::ortho), and height is the virtual screen's. */
  void render(Gfx::SpriteBatch& batch, float height, double interval);
  //! deletes the fonts, e.g. before the render device goes; render loads them again
  void releaseFonts();
  //! shows or hides the texts
  inline void toggle() {show_ = !show_;}
  //! deletes all texts
  void clear();
  //! are there no texts
  inline bool isEmpty() const {return texts_.empty();}
  //! replaces the texts with a status message in white for 10 seconds
  void operator<<(const std::string& text);
  void operator<<(const std::ostringstream& text);
  //! deletes the texts with an id
  void deleteText(short id);

private:
  struct Text{
    int x;
    int y;
    std::string text;
    short set;
    Color color;
    double duration;
    short id;
  };
  bool show_;
  Color color_;
  short id_;
  std::vector<Text> texts_;
  //! set 0 and 1
  std::unique_ptr<Font> fonts_[2];
};

}

#endif
