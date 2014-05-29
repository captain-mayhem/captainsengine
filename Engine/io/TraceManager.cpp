#include "TraceManager.h"

#include <iostream>

#include "Tracing.h"

using namespace CGE;

TraceManager* TraceManager::mManager;
Mutex TraceManager::mMuStat;

TraceManager::TraceManager() : mChannelCount(0), mPutty(NULL){
}

TraceManager::~TraceManager(){
  mTraceBuffer.clear();
  mTraceLevels.clear();
  delete mPutty;
}

unsigned TraceManager::registerChannel(const char* name, int level){
  mMutex.lock();
  unsigned channel = ++mChannelCount;
  setCurrentLevel(channel, level);
  mMutex.unlock();
  return channel;
}

void TraceManager::setCurrentLevel(unsigned channel, int level){
  mTraceLevels[channel] = level;
}

int TraceManager::getCurrentLevel(unsigned channel){
  mMutex.lock();
  int level = mTraceLevels[channel];
  mMutex.unlock();
  return level;
}

void TraceManager::trace(unsigned channel, int level, const char* function, const char* message){
  mMutex.lock();
  if (mPutty)
    mPutty->trace(channel, level, function, message);
  else{
    Message msg;
    msg.channel = channel;
    msg.level = level;
    msg.function = function;
    msg.message = message;
    mTraceBuffer.push_back(msg);
  }
  mMutex.unlock();
}

void TraceManager::setTraceOutputter(TraceOutputter* putty){
  putty->init();
  mPutty = putty;
  for (std::list<Message>::iterator iter = mTraceBuffer.begin(); iter != mTraceBuffer.end(); ++iter){
    mPutty->trace(iter->channel, iter->level, iter->function.c_str(), iter->message.c_str());
  }
  mTraceBuffer.clear();
}