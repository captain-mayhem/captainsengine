//  ==================================================================
// |                         Torchdelve                               |
// |------------------------------------------------------------------|
// |                   A turn-based dungeon crawler                   |
// |                                                                  |
// | Marco Staginski                                                  |
// |------------------------------------------------------------------|
// | File: socketexception.hh                                         |
//  ==================================================================


#ifndef SOCKETEXCEPTION_HH
#define SOCKETEXCEPTION_HH

#include <string>

using std::string;

//! It's an exception class for socket operations
class SocketException {
public:
  //! Constructor that takes the error message
  SocketException (string s) : str(s) {};
  //! Destructor
  ~SocketException (){};
  //! get the error description
  string description() { return str; }
private:
  //! the error message
  string str;
};

#endif
