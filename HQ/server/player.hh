/***************************************************************************
                         player.hh  -  description
                             -------------------
    begin                : Di Feb 15 2005
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

#ifndef PLAYER_HH
#define PLAYER_HH


/**
  *@author Marco Staginski
  */

#include <string>
#include "serversocket.hh"
#include "../heroquest/creature.hh"

using std::string;  

//! The player on the server
class Player {
public: 
  //! Constructor: creates a player from socket ss
  Player(ServerSocket* ss);
  //! Copy constructor
  Player(const Player& p);
  //! Destructor
  ~Player();
  //! Get the socket of the player
  inline ServerSocket* getSock(){return sock;}
  //! Get the name of the player
  inline const string getName() const {return name;}
  //! Set the identity of the player (login)
  /*! \param name the login name of the player
   * \param pwd the password
   */
  void set_identity(string& name, const string& pwd);
  //! Send message to player
  const Player& operator<<( const string& );
  //! Receive message from player
  const Player& operator>>(string& );
  //! Set the status of the player
  /*! 0 means not logged in, 1 is normal player and 2 is admin
   */
  inline void setStatus(int stat) {status = stat;}
  //! get the status of the player
  inline int getStatus() {return status;}
  //! set the last creature
  inline void setLastCreature(Creature* c) {lastCreat_ = c;}
  //! get the last creature
  inline Creature* getLastCreature() {return lastCreat_;}
private:
  //!player name
  string name;
  //! password of the player
  string pwd;
  //! the associated socket
  ServerSocket *sock;
  //! the status of the player
  int status;
  //! the last controlled creature
  Creature* lastCreat_;
};

inline bool operator<(Player a, Player b){
  return a.getName() < b.getName();
}
inline bool operator==(Player a, Player b){
  return a.getName() == b.getName();
}

#endif
