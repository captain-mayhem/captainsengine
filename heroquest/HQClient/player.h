//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: player.hh                                                  |
//  ==================================================================

#ifndef PLAYER_HH
#define PLAYER_HH

#include <string>
#include <vector>
#include "hero.h"
#include "clientsocket.h"

using std::string;
using std::vector;  

class Trade;

//! this class stores informations about the player
class Player {
public: 
  //! Constructor	
  Player();
  //! Copy Constructor
  Player(const Player& p);
  //! Destructor
  ~Player();
  //! login to server
  void login(const string& name);
  //! set player name
  inline void setName(const string& name){name_ = name;}
  //! get player name
  inline const string getName() const {return name_;}
  //! set the status of the player
  inline void setStatus(const char stat){status_=stat;}
  //! get player status
  inline const char getStatus() const {return status_;}
  //! set the creature the player currently controls
  inline void setActCreature(const string name) {activeCreature_ = name;}
  //! returns the name of the currently controlled creature
  inline string getActCreature() {return activeCreature_;}
  //! should be set to true if it's your turn
  inline void setTurn(bool your=true){turn_ = your;}
  //! returns if it's your turn
  inline bool yourTurn() {return turn_;}
  //! adds a hero from the console
  void addHero(const string& filename, const ClientSocket* ss);
  //! adds a hero from the GUI
  Hero* addHero(const string& filename);
  //! the player is Zargon
  void addZargon();
  //! reset zargon status
  inline void resetZargon() {zargon_ = false;}
  //! tells if the player is zargon
  bool isZargon() const;
  //! returns the currently controlled hero or NULL if none is controlled
  Creature* getCreature() const;
  //! load status
  void loadStatus();
  //! save status
  void saveStatus();
  //! get the trading system
  inline Trade* getTrade() {return trade_;}
private:
  //! player name
  string name_;
  //! player state
  char status_;
  //! is the player zargon
  bool zargon_;
  //! the name of the active creature
  string activeCreature_;
  //! is it your turn
  bool turn_;
  //! the last won package
  short wonPackage_;
  //! the last won level of the package
  short wonLevel_;
  //! the shop for trading items
  Trade* trade_;
};

extern Player plyr;

#endif
