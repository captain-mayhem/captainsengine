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
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "vector.hh"

using std::string;
using std::ostringstream;
using std::queue;

//! encapsulates font data with all neccessary variables to be able to be rendered
struct font_data{
  //! position of the text
  Vector2D pos;
  //! the text
  string text;
  //! which font set shound be used
  short set;
  //! the font color
  Vector3D rgb;
  //! how long to display
  float duration;
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
  GLvoid buildFont();
  //! deletes the display lists
  GLvoid killFont();
  //! prints text on the screen
  /*! this does not print directly, but stores all information in the queue to render
   * the text the next time Render() is called
   * \param x the x position of the text
   * \param y the y position of the text
   * \param str the text to be rendered
   * \param set the font set to use
   * \param duration how long the text should be displayed
   */
  GLvoid glPrint(GLint x, GLint y, const char *str, short set, float duration=0.001f);
  //! sets the color of the text
  GLvoid setColor(float r, float g, float b);
  //! sets the base of the display lists
  inline GLvoid setBase(GLuint base) {base_ = base;}
  //! gets the base of the display lists
  inline GLuint getBase() {return base_;}
  //! renders the text
  GLvoid render();
  //! toggles the visibility of the text
  inline void toggle() {show_ = !show_;}
  //! erases all stored print data
  inline void clear() {while(!q_.empty()) q_.pop();}
  //! easy to use output operator for status messages
  void operator<<(const string& text);
  //! output for streams
  void operator<<(const ostringstream& text);
private:
  //! text visible?
  bool show_;
  //! base of display lists
  GLuint base_;
  //! font color
  Vector3D rgb_;
  //! all text strings that are to be rendered
  queue<font_data> q_;
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

extern Font text;
extern Font output;
extern Font line;

#endif
