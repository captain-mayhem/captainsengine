//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: script.cpp                                                 |
//  ==================================================================

#include <ctype.h>

#include "common.h"
#include "world.h"
#include "script.h"
#ifdef _CLIENT_
#include "gui/console.h"
#include "gui/messagebox.h"
#include "message.h"
#include "player.h"
#include "gamestate.h"
#include "camera.h"
#else
#include "../HQServer/gamestate.h"
#include "../HQServer/message.h"
#include "../HQServer/player.h"
#include "../HQServer/admin.h"
#endif
#include "opcodes.h"
#include "templates.h"

#define line *System::Engine::instance()->getFont(0)

#ifdef _CLIENT_
using Gui::MessageBox;
#endif

Script::Script() : L(0) {
}

Script::~Script(){
	if (L)
		lua_close(L);
}


void Script::loadLevelScript(string name){
  unsigned length = name.size();
  name[length-3] = 'l';
  name[length-2] = 'u';
  name[length-1] = 'a';
	if (luaL_loadfile(L, name.c_str()) || lua_pcall(L,0,0,0)){
		cerr << lua_tostring(L, -1);
	}
}

void Script::init(){
	L = lua_open();
	luaopen_base(L);
	luaopen_table(L);
	luaopen_io(L);
	luaopen_string(L);
	luaopen_math(L);

	lua_pushcfunction(L, Script::console);
	lua_setglobal(L, "console");
	lua_pushcfunction(L, Script::enableWallCollision);
	lua_setglobal(L, "enableWallCollision");
	lua_pushcfunction(L, Script::getCreatureProperty);
	lua_setglobal(L, "getCreatureProperty");
	lua_pushcfunction(L, Script::isVisible);
	lua_setglobal(L, "isVisible");
	lua_pushcfunction(L, Script::setCreatureProperty);
	lua_setglobal(L, "setCreatureProperty");
	lua_pushcfunction(L, Script::attack);
	lua_setglobal(L, "attack");
	lua_pushcfunction(L, Script::isMonsterVisible);
	lua_setglobal(L, "isMonsterVisible");
	lua_pushcfunction(L, Script::showRoom);
	lua_setglobal(L, "showRoom");
  lua_pushcfunction(L, Script::dice);
  lua_setglobal(L, "dice");
  lua_pushcfunction(L, Script::enableHeroMonsterCollision);
  lua_setglobal(L, "enableHeroMonsterCollision");
  lua_pushcfunction(L, Script::deleteItem);
  lua_setglobal(L, "deleteItem");
  lua_pushcfunction(L, Script::addItem);
  lua_setglobal(L, "addItem");
  lua_pushcfunction(L, Script::changeMoney);
  lua_setglobal(L, "changeMoney");
  lua_pushcfunction(L, Script::output);
  lua_setglobal(L, "output");
  lua_pushcfunction(L, Script::forceEndTurn);
  lua_setglobal(L, "forceEndTurn");
  lua_pushcfunction(L, Script::setMonster);
  lua_setglobal(L, "setMonster");
  lua_pushcfunction(L, Script::getAdjacentFreeField);
  lua_setglobal(L, "getAdjacentFreeField");
  lua_pushcfunction(L, Script::wearArmory);
  lua_setglobal(L, "wearArmory");
  lua_pushcfunction(L, Script::getArmory);
  lua_setglobal(L, "getArmory");
  lua_pushcfunction(L, Script::takeOff);
  lua_setglobal(L, "takeOff");
  lua_pushcfunction(L, Script::error);
  lua_setglobal(L, "err");
  lua_pushcfunction(L, Script::winHero);
  lua_setglobal(L, "winHero");
  lua_pushcfunction(L, Script::endGame);
  lua_setglobal(L, "endGame");
  lua_pushcfunction(L, Script::setScript);
  lua_setglobal(L, "setScript");
  lua_pushcfunction(L, Script::allowAnotherAction);
  lua_setglobal(L, "allowAnotherAction");
  lua_pushcfunction(L, Script::makeOverlayActive);
  lua_setglobal(L, "makeOverlayActive");
  lua_pushcfunction(L, Script::openDoor);
  lua_setglobal(L, "openDoor");
  lua_pushcfunction(L, Script::getNumberOfHeros);
  lua_setglobal(L, "getNumberOfHeros");
  lua_pushcfunction(L, Script::removeCreature);
  lua_setglobal(L, "removeCreature");
  lua_pushcfunction(L, Script::removeObject);
  lua_setglobal(L, "removeObject");
  lua_pushcfunction(L, Script::releaseBackPointer);
  lua_setglobal(L, "releaseBackPointer");
  lua_pushcfunction(L, Script::setMoves);
  lua_setglobal(L, "setMoves");
  lua_pushcfunction(L, Script::allowSearchAgain);
  lua_setglobal(L, "allowSearchAgain");
  lua_pushcfunction(L, Script::moveObject);
  lua_setglobal(L, "moveObject");
  lua_pushcfunction(L, Script::getItems);
  lua_setglobal(L, "getItems");
  lua_pushcfunction(L, Script::isCreatureAt);
  lua_setglobal(L, "isCreatureAt");
  lua_pushcfunction(L, Script::exchangeModel);
  lua_setglobal(L, "exchangeModel");
  lua_pushcfunction(L, Script::addModel);
  lua_setglobal(L, "addModel");
  lua_pushcfunction(L, Script::moveCamera);
  lua_setglobal(L, "moveCamera");
  lua_pushcfunction(L, Script::messageBox);
  lua_setglobal(L, "messageBox");
  lua_pushcfunction(L, Script::getVisibleCreatures);
  lua_setglobal(L, "getVisibleCreatures");
	
	lua_pcall(L,0,0,0);

	if (luaL_loadfile(L, "scripts/events.lua") || lua_pcall(L,0,0,0)){
		cerr << lua_tostring(L, -1);
	}

	if (luaL_loadfile(L, "scripts/spells.lua") || lua_pcall(L,0,0,0)){
		cerr << lua_tostring(L, -1);
	}
  
  if (luaL_loadfile(L, "scripts/misc.lua") || lua_pcall(L,0,0,0)){
    cerr << lua_tostring(L, -1);
  }

  if (luaL_loadfile(L, "scripts/treasure.lua") || lua_pcall(L,0,0,0)){
    cerr << lua_tostring(L, -1);
  }
  treasureInit();
  if (luaL_loadfile(L, "scripts/items.lua") || lua_pcall(L,0,0,0)){
    cerr << lua_tostring(L, -1);
  }
  if (luaL_loadfile(L, "scripts/armory.lua") || lua_pcall(L,0,0,0)){
    cerr << lua_tostring(L, -1);
  }
}

