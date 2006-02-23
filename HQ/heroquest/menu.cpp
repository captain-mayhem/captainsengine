#include "gui.hh"
#include "player.hh"
#include "world.hh"
#include "camera.hh"
#include "opcodes.hh"
#include "renderer.hh"

extern string path;

// setup main ingame menu
void Renderer::mainMenu(){
  clearListeners();
	void (Renderer::*p)();

	Button* but = new Button();
	but->setPosition(Vector2D(900, 240));
	but->setText("Attack");
	p = &Renderer::attack;
	but->setCbFunc(p);
	gl->addButtonListener(but);
    
  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Search");
	p = &Renderer::search;
	but->setCbFunc(p);
	gl->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Inventory");
	p = &Renderer::inventory;
	but->setCbFunc(p);
	gl->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 150));
  but->setText("Trap");
	p = &Renderer::trapMenu;
	but->setCbFunc(p);
	gl->addButtonListener(but);

	but = new Button();
	but->setPosition(Vector2D(900, 120));
	but->setText("Open Door");
	p = &Renderer::open;
	but->setCbFunc(p);
	gl->addButtonListener(but);

	but = new Button();
	but->setPosition(Vector2D(900, 90));
	but->setText("Move");
	p = &Renderer::move;
	but->setCbFunc(p);
	gl->addButtonListener(but);
    
	but = new Button();
	but->setPosition(Vector2D(900, 60));
	but->setText("Other");
	p = &Renderer::other;
	but->setCbFunc(p);
	gl->addButtonListener(but);
	
  but = new Button();
	but->setPosition(Vector2D(900, 30));
	but->setText("End turn");
	p = &Renderer::endTurn;
	but->setCbFunc(p);
	gl->addButtonListener(but);
}
    
//connect button
void Renderer::connect(){
  if (input_ != NULL){
    input_->removeChar();
    input_->end();
    input_ = NULL;
  }
  list<InputField*>::iterator iter;
  string send = "connect";
  for (iter = listeners_.begin(); iter != listeners_.end();iter++){
    send += " "+(*iter)->getText();
  }
  msg.process(send.c_str());
  //setup login-GUI after connection
  if (msg.isConnected()){
    clearListeners();
    
    output.glPrint(120, 450, "User name:", 1, HUGE_VAL);
    InputField* in = new InputField();
    in->setPosition(Vector2D(220, 450));
    in->setText(msg.getSetting(2));
    addInputListener(in);

    output.glPrint(120, 400, "Password:", 1, HUGE_VAL);
    InputField* in2 = new InputField();
    in2->setPosition(Vector2D(220,400));
    in2->setHidden();
    in2->setText(msg.getSetting(3));
    addInputListener(in2);

    Button* but = new Button();
    but->setPosition(Vector2D(220,300));
    but->setText("    Login");
    addButtonListener(but);
    void (Renderer::*p)();
    p = &Renderer::login;
    but->setCbFunc(p);
  }
}

//login button
void Renderer::login(){
  if (input_ != NULL){
    input_->removeChar();
    input_->end();
    input_ = NULL;
  }
  list<InputField*>::iterator iter;
  string send = toStr(LOGIN);
  for (iter = listeners_.begin(); iter != listeners_.end();iter++){
    if (iter == listeners_.begin())
      plyr.setName((*iter)->getText());
    send += " "+(*iter)->getText();
  }
  msg.send(send);
}

// package button
void Renderer::package(){
  ifstream in(string(path+"levels/levels.dat").c_str());
  int number;
  in >> number;
  if (number <=1 ){
    in.close();
    return;
  }
  list<Button*>::iterator but = buttons_.begin();
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
  Renderer::level();
}

// level button
void Renderer::level(){
  list<Button*>::iterator but = buttons_.begin();
  ifstream in(string(path+"levels/"+(*but)->getText()+"/levels.dat").c_str());
  if(!in.good()){
    (*++but)->setText("");
    return;
  }
  int number;
  in >> number;
  if (number <= 1){
    in.close();
    return;
  }
  but++;
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
}

// load level button
void Renderer::loadLevel(){
  list<Button*>::iterator iter;
  string proc = "create game";
  for (iter = buttons_.begin(); iter != buttons_.end();iter++){
    if (iter == buttons_.end())
      break;
    proc += " "+(*iter)->getText();
  }
  msg.process(proc.c_str());
}


//move button
void Renderer::move(){
  msg.process("move");
}

//end turn button
void Renderer::endTurn(){
  msg.process("end turn");
}

