#include <sstream>
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"
#include "../math/vector.h"
#include "messagebox.h"

using std::istringstream;
using namespace Gui;
using Graphics::Color;
using Math::Vector3D;
using Math::Vector2D;

//The MessageBox constructor
MessageBox::MessageBox(){
  handleClicks_ = NULL;
  input_.setColors(Vector3D(1,1,0.1f),Color(51,2,2,255));
  input_.setSpan(Vector2D(75,18));
  bgColor_ = Color(0,0,0,200);//178);
  fgColor_ = Color(255,0,0,255);
  setText("  OK");
  span_ = Vector2D(800, 600);
  setPosition(Vector2D(100,100));
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
  list< ::Gui::Button*>& buts = System::Engine::instance()->getButtons();
  list< ::Gui::Button* >::iterator iter;
  int idx = 0;
  for (iter = buts.begin(); iter != buts.end(); iter++){
    if (*iter == this){
      break;
    }
    idx++;
  }
  System::Engine::instance()->removeButtonListener(idx, false);
  if (handleClicks_)
    (*handleClicks_)();
}

void MessageBox::setPosition(const ::Math::Vector2D& pos){
  pos_ = pos;
  Vector2D inpos = pos;
  inpos.x += (short)(0.5*span_.x-75.0/2.0);
  inpos.y += (short)(0.05*span_.y);
  input_.setPosition(inpos);
}

void MessageBox::setMessage(const string& text){
  istringstream str(text);
  unsigned maxchars = span_.x/10-1;
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
  rend->setColor(&bgColor_);//_.r, bgColor_.g, bgColor_.b, opacity_);
  System::Engine::instance()->getForms()->activateQuad();
  System::Engine::instance()->getForms()->drawQuad(pos_, span_);
  
  //draw console text
  rend->enableTexturing(true);
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
  System::Engine::instance()->getFont(1)->setColor(fgColor_);
  int y = pos_.y + span_.y - 20;
  for (list<string>::iterator iter = msg_.begin(); iter != msg_.end(); iter++){
    System::Engine::instance()->getFont(1)->glPrint(pos_.x, y, (*iter).c_str(), 0, 0.0);
    y -= 20;
  }
  input_.render();
}
