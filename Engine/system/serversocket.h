/***************************************************************************
                        serversocket.hh -  description
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

#ifndef SERVERSOCKET_HH
#define SERVERSOCKET_HH

#include "socket.h"

namespace CGE{

//! The socket on the server side
class ServerSocket : private Socket
{
 public:
  //! Constructor, takes port
  ServerSocket (int port);
  //! Destructor
  ServerSocket();
  virtual ~ServerSocket();
  //! send message through socket
  const ServerSocket& operator<<(const std::string& s) const;
  //! receive message from socket
  const ServerSocket& operator>>(std::string& s) const;
  //! accept a connection
  void accept (ServerSocket& sock);
};

}


#endif