bool Script::spell(Vector2D source, Vector2D target, short index){
	lua_getglobal(L, "spell");
	lua_pushnumber(L, source.x);
	lua_pushnumber(L, source.y);
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	lua_pushnumber(L, index);

	if (lua_pcall(L, 5, 1, 0) != 0){
		cerr << lua_tostring(L, -1);
		return false;
	}

	bool success = (lua_toboolean(L, -1) != 0) ? true : false;
	lua_pop(L, 1);

	return success;
}


bool Script::item(Vector2D source, Vector2D target, short index){
  lua_getglobal(L, "item");
  lua_pushnumber(L, source.x);
	lua_pushnumber(L, source.y);
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	lua_pushnumber(L, index);

	if (lua_pcall(L, 5, 1, 0) != 0){
		cerr << lua_tostring(L, -1);
		return false;
	}

	bool success = (lua_toboolean(L, -1) != 0) ? true : false;
	lua_pop(L, 1);

	return success;
}
  
bool Script::armory(Vector2D source, Vector2D target, short index){
  lua_getglobal(L, "armory");
  lua_pushnumber(L, source.x);
	lua_pushnumber(L, source.y);
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	lua_pushnumber(L, index);
	if (lua_pcall(L, 5, 1, 0) != 0){
		cerr << lua_tostring(L, -1);
		return false;
	}
	bool success = (lua_toboolean(L, -1) != 0) ? true : false;
	lua_pop(L, 1);
	return success;
}

void Script::armoryOff(Vector2D source, short index){
  lua_getglobal(L, "armoryOff");
  lua_pushnumber(L, source.x);
  lua_pushnumber(L, source.y);
  lua_pushnumber(L, index);
  if (lua_pcall(L, 3, 0, 0) != 0){
    cerr << lua_tostring(L, -1);
  }
}


// use weapon
short Script::useWeapon(Vector2D source, Vector2D target){ 
  Inventory* inv = dynamic_cast<Creature*>(wrld.getObject(source))->getInventory();
  Item it = inv->getArmory("right hand");
  short index = it.getId();
  lua_getglobal(L, "use_weapon");
  lua_pushnumber(L, source.x);
  lua_pushnumber(L, source.y);
  lua_pushnumber(L, target.x);
  lua_pushnumber(L, target.y);
  lua_pushnumber(L, index);
  if (lua_pcall(L, 5, 1, 0) != 0){
    cerr << lua_tostring(L, -1);
  }
  short attack = (short)luaL_checknumber(L, -1);
  lua_pop(L, 1);
  return attack;
}

