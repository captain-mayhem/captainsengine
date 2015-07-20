/***************************************************************************
                          gamestate.cpp  -  description
                             -------------------
    begin                : Di Mär 22 2005
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

#include <stdlib.h>
#include <time.h>

#include "message.hh"
#include "../heroquest/world.hh"
#include "../heroquest/script.hh"
#include "../heroquest/templates.hh"
#include "../heroquest/opcodes.hh"
#include "admin.hh"
#include "gamestate.hh"

GameState::GameState(){
  cerr << "Server is running...\n";
  pendingDamage = 0;
}

GameState::~GameState(){
}

void GameState::init(){
  status = INIT;
  Templates::init();
  wrld.init();
  scr.init();
}

void GameState::start(){
  numPlayers = wrld.getHeroSize()+wrld.getMonsters().size();
  activePlayer = numPlayers; //last monster of zargon
  //init random number generator
  srand((unsigned int) time(NULL));
}

ServerSocket* GameState::getNextPlayer(){
  moves = 0;
  moved = false;
  actionPerformed = false;
  short count = 0;
  //run through all players
  short heroMax = wrld.getHeroSize();
  short monsterMax = wrld.getMonsters().size();
  numPlayers = heroMax+monsterMax-1;
  while(count <= numPlayers){
    activePlayer++;
    if (activePlayer > numPlayers){
      activePlayer = 0;
    }
    // zargon should be next
    if (activePlayer >= heroMax){
      //if no zargon plays
      //if (globl.getZargon() == NULL){
      //  count++;
      //  continue;
      //}
      string player = wrld.getMonsters()[activePlayer-heroMax]->getPlayer();
      Player* pl = globl.getPlayer(player);
      //no zargon player
      if (pl == NULL){
        count++;
        continue;
      }
      //is the monster not yet seen
      if (!wrld.getMonsters()[activePlayer-heroMax]->movable()){
        count++;
	      continue;
      }
      //is the monster already dead
      if (!wrld.getMonsters()[activePlayer-heroMax]->getStatus()){
        count++;
	      continue;
      }
      //return active=globl.getZargon();
      return active=pl->getSock();
    }
    else{
      //Is this Hero position unused?
      if (wrld.getStarts()[activePlayer] != Vector2D(-1,-1)){
        count++;
        continue;
      }
      //we found our next player
      string player = wrld.getHeros()[activePlayer].getPlayer();
      //is the hero already dead
      if (!wrld.getHeros()[activePlayer].getStatus()){
        count++;
	      continue;
      }
      Player* pl = globl.getPlayer(player);
      //if he disconnected in the meantime
      if (pl == NULL){
        count++;
        continue;
      }
      return active=pl->getSock();
    }
  }
  return active=NULL;
}

bool GameState::isActivePlayer(const ServerSocket* ss) const{
  return active == ss;
}

//roll dice
string GameState::die(const short num, const bool movementDice, short* s, short* bs, short* ws, short* m){
  string ret;
  //movement dice
  if (m)
    *m = 0;
  if (movementDice){
    for (short i = 0; i < num; i++){
      double number = ((double)rand() / RAND_MAX * 6.0)+1;
      *m += (short)number;
      ret += toStr((int)number)+" ";
    }
    return ret;
  }
  //attack/defend dice
  if (s)
    *s = 0;
  if (bs)
    *bs = 0;
  if (ws)
    *ws = 0;
  for (short i = 0; i < num; i++){
    double number = ((double)rand() / RAND_MAX * 6.0)+1;
    int result = (int)number;
    switch (result){
      //attack
      case 1:
      case 2:
      case 3:
        ret += "Skull ";
	if (s)
	  *s = *s+1;
        break;
      //good shield
      case 4:
      case 5:
        ret += "White_Shield ";
	if (ws)
	  *ws = *ws+1;
        break;
      //evil shield
      case 6:
        ret += "Black_Shield ";
	if (bs)
	  *bs = *bs+1;
        break;
    }
  }
  return ret;
}


void GameState::performAction(bool action){
  actionPerformed = action;
}


void GameState::end(){
  status = INIT;
  wrld.unload();
}


void GameState::endTurn(ServerSocket* ss){
  //is damage pending?
  if (game.getPendingDamage() != 0){
    Vector2D pos = game.getActionPlace();
    string msg = toStr(DAMAGE)+" "+toStr(pos.x)+" "+toStr(pos.y)+" "+toStr(game.getPendingDamage());
    GameObject* o = wrld.getObject(pos);
    Creature* c = dynamic_cast<Creature*>(o);
    c->setBody(c->getBody()-game.getPendingDamage());
    if (c->getBody() <= 0){
      c->setActive(false);
      wrld.removeObject(c->getPosition());
    }
    game.setPendingDamage(0);
    globl.broadcast(msg);
    wait();
  }
	//scr.endTurn(game.getActionPlace());
  short oldidx = game.getActivePlayerIdx();
  Vector2D pos;
  if (oldidx < wrld.getHeroSize()){
    pos = wrld.getHeros()[oldidx].getPosition();
  }
  else{
    pos = wrld.getMonsters()[oldidx-wrld.getHeroSize()]->getPosition();
  }
  scr.endTurn(pos);
  ServerSocket* actSs = game.getNextPlayer();
  if (actSs == NULL){
    //*ss << toStr(CHAT)+ "Nobody plays a role. Game cannot run";
    game.end();
    globl.broadcast(toStr(END)+" game");
    return;
  }
  scr.startTurn(msg.getCreature()->getPosition());
  //is creature sleeping?
  if (msg.getCreature()->isSleeping()){
    string newop = toStr(END)+" turn";
    wait();
    msg.process(actSs, newop);
    return;
  }
  globl.getPlayer(actSs)->setLastCreature(msg.getCreature());
  *actSs << toStr(TURN)+" "+ msg.getCreature()->getName();
  globl.chat(actSs, toStr(CHAT)+" It's "+ globl.getPlayer(actSs)->getName()+ "'s turn.");
}

