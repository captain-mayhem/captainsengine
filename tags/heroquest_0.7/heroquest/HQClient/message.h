//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: message.hh                                                 |
//  ==================================================================

#ifndef MESSAGE_HH
#define MESSAGE_HH

#include <string>
#include <map>
#include <vector>
#include <queue>

#include <system/thread.h>
//#include "SDL_thread.h"
#include "clientsocket.h"
#include "item.h"

class Message;
extern Message msg;

class Hero;

using std::string;
using std::map;
using std::vector;

using System::Thread;

//! The central message handling class
/*! This class is responsible for handling all game commands that
 * come from user input or from the server
 */
class Message {
public:
  //! Consructor
  Message();
  //! Copy Constructor
  Message(const Message& m);
  //! Destructor
  ~Message();
  //! inits some data structures
  void init();
  //! processes a command from user input
  static inline void process(const char* cmd) {msg.process_(cmd);}
  //! receive thread
  /*! this function contains the main loop for the thread
   * that recieves messages from the server
   */
  static void receiver(void*);
  //! processes a command from the server
  void process();
  //! callback function for some console commands
  /*! handles some special functions for the console
   * \param message the message the console has to give
   * \param mode mode can switch the behaviour of this special function
   * \param additional if additional data is required, it can be passed in here
   */
  inline static void special(const string& message, int mode, void* additional) {msg.special_(message, mode, additional); }
  //! quits the application with notifying the server
  void quit();
  //! returns true if the client is connected to the server
  inline bool isConnected() {return ss_ != NULL;}
  //! sends a message directly to the server without preprocessing
  void send(const string& text);
  //! gets an initial setting value
  string& getSetting(short number);
  //! get the defend status
  inline bool isToDefend() {return toDefend_;}
  //! set defended
  inline void setDefended() {toDefend_ = false;}
  //! get all available heros
  inline vector<Hero>& getHeros() {return heros_;}
private:
  //! the special function callback
  void special_(const string& message, int mode, void* additional);
  //! processes a command from user input
  void process_(const char* cmd);
  //! maps typed in commands to server opcodes
  map<string,int> cliToSrv_;
  //! all available heros
  vector<Hero> heros_;
  //! the socket for communication with the server
  ClientSocket* ss_;
  //! the receiving thread
  Thread tid_;
  //! initialization settings
  vector<string> settings_;
  //! if there is something to defend
  bool toDefend_;
  //! the incoming messages
  std::queue<string> messages_;
};


#endif