//! init treasure data structures
void Script::treasureInit(){
  lua_getglobal(L, "treasureInit");
  if (lua_pcall(L, 0, 0, 0) != 0){
    cerr << lua_tostring(L, -1);
  }
}
  
//! return the index of the searched treasure item
short Script::getTreasureIdx(){
  lua_getglobal(L, "getTreasureIdx");
  if (lua_pcall(L, 0, 1, 0) != 0){
    cerr << lua_tostring(L, -1);
    return -1;
  }
  short idx = (short)luaL_checknumber(L, -1);
  lua_pop(L, 1);
  return idx;
}

bool Script::treasure(Vector2D pos, short index){
  lua_getglobal(L, "treasure");
  lua_pushnumber(L, pos.x);
  lua_pushnumber(L, pos.y);
  lua_pushnumber(L, index);

  if (lua_pcall(L, 3, 1, 0) != 0){
    cerr << lua_tostring(L, -1);
    return false;
  }

  bool success = (lua_toboolean(L, -1) != 0) ? true : false;
  lua_pop(L, 1);

  return success;
}

void Script::attacking(Vector2D target){
	lua_getglobal(L, "attacking");
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);

	if (lua_pcall(L, 2, 0, 0) != 0){
		cerr << lua_tostring(L, -1);
	}
}

void Script::hit(Vector2D target){
	lua_getglobal(L, "hit");
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);

	if (lua_pcall(L, 2, 0, 0) != 0){
		cerr << lua_tostring(L, -1);
	}
}

void Script::endTurn(Vector2D target){
	lua_getglobal(L, "endTurn");
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	if (lua_pcall(L, 2, 0, 0) != 0){
		cerr << lua_tostring(L, -1);
	}
}

void Script::startTurn(Vector2D target){
  lua_getglobal(L, "startTurn");
  lua_pushnumber(L, target.x);
  lua_pushnumber(L, target.y);
  if (lua_pcall(L, 2, 0, 0) != 0){
    cerr << lua_tostring(L, -1);
  }
}

void Script::move(Vector2D target){
	lua_getglobal(L, "move");
	lua_pushnumber(L, target.x);
	lua_pushnumber(L, target.y);
	if (lua_pcall(L, 2, 0, 0) != 0){
		cerr << lua_tostring(L, -1);
	}
}


// itits level specific settings
void Script::levelInit(){
	lua_getglobal(L, "levelInit");
	if (lua_pcall(L, 0, 0, 0) != 0){
		cerr << lua_tostring(L, -1);
	}
}


//call intro
void Script::intro(){
  lua_getglobal(L, "intro");
	if (lua_pcall(L, 0, 0, 0) != 0){
		cerr << lua_tostring(L, -1);
	}
}

// process all events that are specified in the level file
void Script::processEvents(const vector<World::scriptPos>& scripts){
  //vector<World::scriptPos>::iterator it = scripts.begin();
  //while (it != scripts.end()){
  for (unsigned i = 0; i < scripts.size(); i++){
    World::scriptPos sp = scripts[i];
    ScriptInfo* si = new ScriptInfo;
    //a trap?
    if (islower(sp.script[0])){
      if (sp.script[0] < 'n')
        lua_getglobal(L, "trapScheduler");
      else
        lua_getglobal(L, "eventScheduler");
      si->isTrap = true;
    }
    else{
      lua_getglobal(L, "eventScheduler");
      si->isTrap = false;
    }
    lua_pushstring(L, sp.script);
	  if (lua_pcall(L, 1, 1, 0) != 0){
		  cerr << lua_tostring(L, -1);
	  }
    string event = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    si->pos = sp.pos;
    si->script[0] = sp.script[0];
    si->script[1] = sp.script[1];
    
    if (event == "OnInit"){
      lua_getglobal(L, sp.script);
	    lua_pushnumber(L, sp.pos.x);
	    lua_pushnumber(L, sp.pos.y);
	    if (lua_pcall(L, 2, 0, 0) != 0){
		    cerr << lua_tostring(L, -1);
	    }  
    }
    else if (event == "OnTreasure"){
      si->event = OnTreasure;
      scripts_.push_back(si);
      Field& f = wrld.getField(si->pos);
      f.script = scripts_.back();
    }
    else if (event == "OnWalkAt"){
      si->event = OnWalkAt;
      scripts_.push_back(si);
      Field& f = wrld.getField(si->pos);
      f.script = scripts_.back(); 
    }
    else if (event == "OnOpen"){
      si->event = OnOpen;
      scripts_.push_back(si);
      Door* d = wrld.getDoor(si->pos, TOP);
      if (d)
        d->setScript(scripts_.back());
      d = wrld.getDoor(si->pos, RIGHT);
      if (d)
        d->setScript(scripts_.back());
      d = wrld.getDoor(si->pos, BOTTOM);
      if (d)
        d->setScript(scripts_.back());
      d = wrld.getDoor(si->pos, LEFT);
      if (d)
        d->setScript(scripts_.back());
    }
    else{
      cerr << "Unknown script event: "+event;
    }
    
    //TODO
    /*
    lua_getglobal(L, sp.script);
	  lua_pushnumber(L, sp.pos.x);
	  lua_pushnumber(L, sp.pos.y);
	  if (lua_pcall(L, 2, 0, 0) != 0){
		  cerr << lua_tostring(L, -1);
	  }
    */
    //it++;
  }
}

