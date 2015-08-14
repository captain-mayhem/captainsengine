#ifndef CGE_TRACEMANAGER_H
#define CGE_TRACEMANAGER_H

#include <cstdlib>
#include <list>
#include <string>
#include <map>

#include <system/thread.h>

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
  static TraceManager* instance() {mMuStat.lock(); if (mManager == NULL) mManager = new TraceManager(); mMuStat.unlock(); return mManager;}
  static void deinit() { TraceManager* mgr = mManager; mManager = NULL; delete mgr; }
  unsigned registerChannel(const char* name, int level);
  void trace(unsigned channel, int level, const char* function, const char* message);
  void setCurrentLevel(unsigned channel, int level);
  int getCurrentLevel(unsigned channel);
  void setTraceOutputter(TraceOutputter* putty);
  void removeTraceOutputter(TraceOutputter* putty);
protected:
  TraceManager();
  static TraceManager* mManager;
  static Mutex mMuStat;
  unsigned mChannelCount;
  std::list<TraceOutputter*> mPutty;
  std::list<Message> mTraceBuffer;
  std::map<unsigned, int> mTraceLevels;
  Mutex mMutex;
};

}

#endif
