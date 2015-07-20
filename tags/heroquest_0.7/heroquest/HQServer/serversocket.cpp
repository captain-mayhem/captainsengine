/***************************************************************************
                        serversocket.cpp -  description
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

#include "../HQClient/socketexception.h"
#include "serversocket.h"


ServerSocket::ServerSocket (int port){
  Socket();
  if (!Socket::create()){
    throw SocketException("Could not create server socket.");
  }

  if (!Socket::bind(port)){
    throw SocketException ("Could not bind to port.");
  }

  if (!Socket::listen()){
    throw SocketException ( "Could not listen to socket." );
  }
}

ServerSocket::ServerSocket(){
  Socket();
  Socket::create();
}

ServerSocket::~ServerSocket(){
}


const ServerSocket& ServerSocket::operator<<(const string& s) const {
  if (!Socket::send(s)){
    throw SocketException ( "Could not write to socket." );
  }
  return *this;
}


const ServerSocket& ServerSocket::operator>>(string& s) const {
  if (!Socket::recv(s)){
    throw SocketException ( "Could not read from socket." );
  }
  return *this;
}

void ServerSocket::accept (ServerSocket& sock){
  if (!Socket::accept(sock)){
    throw SocketException ( "Could not accept socket." );
  }
}