//add script event
ScriptInfo* Script::addEvent(ScriptInfo* si){
  scripts_.push_back(si);
  return scripts_.back();
}


// call a script event
bool Script::call(eventType e, ScriptInfo* si, Vector2D pos){
  if (si->event != e)
    return false;
  lua_getglobal(L, si->script);
  if (pos == Vector2D(-1,-1)){
	  lua_pushnumber(L, si->pos.x);
	  lua_pushnumber(L, si->pos.y);
  }
  else{
	  lua_pushnumber(L, pos.x);
	  lua_pushnumber(L, pos.y);
  }
	if (lua_pcall(L, 2, 1, 0) != 0){
		cerr << lua_tostring(L, -1);
    return false;
	}
  if (lua_isnil(L, -1))
    return true;
  bool success = (bool)lua_toboolean(L, -1);
  lua_pop(L, 1);
  return success;
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


int Script::getCreatureProperty(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
	string what = string(luaL_checkstring(L, 3));
	GameObject* o = wrld.getObject(Vector2D(x,y));
	Creature* c = dynamic_cast<Creature*>(o);
	if (!c){
    cerr << "getCreatureProperty: No creature there\n";
		lua_pushnil(L);
	}
	else if (what == "body"){
		short bp = c->getBody();
		lua_pushnumber(L, bp);
	}
	else if (what == "maxbody"){
		short bp = c->getMaxBody();
		lua_pushnumber(L, bp);
	}
  else if (what == "mind"){
    short mp = c->getMind();
    lua_pushnumber(L, mp);
  }
  else if (what == "maxmind"){
    short mp = c->getMaxMind();
    lua_pushnumber(L, mp);
  }
	else if (what == "defence" || what == "defense"){
		short def = c->getDefence();
		lua_pushnumber(L, def);
	}
	else if (what == "name"){
		string name = c->getName();
		lua_pushstring(L, name.c_str());
	}
	else if (what == "player"){
		string plyr = c->getPlayer();
		lua_pushstring(L, plyr.c_str());
	}
	else if (what == "attack"){
		short att = c->getAttack();
		lua_pushnumber(L, att);
	}
	else if (what == "move"){
		short move = c->getMove();
		lua_pushnumber(L, move);
	}
  else if (what == "sleeping"){
    bool sleep = c->isSleeping();
    lua_pushboolean(L, sleep);
  }
  else if (what == "type"){
    string type = c->getType();
    lua_pushstring(L, type.c_str());
  }
  else if (what == "id"){
    short id = c->getId();
    lua_pushnumber(L, id);
  }
	else{
		lua_pushnil(L);
	}
	return 1;
}

int Script::setCreatureProperty(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
	string what = string(luaL_checkstring(L, 3));
	GameObject* o = wrld.getObject(Vector2D(x,y));
	Creature* c = dynamic_cast<Creature*>(o);
	if (!c){
    cerr << "setCreatureProperty: No creature there\n";
		lua_pushboolean(L, false);
		return 1;
	}
	else if (what == "body"){
		short points = (short)luaL_checknumber(L, 4);
		c->setBody((char)points);
		lua_pushboolean(L, true);
	}
	else if (what == "defence" || what == "defense"){
		short points = (short)luaL_checknumber(L, 4);
		c->setDefence((char)points);
		lua_pushboolean(L, true);
	}
	else if (what == "attack"){
		short points = (short)luaL_checknumber(L, 4);
		c->setAttack((char)points);
		lua_pushboolean(L, true);
	}
	else if (what == "move"){
		short move = (short)luaL_checknumber(L, 4);
		c->setMove((char)move);
		lua_pushboolean(L, true);
	}
  else if (what == "sleeping"){
    bool sleep = (bool)lua_toboolean(L, 4);
    c->setSleeping(sleep);
    lua_pushboolean(L, true);
  }
  else if (what == "name"){
    string name = string(luaL_checkstring(L, 4));
    c->setName(name);
    lua_pushboolean(L, true);
  }
  else if (what == "player"){
    string plyr = string(luaL_checkstring(L, 4));
    c->setPlayer( plyr);
    lua_pushboolean(L, true);
  }
  else if (what == "diceToMove"){
    bool dice = (bool)lua_toboolean(L, 4);
    c->setDiceToMove(dice);
    lua_pushboolean(L, true);
  }
  else if (what == "mind"){
		short points = (short)luaL_checknumber(L, 4);
		c->setMind((char)points);
		lua_pushboolean(L, true);
  }
  else if (what == "maxmind"){
		short points = (short)luaL_checknumber(L, 4);
		c->setMaxMind((char)points);
		lua_pushboolean(L, true);
  }
	else{
		lua_pushboolean(L,false);
	}
	return 1;
}

int Script::isVisible(lua_State* L){
	short sx = (short)luaL_checknumber(L, 1);
	short sy = (short)luaL_checknumber(L, 2);
	short tx = (short)luaL_checknumber(L, 3);
	short ty = (short)luaL_checknumber(L, 4);
	Vector2D pos(sx, sy);
	Vector2D pos2(tx, ty);
	const Field& f1 = wrld.getField(pos);
	const Field& f2 = wrld.getField(pos2);
	if (pos == pos2)
		lua_pushboolean(L, true);
	else if ((f1.getId() == f2.getId()) && (f1.getId() != 0))
		lua_pushboolean(L, true);
	else{
		bool vis = wrld.isVisible(pos, pos2);
		lua_pushboolean(L, vis);
	}
	return 1;
}

int Script::console(lua_State *L){
	string msg = string(luaL_checkstring(L, 1));
#ifdef _CLIENT_
	consol << msg;
#else
	cerr << msg;
#endif
	return 0;
}


int Script::output(lua_State* L){
#ifdef _CLIENT_
  string toone = string(luaL_checkstring(L, 1));
  string toall = string(luaL_checkstring(L, 2));
  if (plyr.yourTurn()){
    line << toone;
    consol << toone;
  }
  else{
    line << toall;
    consol << toall;
  }
#endif
  return 0;
}

int Script::enableWallCollision(lua_State *L){
	bool enable = (bool)lua_toboolean(L,1);
	wrld.enableWallCollision(enable);
	return 0;
}

int Script::attack(lua_State *L){
#ifdef _CLIENT_
	short attack = (short)luaL_checknumber(L, 1);
  string attype = string(luaL_checkstring(L, 2));
	short defend = (short)luaL_checknumber(L, 3);
	string deftype = string(luaL_checkstring(L, 4));
	short sx = (short)luaL_checknumber(L, 5);
	short sy = (short)luaL_checknumber(L, 6);
	short tx = (short)luaL_checknumber(L, 7);
	short ty = (short)luaL_checknumber(L, 8);
  //send only attack if you are the one that is attacking.
  if (plyr.getCreature()->getPosition() == Vector2D(sx,sy))
	  msg.send(toStr(FIGHT)+" "+toStr(attack)+" "+attype+" "+toStr(defend)+" "+deftype+" "+toStr(tx)+" "+toStr(ty));
#endif
	return 0;
}

int Script::isMonsterVisible(lua_State *L){
	Vector2D pos((short)luaL_checknumber(L,1), (short)luaL_checknumber(L,2));
	bool vis = wrld.isMonsterVisible(pos);
	lua_pushboolean(L, vis);
	return 1;
}

int Script::showRoom(lua_State *L){
	short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2);
	wrld.update_visibility(Vector2D(tx, ty));
	return 0;
}

