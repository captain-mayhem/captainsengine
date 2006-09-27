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
#include "trade.h"
#include "menu.h"

//to compile on windows
#undef MessageBox
using Graphics::Font;
using Gui::InputField;
using Gui::Button;
using Gui::MessageBox;
using Gui::DropDownButton;

#define line *System::Engine::instance()->getFont(0)

extern string home;

// setup main ingame menu
void Menu::mainMenu(){
  System::Engine::instance()->clearListeners();
	//void (Renderer::*p)();

	Button* but = new Button();
	but->setPosition(Vector2D(900, 240));
	but->setText("Attack");
	//p = &Renderer::attack;
	but->setCbFunc(attack);
  System::Engine::instance()->addButtonListener(but);
    
  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Search");
	//p = &Renderer::search;
	but->setCbFunc(search);
  System::Engine::instance()->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Inventory");
	//p = &Renderer::inventory;
	but->setCbFunc(inventory);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 150));
  but->setText("Trap");
	//p = &Renderer::trapMenu;
	but->setCbFunc(trapMenu);
  System::Engine::instance()->addButtonListener(but);

	but = new Button();
	but->setPosition(Vector2D(900, 120));
	but->setText("Open Door");
	//p = &Renderer::open;
	but->setCbFunc(open);
  System::Engine::instance()->addButtonListener(but);

	but = new Button();
	but->setPosition(Vector2D(900, 90));
	but->setText("Move");
	//p = &Renderer::move;
	but->setCbFunc(move);
  System::Engine::instance()->addButtonListener(but);
    
	but = new Button();
	but->setPosition(Vector2D(900, 60));
	but->setText("Other");
	//p = &Renderer::other;
	but->setCbFunc(other);
  System::Engine::instance()->addButtonListener(but);
	
  but = new Button();
	but->setPosition(Vector2D(900, 30));
	but->setText("End turn");
	//p = &Renderer::endTurn;
	but->setCbFunc(endTurn);
  System::Engine::instance()->addButtonListener(but);

  if (msg.isToDefend()){
		but = new Button();
		but->setPosition(Vector2D(900, 270));
		but->setText("Defend");
		but->setCbFunc(Menu::defend);
    System::Engine::instance()->addButtonListener(but);
  }
}


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

    System::Engine::instance()->getFont(0)->setColor(1,1,1);
    System::Engine::instance()->getFont(0)->glPrint(120, 450, "User name:", 1, HUGE_VAL);
    InputField* in = new InputField();
    in->setPosition(Vector2D(220, 450));
    in->setText(msg.getSetting(2));
    System::Engine::instance()->addInputListener(in);

    System::Engine::instance()->getFont(0)->glPrint(120, 400, "Password:", 1, HUGE_VAL);
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
  but++;
  DropDownButton* db = dynamic_cast<DropDownButton*>(*but);
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
void Menu::loadLevel(){
  list<Button*>::iterator iter;
  string proc = "create game";
  for (iter = System::Engine::instance()->getButtons().begin(); iter != System::Engine::instance()->getButtons().end(); iter++){
    if (iter == System::Engine::instance()->getButtons().end())
      break;
    proc += " "+(*iter)->getText();
  }
  msg.process(proc.c_str());
}

//move button
void Menu::move(){
  msg.process("move");
}

//end turn button
void Menu::endTurn(){
  msg.process("end turn");
}

//start button
void Menu::start(){
  msg.process("start");
}

//play button
void Menu::play(){
  list<InputField*>::iterator iter = System::Engine::instance()->getInputFields().begin();
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
    string file = home+plyr.getName()+(*iter)->getText() +".sav";
    Hero* heroe = plyr.addHero(file);
    if (heroe != NULL)
      msg.special(toStr(position), PLAY, (void*)heroe);
  }

  //Reset GUI
  System::Engine::instance()->getFont(0)->clear();
  System::Engine::instance()->getFont(1)->clear();
  System::Engine::instance()->removeButtonListener("    Play");
  System::Engine::instance()->removeInputListener(System::Engine::instance()->getInputFields().size()-1);
  /*System::Engine::instance()->clearListeners();
  Button* but = new Button();
  but->setPosition(Vector2D(900,170));
  but->setText("Start");
  //void (Renderer::*p)();
  //p = &Renderer::start;
  but->setCbFunc(start);
  System::Engine::instance()->addButtonListener(but);
*/
  
  //cf_ = Vector2D(-1,-1);
  HQRenderer::instance()->resetClickedField();
}

