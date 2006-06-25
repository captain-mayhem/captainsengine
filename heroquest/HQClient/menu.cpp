#include "system/engine.h"
#include "gui/gui.h"

#include "message.h"
#include "opcodes.h"
#include "player.h"
#include "menu.h"

using Gui::InputField;
using Gui::Button;

void Menu::connect(){
  if (System::Engine::instance()->getActiveInput() != NULL){
    InputField* inp = System::Engine::instance()->getActiveInput();
    inp->removeChar();
    inp->end();
    System::Engine::instance()->setActiveInput(NULL);
  }
  list<InputField*>::iterator iter;
  string send = "connect";
  for (iter = System::Engine::instance()->getInputFields().begin(); iter != System::Engine::instance()->getInputFields().end(); iter++){
    send += " "+(*iter)->getText();
  }
  msg.process(send.c_str());
  //setup login-GUI after connection
  if (msg.isConnected()){
    System::Engine::instance()->clearListeners();

    System::Engine::instance()->getFont()->setColor(1,1,1);
    System::Engine::instance()->getFont()->glPrint(120, 450, "User name:", 1, HUGE_VAL);
    InputField* in = new InputField();
    in->setPosition(Vector2D(220, 450));
    in->setText(msg.getSetting(2));
    System::Engine::instance()->addInputListener(in);

    System::Engine::instance()->getFont()->glPrint(120, 400, "Password:", 1, HUGE_VAL);
    InputField* in2 = new InputField();
    in2->setPosition(Vector2D(220, 400));
    in2->setHidden();
    in2->setText(msg.getSetting(3));
    System::Engine::instance()->addInputListener(in2);

    Button* but = new Button();
    but->setPosition(Vector2D(220,300));
    but->setText("    Login");
    System::Engine::instance()->addButtonListener(but);
    but->setCbFunc(login);
  }
}

//login button
void Menu::login(){
  if (System::Engine::instance()->getActiveInput() != NULL){
    InputField* inp = System::Engine::instance()->getActiveInput();
    inp->removeChar();
    inp->end();
    System::Engine::instance()->setActiveInput(NULL);
  }
  list<InputField*>::iterator iter;
  string send = toStr(LOGIN);
  for (iter = System::Engine::instance()->getInputFields().begin(); iter != System::Engine::instance()->getInputFields().end(); iter++){
    if (iter == System::Engine::instance()->getInputFields().begin())
      plyr.setName((*iter)->getText());
    send += " "+(*iter)->getText();
  }
  msg.send(send);
}

//package button
void Menu::package(){
  ifstream in("levels/levels.dat");
  int number;
  in >> number;
  if (number <= 1){
    in.close();
    return;
  }
  list<Button*>::iterator but = System::Engine::instance()->getButtons().begin();
  string level;
  string firstlevel;
  in >> firstlevel;
  bool found = (firstlevel == (*but)->getText());
  while(in >> level){
    if (found){
      found = false;
      break;
    }
    if (level == (*but)->getText())
      found = true;
  }
  if(found)
    level = firstlevel;
  (*but)->setText(level);
  in.close();
  Menu::level();
}

//level button
void Menu::level(){
  list<Button*>::iterator but = System::Engine::instance()->getButtons().begin();
  ifstream in(string("levels/"+(*but)->getText()+"/levels.dat").c_str());
  if(!in.good()){
    (*++but)->setText("");
    return;
  }

}

//load level button
void Menu::loadLevel(){
}
