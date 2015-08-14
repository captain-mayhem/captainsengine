#include "TraceManager.h"

#include <iostream>

#include "Tracing.h"

using namespace CGE;

TraceManager* TraceManager::mManager;
Mutex TraceManager::mMuStat;

TraceManager::TraceManager() : mChannelCount(0){
}

TraceManager::~TraceManager(){
  mTraceBuffer.clear();
  mTraceLevels.clear();
  for (std::list<TraceOutputter*>::iterator iter = mPutty.begin(); iter != mPutty.end(); ++iter){
    delete *iter;
  }
  mPutty.clear();
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
  if (!mPutty.empty()){
    for (std::list<TraceOutputter*>::iterator iter = mPutty.begin(); iter != mPutty.end(); ++iter){
      (*iter)->trace(channel, level, function, message);
    }
  }
  Message msg;
  msg.channel = channel;
  msg.level = level;
  msg.function = function;
  msg.message = message;
  mTraceBuffer.push_back(msg);
  if (mTraceBuffer.size() > 1000)
    mTraceBuffer.pop_front();
  mMutex.unlock();
}

void TraceManager::setTraceOutputter(TraceOutputter* putty){
  putty->init();
  mPutty.push_back(putty);
  for (std::list<Message>::iterator iter = mTraceBuffer.begin(); iter != mTraceBuffer.end(); ++iter){
    putty->trace(iter->channel, iter->level, iter->function.c_str(), iter->message.c_str());
  }
}

void TraceManager::removeTraceOutputter(TraceOutputter* putty){
  for (std::list<TraceOutputter*>::iterator iter = mPutty.begin(); iter != mPutty.end(); ++iter){
    if (*iter == putty){
      mPutty.erase(iter);
      break;
    }
  }
}