int Script::dice(lua_State *L){
//#ifndef _CLIENT_
  string type = string(luaL_checkstring(L, 1));
  short number = (short)luaL_checknumber(L, 2);
  if (type == "normal"){
    for (int i = 0; i < number; i++){
      int result = game.getRandomNumber();
      lua_pushnumber(L, result);
    }
    return number;
  }
  return 0;
//#else
//  return 0;
//#endif
}

//! enable hero/monster collision
int Script::enableHeroMonsterCollision(lua_State* L){
	bool enable = (bool)lua_toboolean(L,1);
	wrld.setRespectClasses(enable);
	return 0;
}

//! deletes one item from inventory
int Script::deleteItem(lua_State* L){
	short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2);
  string name = string(luaL_checkstring(L, 3));
  Creature* creat = dynamic_cast<Creature*>(wrld.getObject(Vector2D(tx,ty)));
  Inventory* inv = creat->getInventory();
  bool done = inv->deleteItem(name);
  lua_pushboolean(L, done);
  return 1;
}

//add item
int Script::addItem(lua_State* L){
  short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2);
  string name = string(luaL_checkstring(L, 3));
  Item it = Templates::instance()->searchItem(name);
  Creature* creat = dynamic_cast<Creature*>(wrld.getObject(Vector2D(tx,ty)));
  Inventory* inv = creat->getInventory();
  inv->addItem(it);
  return 0;

}

