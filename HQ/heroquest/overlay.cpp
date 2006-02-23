//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | Stefan Ruff                                                      |
// |------------------------------------------------------------------|
// | File: overlay.cpp                                                |
//  ==================================================================

#include "overlay.hh"
#ifdef _CLIENT_
  #include "texture.hh"
#endif

//Constructor
Overlay::Overlay(){
  active_ = false;
  rendered_ = false;
  found_ = false;
}

//Destructor
Overlay::~Overlay(){
}

Overlay::Overlay(const Overlay& o){
  position_ = o.position_;
  name_ = o.name_;
  id_ = o.id_;
  active_ = o.active_;
  description_ = string(o.description_);
  orientation_ = o.orientation_;
  short_ = string(o.short_);
  width_ = o.width_;
  height_ = o.height_;
  rendered_ = o.rendered_;
  found_ = o.found_;
}

void Overlay::render2D() const {
#ifdef _CLIENT_
  if (rendered_ || (!active_ && !found_))
    return;
  if (found_){
    glColor4f(1.,1.,1.,0.4);
    glEnable(GL_BLEND);
  }
  int height = wrld.getMapSize().y;
  int dx = SCREENWIDTH/wrld.getMapSize().x;
  int dy = SCREENHEIGHT/height;
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex.overlayTex[id_]);
  switch(orientation_){ 
    case TOP:
      glBegin(GL_QUADS);
        glTexCoord2f(1, 0);
        glVertex2i(position_.x*dx+dx*width_, (height-position_.y)*dy);
        glTexCoord2f(0, 0);
        glVertex2i(position_.x*dx, (height-position_.y)*dy);
        glTexCoord2f(0, 1);
        glVertex2i(position_.x*dx, (height-position_.y-height_)*dy);
        glTexCoord2f(1, 1);
        glVertex2i(position_.x*dx+dx*width_, (height-position_.y-height_)*dy);
      glEnd();
      break;
    case BOTTOM:
      glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex2i((position_.x+1)*dx, (height-(position_.y-height_+1))*dy);
        glTexCoord2f(1, 1);
        glVertex2i((position_.x-width_+1)*dx, (height-(position_.y-height_+1))*dy);
        glTexCoord2f(1, 0);
        glVertex2i((position_.x-width_+1)*dx, (height-(position_.y+1))*dy);
        glTexCoord2f(0, 0);
        glVertex2i((position_.x+1)*dx, (height-(position_.y+1))*dy);
      glEnd();
      break;
    case RIGHT:
//      cerr<<"height: "<<height<<" height_: "<<height_<<" width_: "<<width_<<" pos x: "<<position_.x<<" position y: "<<position_.y<<endl;
      glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2i((position_.x+1)*dx, (height-(position_.y))*dy);
        glTexCoord2f(0, 1);
        glVertex2i((position_.x-height_+1)*dx, (height-(position_.y))*dy);
        glTexCoord2f(1, 1);
        glVertex2i((position_.x-height_+1)*dx, (height-(position_.y+width_))*dy);
        glTexCoord2f(1, 0);
        glVertex2i((position_.x+1)*dx, (height-(position_.y+width_))*dy);
      glEnd();
      break;
    case LEFT:
      glBegin(GL_QUADS);
        glTexCoord2f(1, 1);
        glVertex2i(position_.x*dx+dx*height_, (height-position_.y+width_-1)*dy);
        glTexCoord2f(1, 0);
        glVertex2i(position_.x*dx, (height-position_.y+width_-1)*dy);
        glTexCoord2f(0, 0);
        glVertex2i(position_.x*dx, (height-position_.y)*dy-dy);
        glTexCoord2f(0, 1);
        glVertex2i(position_.x*dx+dx*height_, (height-position_.y)*dy-dy);
      glEnd();
      break;
  }
  if (found_){
    glColor4f(1.,1.,1.,1.);
    glDisable(GL_BLEND);
  }
#endif
}
