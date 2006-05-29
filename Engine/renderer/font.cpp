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

#include "../system/engine.h"
#include "texture.h"
#include "renderer.h"
#include "font.h"

using namespace Graphics;

//Constructor
Font::Font(){
  show_ = true;
  rgb_ = Vector3D(1,1,1);
  tex_ = System::Engine::instance()->getRenderer()->createTexture("textures/font/font.bmp");
  buffers_ = new VertexBuffer*[256];
  for (int i = 0; i < 256; i++){
    buffers_[i] = System::Engine::instance()->getRenderer()->createVertexBuffer();
  }
}

Font::Font(const Font& f){
  show_ = f.show_;
  rgb_ = f.rgb_;
  q_ = queue<font_data>(f.q_);
}

//Destructor
Font::~Font(){
  return;
  //Do not call KillFont() in here, because
  //the display lists are shared by many font objects
  SAFE_DELETE(tex_);
  for (int i = 0; i < 256; i++){
    SAFE_DELETE(buffers_[i]);
  }
  SAFE_DELETE_ARRAY(buffers_);
}

//Build font display list
void Font::buildFont()
{
  //character coords
  float	cx;
  float	cy;

  tex_->activate();
  for (int i=0; i<256; i++)
  {
    //position of current character in font texture
    cx=float(i%16)/16.0f;
    cy=float(i/16)/16.0f;

    //A Quad for each character
    buffers_[i]->create(VB_POSITION | VB_TEXCOORD, 4, 6);
    buffers_[i]->lockVertexPointer();
    buffers_[i]->setPosition(0, Vertex(0,0,0));
    buffers_[i]->setPosition(1, Vertex(16,0,0));
    buffers_[i]->setPosition(2, Vertex(16,16,0));
    buffers_[i]->setPosition(3, Vertex(0,16,0));
    buffers_[i]->setTexCoord(0, Vec2f(cx,1-cy-0.0625f));
    buffers_[i]->setTexCoord(1, Vec2f(cx+0.0625f,1-cy-0.0625f));
    buffers_[i]->setTexCoord(2, Vec2f(cx+0.0625f,1-cy));
    buffers_[i]->setTexCoord(3, Vec2f(cx,1-cy));
    //buffers_[i]->setColor(0, Color(255,0,0,255));
    //buffers_[i]->setColor(1, Color(255,0,0,255));
    //buffers_[i]->setColor(2, Color(255,0,0,255));
    //buffers_[i]->setColor(3, Color(255,0,0,255));
    buffers_[i]->unlockVertexPointer();
    short* in = buffers_[i]->lockIndexPointer();
    in[0] = 0; in[1] = 1; in[2] = 3;
    in[3] = 3; in[4] = 1; in[5] = 2;
    buffers_[i]->unlockIndexPointer();
  }
}

// Delete Font
void Font::killFont()
{
  //glDeleteLists(base_,256);
}

// Stores all neccessary information to render the text
void Font::glPrint(int x, int y, const char *str, short set, float duration)
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
void Font::setColor(float r, float g, float b){
  rgb_.x = r;
  rgb_.y = g;
  rgb_.z = b;
}

// Render all stored text strings
void Font::render(){
  Renderer* rend = System::Engine::instance()->getRenderer();
  queue<font_data> temp;
  while(!q_.empty()){
    //take out the next font_data
    font_data f = q_.front();
    q_.pop();
    f.duration -= 1;//gl->getFrameInterval();
    
    //not displayed long enough, so display again in the next cycle
    if (f.duration > 0){
      temp.push(f);
    }
    if (show_){
      rend->setColor(f.rgb.x*255, f.rgb.y*255, f.rgb.z*255, 255);
      if (f.set>1){
        f.set=1;
      }
      tex_->activate();
      //glBindTexture(GL_TEXTURE_2D, tex.fontTex[0]);
      //glLoadIdentity();
      rend->translate(f.pos.x,f.pos.y,0);
      
      //select and call list. The lists are selected such that 
      //the ascii code of a character corresponds to the right list
      //glListBase(base_-32+(128*f.set));
      //glCallLists(strlen(f.text.c_str()),GL_BYTE, f.text.c_str());
      //glColor3f(1,1,1);
      for (int i = 0; i < f.text.size(); i++){
        //increase translation
        rend->translate(10,0,0);
        buffers_[f.text[i]-32+(128*f.set)]->activate();
        buffers_[f.text[i]-32+(128*f.set)]->draw();
      }

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