// change money
int Script::changeMoney(lua_State* L){
	short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2); 
  int amount = (int)luaL_checknumber(L, 3);
  Hero* heroe = dynamic_cast<Hero*>(wrld.getObject(Vector2D(tx,ty)));
  if (!heroe){
    return 0;
  }
  heroe->changeMoney(amount);
  return 0;
}


//end the turn of a creature
int Script::forceEndTurn(lua_State* L){
#ifdef _CLIENT_
  wait();
  string m = "end turn";
  if (plyr.yourTurn())
    msg.process(m.c_str());
#endif
  return 0;
}


//create a monster
int Script::setMonster(lua_State* L){
	short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2); 
  string name = string(luaL_checkstring(L, 3));
  Monster m = Templates::instance()->getMonster(name);
  Monster* ptr = new Monster();
  *ptr = m;
  //TODO What should be done if no monsters are in the map?
  ptr->setPlayer(wrld.getMonsters()[0]->getPlayer());
  ptr->canMove();
  wrld.addMonster(ptr, Vector2D(tx,ty));
  return 0;
}


//get a free field near to a certain position
int Script::getAdjacentFreeField(lua_State* L){
  short sx = (short)luaL_checknumber(L, 1);
	short sy = (short)luaL_checknumber(L, 2);
	Vector2D pos(sx, sy);

  if (isFreeAndVisible(pos, Vector2D(sx+1, sy))){
    lua_pushboolean(L, true);
    lua_pushnumber(L, sx+1);
    lua_pushnumber(L, sy);
    return 3;
  }
  if (isFreeAndVisible(pos, Vector2D(sx-1, sy))){
    lua_pushboolean(L, true);
    lua_pushnumber(L, sx-1);
    lua_pushnumber(L, sy);
    return 3;
  }
  if (isFreeAndVisible(pos, Vector2D(sx, sy+1))){
    lua_pushboolean(L, true);
    lua_pushnumber(L, sx);
    lua_pushnumber(L, sy+1);
    return 3;
  }
  if (isFreeAndVisible(pos, Vector2D(sx, sy-1))){
    lua_pushboolean(L, true);
    lua_pushnumber(L, sx);
    lua_pushnumber(L, sy-1);
    return 3;
  }

  lua_pushboolean(L, false);
  if (isFreeAndVisible(pos, Vector2D(sx+1, sy+1))){
    lua_pushnumber(L, sx+1);
    lua_pushnumber(L, sy+1);
    return 3;
  }
  if (isFreeAndVisible(pos, Vector2D(sx+1, sy-1))){
    lua_pushnumber(L, sx+1);
    lua_pushnumber(L, sy-1);
    return 3;
  }
  if (isFreeAndVisible(pos, Vector2D(sx-1, sy+1))){
    lua_pushnumber(L, sx-1);
    lua_pushnumber(L, sy+1);
    return 3;
  }
  if (isFreeAndVisible(pos, Vector2D(sx-1, sy-1))){
    lua_pushnumber(L, sx-1);
    lua_pushnumber(L, sy-1);
    return 3;
  }
  lua_pushnumber(L, -1);
  lua_pushnumber(L, -1);
  return 3;
}

bool Script::isFreeAndVisible(const Vector2D& from, const Vector2D& to){
	const Field& f1 = wrld.getField(from);
	const Field& f2 = wrld.getField(to);
  GameObject* o = wrld.getObject(to);
  Vector2D size = wrld.getMapSize();
  if (o != NULL || to.x >= size.x || to.y >= size.y)
    return false;
	if (from == to)
		return true;
	else if ((f1.getId() == f2.getId()) && (f1.getId() != 0))
		return true;
	else{
		bool vis = wrld.isVisible(from, to);
		return vis;
	}
}


// wear armory
int Script::wearArmory(lua_State* L){
	short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2);
  string weapon = string(luaL_checkstring(L, 3));
  string where1 = string(luaL_checkstring(L, 4));
  string where2 = string(luaL_checkstring(L, 5));
  Creature* creat = dynamic_cast<Creature*>(wrld.getObject(Vector2D(tx,ty)));
  Inventory* inv = creat->getInventory();
  inv->wearArmory(weapon, where1, where2);
  return 0;
}


// is armory already worn
int Script::getArmory(lua_State* L){
	short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2);
  string weapon = string(luaL_checkstring(L, 3));
  Inventory* inv = dynamic_cast<Creature*>(wrld.getObject(Vector2D(tx,ty)))->getInventory();
  Item it = inv->getArmory(weapon);
  if (it.isValid())
    lua_pushnumber(L, it.getId());
  else
    lua_pushnumber(L, -1);
  return 1;
}

