//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: socket.cpp                                                 |
//  ==================================================================

#include <iostream>
#include <string>
#include <errno.h>
#include <fcntl.h>
#ifndef WIN32
#include <netdb.h>
#endif
#include "socket.h"

using std::cerr;

//Constructor
Socket::Socket(){
#ifdef WIN32
	sock_ = NULL;
#else
	sock_ = -1;
#endif
  memset(&addr_, 0, sizeof(addr_));
}

// closes the socket
Socket::~Socket(){
  if (is_valid())
#ifdef WIN32
	closesocket(sock_);
#else
    ::close(sock_);
#endif
}

//Creates a socket
bool Socket::create(){
#ifdef WIN32
	WSADATA data;
	WSAStartup(MAKEWORD(1,1), &data);
#endif
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (!is_valid())
    return false;

  // no TIME_WAIT
  int on = 1;
  if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1)
    return false;

  return true;
}

// bind socket to port
bool Socket::bind (const int port){
  if (!is_valid()){
    return false;
  }

  //some UNIX socket stuff
  addr_.sin_family = AF_INET;
  addr_.sin_addr.s_addr = INADDR_ANY;
  addr_.sin_port = htons ((unsigned short)port);

  int bind_return = ::bind(sock_, (struct sockaddr *)&addr_, sizeof (addr_));

  if (bind_return == -1){
    return false;
  }
  return true;
}

//listen on socket
bool Socket::listen() const {
  if (!is_valid()){
    return false;
  }

  int listen_return = ::listen(sock_, MAXCONNECTIONS);

  if (listen_return == -1){
    return false;
  }
  return true;
}

//accept a connection on a socket
bool Socket::accept(Socket& new_socket) const{
  int addr_length = sizeof(addr_);
#ifdef WIN32
  typedef int socklen_t;
#endif
  new_socket.sock_ = ::accept(sock_, (sockaddr*)&addr_, (socklen_t *)&addr_length);

  if (new_socket.sock_ <= 0)
    return false;
  else
    return true;
}

//send message
bool Socket::send(const string s) const{
#ifdef WIN32
#define MSG_NOSIGNAL 0
#endif
  int status = ::send(sock_, s.c_str(), s.size(), MSG_NOSIGNAL);
  if (status == -1){
    perror("send");
    return false;
  }
  else{
    return true;
  }
}

//receive from socket
int Socket::recv(string& s) const{
  char buf[MAXRECV+1];
  s = "";
  memset(buf, 0, MAXRECV+1);
  
  int status = ::recv(sock_, buf, MAXRECV, 0);
  if (status == -1){
    perror("recv");
    cerr << "status == -1   errno == " << errno << "  in Socket::recv\n";
    return 0;
  }
  else if (status == 0){
    return 0;
  }
  else{
    s = buf;
    return status;
  }
}

//connect to a socket
bool Socket::connect(const string host, const int port){
  if (!is_valid()) 
    return false;

  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);

  struct hostent* conn;
  conn = gethostbyname(host.c_str());

  //set the IP-Address
  if (conn != NULL){
    memcpy(&addr_.sin_addr,conn->h_addr_list[0],sizeof(addr_.sin_addr));

    int status = ::connect(sock_, (sockaddr*)&addr_, sizeof(addr_));
    if (status == 0)
      return true;
    else
      return ipconnect(host);
  }
  else{
    return ipconnect(host);
  }
}

//if the host string was given as IP-address
bool Socket::ipconnect(const string host){
  //Error occured with gethostbyname or connect, so host may be speciifed as IP-Address
#ifdef WIN32
#define EAFNOSUPPORT WSAEAFNOSUPPORT
	struct hostent* conn;
	conn = gethostbyaddr(host.c_str(), host.size(),SOCK_STREAM);
	 //set the IP-Address
  if (conn != NULL){
    memcpy(&addr_.sin_addr,conn->h_addr_list[0],sizeof(addr_.sin_addr));
  }
#else
  inet_pton(AF_INET, host.c_str(), &addr_.sin_addr);
#endif
  if (errno == EAFNOSUPPORT) 
    return false;
  int status = ::connect(sock_, (sockaddr*)&addr_, sizeof(addr_));
  if (status == 0)
    return true;
  //Also an error occured, ==> user specified nonsense
  return false;
}

//set the socket to non-blocking operation
void Socket::set_non_blocking (const bool b){
#ifndef WIN32
  int opts = fcntl(sock_, F_GETFL);
  if (opts < 0){
    return;
  }

  if (b)
    opts = (opts | O_NONBLOCK);
  else
    opts = (opts & ~O_NONBLOCK);

  fcntl(sock_, F_SETFL, opts);
#endif
}
