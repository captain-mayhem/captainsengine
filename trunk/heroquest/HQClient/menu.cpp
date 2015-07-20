#include "system/engine.h"
#include "gui/gui.h"
#include "gui/messagebox.h"
#include "gui/dropdown.h"

#include "message.h"
#include "opcodes.h"
#include "player.h"
#include "world.h"
#include "camera.h"
#include "renderer.h"
#include "templates.h"
#include "trade.h"
#include "menu.h"

//to compile on windows
#undef MessageBox
using CGE::Font;
using Gui::GuiElement;
using Gui::InputField;
using Gui::Button;
using Gui::MessageBox;
using Gui::DropDownButton;

#define line *CGE::Engine::instance()->getFont(0)

extern string home;

// setup main ingame menu
void Menu::mainMenu(GuiElement*){
  CGE::Engine::instance()->clearListeners();
	//void (Renderer::*p)();

	Button* but = new Button();
	but->setPosition(Vector2D(900, 240));
	but->setText("Attack");
	//p = &Renderer::attack;
	but->setCbFunc(attack);
  CGE::Engine::instance()->addGuiListener(but);
    
  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Search");
	//p = &Renderer::search;
	but->setCbFunc(search);
  CGE::Engine::instance()->addGuiListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Inventory");
	//p = &Renderer::inventory;
	but->setCbFunc(inventory);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 150));
  but->setText("Trap");
	//p = &Renderer::trapMenu;
	but->setCbFunc(trapMenu);
  CGE::Engine::instance()->addGuiListener(but);

	but = new Button();
	but->setPosition(Vector2D(900, 120));
	but->setText("Open Door");
	//p = &Renderer::open;
	but->setCbFunc(open);
  CGE::Engine::instance()->addGuiListener(but);

	but = new Button();
	but->setPosition(Vector2D(900, 90));
	but->setText("Move");
	//p = &Renderer::move;
	but->setCbFunc(move);
  CGE::Engine::instance()->addGuiListener(but);
    
	but = new Button();
	but->setPosition(Vector2D(900, 60));
	but->setText("Other");
	//p = &Renderer::other;
	but->setCbFunc(other);
  CGE::Engine::instance()->addGuiListener(but);
	
  but = new Button();
	but->setPosition(Vector2D(900, 30));
	but->setText("End turn");
	//p = &Renderer::endTurn;
	but->setCbFunc(endTurn);
  CGE::Engine::instance()->addGuiListener(but);

  if (msg.isToDefend()){
		but = new Button();
		but->setPosition(Vector2D(900, 270));
		but->setText("Defend");
		but->setCbFunc(Menu::defend);
    CGE::Engine::instance()->addGuiListener(but);
  }
}


void Menu::connect(GuiElement*){
  if (CGE::Engine::instance()->getActiveInput() != NULL){
    InputField* inp = CGE::Engine::instance()->getActiveInput();
    inp->removeChar();
    inp->end();
    CGE::Engine::instance()->setActiveInput(NULL);
  }
  list<GuiElement*>::iterator iter;
  string send = "connect";
  for (iter = CGE::Engine::instance()->getGuiElements().begin(); iter != CGE::Engine::instance()->getGuiElements().end(); iter++){
    InputField* inp = dynamic_cast<InputField*>(*iter);
    if ((*iter)->getType() == Gui::InputFieldT && inp)
      send += " "+inp->getText();
  }
  msg.process(send.c_str());
}

void Menu::setupLogin(){
  //setup login-GUI after connection
  if (msg.isConnected()){
    CGE::Engine::instance()->clearListeners();

    CGE::Engine::instance()->getFont(0)->setColor(1, 1, 1);
    CGE::Engine::instance()->getFont(0)->print(120, 450, "User name:", 1, (float)HUGE_VAL);
    InputField* in = new InputField();
    in->setPosition(Vector2D(220, 450));
    in->setText(msg.getSetting(2));
    CGE::Engine::instance()->addGuiListener(in);

    CGE::Engine::instance()->getFont(0)->print(120, 400, "Password:", 1, (float)HUGE_VAL);
    InputField* in2 = new InputField();
    in2->setPosition(Vector2D(220, 400));
    in2->setHidden();
    in2->setText(msg.getSetting(3));
    CGE::Engine::instance()->addGuiListener(in2);

    Button* but = new Button();
    but->setPosition(Vector2D(220, 300));
    but->setText("    Login");
    CGE::Engine::instance()->addGuiListener(but);
    but->setCbFunc(login);
  }
}

