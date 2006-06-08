//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: script.hh                                                  |
//  ==================================================================

#ifndef SCRIPT_HH
#define SCRIPT_HH

extern "C"{
#include "../include/lua.h"
#include "../include/lauxlib.h"
#include "../include/lualib.h"
}
#include "math/vector.h"
#include "monster.h"
#include "world.h"
//#include "array.hh"

//! events
enum eventType{
  OnTreasure,
  OnWalkAt,
  OnOpen,
};

//! holds infos about lua scripts
struct ScriptInfo{
  char script[2];
  Vector2D pos;
  eventType event;
  bool isTrap;
};

//! Provides a simple interface to the LUA scripting language
/*! Functions to call LUA from C and C-Functions to call from LUA
 * are provided here
 */
class Script{
public:
	//! Constructor
	Script();
	//! Destructor
	~Script();
	//! init scripting environment
	void init();
	
	//C to LUA
  //! load level script file
  void loadLevelScript(string name);
	//! do a magical spell
	bool spell(Vector2D source, Vector2D target, short index);
  //! init treasure data structures
  void treasureInit();
  //! return the index of the searched treasure item
  short getTreasureIdx();
  //! search for treasure
  bool treasure(Vector2D pos, short index);
  //! use an item
  bool item(Vector2D source, Vector2D target, short index);
  //! use armory
  bool armory(Vector2D source, Vector2D target, short index);
  //! unuse armory
  void armoryOff(Vector2D source, short index);
  //! use weapon
  short useWeapon(Vector2D source, Vector2D target);
  //! itits level specific settings
  void levelInit();
	
  //Event handling
  //! if a creature is attacked
  void attacking(Vector2D target);
	//! if a creature is hit
	void hit(Vector2D target);
	//! if a turn is ended
	void endTurn(Vector2D target);
  //! if a turn is started
  void startTurn(Vector2D target);
	//! if a creature moves
	void move(Vector2D target);
  //! process all events that are specified in the level file
  void processEvents(const vector<World::scriptPos>& scripts);
  //! call a script event
  bool call(eventType e, ScriptInfo* si, Vector2D pos = Vector2D(-1,-1));
  //! add event
  ScriptInfo* addEvent(ScriptInfo* si);
  //! get number of events
  inline unsigned getNumEvents() {return (unsigned)scripts_.size();}
  //! return an event
  inline ScriptInfo* getEvent(unsigned i) {return scripts_[i];}
	
	//LUA to C
	
	//! get a property
	static int getCreatureProperty(lua_State* L);
	//! set a property
	static int setCreatureProperty(lua_State* L);
	//! is it visible
	static int isVisible(lua_State* L);
	//! write message to console
	static int console(lua_State* L);
  //! write message to output
  static int output(lua_State* L);
	//! enable wall collision
	static int enableWallCollision(lua_State* L);
	//! attack a creature
	static int attack(lua_State* L);
	//! is monster visible
	static int isMonsterVisible(lua_State* L);
	//! shows content of a room
	static int showRoom(lua_State* L);
  //! dice a dice
  static int dice(lua_State *L);
  //! enable hero/monster collision
  static int enableHeroMonsterCollision(lua_State* L);
  //! deletes one item from inventory
  static int deleteItem(lua_State* L);
  //! adds an item to the inventory
  static int addItem(lua_State* L);
  //! changes the amount of money
  static int changeMoney(lua_State* L);
  //! end the turn of a creature
  static int forceEndTurn(lua_State* L);
  //! set a monster to a position
  static int setMonster(lua_State* L);
  //! get a free field near to a certain position
  static int getAdjacentFreeField(lua_State* L);
  //! is a field free and visible from another field
  static bool isFreeAndVisible(const Vector2D& from, const Vector2D& to);
  //! wear armory
  static int wearArmory(lua_State* L);
  //! is armory already worn
  static int getArmory(lua_State* L);
  //! take off armory
  static int takeOff(lua_State* L);
  //! send (error) message to client
  static int error(lua_State* L);
  //! let a hero win the game
  static int winHero(lua_State* L);
  //! end the game
  static int endGame(lua_State* L);
  //! set a scriptInfo
  static int setScript(lua_State* L);
  //! allow another action
  static int allowAnotherAction(lua_State* L);
  //! set active status of overlays
  static int makeOverlayActive(lua_State* L);
  //! open a door
  static int openDoor(lua_State* L);
  //! get number of heros
  static int getNumberOfHeros(lua_State* L);
  //! remove creature from world
  static int removeCreature(lua_State* L);
  //! release backpointer
  static int releaseBackPointer(lua_State* L);
  //! set moves
  static int setMoves(lua_State* L);
  //! allow another treasure search
  static int allowSearchAgain(lua_State* L);
  //! move object
  static int moveObject(lua_State* L);
  //! get the names of the items in the inventory
  static int getItems(lua_State* L);
  //! is a creature at a position
  static int isCreatureAt(lua_State* L);
  
private:
	//! The stack for communication between LUA and C
	lua_State* L;
  //! The level dependent scripts
  vector<ScriptInfo*> scripts_;
};

extern Script scr;


#endif

