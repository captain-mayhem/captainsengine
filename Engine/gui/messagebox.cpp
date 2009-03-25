#include <sstream>
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"
#include "../math/vector.h"
#include "messagebox.h"

using std::istringstream;
using namespace Gui;
using Graphics::Color;
using CGE::Vector3D;
using CGE::Vector2D;

//The MessageBox constructor
MessageBox::MessageBox(){
  handleClicks_ = NULL;
  setColors(Vector3D(1,1,0.1f),Color(0.2,0.01,0.01,1.0));
  setSpan(Vector2D(75,18));
  surBgColor_ = Color(0,0,0,0.8);//178);
  surFgColor_ = Color(1.0,0,0,1.0);
  setText("  OK");
  surSpan_ = Vector2D(800, 600);
  setPosition(Vector2D(100,100));
  type_ = MessageBoxT;
}

MessageBox::MessageBox(const MessageBox& m){
  std::cerr << "MessageBox copy constructor not yet implemented\n";
}

//and destructor
MessageBox::~MessageBox(){
}

void MessageBox::process(){
  //close messageBox
  //find yourself
  list< ::Gui::GuiElement*>& buts = System::Engine::instance()->getGuiElements();
  list< ::Gui::GuiElement* >::iterator iter;
  int idx = 0;
  for (iter = buts.begin(); iter != buts.end(); iter++){
    if (*iter == this){
      break;
    }
    idx++;
  }
  System::Engine::instance()->removeGuiListener(idx, false);
  if (handleClicks_)
    (*handleClicks_)(this);
}

void MessageBox::setPosition(const ::CGE::Vector2D& pos){
  surPos_ = pos;
  Vector2D inpos = pos;
  inpos.x += (short)(0.5*surSpan_.x-75.0/2.0);
  inpos.y += (short)(0.05*surSpan_.y);
  GuiElement::setPosition(inpos);
}

void MessageBox::setMessage(const string& text){
  istringstream str(text);
  unsigned maxchars = surSpan_.x/10-1;
  unsigned chars = 0;
  string word;
  string line = "";
  while (str >> word){
    //the word does not fit into the current line
    if (chars + word.size() > maxchars){
      msg_.push_back(line);
      line = "";
      chars = 0;
    }
    //concatenate word to current line
    line += word + " ";
    chars += word.size()+1;
  }
  msg_.push_back(line);
}

void MessageBox::render(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE_MINUS_SRC_ALPHA);
  rend->enableTexturing(false);

  //draw background
  rend->setColor(&surBgColor_);//_.r, bgColor_.g, bgColor_.b, opacity_);
  System::Engine::instance()->getForms()->activateQuad();
  System::Engine::instance()->getForms()->drawQuad(surPos_, surSpan_);
  
  //draw console text
  rend->enableTexturing(true);
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
  System::Engine::instance()->getFont(1)->setColor(surFgColor_);
  int y = surPos_.y + surSpan_.y - 20;
  for (list<string>::iterator iter = msg_.begin(); iter != msg_.end(); iter++){
    System::Engine::instance()->getFont(1)->glPrint(surPos_.x, y, (*iter).c_str(), 0, 0.0);
    y -= 20;
  }
  InputField::render();
}