void Menu::playLocal(Gui::GuiElement* elem){
  if (CGE::Engine::instance()->getActiveInput() != NULL){
    InputField* inp = CGE::Engine::instance()->getActiveInput();
    inp->removeChar();
    inp->end();
    CGE::Engine::instance()->setActiveInput(NULL);
  }
  list<GuiElement*>::iterator iter;
  string send = "startserver";
  int count = 0;
  for (iter = CGE::Engine::instance()->getGuiElements().begin(); iter != CGE::Engine::instance()->getGuiElements().end(); iter++){
    InputField* inp = dynamic_cast<InputField*>(*iter);
    if ((*iter)->getType() == Gui::InputFieldT && inp){
      if (count > 0)
        send += " " + inp->getText();
      ++count;
    }
  }
  msg.process(send.c_str());
}

//login button
void Menu::login(GuiElement*){
  if (CGE::Engine::instance()->getActiveInput() != NULL){
    InputField* inp = CGE::Engine::instance()->getActiveInput();
    inp->removeChar();
    inp->end();
    CGE::Engine::instance()->setActiveInput(NULL);
  }
  list<GuiElement*>::iterator iter;
  string send = toStr(LOGIN);
  bool firstInput = true;
  for (iter = CGE::Engine::instance()->getGuiElements().begin(); iter != CGE::Engine::instance()->getGuiElements().end(); iter++){
    InputField* inp = dynamic_cast<InputField*>(*iter);
    if ((*iter)->getType() == Gui::InputFieldT && inp){
      if (firstInput){
        plyr.setName(inp->getText());
        firstInput = false;
      }
      send += " "+inp->getText();
    }
  }
  msg.send(send);
}

//package button
void Menu::package(GuiElement* elem){
  ifstream in("levels/levels.dat");
  int number;
  in >> number;
  if (number <= 1){
    in.close();
    return;
  }
  list<GuiElement*>::iterator gui = CGE::Engine::instance()->getGuiElements().begin();
  Button* but = dynamic_cast<Button*>(*gui);
  string level;
  string firstlevel;
  in >> firstlevel;
  bool found = (firstlevel == but->getText());
  while(in >> level){
    if (found){
      found = false;
      break;
    }
    if (level == but->getText())
      found = true;
  }
  if(found)
    level = firstlevel;
  but->setText(level);
  in.close();
  Menu::level(elem);
}

//level button
void Menu::level(GuiElement*){
  list<GuiElement*>::iterator gui = CGE::Engine::instance()->getGuiElements().begin();
  Button* but = dynamic_cast<Button*>(*gui);
  ifstream in(string("levels/"+but->getText()+"/levels.dat").c_str());
  if(!in.good()){
    gui++;
    but = dynamic_cast<Button*>(*gui);
    but->setText("");
    return;
  }
  gui++;
  DropDownButton* db = dynamic_cast<DropDownButton*>(*gui);
  if (db){
    if (db->isOpen()){
      db->process();
    }
    db->clear();
    string level;
    in >> level;
    //first entry
    in >> level;
    db->addEntry(level);
    db->setText(level);
    while (in >> level){
      db->addEntry(level);
    }
  }
  /*
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
  (*but)->setText(level);*/
  in.close();
}

//load level button
void Menu::loadLevel(GuiElement*){
  list<GuiElement*>::iterator iter;
  string proc = "create game";
  for (iter = CGE::Engine::instance()->getGuiElements().begin(); iter != CGE::Engine::instance()->getGuiElements().end(); iter++){
    if (iter == CGE::Engine::instance()->getGuiElements().end())
      break;
    Button* but = dynamic_cast<Button*>(*iter);
    proc += " "+but->getText();
  }
  msg.process(proc.c_str());
}

