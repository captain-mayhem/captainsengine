#ifndef CGE_TRACEMANAGER_H
#define CGE_TRACEMANAGER_H

#include <cstdlib>
#include <list>
#include <string>
#include <map>

namespace CGE{

class TraceOutputter;

class TraceManager{
  struct Message{
    Message(){
    }
    Message(const Message& msg){
      channel = msg.channel;
      level = msg.level;
      function = msg.function;
      message = msg.message;
    }
    unsigned channel;
    int level;
    std::string function;
    std::string message;
  };
public:
  ~TraceManager();
  static TraceManager* instance() {if (mManager == NULL) mManager = new TraceManager(); return mManager;}
  static void deinit() {delete mManager; mManager = NULL;}
  unsigned registerChannel(const char* name, int level);
  void trace(unsigned channel, int level, const char* function, const char* message);
  void setCurrentLevel(unsigned channel, int level);
  int getCurrentLevel(unsigned channel);
  void setTraceOutputter(TraceOutputter* putty);
protected:
  TraceManager();
  static TraceManager* mManager;
  unsigned mChannelCount;
  TraceOutputter* mPutty;
  std::list<Message> mTraceBuffer;
  std::map<unsigned, int> mTraceLevels;
};

}

#endif