//start button
void Renderer::start(){
  msg.process("start");
}

//play button
void Renderer::play(){
  list<InputField*>::iterator iter = listeners_.begin();
  int position = -1;
  for (int i = 0; i < wrld.getHeroSize(); i++){
    if (cf_ == wrld.getStarts()[i]){
      position = i;
      break;
    }
  }
  if (position == -1)
    line << "Invalid starting position";
  //position is valid
  else {
    string file = path+"data/"+plyr.getName()+(*iter)->getText() +".sav";
    Hero* heroe = plyr.addHero(file);
    if (heroe != NULL)
      msg.special(toStr(position), PLAY, (void*)heroe);
  }

  //Reset GUI
  clearListeners();
  Button* but = new Button();
  but->setPosition(Vector2D(900,170));
  but->setText("Start");
  void (Renderer::*p)();
  p = &Renderer::start;
  but->setCbFunc(p);
  gl->addButtonListener(but);
  
  cf_ = Vector2D(-1,-1);
}

//open button
void Renderer::open(){
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
void Renderer::attack(){
  Vector2D pos = cam.modelPos();
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

//defend button
void Renderer::defend(){
  msg.process("defend");
  removeButtonListener(buttons_.size()-1);
}

// search menu
void Renderer::search(){
    clearListeners();
		void (Renderer::*p)();
    
    Button* but = new Button();
    but->setPosition(Vector2D(900, 240));
    but->setText("Secretdoor");
		p = &Renderer::secretdoor;
		but->setCbFunc(p);
		gl->addButtonListener(but);

    but = new Button();
    but->setPosition(Vector2D(900, 210));
    but->setText("Treasure");
		p = &Renderer::treasure;
		but->setCbFunc(p);
		gl->addButtonListener(but);
    
    but = new Button();
    but->setPosition(Vector2D(900, 180));
    but->setText("Trap");
		p = &Renderer::trap;
		but->setCbFunc(p);
		gl->addButtonListener(but);
    
    but = new Button();
    but->setPosition(Vector2D(900, 140));
    but->setText("Back");
		p = &Renderer::mainMenu;
		but->setCbFunc(p);
		gl->addButtonListener(but);
}

// search button
void Renderer::secretdoor(){
  msg.process("secretdoor");
  mainMenu();
}

// search button
void Renderer::treasure(){
  msg.process("treasure");
  mainMenu();
}
 
// search button
void Renderer::trap(){
  msg.process("trap");
  mainMenu();
}


// inventory button
void Renderer::inventory(){
  clearListeners();
	void (Renderer::*p)();
  Button* but;
  
  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Use");
	p = &Renderer::mainMenu;
	but->setCbFunc(p);
	gl->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Take off");
	p = &Renderer::mainMenu;
	but->setCbFunc(p);
	gl->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 170));
  but->setText("Back");
	p = &Renderer::mainMenu;
	but->setCbFunc(p);
	gl->addButtonListener(but);

  //get current inventory
}

// trap button
void Renderer::trapMenu(){
  clearListeners();
	void (Renderer::*p)();
  Button* but;

  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Search");
	p = &Renderer::trap;
	but->setCbFunc(p);
	gl->addButtonListener(but);
 
  but = new Button();
  but->setPosition(Vector2D(900, 210));
  but->setText("Disarm");
	p = &Renderer::disarm;
	but->setCbFunc(p);
	gl->addButtonListener(but);

  but = new Button();
  but->setPosition(Vector2D(900, 180));
  but->setText("Jump");
	p = &Renderer::trap;
	but->setCbFunc(p);
	gl->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 140));
  but->setText("Back");
	p = &Renderer::mainMenu;
	but->setCbFunc(p);
	gl->addButtonListener(but);
}

void Renderer::disarm(){
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

// other menu
void Renderer::other(){
  clearListeners();
	void (Renderer::*p)();
  Button* but;

  but = new Button();
  but->setPosition(Vector2D(900, 240));
  but->setText("Who is");
	p = &Renderer::whois;
	but->setCbFunc(p);
	gl->addButtonListener(but);
  
  but = new Button();
  but->setPosition(Vector2D(900, 200));
  but->setText("Back");
	p = &Renderer::mainMenu;
	but->setCbFunc(p);
	gl->addButtonListener(but);
}

// whois button
void Renderer::whois(){
  setViewTo3D(false);
  awaitMapClick_ = &Renderer::whois;
}
  
//! whois button
void Renderer::whois(Vector2D click){
  msg.process(("whois "+toStr(click.x)+" "+toStr(click.y)).c_str());
}
