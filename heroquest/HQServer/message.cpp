/***************************************************************************
                          message.cpp  -  description
                             -------------------
    begin                : Mo Feb 14 2005
    copyright            : (C) 2005 by Marco Staginski
    email                : captain@captain-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sstream>
#include <streambuf>
#include <iostream>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
//#include "SDL.h"
//#include "SDL_thread.h"
#include "system/thread.h"

#include "../HQClient/common.h"
#include "../HQClient/opcodes.h"
#include "../HQClient/world.h"
#include "../HQClient/script.h"
#include "../HQClient/templates.h"
#include "admin.h"
#include "gamestate.h"
#include "message.h"

using std::istringstream;
using std::cout;
using std::cerr;
using std::endl;

// a mutex to lock data structures for save multithread access
System::Mutex gamtex;

Message::Message(){
  //gamtex = SDL_CreateMutex();
}

Message::~Message(){
  //SDL_DestroyMutex(gamtex);
}

void Message::process(ServerSocket* ss, const string& cmd){
  istringstream temp(cmd.c_str());
  vector<string> argv;
  string message;
  int opcode;

  temp >> opcode;
  //cerr << cmd << endl;;
  /*
  while(temp.good()){
    string tmp;
    temp >> tmp;
    args += tmp + " ";
  } 
  */
  string t;
  while(temp >> t){
    argv.push_back(t);
  }
  //a dummy to prevent segfaults
  argv.push_back(string());
  
  switch(opcode){
    case QUIT:
      globl.end(ss);
      break;
      
    case CHAT:
      message = toStr(CHAT)+" "+globl.getPlayer(ss)->getName()+": ";
      message += cmd.substr(4);
      globl.chat(ss, message);
      break;
      
    case CHAT_PERSON: {
      message = toStr(CHAT)+" "+globl.getPlayer(ss)->getName()+":";
      for(unsigned i = 1; i < argv.size()-1; i++){
        message += " "+argv[i];
      }
      Player* pl = globl.getPlayer(argv[0]);
      if(pl)
        *pl << message;
      }
      break;
      
    case LOGIN:
      if (globl.accessGranted(argv[0], argv[1])){
        Player* pl = globl.getPlayer(ss);
        //user already logged in?
        if (globl.getPlayer(argv[0]))
          *ss << toStr(CHAT) + " " + argv[0] + " is already logged in.";
        else{
          *ss << toStr(LOGIN)+ " "+ toStr(pl->getStatus());
          pl->set_identity(argv[0], argv[1]);
          string toSend = toStr(CHAT) + argv[0] + " joined the server.";
          globl.chat(ss, toSend);
          //send queued messages to update state
          globl.update(ss);
        }
      }
      else{
        *ss << toStr(NAK);
      }
      break;
      
    case LIST:
      if (argv[0] == "players"){
        string ret = globl.listPlayers();
        *ss << toStr(LIST) + " " + ret;
      }
      else{
        *ss << toStr(NAK);
      }
      break;
      
    case CREATE:
      if (argv[0] == "game"){
        //SDL_mutexP(gamtex);
        gamtex.lock();
        //Race conditions???
        string levelFile = globl.getPath()+"levels/"+argv[1]+"/"+argv[2]+".hqm";
        if(!wrld.load(levelFile)){
          *ss << toStr(CHAT)+ " "+"File not found or wrong version";
          //SDL_mutexV(gamtex);
          gamtex.unlock();
          break;
        }
        //Level loaded correctly
        globl.broadcast(cmd);
        //SDL_mutexV(gamtex);
        gamtex.unlock();
        break;
      }
      else{
        *ss << toStr(NAK);
      }
      break;
      
    case PLAY:{
      if (argv[0] == "zargon"){
        if(globl.setZargon(ss)){
          string name = globl.getPlayer(ss)->getName();
          for (unsigned i = 0; i < wrld.getMonsters().size(); i++){
            wrld.getMonsters()[i]->setPlayer(name);
          }
          string msg = toStr(PLAY)+" "+argv[0]+" "+name;
          *ss << cmd;
          globl.chat(ss, toStr(CHAT)+" "+name +" plays the evil Zargon.");
          wait();
          globl.broadcast(msg);
        }
        else{
          *ss << toStr(NAK);
        }
        break;
      }
      Vector2D pos(toInt(argv[7]), toInt(argv[8]));
      Hero heroe(argv[0], argv[1], argv[2], toInt(argv[3]), toInt(argv[4]), toInt(argv[5]), toInt(argv[6]), toInt(argv[12]), toInt(argv[11]), toInt(argv[9]), pos, toInt(argv[14]));
      short posidx = toInt(argv[10]);
      Inventory* invent = heroe.getInventory();
      invent->fromString(argv[13]);
      //SDL_mutexP(gamtex);
      gamtex.lock();
      if (wrld.getStarts()[posidx] == Vector2D(-1,-1)){
        //SDL_mutexV(gamtex);
        gamtex.unlock();
        *ss << toStr(NAK);
        break;
      }
      wrld.addHero(heroe, posidx);
      globl.broadcast(cmd);
      //SDL_mutexV(gamtex);
      gamtex.unlock();
      break;
    }
      
    case START:{
      scr.levelInit();
      game.start();
      ServerSocket* actSs = game.getNextPlayer();
      if (actSs == NULL){
        *ss << toStr(CHAT)+ "Nobody plays a role. Game cannot run";
        break;
      }
      globl.broadcast(cmd);
      wait();
      globl.getPlayer(actSs)->setLastCreature(getCreature());
      *actSs << toStr(TURN)+" "+getCreature()->getName();
      globl.chat(actSs, toStr(CHAT)+" It's "+ globl.getPlayer(actSs)->getName()+ "'s turn.");
      break;
    }

    case END:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
        break;
      }
      if (argv[0] == "turn"){
        game.endTurn(ss);
      }
      break;
      
    case MOVE:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
        break;
      }
      if(game.hasMoved()){
        *ss << toStr(NAK);
        break;
      }
      {
      short idx = game.getActivePlayerIdx();
      string rolled;
      string name;
      short num;
      if(idx < wrld.getHeroSize()){
        num = wrld.getHeros()[idx].getMovement();
        name = wrld.getHeros()[idx].getName();
        bool dice = wrld.getHeros()[idx].diceToMove();
        short move = num;
        if (dice)
          rolled = game.die(num, true, NULL, NULL, NULL, &move);
        else{
          game.die(0, true);
          rolled = toStr(num);
          num = 1;
        }
        game.setMovement(move);
      }
      else{
        num = wrld.getMonsters()[idx-wrld.getHeroSize()]->getMovement();
        name = wrld.getMonsters()[idx-wrld.getHeroSize()]->getName();
        bool dice = wrld.getMonsters()[idx-wrld.getHeroSize()]->diceToMove();
        short move = num;
        if (dice)
          rolled = game.die(num, true, NULL, NULL, NULL, &move);
        else{
          game.die(0, true);
          rolled = toStr(num);
          num = 1;
        }
        game.setMovement(move);
      }
      message = toStr(MOVE)+" "+toStr(num)+" "+rolled;
      *ss << message;
      globl.chat(ss,toStr(CHAT)+" "+name+" rolled "+rolled);
      }
      break;
    case UP_DIR:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
        break;
      }
      {
      game.setMovement(game.getMovement()-1);
      Vector2D pos = getCreature()->getPosition();
      GameObject* o = wrld.getObject(pos);
      Vector2D newPos = Vector2D(pos.x,pos.y-1);
      wrld.setObject(o, newPos);
			scr.move(newPos);
      //check if script is activated
      Field& f = wrld.getField(newPos);
      if (f.script != NULL){
        if (scr.call(OnWalkAt, f.script))
          f.script = NULL;
      }
      message = toStr(UP_DIR)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(newPos.x)+" "+toStr(newPos.y);
      globl.broadcast(message);
      break;
      }
    case DOWN_DIR:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
        break;
      }
      {
      game.setMovement(game.getMovement()-1);
      Vector2D pos = getCreature()->getPosition();
      GameObject* o = wrld.getObject(pos);
      Vector2D newPos = Vector2D(pos.x,pos.y+1);
      wrld.setObject(o, newPos);
			scr.move(newPos);
      //check if script is activated
      Field& f = wrld.getField(newPos);
      if (f.script != NULL){
        if (scr.call(OnWalkAt, f.script))
          f.script = NULL;
      }

      message = toStr(DOWN_DIR)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(newPos.x)+" "+toStr(newPos.y);
      globl.broadcast(message);
      break;
      }
    case LEFT_DIR:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
        break;
      }
      {
      game.setMovement(game.getMovement()-1);
      Vector2D pos = getCreature()->getPosition();
      GameObject* o = wrld.getObject(pos);
      Vector2D newPos = Vector2D(pos.x-1,pos.y);
      wrld.setObject(o, newPos);
			scr.move(newPos);
      //check if script is activated
      Field& f = wrld.getField(newPos);
      if (f.script != NULL){
        if (scr.call(OnWalkAt, f.script))
          f.script = NULL;
      }
      message = toStr(LEFT_DIR)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(newPos.x)+" "+toStr(newPos.y);
      globl.broadcast(message);
      break;
      }
    case RIGHT_DIR:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
        break;
      }
      {
      game.setMovement(game.getMovement()-1);
      Vector2D pos = getCreature()->getPosition();
      GameObject* o = wrld.getObject(pos);
      Vector2D newPos = Vector2D(pos.x+1,pos.y);
      wrld.setObject(o, newPos);
			scr.move(newPos);
      //check if script is activated
      Field& f = wrld.getField(newPos);
      if (f.script != NULL){
        if (scr.call(OnWalkAt, f.script))
          f.script = NULL;
      }
      message = toStr(RIGHT_DIR)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(newPos.x)+" "+toStr(newPos.y);
      globl.broadcast(message);
      break;
      }

    case POSITION:{
      Vector2D pos = getCreature()->getPosition();
      GameObject* o = wrld.getObject(pos);
      Vector2D newPos = Vector2D(toInt(argv[0]), toInt(argv[1]));
      wrld.setObject(o, newPos);
			scr.move(newPos);
      message = toStr(POSITION)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(newPos.x)+" "+toStr(newPos.y);
      globl.broadcast(message);
      }
      break;
      
    case OPEN:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      {
      short idx = game.getActivePlayerIdx();
      if (idx >= wrld.getHeroSize()){
        *ss << toStr(CHAT)+" You, as monster, are not allowed to open doors";
        break;
      }
      Vector2D pos = wrld.getHeros()[idx].getPosition();
      Door* doo = wrld.getDoor(pos,(Direction)toInt(argv[0]));
      if (!doo){
        *ss << toStr(CHAT)+ " There is no door.";
	      break;
      }
      else{
	      //can't open secret doors
        if (doo->getId() > 0){
          *ss << toStr(CHAT)+ " There is no door.";
	        break;
	      }
      }
      //is a script on the door
      if (doo->getScript() != NULL){
        scr.call(OnOpen, doo->getScript(), pos);
      }
      doo->setActive(false);
      //Make visible what's behind the door
      if (pos == doo->getPosition())
        wrld.update_visibility(doo->getPosition2());
      else
        wrld.update_visibility(doo->getPosition());
      message = toStr(OPEN)+ " "+toStr(pos.x)+" "+toStr(pos.y)+" "+argv[0];
      globl.broadcast(message);
      }
      break;
    
    case ATTACK:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      if (game.isActionPerformed()){
        *ss << toStr(NO_ACTION);
        break;
      }
      {
      Vector2D target(toInt(argv[0]), toInt(argv[1]));
      game.setActionPlace(target);
      GameObject* o = wrld.getObject(target);
      if (o == NULL){
        *ss << toStr(CHAT)+ " There is nobody to attack.";
	      break;
      }
      Creature* toAttack = dynamic_cast<Creature*>(o);
      //there is a creature on the specified field
      if(toAttack){
	      //get your own position
        Vector2D pos = getCreature()->getPosition();
	      //is there a backpoiter set
	      if(getCreature()->getBackPointer()){
	        *ss << toStr(CHAT)+" You stay on no valid position.";
	        break;
	      }
        /*
	      //neighbouring?
	      if (pos.x == target.x){
	        if (pos.y + 1 != target.y && pos.y-1 != target.y){
	          *ss << toStr(CHAT)+" You are not close enough.";
	          break;
	        }
	      }
	      else if (pos.y == target.y){
	        if (pos.x + 1 != target.x && pos.x-1 != target.x){
	          *ss << toStr(CHAT)+" You are not close enough.";
	          break;
	        }
	      }
	      else{
	        *ss << toStr(CHAT)+" You are not close enough.";
	        break;
	      }
	      //wall in between
	      if (!wrld.isVisible(pos,target)){
	        *ss << toStr(CHAT)+" Your view is blocked by something.";
	        break;
	      }
        */
        short num = scr.useWeapon(pos, target);
        if (num == -1){
	        *ss << toStr(CHAT)+" Your weapon is not capable of such an attack.";
          break;
        }
        *ss << toStr(SCRIPT)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+argv[0]+" "+argv[1]+" 0 attack";
        wait();
        wait();
        //wait();
	      //everything is correct, throw attack dices
	      game.performAction();
	      *ss << toStr(MOVE)+" "+toStr(1)+" "+toStr(0);
        //wait();
        //wait();
        //short num = getCreature()->getAttack();
        string name = getCreature()->getName();
	      short skulls = 0;
        string rolled = game.die(num, false, &skulls);
	      game.setPendingDamage(skulls);
        message = cmd+" "+toStr(num)+" "+rolled;
	      Player* pl = globl.getPlayer(toAttack->getPlayer());
        ServerSocket* attacked = NULL;
        if (pl){
	        string msg = toStr(ATTACK)+" "+argv[0]+" "+argv[1]+" "+toStr(num)+" "+rolled;
          *pl << msg;
          attacked = pl->getSock();
        }
        globl.chat(attacked,toStr(CHAT)+" "+name+
			    " attacked "+toAttack->getName()+" and rolled "+rolled);
        }
      }
      break;
    
		case FIGHT:
			if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      {
      int attack = toInt(argv[0]);
      string attype = argv[1];
      int defend = toInt(argv[2]);
			string deftype = argv[3];
			Vector2D target(toInt(argv[4]), toInt(argv[5]));
      game.setActionPlace(target);
      GameObject* o = wrld.getObject(target);
      
			if (o == NULL){
        *ss << toStr(CHAT)+" There is nobody to attack.";
	      break;
      }
      Creature* toAttack = dynamic_cast<Creature*>(o);
      //there is a creature on the specified field
      if(toAttack){
        short attacked;
        if (attype == "normal")
          game.die(attack, false, &attacked);
        else if (attype == "damage")
          attacked = attack;
        
  		 	short defended;
  		 	if(deftype == "white")
	 		 		game.die(defend, false, NULL, NULL, &defended);
 		 		else if(deftype == "black")
 		 			game.die(defend, false, NULL, &defended);
 		 		else if(deftype == "skull")
 		 			game.die(defend, false, &defended);
 		 		else if(deftype == "normal"){
 		 			if(dynamic_cast<Hero*>(toAttack))
 		 				game.die(defend, false, NULL, NULL, &defended);
 		 			else
 		 				game.die(defend, false, NULL, &defended);
 		 		}
 		 						
 		 		short damage = attacked - defended;
        scr.attacking(target);
 		 		if (damage > 0){
 		 			toAttack->setBody(toAttack->getBody()-damage);
 		 			scr.hit(target);
 		 			//creature died.
 		 			if (toAttack->getBody() <= 0){
 		 				toAttack->setActive(false);
 		 				wrld.removeObject(toAttack->getPosition());
 		 			}
 		 			string msg = toStr(DAMAGE)+" "+toStr(target.x)+" "+toStr(target.y)+" "+toStr(damage);
	 				globl.broadcast(msg);
 	 			}
        else{
 		 			string msg = toStr(DAMAGE)+" "+toStr(target.x)+" "+toStr(target.y)+" "+toStr(0);
	 				globl.broadcast(msg);
        }
			}
      wait();
      wait();
      wait();
			}
      break;

    case DEFEND:{
		game.setPendingDamage(0);
		Vector2D pos = game.getActionPlace();
		GameObject* o = wrld.getObject(pos);
		Creature* creat = dynamic_cast<Creature*>(o);
		short num = creat->getDefence();
		short shields = 0;
		string rolled;
		Hero* h = dynamic_cast<Hero*>(creat);
		//a hero defends with white shields
		if (h)
			rolled = game.die(num, false, NULL, NULL, &shields);
		else
			rolled = game.die(num, false, NULL, &shields, NULL);
		*ss << toStr(CHAT)+" "+creat->getName()+", you defended with "+rolled;
		globl.chat(ss, toStr(CHAT)+" "+creat->getName()+" defended with "+rolled);
		short damage = toInt(argv[0]) - shields;
		scr.attacking(pos);
    if (damage > 0){
			creat->setBody(creat->getBody()-damage);
			scr.hit(pos);
			//creature died.
			if (creat->getBody() <= 0){
				creat->setActive(false);
				wrld.removeObject(creat->getPosition());
			}
			string msg = toStr(DAMAGE)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(damage);
			globl.broadcast(msg);
		}
    else{
			string msg = toStr(DAMAGE)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(0);
			globl.broadcast(msg);
    }
	}
	break;

    case SECRETDOOR:{
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      if (game.isActionPerformed()){
        *ss << toStr(NO_ACTION);
        break;
      }
      short idx = game.getActivePlayerIdx();
      if (idx >= wrld.getHeroSize()){
        *ss << toStr(CHAT)+" You, as monster, are not allowed to search for secret doors";
        break;
      }
      Vector2D pos = wrld.getHeros()[idx].getPosition();
      //get all visible fields
      if(wrld.isMonsterVisible(pos)){
	      *ss << toStr(CHAT)+" You can't search for secret doors with monsters being present.";
	      break;
      }

      vector<Field*> vf = wrld.getVisibleFields(pos);
      vector<Door*> secretDoors;
      //search visible fields for monsters and for secret doors
      for (unsigned i = 0; i < vf.size(); i++){
	      Vector2D pos = vf[i]->getPosition();
	      //no doors on the field
	      if (vf[i]->doorbits.getData() == 0)
		      continue;
	      Door* d;
	      d = wrld.getDoor(pos, TOP);
	      if (d){
	        if (d->getId() == 1){
	          secretDoors.push_back(d);
	        }
	      }
	      d = wrld.getDoor(pos, RIGHT);
	      if (d){
	        if (d->getId() == 1){
	          secretDoors.push_back(d);
	        }
	      }
	      d = wrld.getDoor(pos, BOTTOM);
	      if (d){
	        if (d->getId() == 1){
	          secretDoors.push_back(d);
	        }
	      }
	      d = wrld.getDoor(pos, LEFT);
	      if (d){
	        if (d->getId() == 1){
	          secretDoors.push_back(d);
	        }
	      }
      }
      //search for secret doors
      game.performAction();
      *ss << toStr(MOVE)+" "+toStr(1)+" "+toStr(0);
      if (secretDoors.size() == 0){
        *ss << toStr(CHAT)+"There are no secret doors.";
	      break;
      }
      string msg;
      msg += toStr(SECRETDOOR)+" "+toStr(secretDoors.size());
      for (unsigned i = 0; i < secretDoors.size(); i++){
        Door* d = secretDoors[i];
	      //make door "unsecret";
	      d->setId(0);
	      msg += " "+toStr(d->getPosition().x)+" "+toStr(d->getPosition().y)+" "+
		    toStr(d->getPosition2().x)+" "+toStr(d->getPosition2().y);
      }
      globl.broadcast(msg);

      }
      break;

	case SPELL:
	  if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
    }
    if (game.isActionPerformed()){
      *ss << toStr(NO_ACTION);
      break;
    }
    {
    Vector2D target(toInt(argv[0]), toInt(argv[1]));
    game.setActionPlace(target);
    GameObject* o = wrld.getObject(target);
    bool object = true;
		Creature* toWhich = NULL;
		if (o == NULL){
			object = false;
      //*ss << toStr(CHAT)+ " There is nobody to cast a spell on.";
	  	//break;
    }
		else{
    	toWhich = dynamic_cast<Creature*>(o);
		}
    //there is a creature on the specified field
		//get your own position
    Vector2D pos = getCreature()->getPosition();
		
		//is there a backpoiter set
		if(getCreature()->getBackPointer()){
			*ss << toStr(CHAT)+" You stay on no valid position.";
			break;
		}
	      
    string name = getCreature()->getName();
		
		//cast spell
		if (scr.spell(pos, target, toInt(argv[2]))){
			//everything is correct
			game.performAction();
			*ss << toStr(MOVE)+" "+toStr(1)+" "+toStr(0);
			string msg = toStr(SPELL)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+
				toStr(target.x)+" "+toStr(target.y)+" "+argv[2];
			globl.broadcast(msg);
		}
		//the spell went wrong
		else{
			if(object)
				*ss << toStr(CHAT)+" You cannot cast a spell on "+toWhich->getName();
			else
				*ss << toStr(CHAT)+" You cannot cast a spell on Nobody";
		}
	  }
    break;

  case USE_ITEM:{
    Vector2D target(toInt(argv[0]), toInt(argv[1]));
    GameObject* o = wrld.getObject(target);
    bool object = true;
		Creature* toWhich = NULL;
		if (o == NULL){
			object = false;
      //*ss << toStr(CHAT)+ " There is nobody to cast a spell on.";
	  	//break;
    }
		else{
    	toWhich = dynamic_cast<Creature*>(o);
		}
    //there is a creature on the specified field
		//get your own position
    Creature* c = globl.getPlayer(ss)->getLastCreature();
    if (c == NULL){
      *ss << toStr(CHAT)+" Wait until your first turn before using anything.";
      break;
    }
    Vector2D pos = c->getPosition();
    Inventory *inv = c->getInventory();
    
    if (!inv->useItem(argv[2],pos,target)){
      *ss << toStr(CHAT)+" You have no "+argv[2]+" that could be used in such a way";
      break;
    }
		string msg = toStr(USE_ITEM)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+
			toStr(target.x)+" "+toStr(target.y)+" "+argv[2];
		globl.broadcast(msg);
    
    }

    break;

    
    case TREASURE:{
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      if (game.isActionPerformed()){
        *ss << toStr(NO_ACTION);
        break;
      }
      Vector2D pos = getCreature()->getPosition();
      //on the corridor
      short roomId = wrld.getField(pos).getId();
      if (roomId == 0){
        *ss << toStr(CHAT)+" "+"You can only search for treasure when you are in a room.";
        break;
      }
      //is monster visible
      if(wrld.isMonsterVisible(pos)){
        *ss << toStr(CHAT)+" "+"You can only search for treasure when no monster is present.";
        break;
      }
      //already searched in this room
      Field* f = wrld.getRoom(roomId)[0];
      if (f->treasure){
        *ss << toStr(CHAT)+" "+"You have already searched for treasure in this room.";
        break;
      }
      f->treasure = true;
      
      game.performAction();
      *ss << toStr(MOVE)+" "+toStr(1)+" "+toStr(0);
      wait();
      //is a treasure script availabale?
      bool scripttreasure = false;
      for (unsigned i = 0; i < wrld.getRoom(roomId).size(); i++){
        Field* f = wrld.getRoom(roomId)[i];
        if (f->script != NULL){
          if(scr.call(OnTreasure, f->script, pos)){
            scripttreasure = true;
            string msg = toStr(SCRIPT)+" "+toStr(f->getPosition().x)+" "+toStr(f->getPosition().y)
              +" event "+toStr(OnTreasure)+ " "+toStr(pos.x)+" "+toStr(pos.y);
            globl.broadcast(msg);
            break;
          }
        }
      }
      //treasure card
      if (!scripttreasure){
        short idx = scr.getTreasureIdx();
        scr.treasure(pos, idx);
        string msg = toStr(TREASURE)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(idx);
        globl.broadcast(msg);
      }
    }
    break;
    
    case TAKE_OFF:{
      Creature* c = globl.getPlayer(ss)->getLastCreature();
      if (c == NULL){
        *ss << toStr(CHAT)+" Wait until your first turn before using anything.";
        break;
      }
      Vector2D pos = c->getPosition();
      Item it = Templates::instance()->getArmory(argv[0]);
      short idx = it.getId();
      scr.armoryOff(pos, idx);
      string msg = toStr(TAKE_OFF)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(idx);
      globl.broadcast(msg);
    }
    break;

    case SPAWN:{
      Creature* c = globl.getPlayer(ss)->getLastCreature();
      if (c == NULL){
        *ss << toStr(CHAT)+" Wait until your first turn before spawning anything.";
        break;
      }
      Vector2D pos = c->getPosition();
      Inventory* inv = c->getInventory();
      Item it = Templates::instance()->searchItem(argv[0]);
      if (it.getType() != Item::NoItem){
        inv->addItem(it);
        string msg = toStr(SPAWN)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+argv[0];
        globl.broadcast(msg);
      }
    }
    break;

    case TRAP:{
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      if (game.isActionPerformed()){
        *ss << toStr(NO_ACTION);
        break;
      }
      Vector2D pos = getCreature()->getPosition();
      //is monster visible
      if(wrld.isMonsterVisible(pos)){
        *ss << toStr(CHAT)+" "+"You can only search for traps when no monster is present.";
        break;
      }
      //search for traps in visible area
      game.performAction();
      *ss << toStr(MOVE)+" "+toStr(1)+" "+toStr(0);
      wait();
      vector<Field*> vf = wrld.getVisibleFields(pos);
      bool trapfound = false;
      string message = toStr(TRAP);
      for (unsigned i = 0; i < vf.size(); i++){
        Field* f = vf[i];
        if (f->script && f->script->isTrap){
          trapfound = true;
          f->overlay->find(true);
          message += " "+toStr(f->getPosition().x)+" "+toStr(f->getPosition().y);
        }
      }
      message += " end";
      if (trapfound){
        globl.broadcast(message);
      }
      else{
        *ss << toStr(CHAT)+" "+"There are no traps.";
      }
    }
    break;

    case DISARM:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      {
      short idx = game.getActivePlayerIdx();
      if (idx >= wrld.getHeroSize()){
        *ss << toStr(CHAT)+" You, as monster, are not allowed to disarm traps";
        break;
      }
      if (game.isActionPerformed()){
        *ss << toStr(NO_ACTION);
        break;
      }
      if (game.getMovement() < 1){
        *ss << toStr(CHAT)+" You need at least one field of movement left.";
        break;
      }
      Vector2D pos = wrld.getHeros()[idx].getPosition();
      //has the hero the ability to disarm traps
      Hero* h = &wrld.getHeros()[idx];
      short thrown;
      //dwarf
      if (h->getId() == 1){
        game.die(1, false, NULL, &thrown);
      }
      else{
        Item& it = h->getInventory()->getArmory("belt");
        if (it.isValid() && it.getId() == 0){
          //tool kit
          game.die(1, false, &thrown);
        }
        //not able to disarm traps
        else{
          *ss << toStr(CHAT) + " You are not capable of disarming traps. You need a tool kit to do so.";
          break;
        }
      }
      //go to field that should be disarmed
      Vector2D newpos;
      Direction d = (Direction)toInt(argv[0]);
      switch (d){
        case TOP:
          newpos = Vector2D(pos.x, pos.y-1);
          break;
        case RIGHT:
          newpos = Vector2D(pos.x+1, pos.y);
          break;
        case BOTTOM:
          newpos = Vector2D(pos.x, pos.y+1);
          break;
        case LEFT:
          newpos = Vector2D(pos.x-1, pos.y);
          break;
      }
      Field& f = wrld.getField(newpos);
      //is there a trap that is discovered, but unsprung
      if (!(f.script && f.script->isTrap && f.overlay->isFound())){
        *ss << toStr(CHAT) + " There is no trap that can be disarmed.";
        break;
      }
      game.performAction();
      //is it no furniture trap?
      bool noFurn = false;
      if (!f.object || (f.object && dynamic_cast<Furniture*>(f.object) == NULL)){
        wrld.setObject(wrld.getObject(pos), newpos);
        noFurn = true;
      }
      //if disarming failed, trap is sprung
      if (thrown > 0){
        cerr << "Disarm: NOOO!";
        if (noFurn)
          scr.call(f.script->event, f.script, newpos);
        else
          scr.call(f.script->event, f.script, pos);
        
        string message = toStr(SCRIPT)+" "+toStr(pos.x)+" "+toStr(pos.y)+
          " trap "+toStr(newpos.x)+" "+toStr(newpos.y);
        globl.broadcast(message);
      }
      else{
        //"Normal" trap scripts disappear
        if (f.script->script[0] < 'n'){
          f.script = NULL;
        }
        //furniture trap scripts become non-trap scripts
        else{
          f.script->script[0] = toupper(f.script->script[0]);
        }
        f.overlay->find(false);
        cerr << "Disarm: DONE";
        string message = toStr(DISARM) + " "+toStr(pos.x)+" "+toStr(pos.y)+" "
          +toStr(newpos.x)+" "+toStr(newpos.y);
        globl.broadcast(message);
      }
      }
      break;

    case JUMP:
      if (!game.isActivePlayer(ss)){
        *ss << toStr(NO_TURN);
	      break;
      }
      {
      short idx = game.getActivePlayerIdx();
      if (game.getMovement() < 2){
        *ss << toStr(CHAT)+" You need at least two fields of movement left.";
        break;
      }
      Vector2D pos;
      short thrown = 0;
      if (idx >= wrld.getHeroSize()){
        pos = wrld.getMonsters()[idx-wrld.getHeroSize()]->getPosition();
        thrown = 0;
      }
      else{
        pos = wrld.getHeros()[idx].getPosition();
        game.die(1, false, &thrown);
      }
      //go to field that should be disarmed
      Vector2D newpos;
      Direction d = (Direction)toInt(argv[0]);
      switch (d){
        case TOP:
          newpos = Vector2D(pos.x, pos.y-1);
          break;
        case RIGHT:
          newpos = Vector2D(pos.x+1, pos.y);
          break;
        case BOTTOM:
          newpos = Vector2D(pos.x, pos.y+1);
          break;
        case LEFT:
          newpos = Vector2D(pos.x-1, pos.y);
          break;
      }
      Field& f = wrld.getField(newpos);
      //is there a visible trap?
      if (!(f.script && f.script->isTrap) || !(f.overlay->getStatus() || f.overlay->isFound())){
        *ss << toStr(CHAT) + " There is no trap that can be jumped over.";
        break;
      }
      //is it a furniture trap?
      if (f.object && dynamic_cast<Furniture*>(f.object) != NULL){
        *ss << toStr(CHAT) + " You can't jump over furniture traps.";
        break;
      }
      
      wrld.setObject(wrld.getObject(pos), newpos);
      //pos = newpos;
      
      //if jumping failed, trap is sprung
      if (thrown > 0){
        cerr << "Jump: NOOO!";
        scr.call(f.script->event, f.script, pos);
        string message = toStr(SCRIPT)+" "+toStr(pos.x)+" "+toStr(pos.y)+
          " trap "+toStr(newpos.x)+" "+toStr(newpos.y);
        globl.broadcast(message);
      }
      else{
        //jump successful
        *ss << toStr(CHAT)+" You jumped successfully.";
        wait();
        cerr << "Jump: DONE";
        string message = toStr(JUMP) + " "+toStr(pos.x)+" "+toStr(pos.y)+" "
          +toStr(newpos.x)+" "+toStr(newpos.y);
        globl.broadcast(message);
      }
      }
      break;
      
    default:      
      *ss << toStr(NAK);
  }
}


// Get current Creature
Creature* Message::getCreature(){
  short idx = game.getActivePlayerIdx();
  if (idx < wrld.getHeroSize()){
    return &wrld.getHeros()[idx];
  }
  else{
    return wrld.getMonsters()[idx-wrld.getHeroSize()];
  }
}