//move button
void Menu::move(GuiElement*){
  msg.process("move");
}

//end turn button
void Menu::endTurn(GuiElement*){
  msg.process("end turn");
}

//start button
void Menu::start(GuiElement*){
  msg.process("start");
}

//play button
void Menu::play(GuiElement*){
  GuiElement* elem = CGE::Engine::instance()->getGuiListener("Playname");
  int position = -1;
  for (int i = 0; i < wrld.getHeroSize(); i++){
    //cerr << HQRenderer::instance()->getClickedField().x << " " << HQRenderer::instance()->getClickedField().y << "\n";
    if (HQRenderer::instance()->getClickedField() == wrld.getStarts()[i]){
      position = i;
      break;
    }
  }
  if (position == -1)
    line << "Invalid starting position";
  //position is valid
  else {
    //string file = "data/"+plyr.getName()+(*iter)->getText() +".sav";
    InputField* inp = dynamic_cast<InputField*>(elem);
    string file = home+plyr.getName()+inp->getText() +".sav";
    Hero* heroe = plyr.addHero(file);
    if (heroe != NULL)
      msg.special(toStr(position), PLAY, (void*)heroe);
  }

  //Reset GUI
  CGE::Engine::instance()->getFont(0)->clear();
  CGE::Engine::instance()->getFont(1)->clear();
  CGE::Engine::instance()->removeGuiListener("Play");
  CGE::Engine::instance()->removeGuiListener("Playname");
  /*CGE::Engine::instance()->clearListeners();
  Button* but = new Button();
  but->setPosition(Vector2D(900,170));
  but->setText("Start");
  //void (Renderer::*p)();
  //p = &Renderer::start;
  but->setCbFunc(start);
  CGE::Engine::instance()->addButtonListener(but);
*/
  
  //cf_ = Vector2D(-1,-1);
  HQRenderer::instance()->resetClickedField();
}

//open button
void Menu::open(GuiElement*){
  string dir;
  Direction d = cam.getLookDirection();
  switch(d){
    case TOP:
      dir = "w";
      break;
    case RIGHT:
      dir = "d";
      break;
    case BOTTOM:
      dir = "s";
      break;
    case LEFT:
      dir = "a";
      break;
  }
  string cmd = "open "+dir;
  msg.process(cmd.c_str());
}

//attack button
void Menu::attack(GuiElement*){
  Vector2D pos = cam.modelPos();
  short id = plyr.getCreature()->getInventory()->getArmory("right hand").getId();
  //is it a long range weapon?
  if (id == 3 || id == 9 || id == 6 || id == 7){
    HQRenderer::instance()->setViewTo3D(false);
    HQRenderer::instance()->setAwaitMapclick(attackOn);
    return;
  }
  Direction d = cam.getLookDirection();
  switch(d){
    case TOP:
      pos.y--;
      break;
    case RIGHT:
      pos.x++;
      break;
    case BOTTOM:
      pos.y++;
      break;
    case LEFT:
      pos.x--;
      break;
  }
  string cmd = "attack "+toStr(pos.x)+" "+toStr(pos.y);
  msg.process(cmd.c_str());
}

//! attack button for long range
void Menu::attackOn(Vector2D click){
  HQRenderer::instance()->setViewTo3D(true);
  string cmd = "attack "+toStr(click.x)+" "+toStr(click.y);
  msg.process(cmd.c_str());
}

//defend button
void Menu::defend(GuiElement*){
  msg.setDefended();
  msg.process("defend");
  CGE::Engine::instance()->removeGuiListener(CGE::Engine::instance()->getGuiElements().size()-1);
}

