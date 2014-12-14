//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: message.cpp                                                |
//  ==================================================================

#include <sstream>
#include <streambuf>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
//#include "SDL_thread.h"

#include "system/engine.h"
#include "common.h"
#include "opcodes.h"
#include "renderer.h"
#include "world.h"
#include "camera.h"
#include "player.h"
#include "gamestate.h"
#include "gui/console.h"
#include "gui/dropdown.h"
#include "clientsocket.h"
#include "socketexception.h"
#include "script.h"
#include "templates.h"
#include "menu.h"
#include "trade.h"
#include "message.h"
#include "mesh/mesh.h"
#include "mesh/model.h"

//! the separator
const char SEPARATOR = '\004';

using std::istringstream;
using std::ifstream;
using std::vector;
using std::cerr;
using std::cin;
using std::endl;

using Gui::Button;
using Gui::DropDownButton;

#define line *CGE::Engine::instance()->getFont(0)

extern string path;
extern string home;

TR_CHANNEL(HQ_Client_Message);

//Constructor: init all console commands
Message::Message(): ss_(0) {
	toDefend_ = false;
  cliToSrv_["quit"] = QUIT;
  cliToSrv_["exit"] = QUIT;
	cliToSrv_["connect"] = CONNECT;
	cliToSrv_[":"] = CHAT;
	cliToSrv_["login"] = LOGIN;
	cliToSrv_["create"] = CREATE;
	cliToSrv_["help"] = HELP;
	cliToSrv_["list"] = LIST;
	cliToSrv_["play"] = PLAY;
	cliToSrv_["start"] = START;
	cliToSrv_["end"] = END;
	cliToSrv_["move"] = MOVE;
	cliToSrv_["w"] = UP_DIR;
	cliToSrv_["s"] = DOWN_DIR;
	cliToSrv_["a"] = LEFT_DIR;
	cliToSrv_["d"] = RIGHT_DIR;
	cliToSrv_["open"] = OPEN;
	cliToSrv_["attack"] = ATTACK;
	cliToSrv_["defend"] = DEFEND;
	cliToSrv_["whois"] = WHOIS;
	cliToSrv_["secretdoor"] = SECRETDOOR;
	cliToSrv_["spell"] = SPELL;
  cliToSrv_["use"] = USE_ITEM;
  cliToSrv_["treasure"] = TREASURE;
  cliToSrv_["whatis"] = WHATIS;
  cliToSrv_["takeoff"] = TAKE_OFF;
  cliToSrv_["spawn"] = SPAWN;
  cliToSrv_["trap"] = TRAP;
  cliToSrv_["disarm"] = DISARM;
  cliToSrv_["jump"] = JUMP;
  cliToSrv_["dump"] = DUMP;
  cliToSrv_["replay"] = REPLAY;
  cliToSrv_["drop"] = DROP;
  cliToSrv_["buy"] = BUY;
  cliToSrv_["sell"] = SELL;
  cliToSrv_["pickup"] = PICKUP;
}

Message::Message(const Message& m){
	cliToSrv_ = map<string,int>(m.cliToSrv_);
	heros_ = vector<Hero>(m.heros_);
	ss_ = m.ss_;
	tid_ = m.tid_;
	settings_ = vector<string>(m.settings_);
}

//Destructor
Message::~Message(){
}

//quit game
void Message::quit(){
	if (ss_ != NULL){
		*ss_ << toStr(QUIT);
		//SDL_KillThread(tid_);
    tid_.destroy();
		delete ss_;
	}
	//gl->quit();
  CGE::Engine::instance()->shutdown();
}

