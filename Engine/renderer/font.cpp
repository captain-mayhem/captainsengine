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
#include "../system/file.h"
#include "texture.h"
#include "renderer.h"
#include "font.h"

using namespace CGE;

//Constructor
Font::Font(){
  show_ = true;
  rgb_ = Color(1.0,1.0,1.0,1.0);
  tex_ = CGE::Engine::instance()->getRenderer()->createTexture(CGE::Filesystem::getCwd()+"/textures/font/font.bmp");
  buffer_ = CGE::Engine::instance()->getRenderer()->createVertexBuffer();
  inds_ = CGE::Engine::instance()->getRenderer()->createIndexBuffer(IndexBuffer::IB_USHORT, 4);
  id_ = -1;
}

Font::Font(const Font& f){
  show_ = f.show_;
  rgb_ = f.rgb_;
  q_ = queue<font_data>(f.q_);
  id_ = f.id_;
  tex_ = NULL;
  buffer_ = NULL;
  inds_ = NULL;
}

//Destructor
Font::~Font(){
  while (!q_.empty()){
    font_data& data = q_.front();
    delete data.text;
    q_.pop();
  }
  SAFE_DELETE(tex_);
  SAFE_DELETE(buffer_);
  SAFE_DELETE(inds_);
}

//Build font display list
void Font::buildFont(VertexBuffer* other){
  //character coords
  float	cx;
  float	cy;

  if (tex_ == NULL)
    return;
  tex_->activate();
  if (other == NULL){
    buffer_->create(VB_POSITION | VB_TEXCOORD, 4 * 256);
    buffer_->lockVertexPointer();

    for (int i = 0; i < 256; i++){
      //position of current character in font texture
      cx = float(i % 16) / 16.0f;
      cy = float(i / 16) / 16.0f;

      //A Quad for each character
      buffer_->setPosition(4 * i + 0, Vertex(0, 0, 0));
      buffer_->setPosition(4 * i + 1, Vertex(16, 0, 0));
      buffer_->setPosition(4 * i + 3, Vertex(16, 16, 0));
      buffer_->setPosition(4 * i + 2, Vertex(0, 16, 0));
      buffer_->setTexCoord(4 * i + 0, Vec2f(cx, 1 - cy - 0.0625f), true);
      buffer_->setTexCoord(4 * i + 1, Vec2f(cx + 0.0625f, 1 - cy - 0.0625f), true);
      buffer_->setTexCoord(4 * i + 3, Vec2f(cx + 0.0625f, 1 - cy), true);
      buffer_->setTexCoord(4 * i + 2, Vec2f(cx, 1 - cy), true);
    }

    buffer_->unlockVertexPointer();
  }
  else{
    delete buffer_;
    buffer_ = other;
  }

  short* in = (short*)inds_->lockIndexPointer();
  in[0] = 0; in[1] = 1; in[3] = 3;
  in[2] = 2; //in[4] = 1; in[5] = 2;
  inds_->unlockIndexPointer();
}

// Delete Font
void Font::killFont(){
  //glDeleteLists(base_,256);
}

// Stores all necessary information to render the text
void Font::print(int x, int y, const char* str, short set, float duration){
  font_data f;
  f.pos.x = x;
  f.pos.y = y;
  f.set = set;
  f.text = new string(str);
  f.rgb = rgb_;
  f.duration = duration;
  f.id = id_;
  q_.push(f);
  id_ = -1;
}

// Set font color
void Font::setColor(float r, float g, float b){
  rgb_.r = r;
  rgb_.g = g;
  rgb_.b = b;
}

// Set font color
void Font::setColor(Color& c){
  rgb_ = c;
}

// Render all stored text strings
void Font::render(){
  Renderer* rend = CGE::Engine::instance()->getRenderer();
  queue<font_data> temp;

  if (tex_ == NULL)
    return;

  tex_->activate();
  buffer_->activate();

  while(!q_.empty()){
    
    //take out the next font_data
    font_data f = q_.front();
    q_.pop();
    f.duration -= ::CGE::Engine::instance()->getFrameInterval();//gl->getFrameInterval();
    
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
        //unsigned char tmp = (*f.text)[i]-32;
        buffer_->setVertexOffset(f.text->at(i)-32+(128*f.set));
        buffer_->draw(VB_Tristrip, inds_);
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
  setColor(1.0,1.0,1.0);
  print(20, 750, text.c_str(), 0, 10);
}

//for stringstreams
void Font::operator<<(const ostringstream& text){
  (*this) << text.str();
}

// delete the texts with a certain id
void Font::deleteText(short id){
  queue<font_data> temp;
  
  while(!q_.empty()){ 
    font_data f = q_.front();
    q_.pop();
    if (f.id != id)
      temp.push(f);
    else
      delete f.text;
  }
  q_ = temp;
}

