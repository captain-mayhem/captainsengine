//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: console.cpp                                                |
//  ==================================================================

#include <iostream>
//#ifdef WIN32
//#include <windows.h>
//#endif
//#include <GL/gl.h>

//#include "common.h"
//#include "renderer.h"
#include "../system/engine.h"
#include "../renderer/renderer.h"
#include "../renderer/forms.h"
#include "console.h"

using namespace Gui;

//CONSTRUCTOR
Console::Console(){
  //set standard values
  pos_ = Vector2D(0, 468);
  //fnt_ = System::Engine::instance()->getFont();
  span_ = Vector2D(1024,300);
  opacity_ = 178;
  //put in a nice starting message
  string start("HeroQuest v");
  start += VERSTRING;
  history_.push_front(start);
  history_.push_front("Console started...");
  
  histIter_ = history_.begin(); 
  new_ = false;
  active_ = false;
  special_ = NULL;
  process_ = NULL;
  init();
}

Console::Console(const Console& c){
    input_ = c.input_;
    history_ = list<string>(c.history_);
    histIter_ = list<string>::iterator(c.histIter_);
    pos_ = c.pos_;
    span_ = c.span_;
    bgColor_ = c.bgColor_;
    fgColor_ = c.fgColor_;
    opacity_ = c.opacity_;
    //fnt_ = c.fnt_;
    active_ = c.active_;
    new_ = c.new_;
    process_ = c.process_;
    special_ = c.special_;
    mode_ = c.mode_;
    additional_ = c.additional_;
}

//DESTRUCTOR
Console::~Console(){
  //fnt_ = NULL;
  SAFE_DELETE(input_);
}

//init console
void Console::init(){
  input_ = new InputField();
  input_->setPosition(Vector2D(0, 468));
  input_->setOpacity(0);
  bgColor_ = Graphics::Color(0, 0, 0, 178);
  fgColor_ = Vector3D(1.0, 1.0, 1.0);
  input_->setColors(fgColor_, bgColor_);
  input_->setSpan(Vector2D(1024,18));
}

//adds text to the console
void Console::addText(const string& text){
  int oldpos = 0;
  do{
    //identify newlines and split string there
    int pos = (int)text.find_first_of("\n", oldpos);
    string tmp = text.substr(oldpos, pos-oldpos);
    // is tmp too long?
    while (tmp.size() > 102){
      string part = tmp.substr(0, 102);
      history_.push_front(part);
      histIter_ = history_.begin();
      tmp.erase(0, 102);
    }
    //write it to console
    history_.push_front(tmp);
    histIter_ = history_.begin();
    //console full, delete what is not visible
    while (history_.size() > 15)
      history_.pop_back();
    oldpos = pos+1;
  }
  while(oldpos != 0);
}

//updates console contents
void Console::update(){
  if (!active_)
    return;
  if (input_->isFinished()){
    //get user input
    history_.push_front(input_->getText());
    histIter_ = history_.begin();
    if (history_.size() > 15)
      history_.pop_back();
    new_ = true;
    //reset input line
    input_->clear();
    input_->addChar('_');
    //call special or main callback function
    if (special_){
      void (*s)(const string&, int, void*);
      s = special_;
      special_ = NULL;
      (*s)(history_.front(), mode_, additional_);
    }
    else if (process_)
      (*process_)(history_.front().c_str());
  }
}

//Render console
void Console::render(){
  if (!active_)
    return;
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glDisable(GL_TEXTURE_2D);
  rend->enableTexturing(false);
  //glLoadIdentity();
  //glTranslatef(-512,-384,0);

  //draw background
  //glColor4ub(bgColor_.r,bgColor_.g, bgColor_.b, opacity_);
  rend->setColor(bgColor_.r, bgColor_.g, bgColor_.b, opacity_);
  System::Engine::instance()->getForms()->activateQuad();
  System::Engine::instance()->getForms()->drawQuad(pos_, span_);
  /*
  glBegin(GL_QUADS);
    glVertex2f(pos_.x         , pos_.y+span_.y);
    glVertex2f(pos_.x         , pos_.y         );
    glVertex2f(pos_.x+ span_.x, pos_.y         );
    glVertex2f(pos_.x+ span_.x, pos_.y+span_.y);
  glEnd();*/
  //draw console text
  rend->enableTexturing(true);
  //glEnable(GL_TEXTURE_2D);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
  
  System::Engine::instance()->getFont()->setColor(fgColor_.x,fgColor_.y,fgColor_.z);
  int y = pos_.y + 20;
  for (list<string>::iterator iter = history_.begin(); iter != history_.end(); iter++){
    System::Engine::instance()->getFont()->glPrint(pos_.x, y, (*iter).c_str(), 0, 0.0);
    y += 20;
  }
  //fnt_.render();
  //draw input field
  input_->render();

}

