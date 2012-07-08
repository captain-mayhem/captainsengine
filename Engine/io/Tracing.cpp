#include "Tracing.h"

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <iostream>
#include <system/engine.h>
#include "TraceManager.h"

using namespace CGE;

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

TraceObject::TraceObject(const char* name){
  //std::map<std::string, unsigned>::iterator iter = mChannels.find(name);
  mChannel = TraceManager::instance()->registerChannel(name, TRACE_INFO);
}

TraceObject::TraceObject(const char* name, int level){
  mChannel = TraceManager::instance()->registerChannel(name, level);
}

bool TraceObject::isEnabled(int level){
  return TraceManager::instance()->getCurrentLevel(mChannel) >= level;
}

void TraceObject::trace(int level, const char* function, const char* message, ...){
  va_list list;
  va_start(list, message);
  vsnprintf(buffer, BUF_SIZE, message, list);
  va_end(list);
  TraceManager::instance()->trace(mChannel, level, function, buffer);
  if (level == TRACE_FATAL_ERROR){
#ifdef WIN32
    DebugBreak();
#else
    __builtin_trap();
#endif
  }
  //internal_trace(mChannel, level, file, function, message);
}

LogOutputter::~LogOutputter(){
  mLog.close();
}

bool LogOutputter::init(){
  mLog.open("engine.log");
  if (!mLog)
    return false;
  return true;
}

void LogOutputter::trace(unsigned channel, int level, const char* function, const char* message){
  mLog <<  level << "-" << /*file << " " <<*/ function << ": " << message << std::endl;
  mLog.flush();
}
