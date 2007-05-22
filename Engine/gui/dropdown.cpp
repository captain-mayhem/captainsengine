#include <sstream>
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"
#include "../math/vector.h"
#include "dropdown.h"

using std::istringstream;
using namespace Gui;
using Graphics::Color;
using Math::Vector3D;
using Math::Vector2D;

DDEntryButton::DDEntryButton() : Button(){
  parent_ = NULL;
  handleClicks_ = NULL;
}

DDEntryButton::~DDEntryButton(){
}

void DDEntryButton::process(){
  if (parent_){
    parent_->setText(input_.getText());
    parent_->process();
  }
  if (handleClicks_){
    (*handleClicks_)(); 
  }
}

//The MessageBox constructor
DropDownButton::DropDownButton(){
  handleClicks_ = NULL;
  input_.setColors(Vector3D(1,1,0.1f),Color(0.2,0.01,0.01,1.0));
  input_.setSpan(Vector2D(160,18));
  ddPos_ = Vector2D(0, 768);
  isOpen_ = false;
  direction_ = 0;
}

DropDownButton::DropDownButton(const DropDownButton& d){
  std::cerr << "Drop Down Button copy constructor not yet implemented\n";
}

//and destructor
DropDownButton::~DropDownButton(){
  if (isOpen_)
    process();
  for (unsigned i = 0; i < entries_.size(); i++){
    delete entries_[i];
  }
  entries_.clear();
}

void DropDownButton::calcDDPos(int direction){
  direction_ = direction;
  ddPos_ = input_.getPosition();
  if (direction == 1){
    ddPos_.x += input_.getSpan().x;
  }
  else if (direction == 2){
    ddPos_.y -= input_.getSpan().y;
  }
  else if (direction == 3){
    ddPos_.x -= 2*input_.getSpan().x;
  }
}

void DropDownButton::addEntry(const string& entry){
  DDEntryButton* but = new DDEntryButton();
  but->setPosition(ddPos_);
  but->setSpan(Vector2D(160,18));
  but->setText(entry);
  but->setParent(this);
  but->setCbFunc(handleClicks_);
  entries_.push_back(but);
  ddPos_.y -= input_.getSpan().y;
}

void DropDownButton::clear(){
  for (unsigned i = 0; i < entries_.size(); i++){
    delete entries_[i];
  }
  entries_.clear();
  calcDDPos(direction_);
}

void DropDownButton::process(){
  if (isOpen_){
    //drop down buttons must be deleted
    if (entries_.size() == 0){
      isOpen_ = false;
      return;
    }
    list< ::Gui::GuiElement*>& buts = System::Engine::instance()->getGuiElements();
    list< ::Gui::GuiElement* >::iterator iter;
    int idx = 0;
    for (iter = buts.begin(); iter != buts.end(); iter++){
      if (*iter == entries_[0]){
        //found position
        //delete the next enties_.size buttons because they are the wanted ones
        for (unsigned i = 0; i < entries_.size(); i++){
          iter = buts.erase(iter);
        }
        break;
      }
      idx++;
    }
    isOpen_ = false;
  }
  else{
    //register drop down buttons
    for (unsigned i = 0; i < entries_.size(); i++){
      System::Engine::instance()->addGuiListener(entries_[i]);
    }
    isOpen_ = true;
  }
}

