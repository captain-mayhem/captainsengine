//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: gui.cpp                                                    |
//  ==================================================================

#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/font.h"
#include "../renderer/forms.h"
#include "gui.h"

using namespace Gui;
using Graphics::Color;

//CONSTRUCTOR
InputField::InputField(){
  //set standard values
  pos_ = Vector2D(-1,-1);
  fnt_ = System::Engine::instance()->getFont(1);
  span_ = Vector2D(150,18);
  bgColor_ = Color(0.1, 0.1, 0.1, 1.0);
  fgColor_ = Vector3D(0.0, 1.0, 1.0);
  opacity_ = 1.0;
  finished_ = false;
  isHidden_ = false;
}

InputField::InputField(const InputField& i){
    pos_ = i.pos_;
    span_ = i.span_;
    bgColor_ = i.bgColor_;
    fgColor_ = i.fgColor_;
    opacity_ = i.opacity_;
    fnt_ = i.fnt_;
    field_ = string(field_);
    finished_ = i.finished_;
    isHidden_ = i.isHidden_;
}

// renders the field
void InputField::render(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  //appropriate blending
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(false);
  //glDisable(GL_TEXTURE_2D);

  //set color and draw background quad
  //glColor4f(bgColor_.x,bgColor_.y, bgColor_.z, opacity_);
  bgColor_.a = opacity_;
  rend->setColor(&bgColor_);
  Graphics::Forms* form = System::Engine::instance()->getForms();
  form->activateQuad();
  rend->pushMatrix();
  rend->translate(pos_.x+span_.x/2.0f, pos_.y+span_.y/2.0f, 0);
  rend->scale(span_.x, span_.y, 1);
  form->drawQuad();
  rend->popMatrix();
  /*
  glBegin(GL_QUADS);
    glVertex2f(pos_.x         , pos_.y+span_.y );
    glVertex2f(pos_.x         , pos_.y         );
    glVertex2f(pos_.x+span_.x , pos_.y         );
    glVertex2f(pos_.x+span_.x , pos_.y+span_.y );
  glEnd();
  */
  //glLoadIdentity();

  //render text
  //glEnable(GL_TEXTURE_2D);
  rend->enableTexturing(true);
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  fnt_->setColor(fgColor_.x,fgColor_.y,fgColor_.z);
  
  //cut off too long inputs
  unsigned numChars = span_.x/10;
  string out = field_;
  if (out.length() > numChars)
    out.erase(0,field_.length()-numChars);
		  
  if (isHidden_){
    //replace all characters by stars except the cursor
    string replacement(out.length() ,'*');
    if (out.at(out.length()-1) == '_'){
      replacement.erase(replacement.length()-1,1);
      replacement += '_';
    }
    fnt_->glPrint(pos_.x, pos_.y, replacement.c_str(), 0, 0.0);
  }
  //display string without modifications
  else{
    fnt_->glPrint(pos_.x, pos_.y, out.c_str(), 0, 0.0);
  }
  //fnt_.render();
  //fnt_->setColor(0,1,0);
}

//If the input field is hit by the mouse
bool InputField::isClicked(const Vector2D& pos){
  if (pos.x < pos_.x || pos.x > pos_.x+span_.x)
    return false;
  if (pos.y < pos_.y || pos.y > pos_.y+span_.y)
    return false;
  return true;
}


//The Button constructor
Button::Button(){
  handleClicks_ = NULL;
  input_.setColors(Vector3D(1,1,0.1f),Color(0.2,0.01,0.01,1.0));
  name_ = "";
}

Button::Button(const Button& b){
  input_ = InputField(b.input_);
  handleClicks_ = b.handleClicks_;
  name_ = string(b.name_);
}

//and destructor
Button::~Button(){
}
