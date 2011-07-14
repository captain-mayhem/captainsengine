#include "Tracing.h"

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <system/engine.h>

#define BUF_SIZE 1024

char buffer[BUF_SIZE];

#ifdef UNDER_CE
#define vsnprintf _vsnprintf
#endif

unsigned internal_groups = 0;

void internal_trace(unsigned group, int level, const char* file, const char* function, const char* message, ...){
  va_list list;
  va_start(list, message);
  vsnprintf(buffer, BUF_SIZE, message, list);
  va_end(list);
  std::cout << level << "-" << /*file << " " <<*/ function << ": " << buffer << std::endl;
  if (level == TRACE_ERROR){
    return; //for breakpoints
  }
	if (level == TRACE_FATAL_ERROR){
		exit(EXIT_FAILURE);
	}
}

TraceManager* TraceManager::mManager;

TraceManager::TraceManager() : mChannelCount(0){
}

TraceManager::~TraceManager(){
}

unsigned TraceManager::registerChannel(const char* name){
  return ++mChannelCount;
}

int TraceManager::getCurrentLevel(unsigned channel){
  return TRACE_CUSTOM;
}

void TraceManager::trace(unsigned channel, int level, const char* file, const char* function, int line, const char* message){
  std::cout << level << "-" << /*file << " " <<*/ function << ": " << buffer << std::endl;
}

TraceObject::TraceObject(const char* name){
  //std::map<std::string, unsigned>::iterator iter = mChannels.find(name);
  mChannel = TraceManager::instance()->registerChannel(name);
}

bool TraceObject::isEnabled(int level){
  return TraceManager::instance()->getCurrentLevel(mChannel) >= level;
}

void TraceObject::trace(int level, const char* file, const char* function, int line, const char* message, ...){
    va_list list;
    va_start(list, message);
    vsnprintf(buffer, BUF_SIZE, message, list);
    va_end(list);
    TraceManager::instance()->trace(mChannel, level, file, function, line, buffer);
    //internal_trace(mChannel, level, file, function, message);
  }
