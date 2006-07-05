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
  rgb_ = Color(255,255,255,255);
  tex_ = System::Engine::instance()->getRenderer()->createTexture("textures/font/font.bmp");
  buffer_ = System::Engine::instance()->getRenderer()->createVertexBuffer();
}

Font::Font(const Font& f){
  show_ = f.show_;
  rgb_ = f.rgb_;
  q_ = queue<font_data>(f.q_);
}

//Destructor
Font::~Font(){
  SAFE_DELETE(tex_);
  SAFE_DELETE(buffer_);
}

//Build font display list
void Font::buildFont(){
  //character coords
  float	cx;
  float	cy;

  tex_->activate();
  buffer_->create(VB_POSITION | VB_TEXCOORD, 4*256, 4);
  buffer_->lockVertexPointer();
  
  for (int i=0; i<256; i++){
    //position of current character in font texture
    cx=float(i%16)/16.0f;
    cy=float(i/16)/16.0f;

    //A Quad for each character
    buffer_->setPosition(4*i+0, Vertex(0,0,0));
    buffer_->setPosition(4*i+1, Vertex(16,0,0));
    buffer_->setPosition(4*i+3, Vertex(16,16,0));
    buffer_->setPosition(4*i+2, Vertex(0,16,0));
    buffer_->setTexCoord(4*i+0, Vec2f(cx,1-cy-0.0625f), true);
    buffer_->setTexCoord(4*i+1, Vec2f(cx+0.0625f,1-cy-0.0625f), true);
    buffer_->setTexCoord(4*i+3, Vec2f(cx+0.0625f,1-cy), true);
    buffer_->setTexCoord(4*i+2, Vec2f(cx,1-cy), true);
  }

  buffer_->unlockVertexPointer();

  short* in = buffer_->lockIndexPointer();
  in[0] = 0; in[1] = 1; in[3] = 3;
  in[2] = 2; //in[4] = 1; in[5] = 2;
  buffer_->unlockIndexPointer();
}

// Delete Font
void Font::killFont(){
  //glDeleteLists(base_,256);
}

// Stores all necessary information to render the text
void Font::glPrint(int x, int y, const char* str, short set, float duration){
  font_data f;
  f.pos.x = x;
  f.pos.y = y;
  f.set = set;
  f.text = new string(str);
  f.rgb = rgb_;
  f.duration = duration;
  q_.push(f);
}

// Set font color
void Font::setColor(float r, float g, float b){
  rgb_.r = (unsigned char)(r*255);
  rgb_.g = (unsigned char)(g*255);
  rgb_.b = (unsigned char)(b*255);
}

// Render all stored text strings
void Font::render(){
  Renderer* rend = System::Engine::instance()->getRenderer();
  queue<font_data> temp;

  tex_->activate();
  buffer_->activate();

  while(!q_.empty()){
    
    //take out the next font_data
    font_data f = q_.front();
    q_.pop();
    f.duration -= ::System::Engine::instance()->getFrameInterval();//gl->getFrameInterval();
    
    //not displayed long enough, so display again in the next cycle
    if (f.duration > 0){
      temp.push(f);
    }
    if (show_){
      rend->pushMatrix();

      rend->setColor(&f.rgb);
      if (f.set>1){
        f.set=1;
      }
      //glBindTexture(GL_TEXTURE_2D, tex.fontTex[0]);
      //glLoadIdentity();
      //rend->resetModelView();
      //rend->translate(-512, -384, 0);
      rend->translate(f.pos.x,f.pos.y,0);
      
      //The bases are selected such that 
      //the ascii code of a character corresponds to the right list
      for (unsigned i = 0; i < f.text->size(); i++){
        unsigned char tmp = (*f.text)[i]-32;
        buffer_->setVertexOffset(f.text->at(i)-32+(128*f.set));
        buffer_->draw(VB_Tristrip);
        //increase translation
        rend->translate(10,0,0);
      }
      rend->popMatrix();
    }
    if (f.duration <= 0)
      delete f.text;
  }
  q_ = temp;
}

//simplified glPrint
void Font::operator<<(const string& text){
  clear();
  setColor(1,1,1);
  glPrint(20, 750, text.c_str(), 0, 10);
}

//for stringstreams
void Font::operator<<(const ostringstream& text){
  (*this) << text.str();
}
