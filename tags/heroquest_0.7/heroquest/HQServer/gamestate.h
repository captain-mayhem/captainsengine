/***************************************************************************
                          gamestate.hh -  description
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

#ifndef GAMESTATE_HH
#define GAMESTATE_HH

#include <queue>
#include <string>

#include "serversocket.h"
#include "math/vector.h"

/**
  *@author Marco Staginski
  */

//! the state in which the game is
enum state{
  INIT,
  PREPARE,
  RUN
};

//! manages the state of the game
class GameState {
public:
  enum ActionType{
    Generic,
    Attack,
    Spell,
    Search,
    Disarm,
  };
  //! Constructor
  GameState();
  //! Destructor
  ~GameState();
  //! set the state of the game
  inline void setState(enum state status){this->status = status;}
  //! return the game state
  inline state getState() const {return status;}
  //! get the index of the player whose turn it is at the moment
  inline short getActivePlayerIdx() const {return activePlayer;}
  //! init the game
  void init();
  //! start the game
  void start();
  //! get the next player, after the last ended his turn
  ServerSocket* getNextPlayer();
  //! returns, if it is this socket that belongs to the currently active player
  bool isActivePlayer(const ServerSocket* ss) const;
  //! roll the dice
  /* \param num how many dice to roll
   * \param movementDice if these are normal or attack/defend dices
   */
  string die(const short num, const bool movementDice, short* s=NULL, short* bs=NULL, short* ws=NULL, short* m=NULL);
  //! if the player has already moved
  inline const bool hasMoved(){return moved;}
  //! if an action was performed, it must be set here
  void performAction(ActionType type, bool action=true);
  //! if an action has been performed
  inline bool isActionPerformed(ActionType type) {return actionPerformed[type] == 0 && actionPerformed[Generic] == 0;}
  //! set the place of the action
  inline void setActionPlace(Vector2D& pos) {actionPlace = pos;}
  //! get the place of the action
  inline Math::Vector2D getActionPlace() {return actionPlace;}
  //! set the pending damage
  inline void setPendingDamage(short damage){pendingDamage = damage;}
  //! get the pending damage
  inline short getPendingDamage() {return pendingDamage;}
  //! end the game
  void end();
  //! set movement
  inline void setMovement(short num) {moved = true;moves = num;}
  //! get movement
  inline short getMovement() {return moves;}
  //! end a turn
  void endTurn(ServerSocket* ss);
  //! get and generate random number
  int getRandomNumber();
private:
  //! the gamestate
  enum state status;
  //! the currently active player
  short activePlayer;
  //! the number of players in the game
  short numPlayers;
  //! the socket of the active player
  ServerSocket* active;
  //! already moved
  bool moved;
  //! an action was already performed
  short actionPerformed[5];
  //! where the last action took place
  Math::Vector2D actionPlace;
  //! the damage that is pending
  short pendingDamage;
  //! the currently remaining moves
  short moves;
  //! the random numbers
  std::queue<std::string> randoms_;
  //! the last thing that led to an attack
  std::string lastWeapon_;
};

extern GameState game;

#endif
