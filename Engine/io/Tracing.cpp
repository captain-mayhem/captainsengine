#include "Tracing.h"

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <iostream>

#define BUF_SIZE 1024

char buffer[BUF_SIZE];

#ifdef UNDER_CE
#define vsnprintf _vsnprintf
#endif

void internal_trace(int group, int level, char* file, char* function, char* message, ...){
  va_list list;
  va_start(list, message);
  vsnprintf(buffer, BUF_SIZE, message, list);
  va_end(list);
  std::cout << level << "-" << /*file << " " <<*/ function << ": " << buffer << std::endl;
  if (level == TRACE_ERROR){
    return; //for breakpoints
  }
  if (level == TRACE_FATAL_ERROR)
    exit(EXIT_FAILURE);
}
