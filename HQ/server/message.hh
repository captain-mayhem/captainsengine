/***************************************************************************
                         message.hh  -  description
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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <map>
#include "serversocket.hh"
#include "../heroquest/creature.hh"

using std::string;
using std::map;

/**
  *@author Marco Staginski
  */

//! The message handling of the server
class Message {
public:
  //! Constructor
  Message();
  //! Destructor
  ~Message();
  //! Initialization
  void init();
  //! Process messages from clients
  void process(ServerSocket* ss, const string& cmd);
  //! Get current Creature
  Creature* getCreature();
private:
  //! map message to opcode
  map<string,int> cliToSrv;
};

extern Message msg;

#endif
