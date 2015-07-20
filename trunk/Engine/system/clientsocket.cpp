//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: clientsocket.cpp                                                 |
//  ==================================================================

#include <iostream>
#include <cstring>
#include <cstdio>

#include "clientsocket.h"
#include <system/engine.h>

using std::cerr;
using namespace CGE;

// Creates a socket and connects it to port at host
ClientSocket::ClientSocket(std::string host, int port){
  if (!Socket::create()){
    CGE::Log << "Could not create client socket.";
  }

  if (!Socket::connect(host, port)){
    CGE::Log << "Could not bind to port.";
  }
}

// Destructor
ClientSocket::~ClientSocket()
{
}

// write strings to socket
const ClientSocket& ClientSocket::operator<<(const std::string& s) const{
  if (!Socket::send(s)){
    CGE::Log << "Could not write to socket.";
  }
  return *this;
}


// read strings from socket
const ClientSocket& ClientSocket::operator>>(std::string& s) const{
  if (!Socket::recv(s)){
    CGE::Log << "Could not read from socket.";
  }
  return *this;
}

// write integers to socket
const ClientSocket& ClientSocket::operator<<(const int& i) const{
  char tmp[16];
  sprintf(tmp, "%d", i);
  std::string s(tmp);

  if (!Socket::send(s)){
    CGE::Log << "Could not write to socket.";
  }
  return *this;
}