// take off armory
int Script::takeOff(lua_State* L){
	short tx = (short)luaL_checknumber(L, 1);
	short ty = (short)luaL_checknumber(L, 2);
  string weapon = string(luaL_checkstring(L, 3));
  Inventory* inv = dynamic_cast<Creature*>(wrld.getObject(Vector2D(tx,ty)))->getInventory();
  inv->removeArmory(weapon);
  return 0;
}


// send (error) message to client
int Script::error(lua_State* L){
#ifndef _CLIENT_
  wait();
  wait();
	short sx = (short)luaL_checknumber(L, 1);
	short sy = (short)luaL_checknumber(L, 2);
  string message = string(luaL_checkstring(L, 3));
  string plyr = dynamic_cast<Creature*>(wrld.getObject(Vector2D(sx,sy)))->getPlayer();
  ServerSocket* ss = globl.getPlayer(plyr)->getSock();
  *ss << toStr(CHAT)+" "+message;
  wait();
  wait();
#endif
  return 0;
}


// let a hero win the game
int Script::winHero(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Hero* h = dynamic_cast<Hero*>(wrld.getObject(Vector2D(x,y)));
  h->win();
  return 0;
}


// end the game
int Script::endGame(lua_State* L){
  game.end();
  return 0;
}


// set a scriptInfo
int Script::setScript(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  string name = string(luaL_checkstring(L, 3));
  for (unsigned i = 0; i < scr.getNumEvents(); i++){
    ScriptInfo* sip = scr.getEvent(i);
    if (string(sip->script) == name){
      ScriptInfo* si = new ScriptInfo;
      si->script[0] = name[0];
      si->script[1] = name[1];
      si->pos = Vector2D(x,y);
      si->event = sip->event;
      sip = scr.addEvent(si);
      Field& f = wrld.getField(sip->pos);
      f.script = sip;
      return 0;
    }
  }
  cerr << "set Script failed: Unknown "+name;
  return 0;
}


int Script::allowAnotherAction(lua_State* L){
#ifndef _CLIENT_
  string action = string(luaL_checkstring(L, 1));
  if (action == "attack")
    game.performAction(GameState::Attack, false);
  else if (action == "spell")
    game.performAction(GameState::Spell, false);
  else if (action == "generic")
    game.performAction(GameState::Generic, false);
  else
    cerr << "allowAnotherAction: Unknown action: " << action << "\n";
  game.performAction(GameState::Generic, false);
#endif
  return 0;
}


// set active status of overlays
int Script::makeOverlayActive(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  bool active = (bool)(lua_toboolean(L, 3));
  Field& f = wrld.getField(Vector2D(x,y));
  f.overlay->find(false);
  f.overlay->setActive(active);
  return 0;
}


// open a door
int Script::openDoor(lua_State* L){
  short x = (short)luaL_checknumber(L, 1);
  short y = (short)luaL_checknumber(L, 2);
  Vector2D pos = Vector2D(x,y);
  string direction = string(luaL_checkstring(L, 3));
  Direction dir = TOP;
  if (direction == "TOP")
    dir = TOP;
  else if (direction == "RIGHT")
    dir = RIGHT;
  else if (direction == "BOTTOM")
    dir = BOTTOM;
  else if (direction == "LEFT")
    dir = LEFT;
  else
    cerr << "openDoor: wrong direction";
  Door* d = wrld.getDoor(Vector2D(x,y), dir);
  if (d == NULL){
    cerr << "openDoor: No Door!";
    return 0;
  }
  d->setId(0);
  if(d->getScript() != NULL){
    scr.call(OnOpen, d->getScript(), Vector2D(x,y));
  }
  d->setActive(false);
  //update visibility
  wrld.update_visibility(d->getPosition());
  wrld.update_visibility(d->getPosition2());
  
  return 0;
}


//! get number of heros
int Script::getNumberOfHeros(lua_State* L){
  short num = 0;
  for (int i = 0; i < wrld.getHeroSize(); i++){
    Vector2D pos = wrld.getStarts()[i];
    if (pos == Vector2D(-1,-1))
      ++num;
  }
  lua_pushnumber(L, num);
  return 1;
}


// remove creature from world
int Script::removeCreature(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Vector2D pos = Vector2D(x, y);
  Creature* c = dynamic_cast<Creature*>(wrld.getObject(pos));
  if (!c){
    cerr << "removeCreature: There is no creature";
    return 0;
  }
  c->setActive(false);
  wrld.removeObject(pos);
  return 0;
}

