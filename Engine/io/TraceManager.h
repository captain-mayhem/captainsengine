#ifndef CGE_TRACEMANAGER_H
#define CGE_TRACEMANAGER_H

#include <cge_export.h>

#include <cstdlib>
#include <list>
#include <string>
#include <map>

#include <system/thread.h>

namespace CGE{

class TraceOutputter;

class CGE_API TraceManager{
  struct CGE_API Message{
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
  static TraceManager* instance() {mMuStat.lock(); if (mManager == nullptr) mManager = new TraceManager(); mMuStat.unlock(); return mManager;}
  static void deinit() { TraceManager* mgr = mManager; mManager = nullptr; delete mgr; }
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
