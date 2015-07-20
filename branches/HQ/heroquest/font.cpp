//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: font.cpp                                                   |
//  ==================================================================

#include "texture.hh"
#include "renderer.hh"
#include "common.hh"
#include "font.hh"

//Constructor
Font::Font(){
  show_ = true;
  rgb_ = Vector3D(1,1,1);
}

Font::Font(const Font& f){
  show_ = f.show_;
  base_ = f.base_;
  rgb_ = f.rgb_;
  q_ = queue<font_data>(f.q_);
}

//Destructor
Font::~Font(){
  //Do not call KillFont() in here, because
  //the display lists are shared by many font objects
}

//Build font display list
GLvoid Font::buildFont()
{
  //character coords
  float	cx;
  float	cy;

  //generate 256 lists
  base_ = glGenLists(256);
  glBindTexture(GL_TEXTURE_2D, tex.fontTex[0]);
  for (int i=0; i<256; i++)
  {
    //position of current character in font texture
    cx=float(i%16)/16.0f;
    cy=float(i/16)/16.0f;

    glNewList(base_+i,GL_COMPILE);
    //A Quad for each character
    glBegin(GL_QUADS);
      glTexCoord2f(cx,1-cy-0.0625f);
      glVertex2i(0,0);
      glTexCoord2f(cx+0.0625f,1-cy-0.0625f);
      glVertex2i(16,0);
      glTexCoord2f(cx+0.0625f,1-cy);
      glVertex2i(16,16);
      glTexCoord2f(cx,1-cy);
      glVertex2i(0,16);
    glEnd();
    // The next character will come right
    glTranslated(10,0,0);
    glEndList();
  }
}

// Delete Font
GLvoid Font::killFont()
{
  glDeleteLists(base_,256);
}

// Stores all neccessary information to render the text
GLvoid Font::glPrint(GLint x, GLint y, const char *str, short set, float duration)
{
  font_data f;
  f.pos.x = x;
  f.pos.y = y;
  f.set = set;
  f.text = string(str);
  f.rgb = rgb_;
  f.duration = duration;
  q_.push(f);
}

// Set font color
GLvoid Font::setColor(float r, float g, float b){
  rgb_.x = r;
  rgb_.y = g;
  rgb_.z = b;
}

// Render all stored text strings
GLvoid Font::render(){
  queue<font_data> temp;
  while(!q_.empty()){
    //take out the next font_data
    font_data f = q_.front();
    q_.pop();
    f.duration -= gl->getFrameInterval();
    
    //not displayed long enough, so display again in the next cycle
    if (f.duration > 0){
      temp.push(f);
    }
    if (show_){
      glColor3f(f.rgb.x, f.rgb.y, f.rgb.z);
      if (f.set>1){
        f.set=1;
      }
      glBindTexture(GL_TEXTURE_2D, tex.fontTex[0]);
      glLoadIdentity();
      glTranslated(f.pos.x,f.pos.y,0);
      
      //select and call list. The lists are selected such that 
      //the ascii code of a character corresponds to the right list
      glListBase(base_-32+(128*f.set));
      glCallLists(strlen(f.text.c_str()),GL_BYTE, f.text.c_str());

    }
  }
  q_ = temp;
}

//simplified glPrint
void Font::operator<<(const string& text){
  clear();
  glPrint(20, 750, text.c_str(), 0, 10);
}

//for stringstreams
void Font::operator<<(const ostringstream& text){
  (*this) << text.str();
}