//open button
void Menu::open(){
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
void Menu::attack(){
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
void Menu::defend(){
  msg.setDefended();
  msg.process("defend");
  System::Engine::instance()->removeButtonListener(System::Engine::instance()->getButtons().size()-1);
}

// search menu
void Menu::search(){
  System::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
    
  Button* but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Secretdoor");
	//p = &Renderer::secretdoor;
	but->setCbFunc(secretdoor);
	System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Treasure");
	//p = &Renderer::treasure;
	but->setCbFunc(treasure);
	System::Engine::instance()->addButtonListener(but);
    
  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Trap");
	//p = &Renderer::trap;
	but->setCbFunc(trap);
	System::Engine::instance()->addButtonListener(but);
    
  but = new Button();
  but->setPosition(Vector2D(900, 140));
  but->setText("Back");
	//p = &Renderer::mainMenu;
	but->setCbFunc(mainMenu);
	System::Engine::instance()->addButtonListener(but);
}

// search button
void Menu::secretdoor(){
  msg.process("secretdoor");
  mainMenu();
}

// search button
void Menu::treasure(){
  msg.process("treasure");
  mainMenu();
}
 
// search button
void Menu::trap(){
  msg.process("trap");
  mainMenu();
}


// inventory button
void Menu::inventory(){
  System::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
  Button* but;
  
  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Use");
	//p = &Renderer::use;
	but->setCbFunc(use);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Take off");
	//p = &Renderer::takeoff;
	but->setCbFunc(takeoff);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("What is");
	//p = &Renderer::whatis;
	but->setCbFunc(whatis);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 150));
  but->setText("Drop");
  but->setCbFunc(drop);
  System::Engine::instance()->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 110));
  but->setText("Close");
	//p = &Renderer::close;
	but->setCbFunc(close);
  System::Engine::instance()->addButtonListener(but);

  //get current inventory
  HQRenderer::instance()->setInventory(plyr.getCreature()->getInventory());
}

//use button
void Menu::use(){
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
void Menu::takeoff(){
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
void Menu::whatis(){
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
void Menu::close(){
  HQRenderer::instance()->getInventory()->makeVisible(true);
  HQRenderer::instance()->getInventory()->deselect();
  HQRenderer::instance()->setInventory(NULL);
  HQRenderer::instance()->setAwaitMapclick(NULL);
  mainMenu();
}

// trap button
void Menu::trapMenu(){
  System::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
  Button* but;

  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Search");
	//p = &Renderer::trap;
	but->setCbFunc(trap);
  System::Engine::instance()->addButtonListener(but);
 
  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Disarm");
	//p = &Renderer::disarm;
	but->setCbFunc(disarm);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Jump");
	//p = &Renderer::trap;
	but->setCbFunc(jump);
  System::Engine::instance()->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 140));
  but->setText("Back");
	//p = &Renderer::mainMenu;
	but->setCbFunc(mainMenu);
  System::Engine::instance()->addButtonListener(but);
}

void Menu::disarm(){
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
  mainMenu();
}

void Menu::jump(){
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
  mainMenu();
}

// other menu
void Menu::other(){
  System::Engine::instance()->clearListeners();
	//void (Renderer::*p)();
  Button* but;

  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Who is");
	//p = &Renderer::whois;
	but->setCbFunc(whois);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Pick up");
  but->setCbFunc(pickup);
  System::Engine::instance()->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 170));
  but->setText("Back");
	//p = &Renderer::mainMenu;
	but->setCbFunc(mainMenu);
  System::Engine::instance()->addButtonListener(but);
}

