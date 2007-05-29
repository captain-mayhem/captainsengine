//  ==================================================================
// |                         Hero Engine                              |
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

//! Constructor
Dialog::Dialog(){
  type_ = DialogT;
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
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(false);
  //draw background
  rend->setColor(&bgColor_);
  System::Engine::instance()->getForms()->activateQuad();
  System::Engine::instance()->getForms()->drawQuad(pos_, span_);

  list<GuiElement*>::iterator iter;
  for (iter = elements_.begin(); iter != elements_.end(); iter++){
    (*iter)->render();
  }
}

//! is the dialog clicked
bool Dialog::isClicked(const ::Math::Vector2D& pos){
  return true;
}

