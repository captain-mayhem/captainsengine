//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: common.hh                                                  |
//  ==================================================================

#ifndef COMMON_HH
#define COMMON_HH

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>


//maximum path length
#ifndef WIN32
#define MAX_PATH 1024
#endif

//progam version
//#define VERSION 0.5
//#define VERSTRING "0.5"

//! width of the virtual screen
//! const int SCREENWIDTH = 1024;
//! height of the virtual screen
//! const int SCREENHEIGHT = 768;

//! waits a short period to avoid buffer overflow
inline void wait(){
#ifdef UNIX_OLD
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 250000000;
  nanosleep(&ts, NULL);
#endif
  //SDL_Delay(3);
}

#endif
