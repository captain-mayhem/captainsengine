//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |                 The Open Source game engine                      |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: dialog.cpp                                                 |
//  ==================================================================

#include "dialog.h"

#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"

using namespace Gui;
using namespace CGE;

//! Constructor
Dialog::Dialog(){
  type_ = DialogT;
  pos_ = Vector2D(112, 84);
  span_ = Vector2D(800,600);
  bgColor_ = Color(0.0,0.0,0.0,0.8f);
}
  
//! Destructor
Dialog::~Dialog(){
  list<GuiElement*>::iterator iter;
  for (iter = elements_.begin(); iter != elements_.end(); iter++){
    delete *iter;
  }
  elements_.clear();
}

//! The render function
void Dialog::render(){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(false);
  //draw background
  rend->setColor(&bgColor_);
  CGE::Engine::instance()->getForms()->activateQuad();
  CGE::Engine::instance()->getForms()->drawQuad(pos_, span_);

  list<GuiElement*>::iterator iter;
  for (iter = elements_.begin(); iter != elements_.end(); iter++){
    (*iter)->render();
  }
}

//! Is the dialog clicked
bool Dialog::isClicked(const ::CGE::Vector2D& pos){
  list<GuiElement*>::iterator iter;
  //handle click
  for (iter = elements_.begin(); iter != elements_.end(); iter++){
    if ((*iter)->isClicked(pos)){
      (*iter)->process();
      break;
    }
  }
  //return if it was clicked upon
  if (pos.x < pos_.x || pos.x > pos_.x+span_.x)
    return false;
  if (pos.y < pos_.y || pos.y > pos_.y+span_.y)
    return false;
  return true;
}

//! Process an event
void Dialog::process(){
}

//! Add an element
void Dialog::addElement(GuiElement* elem){
  //Change coordinates to dialog-relative ones
  Vector2D pos = elem->getPosition();
  Vector2D span = elem->getSpan();
  pos.x = pos_.x+(short)((float)pos.x/SCREENWIDTH*span_.x);
  pos.y = pos_.y+(short)((float)pos.y/SCREENHEIGHT*span_.y);
  span.x = (short)((float)span.x/SCREENWIDTH*span_.x);
  span.y = (short)((float)span.y/SCREENHEIGHT*span_.y);
  elem->setPosition(pos);
  elem->setSpan(span);
  elem->setParent(this);
  elements_.push_back(elem);
}

//! Add an unscaled element
void Dialog::addUnscaledElement(GuiElement* elem){
   //Change position to dialog-relative ones
  Vector2D pos = elem->getPosition();
  pos.x = pos_.x+(short)((float)pos.x/SCREENWIDTH*span_.x);
  pos.y = pos_.y+(short)((float)pos.y/SCREENHEIGHT*span_.y);
  elem->setPosition(pos);
  //elem->setSpan(elem->getSpan());
  elem->setParent(this);
  elements_.push_back(elem);
}

//! Add an unscaled element
void Dialog::addOriginalElement(GuiElement* elem){
  elem->setParent(this);
  elements_.push_back(elem);
}

void Dialog::preparePosition(GuiElement* elem) const {
  Vector2D pos = elem->getPosition();
  pos.x = pos_.x+(short)((float)pos.x/SCREENWIDTH*span_.x);
  pos.y = pos_.y+(short)((float)pos.y/SCREENHEIGHT*span_.y);
  elem->setPosition(pos);
}

//! Get an element
GuiElement* Dialog::getElement(int idx) const{
  list<GuiElement*>::const_iterator iter;
  for (iter = elements_.begin(); iter != elements_.end(); iter++){
    if (idx == 0)
      return *iter;
    idx--;
  }
  return NULL;
}