// search menu
void Menu::search(GuiElement*){
  CGE::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
    
  Button* but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Secretdoor");
	//p = &Renderer::secretdoor;
	but->setCbFunc(secretdoor);
	CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Treasure");
	//p = &Renderer::treasure;
	but->setCbFunc(treasure);
	CGE::Engine::instance()->addGuiListener(but);
    
  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Trap");
	//p = &Renderer::trap;
	but->setCbFunc(trap);
	CGE::Engine::instance()->addGuiListener(but);
    
  but = new Button();
  but->setPosition(Vector2D(900, 140));
  but->setText("Back");
	//p = &Renderer::mainMenu;
	but->setCbFunc(mainMenu);
	CGE::Engine::instance()->addGuiListener(but);
}

// search button
void Menu::secretdoor(GuiElement* elem){
  msg.process("secretdoor");
  mainMenu(elem);
}

// search button
void Menu::treasure(GuiElement* elem){
  msg.process("treasure");
  mainMenu(elem);
}
 
// search button
void Menu::trap(GuiElement* elem){
  msg.process("trap");
  mainMenu(elem);
}


// inventory button
void Menu::inventory(GuiElement*){
  CGE::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
  Button* but;
  
  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Use");
	//p = &Renderer::use;
	but->setCbFunc(use);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Take off");
	//p = &Renderer::takeoff;
	but->setCbFunc(takeoff);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("What is");
	//p = &Renderer::whatis;
	but->setCbFunc(whatis);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 150));
  but->setText("Drop");
  but->setCbFunc(drop);
  CGE::Engine::instance()->addGuiListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 110));
  but->setText("Close");
	//p = &Renderer::close;
	but->setCbFunc(close);
  CGE::Engine::instance()->addGuiListener(but);

  //get current inventory
  HQRenderer::instance()->setInventory(plyr.getCreature()->getInventory());
}

//use button
void Menu::use(GuiElement*){
  Item* ite = HQRenderer::instance()->getInventory()->getChosenItem();
  if (ite == NULL){
    line << "Please select an item first.";
  }
  else{
    // map click is needed
    if (ite->getType() == Item::Spell){
      HQRenderer::instance()->setViewTo3D(false);
      HQRenderer::instance()->setAwaitMapclick(useOn);
      HQRenderer::instance()->getInventory()->makeVisible(false);
    }
    else{
      HQRenderer::instance()->getInventory()->deselect();
      msg.process(("use "+ite->getName()).c_str());
    }
  }
}
  
void Menu::useOn(Vector2D click){
  Item* ite = HQRenderer::instance()->getInventory()->getChosenItem();
  HQRenderer::instance()->getInventory()->makeVisible(true);
  HQRenderer::instance()->getInventory()->deselect();
  msg.process(("use "+ite->getName()+" "+toStr(click.x)+" "+toStr(click.y)).c_str());
}

//takeoff button
void Menu::takeoff(GuiElement*){
  Item* ite = HQRenderer::instance()->getInventory()->getChosenItem();
  if (ite == NULL){
    line << "Please select an item first.";
  }
  else{
    HQRenderer::instance()->getInventory()->deselect();
    msg.process(("takeoff "+ite->getName()).c_str());
  }
}

//whatis button
void Menu::whatis(GuiElement*){
  Item* ite = HQRenderer::instance()->getInventory()->getChosenItem();
  if (ite == NULL){
    line << "Please select an item first.";
  }
  else{
    HQRenderer::instance()->getInventory()->deselect();
    msg.process(("whatis "+ite->getName()).c_str());
  }
}

//close inventory
void Menu::close(GuiElement* elem){
  HQRenderer::instance()->getInventory()->makeVisible(true);
  HQRenderer::instance()->getInventory()->deselect();
  HQRenderer::instance()->setInventory(NULL);
  HQRenderer::instance()->setAwaitMapclick(NULL);
  mainMenu(elem);
}

