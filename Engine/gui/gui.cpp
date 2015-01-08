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

#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/font.h"
#include "../renderer/forms.h"
#include "gui.h"

using namespace Gui;
using CGE::Color;
using namespace CGE;

GuiElement::GuiElement(){
  //set standard values
  pos_ = Vector2D(-1,-1);
  span_ = Vector2D(150,18);
  bgColor_ = Color(0.1f, 0.1f, 0.1f, 1.0f);
  fgColor_ = Vector3D(0.0f, 1.0f, 1.0f);
  opacity_ = 1.0f;
  name_ = "";
  parent_ = NULL;
  type_ = UnknownT;
}

//If the input field is hit by the mouse
bool GuiElement::isClicked(const Vector2D& pos){
  if (pos.x < pos_.x || pos.x > pos_.x+span_.x)
    return false;
  if (pos.y < pos_.y || pos.y > pos_.y+span_.y)
    return false;
  return true;
}

//CONSTRUCTOR
InputField::InputField(){
  //set standard values
  pos_ = Vector2D(-1,-1);
  fnt_ = CGE::Engine::instance()->getFont(1);
  span_ = Vector2D(150,18);
  bgColor_ = Color(0.1f, 0.1f, 0.1f, 1.0f);
  fgColor_ = Vector3D(0.0f, 1.0f, 1.0f);
  opacity_ = 1.0;
  finished_ = false;
  isHidden_ = false;
  type_ = InputFieldT;
}

InputField::InputField(const InputField& i){
    pos_ = i.pos_;
    span_ = i.span_;
    bgColor_ = i.bgColor_;
    fgColor_ = i.fgColor_;
    opacity_ = i.opacity_;
    fnt_ = i.fnt_;
    field_ = string(i.field_);
    finished_ = i.finished_;
    isHidden_ = i.isHidden_;
    name_ = string(i.name_);
    type_ = i.type_;
    parent_ = i.parent_;
}

InputField::~InputField(){
  InputField* input = CGE::Engine::instance()->getActiveInput();
  if (input == this){
    input->removeChar();
  }
  CGE::Engine::instance()->setActiveInput(NULL);
}

// renders the field
void InputField::render(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  //appropriate blending
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(false);
  //glDisable(GL_TEXTURE_2D);

  //set color and draw background quad
  bgColor_.a = opacity_;
  rend->setColor(&bgColor_);
  CGE::Forms* form = CGE::Engine::instance()->getForms();
  form->activateQuad();
  form->drawQuad(pos_, span_);

  //render text
  rend->enableTexturing(true);
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE);
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
    fnt_->print(pos_.x, pos_.y, replacement.c_str(), 0, 0.0);
  }
  //display string without modifications
  else{
    fnt_->print(pos_.x, pos_.y, out.c_str(), 0, 0.0);
  }
}

void InputField::process(){
  CGE::Engine::instance()->setActiveInput(this);
}

//The Button constructor
Button::Button(){
  handleClicks_ = NULL;
  setColors(Vector3D(1,1,0.1f),Color(0.2f,0.01f,0.01f,1.0f));
  type_ = ButtonT;
}

Button::Button(const Button& b){
  handleClicks_ = b.handleClicks_;
  pos_ = b.pos_;
  span_ = b.span_;
  bgColor_ = b.bgColor_;
  fgColor_ = b.fgColor_;
  opacity_ = b.opacity_;
  fnt_ = b.fnt_;
  field_ = string(b.field_);
  finished_ = b.finished_;
  isHidden_ = b.isHidden_;
  name_ = string(b.name_);
  type_ = b.type_;
  parent_ = b.parent_;
}

//and destructor
Button::~Button(){
}

void PDButton::process(){
 if (handleClicks_)
  (*handleClicks_)(this);
 //try to find the parent
 //TODO: go deeper into dialogs
 list< ::Gui::GuiElement*>& elems = CGE::Engine::instance()->getGuiElements();
 list< ::Gui::GuiElement* >::iterator iter;
 int idx = 0;
 for (iter = elems.begin(); iter != elems.end(); iter++){
   if (*iter == parent_){
     break;
   }
   idx++;
 }
 CGE::Engine::instance()->removeGuiListener(idx, false);
}