// remove object from world
int Script::removeObject(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Vector2D pos = Vector2D(x, y);
  GameObject* o = wrld.getObject(pos);
  if (!o){
    cerr << "removeObject: There is no object";
    return 0;
  }
  wrld.removeObject(pos);
  return 0;
}


// release back pointer of the hero
int Script::releaseBackPointer(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Vector2D pos = Vector2D(x, y);
  Creature* c = dynamic_cast<Creature*>(wrld.getObject(pos));
  if (!c){
    cerr << "releaseBackPointer: There is no creature";
    return 0;
  }
  c->setBackPointer(NULL);
  return 0;
}


//! set moves
int Script::setMoves(lua_State* L){
  short value = (short)luaL_checknumber(L, 1);
#ifdef _CLIENT_
  game.setMoves(value);
  Vector2D pos = plyr.getCreature()->getPosition();
  wrld.updateCollisionVertices(pos);
#else
  game.setMovement(value);
#endif
  return 0;
}

//allow to search for treasure again
int Script::allowSearchAgain(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Vector2D pos = Vector2D(x, y);
  Field& f = wrld.getField(pos);
  Field* field = wrld.getRoom(f.getId())[0];
  field->treasure = false;
  return 0;
}

//move object
int Script::moveObject(lua_State* L){
	short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Vector2D src = Vector2D(x, y);
	x = (short)luaL_checknumber(L, 3);
	y = (short)luaL_checknumber(L, 4);
  Vector2D trg = Vector2D(x, y);
  bool interpolate = (bool)(lua_toboolean(L, 5));
  wrld.setObject(wrld.getObject(src), trg, interpolate);
  return 0;
}

//get item names
int Script::getItems(lua_State* L){
  short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Vector2D pos = Vector2D(x, y);
  Creature* c = dynamic_cast<Creature*>(wrld.getObject(pos));
  if (c){
    Inventory* inv = c->getInventory();
    vector<Item> items = inv->getItems();
    for (unsigned i = 0; i < items.size(); i++){
      string name = items[i].getName();
      lua_pushstring(L, name.c_str());
    }
    return items.size();
  }
  return 0;
}


// is a creature at a position
int Script::isCreatureAt(lua_State* L){
  short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Creature* c = dynamic_cast<Creature*>(wrld.getObject(Vector2D(x,y)));
  if (c)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);
  return 1;
}

// exchange model
int Script::exchangeModel(lua_State* L){
#ifdef _CLIENT_
  short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  int fieldidx = (int)luaL_checknumber(L, 3);
  int modelid = (int)luaL_checknumber(L, 4);
	Field& field = wrld.getField(Vector2D(x,y));
  Graphics::ModelInstance old = field.models[fieldidx];
  field.models[fieldidx] = Templates::instance()->getModel(modelid)->clone();
  field.models[fieldidx].setTransform(old.getTransform());
#endif
  return 0;
}


// add a model
int Script::addModel(lua_State* L){
#ifdef _CLIENT_
  short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  int modelid = (int)luaL_checknumber(L, 3);
	Field& field = wrld.getField(Vector2D(x,y));
  unsigned fieldidx = field.models.size();
  field.models.push_back(Templates::instance()->getModel(modelid)->clone());
  Matrix mat(Matrix::Translation, Vector3D(x*QUADSIZE+QUADSIZE/2, 0, y*QUADSIZE+QUADSIZE/2));
  field.models[fieldidx].setTransform(mat);
#endif
  return 0;
}

// move the camera
int Script::moveCamera(lua_State* L){
#ifdef _CLIENT_
  float x = (float)luaL_checknumber(L, 1);
  float y = (float)luaL_checknumber(L, 2);
  float z = (float)luaL_checknumber(L, 3);
  Vector3D to(x,y,z);
  cam.moveTo(to.magnitude(), to);
#endif
  return 0;
}

int Script::messageBox(lua_State* L){
#ifdef _CLIENT_
	string msg = string(luaL_checkstring(L, 1));
  MessageBox* mb = new MessageBox();
  mb->setMessage(msg);
  System::Engine::instance()->addButtonListener(mb, false);
#endif
  return 0;
}

// get visible creatures
int Script::getVisibleCreatures(lua_State* L){
  short x = (short)luaL_checknumber(L, 1);
	short y = (short)luaL_checknumber(L, 2);
  Vector2D pos = Vector2D(x, y);
  vector<Field*> vf = wrld.getVisibleFields(pos);
  int count = 0;
  for (unsigned i = 0; i < vf.size(); i++){
    Vector2D fp = vf[i]->getPosition();
    //a monster?
    Creature* c =dynamic_cast<Creature*>(wrld.getObject(fp));
    if (c){
      lua_pushnumber(L, fp.x);
      lua_pushnumber(L, fp.y);
      count += 2;
    }
  }
  return count;
}