// whois button
void Menu::whois(){
  HQRenderer::instance()->setViewTo3D(false);
  HQRenderer::instance()->setAwaitMapclick(whois);
}
  
//! whois button
void Menu::whois(Vector2D click){
  msg.process(("whois "+toStr(click.x)+" "+toStr(click.y)).c_str());
}

//! shop button
void Menu::shop(){
  System::Engine::instance()->clearListeners();
  
  HQRenderer::instance()->setTrade(true);
  
  Button* but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Buy");
	but->setCbFunc(buy);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Sell");
	but->setCbFunc(sell);
  System::Engine::instance()->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("What is");
	but->setCbFunc(whatisShop);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 140));
  but->setText("Close");
	//p = &Renderer::mainMenu;
	but->setCbFunc(closeShop);
  System::Engine::instance()->addButtonListener(but);
}

//! close shop button
void Menu::closeShop(){
  System::Engine::instance()->clearListeners();
  HQRenderer::instance()->setTrade(false);
  plyr.getTrade()->deselect();
  
  Button* but = new Button();
	but->setPosition(Vector2D(900, 170));
	but->setText("Start");
	but->setCbFunc(Menu::start);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 130));
  but->setText("Shop");
  but->setCbFunc(Menu::shop);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 90));
  but->setText("Zargon");
  but->setCbFunc(Menu::zargon);
  System::Engine::instance()->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 50));
  but->setText("Create hero");
  but->setCbFunc(Menu::createHero);
  System::Engine::instance()->addButtonListener(but);
}

//whatis (shop) button
void Menu::whatisShop(){
  Item ite = plyr.getTrade()->getChosenItem();
  if (!ite.isValid()){
    line << "Please select an item first.";
  }
  else{
    plyr.getTrade()->deselect();
    msg.process(("whatis "+ite.getName()).c_str());
  }
}

void Menu::buy(){
  Item ite = plyr.getTrade()->getChosenItem();
  if (!ite.isValid()){
    line << "Please select an item first.";
  }
  else{
    plyr.getTrade()->deselect();
    msg.process(("buy "+ite.getName()).c_str());
  }
}

void Menu::sell(){
  Item ite = plyr.getTrade()->getChosenItem();
  if (!ite.isValid()){
    line << "Please select an item first.";
  }
  else{
    plyr.getTrade()->deselect();
    msg.process(("sell "+ite.getName()).c_str());
  }
}

void Menu::zargon(){
  msg.process("play zargon");
}

void Menu::drop(){
  Item* ite = HQRenderer::instance()->getInventory()->getChosenItem();
  if (ite == NULL){
    line << "Please select an item first.";
  }
  else{
    HQRenderer::instance()->getInventory()->deselect();
    msg.process(("drop "+ite->getName()).c_str());
  }
}

void Menu::pickup(){
  msg.process("pickup");
}

void Menu::createHero(){
  MessageBox* mb = new MessageBox();
  System::Engine::instance()->addButtonListener(mb);
  Font* fnt = System::Engine::instance()->getFont(1);
  fnt->setColor(1,1,1);
  
  fnt->print(120, 650, "Charakter class:", 1, HUGE_VAL);
  DropDownButton* cls = new DropDownButton();
  cls->setPosition(Vector2D(300, 650));
  cls->calcDDPos(2);
  cls->setCbFunc(Hero::createFromGui);
  cls->setName("class");
  vector<Hero>& heros = msg.getHeros();
  for (unsigned i = 0; i < heros.size(); i++){
    cls->addEntry(heros[i].getType());
  }
  System::Engine::instance()->addButtonListener(cls);
  
  fnt->print(120, 610, "Name:", 1, HUGE_VAL);
  InputField* in = new InputField();
  in->setPosition(Vector2D(180, 610));
  System::Engine::instance()->addInputListener(in);

}