//process the line that the user typed in
void Message::process_(const char* cmd){
	istringstream temp(cmd);
	char buffer[1024];
	int opcode;
	string toSend;
	vector<string> argv;
  
	//check if command is CHAT or something else
	char firstch = temp.peek();
	if (firstch == ':'){
		string what;
		temp >> what;
		//Chat to everybody
		if (what == ":"){
			opcode = CHAT;
			toSend = toStr(opcode);
		}
		//Chat to someone specific
		else{
			string arg = what.erase(0,1);
			argv.push_back(arg);
			opcode = CHAT_PERSON;
			toSend = toStr(opcode);
		}
	}
	else{
		//the command
		temp.get(buffer, 1024, ' ');
		//lower the characters
		for(unsigned i = 0; i < strlen(buffer); i++){
			buffer[i] = tolower(buffer[i]);
		}
		//determine opcode
		string op = string(buffer);
    if (op == "fuck"){
      consol << "fuck yourself, asshole!";
      return;
    }
		opcode = cliToSrv_[op];
		op.clear();
		toSend = toStr(opcode);
	}

	//store all other arguments in a string vector
	string t;
	while(!temp.fail()){
		temp >> t;
		argv.push_back(t);
	}
	//all arguments in one string are also sometimes needed
	string args;
	if (!argv.empty()){
		for (unsigned i = 0; i < argv.size()-1; i++){
			args += argv[i]+ " ";
		}
	}

	//now the big command switch
	switch(opcode){
	case QUIT:
		quit();
	break;

	case CONNECT:
		if (argv.size() < 2){
			consol << "Usage: connect <server name> <port>";
			break;
		}
		if (ss_){
			consol << "You are already connected";
			break;
		}
		try {
			//create socket
			string welcome;
			ss_ = new ClientSocket(argv[0], toInt(argv[1]));
			*ss_ >> welcome;
			consol << welcome;

			//create thread for recieving server messages
			//tid_ = SDL_CreateThread(receiver, (void*)ss_);
			tid_.create(receiver, (void*)ss_);
		}
		catch (SocketException& e){
			consol << "Error: " + e.description()  + "\n";
			line << "Error: " + e.description()  + "\n";
			delete ss_;
			ss_ = NULL;
		}
	break;
 
	case CHAT:
	case CHAT_PERSON:
		if (!ss_){
			consol << "Please connect to a server first";
			break;
		}
		toSend += " "+args;
		*ss_ << toSend;
	break;
      
    case LOGIN:
		if (!ss_){
			consol << "Please connect to a server first";
			break;
		}
		//login name given?
		if (argv.size()-1 >= 1){
			plyr.login(argv[0]);
		}
		else
			consol << "Usage: login <loginname>";
	break;
    
	// just for conveniece, some hints on available commands
	case HELP:
		//not connected
		if (ss_ == NULL){
			consol << "connect <server> <port>";
		}
		//not logged in
		else if ((short)plyr.getStatus() == 0){
			consol << ": <message>\n:<user> <message>\nlist <entity>\nlogin <user>\nquit";
		}
		//logged in
		else if ((short)plyr.getStatus() == 1 || (short)plyr.getStatus() == 2){
			//no game created
			if (game.getState() == INIT){
				consol << ": <message>\n:<user> <message>\ncreate game <package> <level>\ncreate hero\nlist <entity>\nquit";
			}
			//game created but not running
			else if (game.getState() == PREPARE){
				consol << ": <message>\n:<user> <message>\ncreate hero\nlist <entity>\nplay <character>\nstart\nquit";
			}
			//game is running
			else{
				consol << ": <message>\n:<user> <message>\ncreate hero\nend turn\nlist <entity>\nmove\nopen <direction>\nquit\nw,s,a,d (movement)";
			}
		}
	break;
      
  case LIST:
    if (!ss_){
      consol << "Please connect to a server first";
      break;
    }
    if (argv.size() < 2){
      consol << "Usage: list (players | inventory | shop)";
    }
    else if(toLower(argv[0]) == "players"){
      toSend += toLower(argv[0]);
      *ss_ << toSend;
    }else if(toLower(argv[0]) == "inventory"){
      consol<<"this is the inventory:";
      vector<Item> items = plyr.getCreature()->getInventory()->getItems();
      vector<Item>::iterator it = items.begin();
      it++;
      while( it != items.end() ) {
        consol<<toStr(it->getNumber())+" "+it->getName()+" "+it->getAdditional();
        it++;
      }
      // display money if it is a hero
      Hero* h = dynamic_cast<Hero*>(plyr.getCreature());
      if (h){
        consol<<"Money: "+toStr(h->getMoney());
      }
      // display worn items
      Inventory* inv = plyr.getCreature()->getInventory();
      Item ite = inv->getArmory("right hand");
      if (ite.isValid())
        consol << ite.getName()+" is held in the right hand.";
      ite = inv->getArmory("left hand");
      if (ite.isValid())
        consol << ite.getName()+" is held in the left hand.";
      ite = inv->getArmory("head");
      if (ite.isValid())
        consol << ite.getName()+" is worn on the head.";
      ite = inv->getArmory("body");
      if (ite.isValid())
        consol << ite.getName()+" is worn on the body.";
      ite = inv->getArmory("belt");
      if (ite.isValid())
        consol << ite.getName()+" is worn on the belt.";
      ite = inv->getArmory("breast");
      if (ite.isValid())
        consol << ite.getName()+" is worn on the breast.";
    }
    else if(toLower(argv[0]) == "shop"){
      if (plyr.getTrade() == NULL){
        consol<<"There is no shop available at the moment.";
        break;
      }
      consol<<"This can be bought at the shop:";
      vector<Item> items = plyr.getTrade()->getItems();
      vector<Item>::iterator it = items.begin();
      it++;
      while( it != items.end() ) {
        consol<<it->getName()+"   Price: "+it->getAdditional();
        it++;
      }
    }
  break;
      
	case CREATE:
		if (plyr.getStatus() == 0){
			consol << "You must log in first.";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: create (game <pack> <level> | hero)";
			break;
		}
		//create game <package> <level>
		if (toLower(argv[0]) == "game" && argv.size() >= 4 && game.getState() == INIT){
			toSend += " "+args;
			*ss_ << toSend;
		}
		else if (toLower(argv[0]) == "hero"){
			Hero::Create(path, heros_, plyr.getName());
		}
		else{
			consol << "Command is not used correctly.";
		}
	break;

	case PLAY:
		if(argv.size() < 2){
			consol << "Usage: play <character name>";
			break;
		}
		if (game.getState() != PREPARE){
			consol << "Game must be created, but may not be running to use this command.";
			break;
		}
		else{
			string name = argv[0];
			//playing the evil zargon
			if (toLower(name) == "zargon"){
				*ss_ << toStr(PLAY)+ " zargon";
			}
			//or a hero
			else{
				//string file = path+"data/"+plyr.getName()+argv[0]+".sav";
				string file = home+plyr.getName()+argv[0]+".sav";
				plyr.addHero(file, ss_);
			}
			break;
		}

	case START:
		if (game.getState() != PREPARE){
			consol << "Not allowed at the moment";
			break;
		}
		*ss_ << toStr(START);
	break;
    
	case END:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		//currently only end turn exists as command
		if (argv.size() < 2){
			consol << "Usage: end turn";
			break;
		}
		if (toLower(argv[0]) == "turn"){
			//creature must exist
			Creature* creat = plyr.getCreature();
			const Field* back = creat->getBackPointer();
			//move did not succeed?
			if (back != NULL){
        Vector3D move = Vector3D(back->getPosition()-creat->getPosition());
        cam.moveTo(move.magnitude()*QUADSIZE, move);
				*ss_ << toStr(POSITION) + " " + toStr(back->getPosition().x) + " "+ toStr(back->getPosition().y);
			}
			wait();
			game.setMoves(0);
      wrld.updateCollisionVertices(creat->getPosition());
			creat->setLookAt(cam.view());
			creat->setCamPos(cam.position());
			creat->setRot(cam.getCurrRotX());
			plyr.setTurn(false);
			*ss_ << toStr(END)+ " turn";
			scr.endTurn(creat->getPosition());
			break;
		}
		else{
			consol << "Wrong usage!";
			break;
		}
    
	case MOVE:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		*ss_ << toStr(MOVE);
	break;
    
	case UP_DIR:
	case DOWN_DIR:
	case LEFT_DIR:
	case RIGHT_DIR:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (game.getMoves() == 0){
			consol << "Not yet moved or no more moves left.";
			break;
		}
		*ss_ << toStr(opcode);
	break;

	case OPEN:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: open <direction>";
		}
		{
		Direction d=TOP;
		if (argv[0] == "w")
			d = TOP;
		else if (argv[0] == "s")
			d = BOTTOM;
		else if (argv[0] == "a")
			d = LEFT;
		else if (argv[0] == "d")
			d = RIGHT;
		else{
			consol << "wrong direction";
			break;
		}
		*ss_ << toStr(OPEN) + " " + toStr((int)d);
		}
	break;

	case ATTACK:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 3){
			consol << "Usage: Attack <x> <y>";
			break;
		}
		*ss_ << toStr(ATTACK)+" "+argv[0]+" "+argv[2];
	break;

	case DEFEND:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (game.getPendingDamage() == 0){
			consol << "You have nothing to defend";
			break;
		}
		*ss_ << toStr(DEFEND)+" "+toStr(game.getPendingDamage());
		game.setPendingDamage(0);
	break;

	case WHOIS:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 3){
			consol << "Usage: Whois <x> <y>";
			break;
		}
		{
		Vector2D pos(toInt(argv[0]), toInt(argv[1]));
    const Field& f = wrld.getField(pos);
    if (!f.getStatus() && !plyr.isZargon()){
			consol << "You can't examine unknown fields";
      line << "You can't examine unknown fields";
			break;
		}
		GameObject* o = wrld.getObject(pos);
		if (!o){
			consol << "There is no object";
      line << "There is no object";
			break;
		}
		Creature* creat = dynamic_cast<Creature*>(o);
		if (creat){
			string msg = "Name: "+creat->getName()+ "/"+creat->getPlayer()+"("+creat->getType()+"), Body Points: "+toStr(creat->getBody())+ ", Attack: "+toStr(creat->getAttack())+ ", Defense: "+toStr(creat->getDefence());
			consol << msg;
      line << msg;
			break;
		}
		}
	break;

	case SECRETDOOR:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		*ss_ << toStr(SECRETDOOR);
	break;

	case SPELL:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 4){
			consol << "Usage: spell <x> <y> <spellindex>";
			break;
		}
		*ss_ << toStr(SPELL)+" "+argv[0]+" "+argv[1]+" "+argv[2];
	break;

  case USE_ITEM:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: use <item> [<x> <y>]";
			break;
		}
    if (argv.size() < 4)
		  *ss_ << toStr(USE_ITEM)+" "+"0"+" "+"0"+" "+argv[0]+" "+plyr.getActCreature(); 
    else
		  *ss_ << toStr(USE_ITEM)+" "+argv[1]+" "+argv[2]+" "+argv[0]+" "+plyr.getActCreature();
    break;

  case TREASURE:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		*ss_ << toStr(TREASURE);
	  break;
    
  case WHATIS:
  {
    if (argv.size() < 2){
      consol << "Usage: whatis <item>";
      break;
    }
    Item it = Templates::instance()->searchItem(argv[0]);
    if(it.getType() == Item::NoItem)
      consol << "Sorry, unknown item "+argv[0];
    else{
      consol << it.getDescription();
      line << it.getDescription();
    }
    break;
  }

  case TAKE_OFF:
    if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
    }
    if (argv.size() < 2){
      consol << "Usage: takeoff <item>";
      break;
    }
    *ss_ << toStr(TAKE_OFF)+" "+argv[0];
    break;

  case SPAWN:
    if (game.getState() == INIT){
			consol << "Not allowed at the moment";
			break;
    }
    if (argv.size() < 3){
      consol << "Usage: spawn <item> <amount>";
      break;
    }
    *ss_ << toStr(SPAWN)+" "+argv[0]+" "+argv[1];
    break;

  case TRAP:
    if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
    }
    *ss_ << toStr(TRAP);
    break;
   
  case DISARM:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: disarm <direction>";
      break;
		}
		{
		Direction d=TOP;
		if (argv[0] == "w")
			d = TOP;
		else if (argv[0] == "s")
			d = BOTTOM;
		else if (argv[0] == "a")
			d = LEFT;
		else if (argv[0] == "d")
			d = RIGHT;
		else{
			consol << "wrong direction";
			break;
		}
		*ss_ << toStr(DISARM) + " " + toStr((int)d);
		}
	break;
 	
  case JUMP:
    if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: jump <direction>";
      break;
		}
		{
		Direction d=TOP;
		if (argv[0] == "w")
			d = TOP;
		else if (argv[0] == "s")
			d = BOTTOM;
		else if (argv[0] == "a")
			d = LEFT;
		else if (argv[0] == "d")
			d = RIGHT;
		else{
			consol << "wrong direction";
			break;
		}
		*ss_ << toStr(JUMP) + " " + toStr((int)d);
		}
	break;
  
  case DUMP:
		if (argv.size() < 2){
			consol << "Usage: dump <filename>";
      break;
		}
    *ss_ << toStr(DUMP) + " " + argv[0];
    break;

  case REPLAY:
		if (argv.size() < 2){
			consol << "Usage: replay <filename>";
      break;
		}
    *ss_ << toStr(REPLAY) + " " + argv[0];
    break;

  case DROP:
    if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: drop <item>";
      break;
		}
		*ss_ << toStr(DROP) + " " + argv[0];
    break;

  case BUY:
    if (game.getState() != PREPARE){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: buy <item>";
      break;
		}
    if (!plyr.getTrade()->getItem(argv[0]).isValid()){
      consol << "There exists no "+argv[0]+" in the shop";
      break;
    }
    *ss_ << toStr(BUY) + " " + argv[0];
    break;

  case SELL:
    if (game.getState() != PREPARE){
			consol << "Not allowed at the moment";
			break;
		}
		if (argv.size() < 2){
			consol << "Usage: sell <item>";
      break;
		}
    if (!plyr.getCreature()->getInventory()->getItem(argv[0]).isValid()){
      consol << "You possess no "+argv[0]+" that you could sell";
      break;
    }
    *ss_ << toStr(SELL) + " " + argv[0];

    break;

  case PICKUP:
		if (game.getState() != RUN){
			consol << "Not allowed at the moment";
			break;
		}
		*ss_ << toStr(PICKUP);
	  break;

	default:
		consol << "Unknown command. Please enter 'help' for available commands";
	}
}

