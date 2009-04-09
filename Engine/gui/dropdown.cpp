#include <sstream>
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"
#include "../math/vector.h"
#include "dialog.h"
#include "dropdown.h"

using std::istringstream;
using namespace Gui;
using CGE::Color;
using CGE::Vector3D;
using CGE::Vector2D;

DDEntryButton::DDEntryButton(int number) : Button(){
  parent_ = NULL;
  handleClicks_ = NULL;
  entryNum_ = number;
}

DDEntryButton::~DDEntryButton(){
}

void DDEntryButton::process(){
  if (parent_){
    DropDownButton* ddb = dynamic_cast<DropDownButton*>(parent_);
    parent_->setText(getText());
    ddb->setSelection(entryNum_);
    parent_->process();
  }
  if (handleClicks_){
    (*handleClicks_)(this); 
  }
}

//The MessageBox constructor
DropDownButton::DropDownButton(){
  handleClicks_ = NULL;
  setColors(Vector3D(1.0f,1.0f,0.1f),Color(0.2f,0.01f,0.01f,1.0f));
  setSpan(Vector2D(160,18));
  ddPos_ = Vector2D(0, 768);
  isOpen_ = false;
  direction_ = 0;
  entryNum_ = 0;
  selected_ = 0;
  type_ = DropDownButtonT;
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
  ddPos_ = getPosition();
  if (direction == 1){
    ddPos_.x += getSpan().x;
  }
  else if (direction == 2){
    ddPos_.y -= getSpan().y;
  }
  else if (direction == 3){
    ddPos_.x -= 2*getSpan().x;
  }
}

void DropDownButton::addEntry(const string& entry){
  DDEntryButton* but = new DDEntryButton(entryNum_++);
  but->setPosition(ddPos_);
  but->setSpan(Vector2D(160,18));
  but->setText(entry);
  but->setParent(this);
  but->setCbFunc(handleClicks_);
  entries_.push_back(but);
  ddPos_.y -= getSpan().y;
}

void DropDownButton::clear(){
  for (unsigned i = 0; i < entries_.size(); i++){
    delete entries_[i];
  }
  entries_.clear();
  entryNum_ = 0;
  calcDDPos(direction_);
}

void DropDownButton::process(){
  if (isOpen_){
    //drop down buttons must be deleted
    if (entries_.size() == 0){
      isOpen_ = false;
      return;
    }
    list< ::Gui::GuiElement*>* buts;
    if (parent_){
      Dialog* dia = dynamic_cast<Dialog*>(parent_);
      if (dia)
        buts = &dia->getElements();
    }
    else{
      buts = &CGE::Engine::instance()->getGuiElements();
    }
    list< ::Gui::GuiElement* >::iterator iter;
    int idx = 0;
    for (iter = buts->begin(); iter != buts->end(); iter++){
      if (*iter == entries_[0]){
        //found position
        //delete the next enties_.size buttons because they are the wanted ones
        for (unsigned i = 0; i < entries_.size(); i++){
          iter = buts->erase(iter);
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
      if (parent_){
        //is it a dialog
        Dialog* dia = dynamic_cast<Dialog*>(parent_);
        if (dia)
          dia->addOriginalElement(entries_[i]);
      }
      else
        CGE::Engine::instance()->addGuiListener(entries_[i]);
    }
    isOpen_ = true;
  }
}

