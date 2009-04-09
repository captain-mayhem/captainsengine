/***************************************************************************
                          admin.hh -  description
                             -------------------
    begin                : Sa Feb 12 2005
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

#ifndef ADMIN_HH
#define ADMIN_HH
 
#include <vector>
#include <string>
#include <map>
#include <list>
//#include "SDL.h"
//#include "SDL_thread.h"
#include "system/thread.h"

#include "serversocket.h"
#include "player.h"
#include "../HQClient/common.h" 

using std::vector;
using std::string;
using std::map;
using std::list;

//! provides some important server functions in which all threads are involved
/*! each connected player has its own associated socket
 */
class Admin{
 public:
  //! Constructor
  Admin();
  //! Destructor
  ~Admin();
  //Message functions
  //! Add a socket i.e. player
  void add(ServerSocket* ss, CGE::Thread* th);
  //! end a socket connection
  void end(ServerSocket* ss);
  //! Chat message to specific socket
  void chat(const ServerSocket* ss, const string message);
  //! broadcast message to all sockets
  void broadcast(const string message);
  //! send update messages to socket
  /*! these are needed to bring a newly connected client to the current state
   *  of the game
   */
  void update(const ServerSocket* ss);

  //Player management
  //! has a player access to the server
  /* \param name the login name
   * \param owd the password
   */
  bool accessGranted(const string& name, const string& pwd);
  //! Get the player to the socket
  Player* getPlayer(ServerSocket* ss);
  //! Get the player with a certain name
  Player* getPlayer(const string& name);
  //! Returns the number of players
  unsigned getNumPlayers() {return players.size()-1;}
  //! list all players
  const string listPlayers();
  //! change the name of a player
  void alter_idx(string& old_name, string& new_name);
  //! set a plyers to be the evil Zargon
  bool setZargon(ServerSocket* ss);
  //! reset the zargon to nobody
  inline void resetZargon() {zargon = NULL;}
  //! returns the socket that is associated with Zargon
  ServerSocket* getZargon() const;

  //Other stuff
  //! set the absolute path of the directory  where the server runs
  inline void setPath(const string& path){this->path = path;}
  //! get the path
  inline const string& getPath() const {return path;}
  //! inits some important data structures
  void init();

 private:
  //! the players
  //! position zero of this vector is a dummy entry for the std::map
  vector<Player> players;
  //! the threads
  vector<CGE::Thread*> threads;
  //! password lookup table
  map<string,string> pwd_lookup;
  //! map sockets to player indices
  map<ServerSocket*, int> plr_idx1;
  //! map player names to player indices
  map<string, int> plr_idx2;
  //! the absolute path of the program
  string path;
  //! the broadcast messages are stored here for client updates
  list<string> messages;
  //! the zargon socket
  ServerSocket* zargon;
   
};

extern Admin globl;

#endif
