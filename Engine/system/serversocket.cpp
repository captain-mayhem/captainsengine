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

#include "serversocket.h"
#include "system/engine.h"

using namespace CGE;

ServerSocket::ServerSocket (int port){
  Socket();
  if (!Socket::create()){
    CGE::Log << "Could not create server socket.";
  }

  if (!Socket::bind(port)){
    CGE::Log << "Could not bind to port.";
  }

  if (!Socket::listen()){
    CGE::Log << "Could not listen to socket.";
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
    CGE::Log << "Could not write to socket.";
  }
  return *this;
}


const ServerSocket& ServerSocket::operator>>(string& s) const {
  if (!Socket::recv(s)){
    CGE::Log << "Could not read from socket.";
  }
  return *this;
}

void ServerSocket::accept (ServerSocket& sock){
  if (!Socket::accept(sock)){
    CGE::Log << "Could not accept socket.";
  }
}