//////////////////////////////////////////////////
//************************************************
//////////////////////////////////////////////////

//processing of the received server messages
void Message::process(){
  while (!messages_.empty()){
    string answer = messages_.front();
    messages_.pop();
	  istringstream temp(answer);
	  string args;
	  int opcode;

	  temp >> opcode;

	  //build argument vectors as above
	  vector<string> argv;
	  string t;
	  while(temp >> t){
		  argv.push_back(t);
	  }

	  for (unsigned i = 0; i < argv.size(); i++){
		  args += argv[i] + " ";
	  }
	
	  switch(opcode){
	  case CHAT:
		  consol << args;
		  line << args;
	  break;
    
	  case NAK:
		  consol << "Failed.";
		  line << "Failed.";
	  break;
    
	  case ACK:
		  consol << "OK.";
	  break;
      
	  case LOGIN:
		  plyr.setStatus(toInt(argv[0]));
      (line).setColor(1,1,1);
      plyr.loadStatus();
		  consol << "Logged in.";
		  line << "Logged in.";
      
		  //setup GUI to choose level
      CGE::Engine::instance()->getGuiMutex().lock();
      CGE::Engine::instance()->clearListeners(false);
		  //only the player with admin status can create games
		  if (plyr.getStatus() == 2){
			  (line).glPrint(120, 450, "Package:",1, (float)HUGE_VAL);
			  DropDownButton* but = new DropDownButton();
			  but->setPosition(Vector2D(200,450));
        but->calcDDPos(1);
			  but->setText("basic");
        but->setCbFunc(Menu::level);
        CGE::Engine::instance()->addGuiListener(but,false);

			  (line).glPrint(120, 400, "Level:", 1, (float)HUGE_VAL);
			  DropDownButton* but2 = new DropDownButton();
			  but2->setPosition(Vector2D(200,400));
        but2->calcDDPos(1);
        CGE::Engine::instance()->addGuiListener(but2,false);
        game.choosePackage(but, but2);

			  Button* but3 = new Button();
			  but3->setPosition(Vector2D(200,300));
			  but3->setText("     Load");
			  but3->setCbFunc(Menu::loadLevel);
        CGE::Engine::instance()->addGuiListener(but3,false);
		  }
      CGE::Engine::instance()->getGuiMutex().unlock();
	  break;
      
	  case LIST:
		  for (unsigned i = 0; i < argv.size(); i++){
			  consol << argv[i] + "\n";
		  }
	  break;
      
	  case CREATE:
		  //if not yet logged in, you will receive all this later
		  if (plyr.getStatus() == 0)
			  break;
		  if (argv[0] == "game"){
			  //load level
			  string levelFile = path+"levels/"+argv[1]+"/"+argv[2]+".hqm";
			  if(!wrld.load(levelFile)){
				  consol << "File not found or wrong version.";
				  break;
			  }
			  game.setState(PREPARE);
			  consol << argv[2] + " loaded.\n";
			  //yet logged in, so there exist choose-level-GUI-elements on
			  //the screen and they are no longer needed
			  if (plyr.getStatus() != 0){
          CGE::Engine::instance()->clearListeners(false);
			  }

        Button* but = new Button();
        but->setPosition(Vector2D(900, 90));
        but->setText("Zargon");
        but->setCbFunc(Menu::zargon);
        CGE::Engine::instance()->addGuiListener(but, false);
    
        but = new Button();
        but->setPosition(Vector2D(900, 50));
        but->setText("Create hero");
        but->setCbFunc(Menu::createHero);
        CGE::Engine::instance()->addGuiListener(but, false);
        
        scr.intro();
			  break;
		  }
	  break;
      
	  case PLAY:{
		  if (plyr.getStatus() == 0)
			  break;
  		    
		  if (argv[0] == "zargon"){
			  //you sended the command
			  if (argv.size() == 1){
				  //set zargon to the first monster
          plyr.addZargon();
          Vector2D pos = wrld.getMonsters()[0]->getPosition();
          Vector3D p = wrld.modelToRealPos(pos);
          cam.positionCamera(p, Vector3D(p.x+1,p.y,p.z), Vector3D(0,1,0));
          plyr.setActCreature(wrld.getMonsters()[0]->getName());
          wrld.updateCollisionVertices(pos);
  				
          consol << "You are Zargon.\n";
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
          but->setPosition(Vector2D(900, 50));
          but->setText("Create hero");
          but->setCbFunc(Menu::createHero);
          CGE::Engine::instance()->addGuiListener(but);
          
          break;
			  }
			  for (unsigned i = 0; i < wrld.getMonsters().size(); i++){
				  wrld.getMonsters()[i]->setPlayer(argv[1]);
			  }
			  break;
		  }
		  //add the hero in the world
		  Vector2D pos(toInt(argv[7]), toInt(argv[8]));
		  Hero heroe(argv[0], argv[1], argv[2], toInt(argv[3]), toInt(argv[4]), toInt(argv[5]), toInt(argv[6]), toInt(argv[12]), toInt(argv[11]), toInt(argv[9]), pos, toInt(argv[14]));
		  short posidx = toInt(argv[10]);
      Inventory* inv = heroe.getInventory();
      inv->fromString(argv[13]);
      
      //only if you added the creature, it should be the active one
      if (heroe.getPlayer() == plyr.getName()){
        //you play a character, so the button to start the game must appear
		    if (plyr.getActCreature() == "nobody"){
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
		    plyr.setActCreature(heroe.getName());
      }
		  wrld.addHero(heroe, posidx);
		  break;
	  }
  	      
	  case START:
		  game.setState(RUN);
      game.getRandomNumber();
		  consol << "Game started now.\n";
		  {
		  //game is running now, so buttons for ingame actions are needed
		  //void (Renderer::*p)();

		  Button* but = new Button();
		  but->setPosition(Vector2D(900, 240));
		  but->setText("Attack");
		  //p = &Renderer::attack;
		  but->setCbFunc(Menu::attack);
      CGE::Engine::instance()->addGuiListener(but, false);
        
      but = new Button();
      but->setPosition(Vector2D(900, 210));
      but->setText("Search");
		  //p = &Renderer::search;
		  but->setCbFunc(Menu::search);
      CGE::Engine::instance()->addGuiListener(but,false);
      
      but = new Button();
      but->setPosition(Vector2D(900, 180));
      but->setText("Inventory");
		  //p = &Renderer::inventory;
		  but->setCbFunc(Menu::inventory);
      CGE::Engine::instance()->addGuiListener(but,false);

      but = new Button();
      but->setPosition(Vector2D(900, 150));
      but->setText("Trap");
		  //p = &Renderer::trapMenu;
		  but->setCbFunc(Menu::trapMenu);
      CGE::Engine::instance()->addGuiListener(but,false);
   
		  but = new Button();
		  but->setPosition(Vector2D(900, 120));
		  but->setText("Open Door");
		  //p = &Renderer::open;
		  but->setCbFunc(Menu::open);
      CGE::Engine::instance()->addGuiListener(but,false);

		  but = new Button();
		  but->setPosition(Vector2D(900, 90));
		  but->setText("Move");
		  //p = &Renderer::move;
		  but->setCbFunc(Menu::move);
      CGE::Engine::instance()->addGuiListener(but,false);
        
		  but = new Button();
		  but->setPosition(Vector2D(900, 60));
		  but->setText("Other");
		  //p = &Renderer::other;
		  but->setCbFunc(Menu::other);
      CGE::Engine::instance()->addGuiListener(but,false);
  		
      but = new Button();
		  but->setPosition(Vector2D(900, 30));
		  but->setText("End turn");
		  //p = &Renderer::endTurn;
		  but->setCbFunc(Menu::endTurn);
      CGE::Engine::instance()->addGuiListener(but,false);
      
      //now run level init
      scr.levelInit();
	  }
	  break;
        
	  case NO_TURN:
		  consol << "Not possible, because it's not your turn.\n";
	  break;
      
	  case MOVE:{
		  //number of dices
		  short num = toInt(argv[0]);
		  string result;
		  short acc = 0;
		  //what you may move alltogether
		  for(short i = 1; i < num+1; i++){
			  result += argv[i]+" ";
			  acc += toInt(argv[i]);
		  }
		  game.setMoves(acc);
		  wrld.updateCollisionVertices(cam.modelPos());
		  if (acc != 0){
			  consol << "You rolled "+result+"\n";
			  line << "You rolled "+result+"\n";
		  }
	  }
	  break;
        
	  case POSITION:{
		  Vector2D pos(toInt(argv[0]), toInt(argv[1]));
		  Vector2D newPos(toInt(argv[2]), toInt(argv[3]));
		  GameObject* o = wrld.getObject(pos);
      //TODO when backpointer is set, the position call takes place AFTER the turn ended.
      //cam.moveTo has wrong effect
      cam.moveTo(Vector3D(newPos-pos).magnitude()*QUADSIZE, Vector3D(newPos-pos));
		  wrld.setObject(o, newPos);
		  scr.move(newPos);
	  }
	  break;
        
	  case UP_DIR:
	  case DOWN_DIR:
	  case LEFT_DIR:
	  case RIGHT_DIR:
	  {
	  //set hero one field further
		  Vector2D pos(toInt(argv[0]), toInt(argv[1]));
		  Vector2D newPos(toInt(argv[2]), toInt(argv[3]));
		  GameObject* o = wrld.getObject(pos);
		  if (game.getMoves() > 0)
			  game.setMoves(game.getMoves()-1);
		  bool possible = wrld.setObject(o, newPos,true);
		  if (!possible)
			  line << "You can't move there";
		  scr.move(newPos);
      //check if level dependent script is activated
      Field& f = wrld.getField(newPos);
      if (f.script != NULL){
        if (scr.call(OnWalkAt, f.script))
          f.script = NULL;
      }
	  }
	  break;
      
	  case TURN:
	  {
		  // your turn started
		  plyr.setActCreature(argv[0]);
		  plyr.setTurn();
		  Creature* c = plyr.getCreature();
		  if (c){
			  Vector2D creatPos = c->getPosition();
			  //update 3D camera if neccessary
			  if (creatPos != cam.modelPos()){
				  Vector3D pos = c->getCamPos();//wrld.modelToRealPos(creatPos);
				  //choose arbitrary viewing direction, in this case positive x-axis
				  cam.positionCamera(pos, c->getLookAt(), Vector3D(0,1,0));
				  cam.setCurrRotX(c->getRot());
				  wrld.updateCollisionVertices(c->getPosition());
			  }
        wrld.setObjectAnchor(c, creatPos);
        scr.startTurn(creatPos);
		  }
		  consol << argv[0]+ ", it's your turn.";
		  line << argv[0]+ ", it's your turn.";
	  break;
	  }

	  case OPEN:
	  {
		  Vector2D pos(toInt(argv[0]), toInt(argv[1]));
		  Direction dir = (Direction)toInt(argv[2]);
		  Door* doo = wrld.getDoor(pos,dir);
      //is a script on the door
      if (doo->getScript() != NULL){
        scr.call(OnOpen, doo->getScript(), pos);
      }
		  doo->setClosed(false);
		  //Update collition vertices if it's your turn
		  if (plyr.yourTurn()){
			  wrld.updateCollisionVertices(pos);
		  }
		  //Make visible what's behind the door
		  if (pos == doo->getPosition())
			  wrld.update_visibility(doo->getPosition2());
		  else
			  wrld.update_visibility(doo->getPosition());
	  }
	  break;

	  case ATTACK:
	  {
      //get creature to attack
		  Vector2D pos(toInt(argv[0]), toInt(argv[1]));
		  GameObject* o = wrld.getObject(pos);
		  Creature* creat = dynamic_cast<Creature*>(o);
		  int num = toInt(argv[2]);
		  string result = "";
		  int skull_count = 0;
      //cerr << num << " ";
		  for (int i = 0; i < num; i++){
			  result += argv[3+i] + " ";
			  if (argv[3+i] == "Skull"){
				  skull_count++;
			  }
		  }
		  consol << creat->getName()+", you were attacked by "+result;
		  line << creat->getName()+", you were attacked by "+result;
		  if (skull_count > 0){
			  game.setPendingDamage(skull_count);
          
			  Button* but = new Button();
			  but->setPosition(Vector2D(900, 270));
			  but->setText("Defend");

			  //void (Renderer::*p)();
			  //p = &Renderer::defend;
			  but->setCbFunc(Menu::defend);
        CGE::Engine::instance()->addGuiListener(but);
        toDefend_ = true;
		  }
	  }
	  break;

	  case NO_ACTION:
		  consol << "You have already done an action in this round.";
		  line << "You have already done an action in this round.";
	  break;

	  case DAMAGE:{
		  Vector2D pos(toInt(argv[0]), toInt(argv[1]));
		  GameObject* o = wrld.getObject(pos);
		  Creature* creat = dynamic_cast<Creature*>(o);
		  short damage = toInt(argv[2]);
      scr.attacking(pos);
      if (damage == 0)
        break;
		  creat->setBody(creat->getBody()-damage);
		  scr.hit(pos);
		  string msg = creat->getName()+ " suffers a damage of "+toStr(damage);
		  line << msg;
		  consol << msg;
		  //creature died
		  if (creat->getBody() <= 0){
			  creat->setActive(false);
			  wrld.removeObject(creat->getPosition());
        wrld.placeInventory(creat->getInventory(), creat->getPosition());
			  line << creat->getName()+" died.";
		  }
	  }
	  break;

	  case SECRETDOOR:{
		  int num = toInt(argv[0]);
		  for (int i = 0; i < num; i++){
			  Vector2D p1(toInt(argv[i*4+1]), toInt(argv[i*4+2]));
			  Vector2D p2(toInt(argv[i*4+3]), toInt(argv[i*4+4]));
			  Direction dir;
			  //horizontal door
			  if (p1.x == p2.x){
				  if (p1.y < p2.y)
					  dir = BOTTOM;
				  else
					  dir = TOP;
			  }
			  //vertical door
			  else{
				  if (p1.x < p2.x)
					  dir = RIGHT;
				  else
					  dir = LEFT;
			  }
			  Door* d = wrld.getDoor(p1, dir);
			  d->setType(0);
        //remove old door model
        unsigned id = d->getModel()->getID();
        //wrld.getScene().deleteModel(id);
        //set new door model
        MeshGeo::Mesh* msh = NULL;
        std::vector<MeshGeo::Mesh*> meshes = wrld.getScene().getMeshes();
        for (unsigned  i = 0; i < meshes.size(); ++i){
          if (meshes[i]->getFilename() == "models/world/door.obj"){
            msh = meshes[i];
            break;
          }
        }
        assert(msh);
        CGE::Texture* tex = NULL;
        std::vector<CGE::Texture*> textures = wrld.getScene().getTextures();
        for (unsigned i = 0; i < textures.size(); ++i){
          if (textures[i]->getFilename() == "textures/world/door.jpg"){
            tex = textures[i];
            break;
          }
        }
        assert(tex);
        MeshGeo::Model* mdl = new MeshGeo::Model(msh);
        mdl->setTrafo(wrld.getScene().getModel(id)->getTrafo());
        mdl->assignTexture(tex, 0);
        mdl->setAttrib(0, 1003);
        mdl->setAttrib(1, 0);
        d->setModel(mdl);
        //mdl->setID(id);
        //wrld.getScene().addModel(mdl);
		  }
	  }
	  break;

	  case SPELL:{
		  Vector2D pos(toInt(argv[0]), toInt(argv[1]));
		  Vector2D target(toInt(argv[2]), toInt(argv[3]));
		  short idx = toInt(argv[4]);
		  string from = dynamic_cast<Creature*>(wrld.getObject(pos))->getName();
		  scr.spell(pos, target, idx);
		  //string to = dynamic_cast<Creature*>(wrld.getObject(target))->getName();
		  string msg = from+" casted a spell to someone";
		  line << msg;
	  break;
	  }

    case USE_ITEM:{
		  Vector2D pos(toInt(argv[0]), toInt(argv[1]));
		  Vector2D target(toInt(argv[2]), toInt(argv[3]));
      Inventory* inv = dynamic_cast<Creature*>(wrld.getObject(pos))->getInventory();
		  string from = dynamic_cast<Creature*>(wrld.getObject(pos))->getName();
      inv->useItem(argv[4], pos, target);
      string msg = from+" used the item "+argv[4];
      line << msg;
      consol << msg;
    break;
    }

    case TREASURE:{
      Vector2D pos(toInt(argv[0]), toInt(argv[1]));
      short idx = (short)toInt(argv[2]);
      scr.treasure(pos, idx);
    break;
    }
    
    case TAKE_OFF:{
      Vector2D pos(toInt(argv[0]), toInt(argv[1]));
      short idx = (short)toInt(argv[2]);
      scr.armoryOff(pos, idx);
    break;
    }

    case SPAWN:{
      Vector2D pos(toInt(argv[0]), toInt(argv[1]));
      if (argv[2] == "money"){
        Hero* h = dynamic_cast<Hero*>(wrld.getObject(pos));
        h->changeMoney(toInt(argv[3]));
        break;
      }
      Inventory* inv = dynamic_cast<Creature*>(wrld.getObject(pos))->getInventory();
      Item it = Templates::instance()->searchItem(argv[2]);
      for (int i = 1; i < toInt(argv[3]); i++){
        it.increase();
      }
      inv->addItem(it);
    break;
    }

    case SCRIPT:{
      Vector2D src(toInt(argv[0]), toInt(argv[1]));
      if (argv[2] == "event"){
        eventType e = (eventType)toInt(argv[3]);
        ScriptInfo* si = NULL;
        Vector2D pos = Vector2D(-1,-1);
        switch (e){
          case OnTreasure:
            si = wrld.getField(src).script;
            pos.x = toInt(argv[4]);
            pos.y = toInt(argv[5]);
            break;
          case OnWalkAt:
          case OnOpen:
            break;
        }
        //cerr << si->pos.x << " " << si->pos.y << " " << pos.x << " " << pos.y << "\n";
        scr.call(e, si, pos);
        break;
      }
      else if (argv[2] == "trap"){
        Vector2D trg(toInt(argv[3]), toInt(argv[4]));
        Field& f = wrld.getField(trg);
        if (!f.object || (f.object && dynamic_cast<Furniture*>(f.object) == NULL)){
          cam.moveTo(QUADSIZE, Vector3D(trg-src));
          wrld.setObject(wrld.getObject(src), trg, true);
          src = trg;
        }
        scr.call(f.script->event, f.script, src);
        return;
      }
      Vector2D trg(toInt(argv[2]), toInt(argv[3]));
      //short idx = toInt(argv[4]);
      if (argv[5] == "attack")
        scr.useWeapon(src, trg);
        break;
    }

    case END:{
      game.end();
      break;
    }

    case TRAP:{
      int argc = 0;
      Vector2D pos;
      while (argv[argc] != "end"){
        pos.x = toInt(argv[argc++]);
        pos.y = toInt(argv[argc++]);
        Field& f = wrld.getField(pos);
        f.overlay->find(true);
        consol << "Trap found!";
      }
      break;
    }

    case DISARM:{
      Vector2D pos = Vector2D(toInt(argv[0]), toInt(argv[1]));
      Vector2D newpos = Vector2D(toInt(argv[2]), toInt(argv[3]));
      Field& f = wrld.getField(newpos);
      //no furniture trap
      if (!f.object || (f.object && dynamic_cast<Furniture*>(f.object) == NULL)){
        cam.moveTo(QUADSIZE, Vector3D(newpos-pos));
        wrld.setObject(wrld.getObject(pos), newpos, true);
      }
      if (f.script->script[0] < 'n')
        f.script = NULL;
      else
        f.script->script[0] = toupper(f.script->script[0]);
      f.overlay->find(false);
      line << "Trap disarmed!";
      break;
    }

    case JUMP:{
      Vector2D pos = Vector2D(toInt(argv[0]), toInt(argv[1]));
      Vector2D newpos = Vector2D(toInt(argv[2]), toInt(argv[3]));
      //Field& f = wrld.getField(newpos);
      cam.moveTo(QUADSIZE, Vector3D(newpos-pos));
      wrld.setObject(wrld.getObject(pos), newpos, true);
      break;
    }

    case DROP:{
      Vector2D pos(toInt(argv[0]), toInt(argv[1]));
      Inventory* inv = dynamic_cast<Creature*>(wrld.getObject(pos))->getInventory();
      Item it = inv->getItem(argv[2]);
      inv->deleteItemSavely(pos, argv[2]);
      it.reset();
      it.increase();
      //TODO This causes memory leak
      Inventory* newInv = new Inventory();
      newInv->addItem(it);
      wrld.placeInventory(newInv, pos);
    break;
    }

    case BUY:{
      Vector2D pos(toInt(argv[0]), toInt(argv[1]));
      Hero* h = dynamic_cast<Hero*>(wrld.getObject(pos));
      Item good = plyr.getTrade()->getItem(argv[2]);
      int price = toInt(good.getAdditional());
      h->changeMoney(-price);
      h->getInventory()->addItem(good);
    break;
    }

    case SELL:{
      Vector2D pos(toInt(argv[0]), toInt(argv[1]));
      Hero* h = dynamic_cast<Hero*>(wrld.getObject(pos));
      Inventory* inv = h->getInventory();
      Item good = inv->getItem(argv[2]);
      int price = toInt(good.getAdditional());
      h->changeMoney(price/2);
      inv->deleteItemSavely(pos, argv[2]);
    break;
    }

    case PICKUP:{
      Vector2D pos(toInt(argv[0]), toInt(argv[1]));
      Inventory* inv = dynamic_cast<Creature*>(wrld.getObject(pos))->getInventory();
      vector<Field*> vf = wrld.getVisibleFields(pos);
      string names = "";
      for (unsigned i = 0; i < vf.size(); i++){
        Field* f = vf[i];
        //is there an inventory?
        if (f->items){
          //merge inventories
          vector<Item> its = f->items->getItems();
          for (unsigned j = 1; j < its.size(); j++){
            Item& it = its[j];
            names += it.getName()+" ";
            inv->addItem(it);
          }
          //delete inventory on the ground
          wrld.placeInventory(NULL, f->getPosition());
        }
      }
      consol << dynamic_cast<Creature*>(wrld.getObject(pos))->getName()+" picked up "+names;
      line << dynamic_cast<Creature*>(wrld.getObject(pos))->getName()+" picked up "+names;
    break;
    }

    case RANDOM:{
      for (int i = 0; i < toInt(argv[0]); i++){
        game.addRandom(toInt(argv[i+1]));
      }
      break;
    }
  	
    }
  }
    
}

//init data
void Message::init(){
  //read heros file
  string file = path + "data/heros.dat";
  ifstream in(file.c_str());
  if (!in){
    cerr << "Could not load heros.dat";
  }
  string temp;
  while (in >> temp){
    Hero heroe;
    heroe.setType(temp);
    in >> temp; in >> temp; in >> temp;
    heroe.setId(toInt(temp));
    in >> temp; in >> temp;
    heroe.setAttack(toInt(temp));
    in >> temp; in >> temp;
    heroe.setDefence(toInt(temp));
    in >> temp; in >> temp;
    heroe.setBody(toInt(temp));
	  heroe.setMaxBody(toInt(temp));
    in >> temp; in >> temp;
    heroe.setMind(toInt(temp));
    heroe.setMaxMind(toInt(temp));
    in >> temp; in >> temp;
    heroe.setMovement(toInt(temp));
    in >> temp; in >> temp;
    heroe.setSpellClasses(toInt(temp));
    in >> temp; in >> temp;
    Inventory* inv = heroe.getInventory();
    Item weapon = Templates::instance()->getArmory(temp);
    inv->addItem(weapon);
    inv->wearArmory(temp, "right hand");
    heros_.push_back(heroe);
  }
  in.close();
  
  //read init settings
  file = path + "data/heroquest.ini";
  ifstream in2(file.c_str());
  if (!in2){
    cerr << "Could not load .ini file";
  }
  while (in2 >> temp){
    in2 >> temp;
    settings_.push_back(temp);
  }
  in2.close();

}

//receiving thread
void Message::receiver(void* v){
  ClientSocket* sock = (ClientSocket*)v;
  while(true){
    string answer;
    try{
      *sock >> answer;
      //separate messages
      int pos = 0;
      unsigned found;
      while ((found = answer.find(SEPARATOR,pos)) != string::npos){
        string tmp = answer.substr(pos,found-pos);
        pos = found+1;
        msg.messages_.push(tmp);
        //msg.process(tmp);
      }
    }
    catch (SocketException& e){
      TR_USE(HQ_Client_Message);
      TR_ERROR("Error: %s", e.description().c_str());
    }
  }
}

//special console command function
void Message::special_(const string& message, int mode, void* additional){
  string toSend;
  switch (mode){
    case LOGIN:
      //hidden input for password entering
      consol.setParams(Vector3D(1,1,1), CGE::Color(0,0,0,0), 0);
      toSend = toStr(LOGIN)+" "+plyr.getName() + " " + message;
      consol.hideLastInput();
      *ss_ << toSend;
    break;
    
    case CREATE:
      //first pass (called from Hero::Create)
      if (additional == NULL){
        short selection = toInt(message);
        Hero* h = new Hero();
        *h = heros_[selection];
        h->setId(selection);
        consol << "Enter the name of your character: ";
        void (*p)(const string&, int, void*);
        p = &Message::special;
        consol.setSpecialFunc(p, CREATE, (void*)h);
        break;
      }
      //second pass with a Hero as additional parameter
      else{
        Hero* h = (Hero*) additional;
        h->setName(message);
        h->setPlayer(plyr.getName());
        if (h->getSpellClasses() > 0){
          void (*p)(const string&, int, void*);
          p = &Message::special;
          consol.setSpecialFunc(p, SPELL, (void*)h);
          consol << "Enter the spell classes you want to obtain (e.g. \"fire water air\"):";
          break;
        }
        //string p = path + "data/"+plyr.getName()+h->getName()+".sav";
        string p = home +plyr.getName()+h->getName()+".sav";
        h->write(p);
        consol << h->getName()+ " successfully created.";
        delete h;
      }
      break;

    case SPELL:{
      Hero* h = (Hero*) additional;
      //string p = path + "data/"+plyr.getName()+h->getName()+".sav";
      string p = home + plyr.getName()+h->getName()+".sav";
      istringstream temp(message);
      string res;
      Inventory* inv = h->getInventory();
      short spellcount = 0;
      while(temp >> res){
        vector<Item> spells = Templates::instance()->getSpells(res);
        for (unsigned int i = 0; i < spells.size(); i++){
          inv->addItem(spells[i]);
        }
        spellcount++;
        if (spellcount >= h->getSpellClasses())
          break;
      }
      h->write(p);
      consol << h->getName()+" successfully created.";
      delete h;
      }
      break;
    
    case PLAY:
    {
      Hero* heroe = (Hero*) additional;
      short posidx = toInt(message);
      if (posidx >= wrld.getHeroSize()){
        consol << "Invalid number";
        break;
      }
      if (wrld.getStarts()[posidx] == Vector2D(-1,-1)){
        consol << "Already occupied";
        break;
      }
      //valid position
      Vector2D pos = wrld.getStarts()[posidx];
      heroe->setPosition(pos);

      string data = heroe->data(posidx);
      *ss_ << toStr(PLAY)+" "+data;
      delete heroe;
    }
    break;
  }
}

//simply send a message to the server
void Message::send(const string& text){
  assert(ss_);
  *ss_ << text;
}

//get an init setting
string& Message::getSetting(short number){
  return settings_[number];
}