// trap button
void Menu::trapMenu(GuiElement*){
  CGE::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
  Button* but;

  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Search");
	//p = &Renderer::trap;
	but->setCbFunc(trap);
  CGE::Engine::instance()->addGuiListener(but);
 
  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Disarm");
	//p = &Renderer::disarm;
	but->setCbFunc(disarm);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Jump");
	//p = &Renderer::trap;
	but->setCbFunc(jump);
  CGE::Engine::instance()->addGuiListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 140));
  but->setText("Back");
	//p = &Renderer::mainMenu;
	but->setCbFunc(mainMenu);
  CGE::Engine::instance()->addGuiListener(but);
}

void Menu::disarm(GuiElement* elem){
  Vector2D pos = cam.modelPos();
  Direction d = cam.getLookDirection();
  string cmd = "disarm ";
  switch(d){
    case TOP:
      cmd += "w";
      break;
    case RIGHT:
      cmd += "d";
      break;
    case BOTTOM:
      cmd += "s";
      break;
    case LEFT:
      cmd += "a";
      break;
  }
  msg.process(cmd.c_str());
  mainMenu(elem);
}

void Menu::jump(GuiElement* elem){
  Vector2D pos = cam.modelPos();
  Direction d = cam.getLookDirection();
  string cmd = "jump ";
  switch(d){
    case TOP:
      cmd += "w";
      break;
    case RIGHT:
      cmd += "d";
      break;
    case BOTTOM:
      cmd += "s";
      break;
    case LEFT:
      cmd += "a";
      break;
  }
  msg.process(cmd.c_str());
  mainMenu(elem);
}

// other menu
void Menu::other(GuiElement*){
  CGE::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
  Button* but;

  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Who is");
	//p = &Renderer::whois;
	but->setCbFunc(whois);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Pick up");
  but->setCbFunc(pickup);
  CGE::Engine::instance()->addGuiListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 170));
  but->setText("Back");
	//p = &Renderer::mainMenu;
	but->setCbFunc(mainMenu);
  CGE::Engine::instance()->addGuiListener(but);
}

// whois button
void Menu::whois(GuiElement*){
  HQRenderer::instance()->setViewTo3D(false);
  HQRenderer::instance()->setAwaitMapclick(whois);
}
  
//! whois button
void Menu::whois(Vector2D click){
  msg.process(("whois "+toStr(click.x)+" "+toStr(click.y)).c_str());
}

//! shop button
void Menu::shop(GuiElement*){
  CGE::Engine::instance()->clearListeners();
  
  HQRenderer::instance()->setTrade(true);
  
  Button* but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Buy");
	but->setCbFunc(buy);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Sell");
	but->setCbFunc(sell);
  CGE::Engine::instance()->addGuiListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("What is");
	but->setCbFunc(whatisShop);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 140));
  but->setText("Close");
	//p = &Renderer::mainMenu;
	but->setCbFunc(closeShop);
  CGE::Engine::instance()->addGuiListener(but);
}

//! close shop button
void Menu::closeShop(GuiElement*){
  CGE::Engine::instance()->clearListeners();
  HQRenderer::instance()->setTrade(false);
  plyr.getTrade()->deselect();
  
  Button* but = new Button();
	but->setPosition(Vector2D(900, 170));
	but->setText("Start");
	but->setCbFunc(Menu::start);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 130));
  but->setText("Shop");
  but->setCbFunc(Menu::shop);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 90));
  but->setText("Zargon");
  but->setCbFunc(Menu::zargon);
  CGE::Engine::instance()->addGuiListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 50));
  but->setText("Create hero");
  but->setCbFunc(Menu::createHero);
  CGE::Engine::instance()->addGuiListener(but);
}

//whatis (shop) button
void Menu::whatisShop(GuiElement*){
  Item ite = plyr.getTrade()->getChosenItem();
  if (!ite.isValid()){
    line << "Please select an item first.";
  }
  else{
    plyr.getTrade()->deselect();
    msg.process(("whatis "+ite.getName()).c_str());
  }
}

void Menu::buy(GuiElement*){
  Item ite = plyr.getTrade()->getChosenItem();
  if (!ite.isValid()){
    line << "Please select an item first.";
  }
  else{
    plyr.getTrade()->deselect();
    msg.process(("buy "+ite.getName()).c_str());
  }
}

