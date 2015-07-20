//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: socket.hh                                                  |
//  ==================================================================

#ifndef SOCKET_HH
#define SOCKET_HH


#include <sys/types.h>
#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif
#include <string>


using std::string;

//! maximum length of the hostname
const int MAXHOSTNAME = 200;
//! maximum number of connections
const int MAXCONNECTIONS = 5;
//! maximum length of the receive message
const int MAXRECV = 4096;

//! The base class for socket operations
/*! It uses unix socket system calls for providing the functionality
 */
class Socket{
public:
  //! Constructor
  Socket();
  //! Destructor
  /*! Closes the socket
   */
  virtual ~Socket();
  //! creates a socket
  bool create();
  //! bind the socket to a port
  bool bind(const int port);
  //! listen for incoming connections
  bool listen() const;
  //! accept an incoming connection on a socket
  bool accept(Socket& new_socket) const;
  //! connect to a socket on a server
  /*! \param host the host to connect to
   *  \param port the port to which to connect
   */
  bool connect(const string host, const int port);
  //! sends a message through the socket
  bool send(const string s) const;
  //! receives a message from the socket
  int recv(string& s) const;
  //! set the socket to non-blocking system calls
  void set_non_blocking(const bool b);
  //! returns if the socket is valid
  bool is_valid() const { 
#ifdef WIN32
	  return sock_ != NULL;
#else
	  return sock_ != -1;
#endif
 }
private:
  //! trys to connect after normal connect failed
  bool ipconnect(const string host);
  //! the socket
#ifdef WIN32
  SOCKET sock_;
#else
  int sock_;
#endif
  //! the unix socket address struct
  sockaddr_in addr_;
};


#endif
