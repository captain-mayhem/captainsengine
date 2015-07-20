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

#include <string>
#include <sstream>
#include <queue>
#include "vertexbuffer.h"
#include "material.h"
#include "../math/vector.h"

using std::string;
using std::ostringstream;
using std::queue;

namespace CGE{

class Texture;

//! encapsulates font data with all necessary variables to be able to be rendered
struct font_data{
  //! position of the text
  ::CGE::Vector2D pos;
  //! the text
  string* text;
  //! which font set should be used
  short set;
  //! the font color
  Color rgb;
  //! how long to display
  double duration;
  //! the text id
  short id;
};

//! The font class can render text to the screen, using Bitmap-Fonts and OpenGL
class Font
{
public:
  //! Constructor
  Font();
  //! Copy Constructor
  Font(const Font& f);
  //! Destructor
  ~Font();
  //! build the display lists that are used displaying the text
  void buildFont(VertexBuffer* other);
  //! deletes the display lists
  void killFont();
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
  //! renders the text
  void render();
  //! toggles the visibility of the text
  inline void toggle() {show_ = !show_;}
  //! erases all stored print data
  inline void clear() { while (!q_.empty()) { delete q_.front().text; q_.pop(); } }
  //! easy to use output operator for status messages
  void operator<<(const string& text);
  //! output for streams
  void operator<<(const ostringstream& text);
  //! get the font vertex buffer
  inline VertexBuffer* getVB() {return buffer_;}
  //! set the font vertex buffer
  inline void resetVB() {buffer_ = NULL;}
  //! delete the texts with a certain id
  void deleteText(short id);
private:
  //! text visible?
  bool show_;
  //! the texture
  Texture* tex_;
  //! the vertex buffers
  VertexBuffer* buffer_;
  //! the index buffer
  IndexBuffer* inds_;
  //! font color
  Color rgb_;
  //! all text strings that are to be rendered
  queue<font_data> q_;
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