void Menu::sell(GuiElement*){
  Item ite = plyr.getTrade()->getChosenItem();
  if (!ite.isValid()){
    line << "Please select an item first.";
  }
  else{
    plyr.getTrade()->deselect();
    msg.process(("sell "+ite.getName()).c_str());
  }
}

void Menu::zargon(GuiElement*){
  msg.process("play zargon");
}

void Menu::drop(GuiElement*){
  Item* ite = HQRenderer::instance()->getInventory()->getChosenItem();
  if (ite == NULL){
    line << "Please select an item first.";
  }
  else{
    HQRenderer::instance()->getInventory()->deselect();
    msg.process(("drop "+ite->getName()).c_str());
  }
}

void Menu::pickup(GuiElement*){
  msg.process("pickup");
}

void Menu::createHero(GuiElement*){
  MessageBox* mb = new MessageBox();
  mb->setCbFunc(chexec);
  CGE::Engine::instance()->addGuiListener(mb);
  Font* fnt = CGE::Engine::instance()->getFont(1);
  fnt->setColor(1,1,1);
  
  fnt->setId(2);
  fnt->print(120, 650, "Character class:", 1, (float)HUGE_VAL);
  DropDownButton* cls = new DropDownButton();
  cls->setPosition(Vector2D(300, 650));
  cls->calcDDPos(2);
  cls->setCbFunc(Hero::createFromGui);
  cls->setName("class");
  vector<Hero>& heros = msg.getHeros();
  for (unsigned i = 0; i < heros.size(); i++){
    cls->addEntry(heros[i].getType());
  }
  CGE::Engine::instance()->addGuiListener(cls);
  
  fnt->setId(2);
  fnt->print(120, 610, "Name:", 1, (float)HUGE_VAL);
  InputField* in = new InputField();
  in->setName("heroname");
  in->setPosition(Vector2D(180, 610));
  CGE::Engine::instance()->addGuiListener(in);

}

// create hero execute button
void Menu::chexec(GuiElement*){
  //fonts
  CGE::Font* fnt = CGE::Engine::instance()->getFont(1);
  fnt->deleteText(1);
  fnt->deleteText(2);
  
  //character class
  Button* cls = dynamic_cast<Button*>(CGE::Engine::instance()->getGuiListener("class"));
  string charClass = cls->getText();
  CGE::Engine::instance()->removeGuiListener("class");

  //character name
  if (CGE::Engine::instance()->getActiveInput() != NULL){
    InputField* inp = CGE::Engine::instance()->getActiveInput();
    inp->removeChar();
    inp->end();
    CGE::Engine::instance()->setActiveInput(NULL);
  }
  GuiElement* ele = CGE::Engine::instance()->getGuiListener("heroname");
  InputField* inp = dynamic_cast<InputField*>(ele);
  string name = inp->getText();
  CGE::Engine::instance()->removeGuiListener("heroname");
  
  //find hero
  Hero hero;
  const vector<Hero>& heroes = msg.getHeros();
  for (unsigned i = 0; i < heroes.size(); i++){
    hero = heroes[i];
    if (charClass == hero.getType())
      break;
  }
  hero.setName(name);
  hero.setPlayer(plyr.getName());
  
  //spells
  Inventory* inv = hero.getInventory();
  while (Button* but = dynamic_cast<Button*>(CGE::Engine::instance()->getGuiListener("spell"))){
    string spell = but->getText();
    CGE::Engine::instance()->removeGuiListener("spell");
    vector<Item> spells = Templates::instance()->getSpells(spell);
    for (unsigned int i = 0; i < spells.size(); i++){
      inv->addItem(spells[i]);
    }
  }

  //save hero
  if (name.size() == 0){
    line << "Creation failed. Enter a name for the hero.";
  }
  else{
    string p = home + plyr.getName()+hero.getName()+".sav";
    hero.write(p);
    line << hero.getName()+" successfully created.";
  }
}

