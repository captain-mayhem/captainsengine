//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: font.hh                                                    |
//  ==================================================================

#ifndef FONT_HH
#define FONT_HH

#include <cge_export.h>

#include <string>
#include <sstream>
#include <queue>
#include <memory>
#include <vector>
#include "material.h"
#include "../math/vector.h"


namespace CGE{

namespace Gfx{
class Buffer;
class DrawContext;
class RenderDevice;
class Sampler;
class Texture;
}

//! encapsulates font data with all necessary variables to be able to be rendered
struct CGE_API font_data{
  //! position of the text
  ::CGE::Vector2D pos;
  //! the text
  std::string* text;
  //! which font set should be used
  short set;
  //! the font color
  Color rgb;
  //! how long to display
  double duration;
  //! the text id
  short id;
};

//! The font class can render text to the screen, using the bitmap font textures/font/font.bmp
/*! The texts are drawn through a render device, all texts of a font in one
    draw. */
class CGE_API Font
{
public:
  //! loads the font texture onto the device
  explicit Font(Gfx::RenderDevice& device);
  Font(const Font& f) = delete;
  Font& operator=(const Font& f) = delete;
  //! Destructor
  ~Font();
  //! prints text on the screen
  /*! this does not print directly, but stores all information in the queue to render
   * the text the next time Render() is called
   * \param x the x position of the text
   * \param y the y position of the text
   * \param str the text to be rendered
   * \param set the font set to use
   * \param duration how long the text should be displayed
   */
  void print(int x, int y, const char* str, short set, float duration=0.00f);
  //! sets the color of the text
  void setColor(float r, float g, float b);
  //! sets the color of the text
  void setColor(CGE::Color& c);
  //! sets a id with which the text can be identified
  void setId(short id) {id_ = id;}
  //! draws the texts inside the current pass and counts down their durations by interval seconds
  /*! The context's projection is expected to put the virtual screen's
      origin at the bottom left with y up (VirtualScreen::ortho). Uses the
      context's blend mode and leaves its texture unset. */
  void render(Gfx::DrawContext& ctx, double interval);
  //! toggles the visibility of the text
  inline void toggle() {show_ = !show_;}
  //! erases all stored print data
  inline void clear() { while (!q_.empty()) { delete q_.front().text; q_.pop(); } }
  //! are there no texts to render?
  inline bool isEmpty() const { return q_.empty(); }
  //! easy to use output operator for status messages
  void operator<<(const std::string& text);
  //! output for streams
  void operator<<(const std::ostringstream& text);
  //! delete the texts with a certain id
  void deleteText(short id);
private:
  //! text visible?
  bool show_;
  //! the font texture: 16x16 glyphs
  std::unique_ptr<Gfx::Texture> tex_;
  std::unique_ptr<Gfx::Sampler> sampler_;
  //! the glyphs' vertices of a frame, rewritten every frame
  std::unique_ptr<Gfx::Buffer> buffer_;
  //! font color
  Color rgb_;
  //! all text strings that are to be rendered
  std::queue<font_data> q_;
  //! a id to identify the text
  short id_;
};

inline bool operator<(font_data a, font_data b){
  return a.pos < b.pos;
}
inline bool operator==(font_data a, font_data b){
  return a.pos == b.pos;
}
inline bool operator!=(font_data a, font_data b){
  return a.pos != b.pos;
}
inline bool operator>(font_data a, font_data b){
  return !(a.pos < b.pos) && a.pos != b.pos;
}

}

#endif
